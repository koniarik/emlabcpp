#include "emlabcpp/allocator/pool.h"
#include "emlabcpp/experimental/coroutine/owning_coroutine_handle.h"
#include "emlabcpp/experimental/logging.h"

#include <coroutine>
#include <optional>

#pragma once

namespace emlabcpp
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
                const RequestType& await_resume()
                {
                        return *prom_->request;
                }
        };

        struct promise_type
        {
                static constexpr std::size_t ptr_size = sizeof( pool_interface* );
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

                awaiter yield_value( ReplyType out )
                {
                        reply = out;
                        return { this };
                }

                void* operator new( std::size_t sz, auto&, pool_interface* pi, auto&&... )
                {
                        return alloc( sz, pi );
                }

                void* operator new( std::size_t sz, pool_interface* pi, auto&&... )
                {
                        return alloc( sz, pi );
                }

                static void* alloc( std::size_t sz, pool_interface* pi )
                {
                        sz += ptr_size;
                        void* vp = pi->allocate( sz, alignof( promise_type ) );

                        pool_interface** p = reinterpret_cast< pool_interface** >( vp );

                        *p = pi;

                        p++;

                        return p;
                }

                void operator delete( void* ptr, std::size_t )
                {
                        pool_interface** p = reinterpret_cast< pool_interface** >( ptr );

                        p--;

                        ( *p )->deallocate( ptr, alignof( promise_type ) );
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

        const ReplyType* get_reply()
        {
                if ( !h_ ) {
                        EMLABCPP_LOG( "Can't extract reply from empty handle" );
                        return nullptr;
                }
                if ( !h_->promise().reply.has_value() ) {
                        EMLABCPP_LOG( "No reply in coroutine at " << &h_->promise().reply );
                        return nullptr;
                }
                return &*h_->promise().reply;
        }

        bool has_request()
        {
                if ( h_ ) {
                        return h_->promise().request.has_value();
                } else {
                        EMLABCPP_LOG( "Can't check request in empty handle" );
                        return false;
                }
        }

        void store_request( const RequestType& inpt )
        {
                if ( h_ ) {
                        h_->promise().request = inpt;
                } else {
                        EMLABCPP_LOG( "Can't store request in empty handle" );
                }
        }

        [[nodiscard]] operator bool() const
        {
                return h_->done();
        }

        [[nodiscard]] bool done() const
        {
                return h_->done();
        }

        [[nodiscard]] bool tick()
        {
                if ( !h_->promise().request ) {
                        EMLABCPP_LOG( "Can't tick coroutine " << address() << ", no request" );
                        return false;
                }
                if ( !h_ ) {
                        EMLABCPP_LOG( "No handle in coroutine " << address() );
                        return false;
                }
                if ( h_->done() ) {
                        EMLABCPP_LOG(
                            "Ticking coroutine " << address() << " that is finished - skipping" );
                        return false;
                }
                h_->promise().reply.reset();
                h_->resume();
                h_->promise().request.reset();
                return true;
        }

        void* address() const
        {
                return h_->address();
        }

private:
        owning_handle h_;
};

}  // namespace emlabcpp
