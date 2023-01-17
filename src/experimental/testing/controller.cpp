// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//

#include "emlabcpp/experimental/testing/controller.h"

#include "emlabcpp/experimental/contiguous_tree/request_adapter.h"
#include "emlabcpp/experimental/logging.h"
#include "emlabcpp/experimental/testing/coroutine.h"

namespace emlabcpp::testing
{

template < typename T >
struct msg_awaiter : public test_awaiter_interface
{
        T                                                     reply;
        await_state                                           state = await_state::WAITING;
        controller_reactor_variant                            request;
        std::coroutine_handle< test_coroutine::promise_type > coro_handle;

        controller_interface_adapter& iface;

        msg_awaiter( controller_reactor_variant req, controller_interface_adapter& ifa )
          : request( req )
          , iface( ifa )
        {
        }

        await_state get_state() const
        {
                return state;
        }

        bool await_ready() const
        {
                return false;
        }
        void await_suspend( std::coroutine_handle< typename test_coroutine::promise_type > h )
        {
                coro_handle                 = h;
                coro_handle.promise().iface = this;
                iface.set_reply_cb( [this]( const reactor_controller_variant& var ) {
                        const T* val_ptr = std::get_if< T >( &var );
                        if ( val_ptr == nullptr ) {
                                state = await_state::ERRORED;
                                return false;
                        }
                        state = await_state::READY;
                        reply = *val_ptr;
                        return true;
                } );
                iface.send( request );
        }
        T await_resume()
        {
                coro_handle.promise().iface = nullptr;
                return reply;
        }
};

test_coroutine controller::initialize( pmr::memory_resource& )
{

        name_ =
            ( co_await msg_awaiter< get_suite_name_reply >( get_property< SUITE_NAME >{}, iface_ ) )
                .name;
        date_ =
            ( co_await msg_awaiter< get_suite_date_reply >( get_property< SUITE_DATE >{}, iface_ ) )
                .date;
        test_id count =
            ( co_await msg_awaiter< get_count_reply >( get_property< COUNT >{}, iface_ ) ).count;

        for ( const test_id i : range( count ) ) {
                auto name_reply = co_await msg_awaiter< get_test_name_reply >(
                    get_test_name_request{ .tid = i }, iface_ );

                tests_[i] = test_info{ .name = name_reply.name };
        }
}

void controller::start_test( const test_id tid )
{
        if ( !std::holds_alternative< idle_state >( state_ ) ) {
                EMLABCPP_LOG( "Can't start a new test, not in prepared state" );
                return;
        }

        rid_ += 1;

        state_ = test_running_state{ .context = { tid, rid_ } };

        iface_.send( exec_request{ .rid = rid_, .tid = tid } );
}

void controller::on_msg( const std::span< const uint8_t > data )
{
        using h = protocol::handler< reactor_controller_group >;
        h::extract( view_n( data.data(), data.size() ) )
            .match(
                [this]( const reactor_controller_variant& var ) {
                        on_msg( var );
                },
                [this]( const protocol::error_record& rec ) {
                        std::ignore = rec;
                        EMLABCPP_LOG( "Failed to extract incoming msg: " << rec );
                } );
}
void controller::on_msg( const reactor_controller_variant& var ) 
{
        using opt_state     = std::optional< states >;
        opt_state new_state = match(
            state_,
            [&]( const initializing_state& ) -> opt_state {
                    if ( !iface_.on_msg_with_cb( var ) ) {
                            EMLABCPP_LOG( "Got wrong message, bailing out: " << var );
                            std::abort();
                    }
                    return std::nullopt;
            },
            [&]( test_running_state& rs ) -> opt_state {
                    const auto& tf_ptr = std::get_if< test_finished >( &var );
                    if ( tf_ptr == nullptr ) {
                            EMLABCPP_LOG( "Got wrong message, bailing out: " << var );
                            std::abort();
                    }

                    rs.context.failed  = tf_ptr->failed;
                    rs.context.errored = tf_ptr->errored;
                    iface_->on_result( rs.context );
                    return states{ idle_state{} };
            },
            [&]( idle_state ) -> opt_state {
                    return std::nullopt;
            } );
        if ( new_state ) {
                state_ = std::move( *new_state );
        }
}

void controller::tick() 
{
        using opt_state     = std::optional< states >;
        opt_state new_state = match(
            state_,
            [&]( initializing_state& st ) -> opt_state {
                    if ( !st.coro.done() ) {
                            st.coro.tick();
                            return std::nullopt;
                    }

                    EMLABCPP_LOG( "Controller finished initialization and is prepared" );
                    return states{ idle_state{} };
            },
            [&]( const test_running_state& ) -> opt_state {
                    return std::nullopt;
            },
            [&]( idle_state ) -> opt_state {
                    return std::nullopt;
            } );
        if ( new_state ) {
                state_ = std::move( *new_state );
        }
}
}  // namespace emlabcpp::testing
