#include "emlabcpp/experimental/coro/memory_promise.h"
#include "emlabcpp/experimental/coro/owning_coroutine_handle.h"
#include "emlabcpp/experimental/logging.h"

#include <coroutine>
#include <optional>

#pragma once

namespace emlabcpp::coro
{

template < typename RequestType, typename ReplyType >
class request_reply
{
public:
        using request_type = RequestType;
        using reply_type   = ReplyType;

        struct promise_type;

        struct awaiter
        {
                promise_type* prom_;

                bool await_ready()
                {
                        return false;
                }
                void await_suspend( std::coroutine_handle<> )
                {
                }
                const ReplyType& await_resume()
                {
                        return *prom_->reply;
                }
        };

        struct promise_type : memory_promise< promise_type >
        {
                std::optional< RequestType > request;
                std::optional< ReplyType >   reply;

                request_reply get_return_object()
                {
                        return { handle::from_promise( *this ) };
                }
                std::suspend_never initial_suspend()
                {
                        return {};
                }
                std::suspend_always final_suspend() noexcept
                {
                        return {};
                }

                void unhandled_exception()
                {
                }

                void return_void()
                {
                }

                awaiter yield_value( RequestType out )
                {
                        request = out;
                        return { this };
                }
        };

        using handle        = std::coroutine_handle< promise_type >;
        using owning_handle = owning_coroutine_handle< promise_type >;

        request_reply( const handle cor )
          : h_( cor )
        {
        }

        request_reply( const request_reply& )            = delete;
        request_reply& operator=( const request_reply& ) = delete;

        request_reply( request_reply&& )            = default;
        request_reply& operator=( request_reply&& ) = default;

        const RequestType* get_request()
        {
                if ( !h_ ) {
                        EMLABCPP_ERROR_LOG( "Can't extract request from empty handle" );
                        return nullptr;
                }
                if ( !h_.promise().request.has_value() ) {
                        EMLABCPP_ERROR_LOG( "No request in coroutine at ", &h_.promise().request );
                        return nullptr;
                }
                return &*h_.promise().request;
        }

        bool has_reply()
        {
                if ( h_ ) {
                        return h_.promise().reply.has_value();
                } else {
                        EMLABCPP_ERROR_LOG( "Can't check reply in empty handle" );
                        return false;
                }
        }

        void store_reply( const ReplyType& inpt )
        {
                if ( h_ ) {
                        h_.promise().reply = inpt;
                } else {
                        EMLABCPP_ERROR_LOG( "Can't store reply in empty handle" );
                }
        }

        [[nodiscard]] operator bool() const
        {
                return h_.done();
        }

        [[nodiscard]] bool done() const
        {
                return h_.done();
        }

        [[nodiscard]] bool tick()
        {
                if ( !h_.promise().reply ) {
                        EMLABCPP_ERROR_LOG( "Can't tick coroutine ", address(), ", no reply" );
                        return false;
                }
                if ( !h_ ) {
                        EMLABCPP_ERROR_LOG( "No handle in coroutine ", address() );
                        return false;
                }
                if ( h_.done() ) {
                        EMLABCPP_ERROR_LOG(
                            "Ticking coroutine ", address(), " that is finished - skipping" );
                        return false;
                }
                h_.promise().request.reset();
                h_();
                h_.promise().reply.reset();
                return true;
        }

        void* address() const
        {
                return h_.address();
        }

private:
        owning_handle h_;
};

}  // namespace emlabcpp::coro
