///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#include "emlabcpp/experimental/testing/controller.h"

#include "emlabcpp/experimental/contiguous_tree/request_adapter.h"
#include "emlabcpp/experimental/logging.h"
#include "emlabcpp/experimental/testing/coroutine.h"

namespace emlabcpp::testing
{

template < typename T >
struct msg_awaiter : public coro::wait_interface
{
        T                          reply;
        coro::wait_state           state = coro::wait_state::WAITING;
        controller_reactor_variant request;

        controller_interface_adapter& iface;

        msg_awaiter( const controller_reactor_variant& req, controller_interface_adapter& ifa )
          : request( req )
          , iface( ifa )
        {
        }

        [[nodiscard]] coro::wait_state get_state() const override
        {
                return state;
        }

        void tick() override
        {
        }

        [[nodiscard]] bool await_ready() const
        {
                return false;
        }

        void await_suspend( const std::coroutine_handle< typename test_coroutine::promise_type > h )
        {
                h.promise().iface = this;
                iface.set_reply_cb( [this]( const reactor_controller_variant& var ) {
                        const T* val_ptr = std::get_if< T >( &var );
                        if ( val_ptr == nullptr ) {
                                state = coro::wait_state::ERRORED;
                                return false;
                        }
                        state = coro::wait_state::READY;
                        reply = *val_ptr;
                        return true;
                } );
                // TODO: this hsould not be ignored
                std::ignore = iface.send( request );
        }

        T await_resume()
        {
                return reply;
        }
};

test_coroutine controller::initialize( pmr::memory_resource& )
{

        name_ = ( co_await msg_awaiter< get_suite_name_reply >(
                      get_property< msgid::SUITE_NAME >{}, iface_ ) )
                    .name;
        date_ = ( co_await msg_awaiter< get_suite_date_reply >(
                      get_property< msgid::SUITE_DATE >{}, iface_ ) )
                    .date;
        const test_id count =
            ( co_await msg_awaiter< get_count_reply >( get_property< msgid::COUNT >{}, iface_ ) )
                .count;

        for ( const test_id i : range( count ) ) {
                const auto name_reply = co_await msg_awaiter< get_test_name_reply >(
                    get_test_name_request{ .tid = i }, iface_ );

                tests_[i] = test_info{ .name = name_reply.name };
        }
}

void controller::start_test( const test_id tid )
{
        if ( !std::holds_alternative< idle_state >( state_ ) ) {
                EMLABCPP_ERROR_LOG( "Can't start a new test, not in prepared state" );
                return;
        }

        rid_ += 1;

        state_ = test_running_state{ .context = { tid, rid_ } };

        // TODO: this hsould be propagated
        std::ignore = iface_.send( exec_request{ .rid = rid_, .tid = tid } );
}

outcome controller::on_msg( const std::span< const std::byte > data )
{
        using h = protocol::handler< reactor_controller_group >;
        return h::extract( view_n( data.data(), data.size() ) )
            .match(
                [this]( const reactor_controller_variant& var ) {
                        return on_msg( var );
                },
                [this]( const protocol::error_record& rec ) -> outcome {
                        EMLABCPP_ERROR_LOG( "Failed to extract incoming msg: ", rec );
                        iface_.report_error( controller_protocol_error{ rec } );
                        return ERROR;
                } );
}

outcome controller::on_msg( const reactor_controller_variant& var )
{
        using opt_state     = std::optional< states >;
        outcome   res       = ERROR;
        opt_state new_state = match(
            state_,
            [this, &var, &res]( const initializing_state& ) -> opt_state {
                    if ( !iface_.on_msg_with_cb( var ) ) {
                            EMLABCPP_ERROR_LOG( "Got wrong message: ", var.index() );
                            visit(
                                [&]< typename T >( T& ) {
                                        iface_.report_error( controller_internal_error{ T::id } );
                                },
                                var );
                    } else {
                            res = SUCCESS;
                    }
                    return std::nullopt;
            },
            [this, &var, &res]( test_running_state& rs ) -> opt_state {
                    const auto* err_ptr = std::get_if< reactor_internal_error_report >( &var );
                    if ( err_ptr != nullptr ) {
                            iface_.report_error( internal_reactor_error{ err_ptr->var } );
                            return std::nullopt;
                    }

                    const auto* tf_ptr = std::get_if< test_finished >( &var );
                    if ( tf_ptr == nullptr ) {
                            visit(
                                [&]< typename T >( T& ) {
                                        iface_.report_error( controller_internal_error{ T::id } );
                                },
                                var );
                            return std::nullopt;
                    }

                    EMLABCPP_INFO_LOG(
                        "Test finished, rid: ",
                        tf_ptr->rid,
                        " errored: ",
                        tf_ptr->errored,
                        " failed: ",
                        tf_ptr->failed );

                    rs.context.failed  = tf_ptr->failed;
                    rs.context.errored = tf_ptr->errored;
                    iface_->on_result( rs.context );

                    res = SUCCESS;
                    return states{ idle_state{} };
            },
            [&res]( const idle_state ) -> opt_state {
                    res = SUCCESS;
                    return std::nullopt;
            } );
        if ( new_state ) {
                state_ = std::move( *new_state );
        }

        // TODO: maybe better error handling? or maybe none at all?
        return res;
}

void controller::tick()
{
        using opt_state     = std::optional< states >;
        opt_state new_state = match(
            state_,
            []( initializing_state& st ) -> opt_state {
                    if ( !st.coro.done() ) {
                            st.coro.tick();
                            return std::nullopt;
                    }

                    EMLABCPP_INFO_LOG( "Controller finished initialization and is prepared" );
                    return states{ idle_state{} };
            },
            []( const test_running_state& ) -> opt_state {
                    return std::nullopt;
            },
            []( const idle_state ) -> opt_state {
                    return std::nullopt;
            } );
        if ( new_state ) {
                state_ = std::move( *new_state );
        }
}
}  // namespace emlabcpp::testing
