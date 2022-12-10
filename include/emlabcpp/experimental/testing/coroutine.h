
#include "emlabcpp/experimental/coro/memory_promise.h"
#include "emlabcpp/experimental/coro/owning_coroutine_handle.h"
#include "emlabcpp/experimental/testing/convert.h"
#include "emlabcpp/experimental/testing/reactor_interface_adapter.h"

#include <coroutine>

#pragma once

namespace emlabcpp::testing
{

class test_coroutine
{
public:
        struct promise_type : coro::memory_promise< promise_type >
        {

                test_coroutine get_return_object()
                {
                        return { handle::from_promise( *this ) };
                }

                std::suspend_always initial_suspend()
                {
                        return {};
                }
                std::suspend_never final_suspend() noexcept
                {
                        return {};
                }

                void unhandled_exception()
                {
                }

                void return_void()
                {
                }
        };

        using handle        = std::coroutine_handle< promise_type >;
        using owning_handle = coro::owning_coroutine_handle< promise_type >;

        test_coroutine( const handle cor )
          : h_( cor )
        {
        }

        // TODO: this is shady API as fuck
        bool spin( reactor_interface_adapter* comm )
        {
                h_();

                while ( !h_->done() ) {
                        bool success = comm->read_with_handler();
                        if ( !success ) {
                                return false;
                        }
                        h_();
                }
                return true;
        }

private:
        owning_handle h_;
};

template < typename Processor >
struct record_awaiter
{
        Processor                  proc;
        reactor_interface_adapter* comm_ptr;

        using request_type = decltype( proc.req );

        record_awaiter( request_type req, reactor_interface_adapter* comm )
          : proc{ .reply = {}, .req = req }
          , comm_ptr( comm )
        {
        }

        bool await_ready()
        {
                return false;
        }

        bool await_suspend( std::coroutine_handle< test_coroutine::promise_type > )
        {
                if ( proc.expects_reply() ) {
                        comm_ptr->register_incoming_handler(
                            [this]( const controller_reactor_variant& var ) -> bool {
                                    return proc.set_value( var );
                            } );
                }
                comm_ptr->reply( proc.req );
                return proc.expects_reply();
        }

        decltype( auto ) await_resume()
        {
                return proc.reply;
        }

        decltype( auto ) busy_wait()
        {
                if ( proc.expects_reply() ) {
                        comm_ptr->register_incoming_handler(
                            [this]( const controller_reactor_variant& var ) -> bool {
                                    return proc.set_value( var );
                            } );
                }
                comm_ptr->reply( proc.req );
                if ( proc.expects_reply() ) {
                        std::ignore = comm_ptr->read_with_handler();
                }
                return proc.reply;
        }
};

struct collect_processor
{
        node_id         reply;
        collect_request req;

        [[nodiscard]] bool expects_reply() const
        {
                return req.expects_reply;
        }

        [[nodiscard]] bool set_value( const controller_reactor_variant& var )
        {
                const auto* val_ptr = std::get_if< collect_reply >( &var );
                if ( val_ptr == nullptr ) {
                        return false;
                }
                reply = val_ptr->nid;
                return true;
        }
};
using collect_awaiter = record_awaiter< collect_processor >;

template < typename T >
struct param_value_processor
{
        T                   reply;
        param_value_request req;

        [[nodiscard]] bool expects_reply() const
        {
                return true;
        }

        [[nodiscard]] bool set_value( const controller_reactor_variant& var )
        {
                const auto* val_ptr = std::get_if< param_value_reply >( &var );
                if ( val_ptr == nullptr ) {
                        return false;
                }
                auto opt_res = value_type_converter< T >::from_value( val_ptr->value );
                if ( opt_res ) {
                        reply = *opt_res;
                }
                return opt_res.has_value();
        }
};
template < typename T >
using param_value_awaiter = record_awaiter< param_value_processor< T > >;

template < typename T >
struct param_value_key_processor
{
        T                       reply;
        param_value_key_request req;

        [[nodiscard]] bool expects_reply() const
        {
                return true;
        }

        [[nodiscard]] bool set_value( const controller_reactor_variant& var )
        {
                const auto* val_ptr = std::get_if< param_value_key_reply >( &var );
                if ( val_ptr == nullptr ) {
                        return false;
                }
                auto opt_res = value_type_converter< T >::from_value( val_ptr->value );
                if ( opt_res ) {
                        reply = *opt_res;
                }
                return opt_res.has_value();
        }
};
template < typename T >
using param_value_key_awaiter = record_awaiter< param_value_key_processor< T > >;

struct param_type_processor
{
        node_type_enum     reply;
        param_type_request req;
        using reply_type = param_type_reply;

        [[nodiscard]] bool expects_reply() const
        {
                return true;
        }

        [[nodiscard]] bool set_value( const controller_reactor_variant& var )
        {
                const auto* val_ptr = std::get_if< param_type_reply >( &var );
                if ( val_ptr == nullptr ) {
                        return false;
                }
                reply = val_ptr->type;
                return true;
        }
};
using param_type_awaiter = record_awaiter< param_type_processor >;

struct param_child_processor
{
        node_id             reply;
        param_child_request req;

        [[nodiscard]] bool expects_reply() const
        {
                return true;
        }

        [[nodiscard]] bool set_value( const controller_reactor_variant& var )
        {
                const auto* val_ptr = std::get_if< param_child_reply >( &var );
                if ( val_ptr == nullptr ) {
                        return false;
                }
                reply = val_ptr->chid;
                return true;
        }
};
using param_child_awaiter = record_awaiter< param_child_processor >;

struct param_child_count_processor
{
        child_count               reply;
        param_child_count_request req;

        [[nodiscard]] bool expects_reply() const
        {
                return true;
        }

        [[nodiscard]] bool set_value( const controller_reactor_variant& var )
        {
                const auto* val_ptr = std::get_if< param_child_count_reply >( &var );
                if ( val_ptr == nullptr ) {
                        return false;
                }
                reply = val_ptr->count;
                return true;
        }
};
using param_child_count_awaiter = record_awaiter< param_child_count_processor >;

struct param_key_processor
{
        key_type          reply;
        param_key_request req;

        [[nodiscard]] bool expects_reply() const
        {
                return true;
        }

        [[nodiscard]] bool set_value( const controller_reactor_variant& var )
        {
                const auto* val_ptr = std::get_if< param_key_reply >( &var );
                if ( val_ptr == nullptr ) {
                        return false;
                }
                reply = val_ptr->key;
                return true;
        }
};
using param_key_awaiter = record_awaiter< param_key_processor >;

}  // namespace emlabcpp::testing
