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
#pragma once

#include "./data_promise.h"
#include "./memory_promise.h"
#include "./owning_coroutine_handle.h"

#include <coroutine>

namespace emlabcpp::coro
{

enum class wait_state : uint8_t
{
        WAITING,
        READY,
        ERRORED
};

struct wait_interface
{
        [[nodiscard]] virtual wait_state get_state() const = 0;
        virtual void                     tick()            = 0;
        virtual ~wait_interface()                          = default;
};

struct noop_awaiter : public wait_interface
{
        [[nodiscard]] wait_state get_state() const override
        {
                return wait_state::READY;
        }

        void tick() override
        {
        }

        [[nodiscard]] bool await_ready() const
        {
                return false;
        }

        template < typename T >
        void await_suspend( std::coroutine_handle< T > )
        {
        }

        void await_resume() const
        {
        }
};

struct error_awaiter : public wait_interface
{
        [[nodiscard]] wait_state get_state() const override
        {
                return wait_state::ERRORED;
        }

        void tick() override
        {
        }

        [[nodiscard]] bool await_ready() const
        {
                return false;
        }

        template < typename T >
        void await_suspend( std::coroutine_handle< T > )
        {
        }

        void await_resume() const
        {
        }
};

template < typename T >
class recursive_coroutine : public wait_interface
{
public:
        struct promise_type : data_promise< T >, memory_promise< promise_type >
        {
                static recursive_coroutine get_return_object_on_allocation_failure()
                {
                        return recursive_coroutine{};
                }

                [[nodiscard]] recursive_coroutine get_return_object()
                {
                        return recursive_coroutine{ handle::from_promise( *this ) };
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
                }

                wait_interface* iface = nullptr;
        };

        using handle        = std::coroutine_handle< promise_type >;
        using owning_handle = owning_coroutine_handle< promise_type >;

        recursive_coroutine() = default;

        explicit recursive_coroutine( handle const& cor )
          : h_( cor )
        {
        }

        [[nodiscard]] bool done() const
        {
                if ( h_ )
                        return h_.done();
                return true;
        }

        [[nodiscard]] wait_state get_state() const override
        {
                if ( done() )
                        return wait_state::READY;
                return wait_state::WAITING;
        }

        [[nodiscard]] bool await_ready() const
        {
                return false;
        }

        template < typename U >
        void await_suspend( U const h )
        {
                h.promise().iface = this;
        }

        auto await_resume()
        {
                if constexpr ( !std::is_void_v< T > )
                        return h_.promise().value;
        }

        auto get_value()
        {
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
                if ( !h_ )
                        return;
                wait_interface* iface = h_.promise().iface;

                if ( iface != nullptr ) {
                        wait_state const s = iface->get_state();
                        if ( s == wait_state::WAITING ) {
                                iface->tick();
                                return;
                        } else if ( s == wait_state::ERRORED ) {
                                h_ = owning_handle();
                                return;
                        } else {
                                // Intentionally does nothing
                        }
                }
                if ( !h_.done() )
                        h_();
        }

private:
        owning_handle h_;
};

}  // namespace emlabcpp::coro
