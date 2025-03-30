/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///

#include "emlabcpp/experimental/testing/controller.h"

#include "emlabcpp/experimental/coro/recursive.h"
#include "emlabcpp/experimental/testing/base.h"
#include "emlabcpp/experimental/testing/controller_interface_adapter.h"
#include "emlabcpp/experimental/testing/coroutine.h"
#include "emlabcpp/experimental/testing/error.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/match.h"
#include "emlabcpp/outcome.h"
#include "emlabcpp/pmr/memory_resource.h"
#include "emlabcpp/protocol/error.h"
#include "emlabcpp/protocol/handler.h"
#include "emlabcpp/range.h"
#include "emlabcpp/result.h"
#include "emlabcpp/view.h"
#include "emlabcpp/visit.h"

#include <coroutine>
#include <cstddef>
#include <optional>
#include <span>
#include <tuple>
#include <utility>
#include <variant>

namespace emlabcpp::testing
{

template < typename T >
struct msg_awaiter : public wait_interface
{
        T                          reply;
        coro_state                 state = coro_state::WAITING;
        controller_reactor_variant request;

        controller_interface_adapter& iface;

        msg_awaiter( controller_reactor_variant const& req, controller_interface_adapter& ifa )
          : request( req )
          , iface( ifa )
        {
        }

        [[nodiscard]] coro_state get_state() const override
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

        void
        await_suspend( std::coroutine_handle< typename coroutine< void >::promise_type > const h )
        {
                h.promise().iface = this;
                iface.set_reply_cb( [this]( reactor_controller_variant const& var ) {
                        T const* val_ptr = std::get_if< T >( &var );
                        if ( val_ptr == nullptr ) {
                                state = coro_state::ERRORED;
                                return false;
                        }
                        state = coro_state::DONE;
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

coroutine< void > controller::initialize( pmr::memory_resource& )
{
        name_ = ( co_await msg_awaiter< get_suite_name_reply >(
                      get_property< msgid::SUITE_NAME >{}, iface_ ) )
                    .name;
        date_ = ( co_await msg_awaiter< get_suite_date_reply >(
                      get_property< msgid::SUITE_DATE >{}, iface_ ) )
                    .date;
        test_id const count =
            ( co_await msg_awaiter< get_count_reply >( get_property< msgid::COUNT >{}, iface_ ) )
                .count;

        for ( test_id const i : range( count ) ) {
                auto const name_reply = co_await msg_awaiter< get_test_name_reply >(
                    get_test_name_request{ .tid = i }, iface_ );

                tests_[i] = name_reply.name;
        }
}

void controller::start_test( test_id const tid )
{
        if ( !std::holds_alternative< idle_state >( state_ ) )
                return;

        rid_ += 1;

        state_ = test_running_state{ .context = { tid, rid_ } };

        // TODO: this hsould be propagated
        std::ignore = iface_.send( exec_request{ .rid = rid_, .tid = tid } );
}

outcome controller::on_msg( std::span< std::byte const > const data )
{
        using h = protocol::handler< reactor_controller_group >;
        return match(
            h::extract( view_n( data.data(), data.size() ) ),
            [this]( reactor_controller_variant const& var ) {
                    return on_msg( var );
            },
            [this]( protocol::error_record const& rec ) -> outcome {
                    iface_.report_error( controller_protocol_error{ rec } );
                    return outcome::ERROR;
            } );
}

outcome controller::on_msg( reactor_controller_variant const& var )
{
        using opt_state     = std::optional< states >;
        outcome   res       = outcome::ERROR;
        opt_state new_state = match(
            state_,
            [this, &var, &res]( initializing_state const& ) -> opt_state {
                    if ( std::holds_alternative< boot >( var ) ) {
                            res = outcome::SUCCESS;
                            return std::nullopt;
                    }
                    if ( !iface_.on_msg_with_cb( var ) ) {
                            visit(
                                [&]< typename T >( T& ) {
                                        iface_.report_error( controller_internal_error{ T::id } );
                                },
                                var );
                    } else {
                            res = outcome::SUCCESS;
                    }
                    return std::nullopt;
            },
            [this, &var, &res]( test_running_state& rs ) -> opt_state {
                    auto const* err_ptr = std::get_if< reactor_internal_error_report >( &var );
                    if ( err_ptr != nullptr ) {
                            iface_.report_error( internal_reactor_error{ err_ptr->var } );
                            return std::nullopt;
                    }

                    auto const* tf_ptr = std::get_if< test_finished >( &var );
                    if ( tf_ptr == nullptr ) {
                            visit(
                                [&]< typename T >( T& ) {
                                        iface_.report_error( controller_internal_error{ T::id } );
                                },
                                var );
                            return std::nullopt;
                    }

                    rs.context.status = tf_ptr->status;
                    iface_->on_result( rs.context );

                    res = outcome::SUCCESS;
                    return states{ idle_state{} };
            },
            [&res]( idle_state const ) -> opt_state {
                    res = outcome::SUCCESS;
                    return std::nullopt;
            } );
        if ( new_state )
                state_ = std::move( *new_state );

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

                    return states{ idle_state{} };
            },
            []( test_running_state const& ) -> opt_state {
                    return std::nullopt;
            },
            []( idle_state const ) -> opt_state {
                    return std::nullopt;
            } );
        if ( new_state )
                state_ = std::move( *new_state );
}
}  // namespace emlabcpp::testing
