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

#pragma once

#include "emlabcpp/experimental/coro/data_promise.h"
#include "emlabcpp/experimental/coro/memory_promise.h"
#include "emlabcpp/experimental/coro/owning_coroutine_handle.h"

#include <coroutine>

namespace emlabcpp::testing
{

enum class coro_state : uint8_t
{
        WAITING = 0x1,
        ERRORED = 0x3,
        SKIPPED = 0x4,
        FAILED  = 0x5,
        DONE    = 0x6
};

struct wait_interface
{
        [[nodiscard]] virtual coro_state get_state() const = 0;
        virtual void                     tick(){};
        virtual ~wait_interface() = default;
};

template < typename T >
class coroutine : public wait_interface
{
public:
        struct promise_type : coro::data_promise< T >, coro::memory_promise< promise_type >
        {
                static coroutine get_return_object_on_allocation_failure()
                {
                        return coroutine{ coro_state::ERRORED };
                }

                [[nodiscard]] coroutine get_return_object()
                {
                        return coroutine{ handle::from_promise( *this ) };
                }

                [[nodiscard]] std::suspend_always initial_suspend() const
                {
                        return {};
                }

                [[nodiscard]] std::suspend_always final_suspend() const noexcept
                {
                        return {};
                }

                void unhandled_exception() const
                {
                        EMLABCPP_ERROR_LOG( "Got unhandled exception" );
                }

                wait_interface* iface = nullptr;
        };

        using handle        = std::coroutine_handle< promise_type >;
        using owning_handle = coro::owning_coroutine_handle< promise_type >;

        explicit coroutine() = default;

        explicit coroutine( const handle& cor )
          : state_( cor ? coro_state::WAITING : coro_state::ERRORED )
          , h_( cor )
        {
        }

        explicit coroutine( coro_state s )
          : state_( s )
        {
        }

        [[nodiscard]] bool done() const
        {
                if ( h_ )
                        return h_.done();
                return true;
        }

        [[nodiscard]] coro_state get_state() const override
        {
                return state_;
        }

        [[nodiscard]] bool await_ready() const
        {
                return false;
        }

        void await_suspend( const auto& h )
        {
                h.promise().iface = this;
        }

        auto await_resume()
        {
                if constexpr ( !std::is_void_v< T > )
                        return h_.promise().value;
        }

        auto run()
        {
                while ( !done() )
                        tick();

                if constexpr ( !std::is_void_v< T > )
                        return h_.promise().value;
        }

        void tick() override
        {
                if ( state_ != coro_state::WAITING )
                        return;

                if ( done() ) {
                        EMLABCPP_ERROR_LOG( "Prematurely done" );
                        state_ = coro_state::ERRORED;
                        return;
                }

                wait_interface* iface = h_.promise().iface;

                if ( iface != nullptr ) {
                        const coro_state s = iface->get_state();
                        switch ( s ) {
                        case coro_state::WAITING:
                                iface->tick();
                                return;
                        case coro_state::ERRORED:
                                EMLABCPP_ERROR_LOG( "Coroutine errored, killing it" );
                                [[fallthrough]];
                        case coro_state::SKIPPED:
                        case coro_state::FAILED:
                                h_     = owning_handle();
                                state_ = s;
                                return;
                        case coro_state::DONE:
                                break;
                        }
                }
                h_();

                if ( done() )
                        state_ = coro_state::DONE;
        }

private:
        coro_state    state_ = coro_state::DONE;
        owning_handle h_;
};

struct status_awaiter : wait_interface
{
        coro_state state;

        status_awaiter( coro_state s )
          : state( s )
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
                return state == coro_state::DONE;
        }

        template < typename T >
        void await_suspend( std::coroutine_handle< T > h )
        {
                h.promise().iface = this;
        }

        void await_resume() const
        {
        }
};

inline status_awaiter expect( bool expr )
{
        return { expr ? coro_state::DONE : coro_state::FAILED };
}

inline status_awaiter fail()
{
        return { coro_state::FAILED };
}

inline status_awaiter skip()
{
        return { coro_state::SKIPPED };
}

}  // namespace emlabcpp::testing
