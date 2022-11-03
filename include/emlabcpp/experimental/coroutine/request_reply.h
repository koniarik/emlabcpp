#include "emlabcpp/allocator/pool.h"
#include "emlabcpp/experimental/coroutine/owning_coroutine_handle.h"
#include "emlabcpp/experimental/logging.h"

#include <coroutine>
#include <optional>

#pragma once

namespace emlabcpp
{

template < typename InputType, typename OutputType >
class request_reply
{
public:
        using input_type  = InputType;
        using output_type = OutputType;

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
                const InputType& await_resume()
                {
                        return *prom_->input;
                }
        };

        struct promise_type
        {
                static constexpr std::size_t ptr_size = sizeof( pool_interface* );
                std::optional< InputType >   input;
                std::optional< OutputType >  output;

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

                awaiter yield_value( OutputType out )
                {
                        output = out;
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

        const OutputType* get_output()
        {
                if ( !h_ ) {
                        EMLABCPP_LOG( "Can't extract output from empty handle" );
                        return nullptr;
                }
                if ( !h_->promise().output.has_value() ) {
                        EMLABCPP_LOG( "No output in coroutine at " << &h_->promise().output );
                        return nullptr;
                }
                return &*h_->promise().output;
        }

        bool has_input()
        {
                if ( h_ ) {
                        return h_->promise().input.has_value();
                } else {
                        EMLABCPP_LOG( "Can't check input in empty handle" );
                        return false;
                }
        }

        void store_input( const InputType& inpt )
        {
                if ( h_ ) {
                        h_->promise().input = inpt;
                } else {
                        EMLABCPP_LOG( "Can't store input in empty handle" );
                }
        }

        [[nodiscard]] bool done() const
        {
                return h_->done();
        }

        [[nodiscard]] bool tick()
        {
                if ( !h_->promise().input ) {
                        EMLABCPP_LOG( "Can't tick coroutine " << address() << ", no input" );
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
                h_->promise().output.reset();
                h_->resume();
                h_->promise().input.reset();
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
