#include "emlabcpp/experimental/coro/memory_promise.h"
#include "emlabcpp/experimental/coro/owning_coroutine_handle.h"

#include <coroutine>

#pragma once

namespace emlabcpp::coro
{

enum class wait_state : uint8_t
{
        WAITING,
        READY,
        ERRORED
};

class wait_interface
{
public:
        [[nodiscard]] virtual wait_state get_state() const = 0;
        virtual void                     tick()            = 0;
        virtual ~wait_interface()                          = default;
};

template < typename T >
struct data_promise
{
        template < typename U >
        void return_value( U&& val )
        {
                value = std::forward< U >( val );
        }

        T value;
};

template <>
struct data_promise< void >
{
        void return_void() const
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

        explicit recursive_coroutine( const handle& cor )
          : h_( cor )
        {
        }

        [[nodiscard]] bool done() const
        {
                if ( h_ ) {
                        return h_.done();
                }
                return true;
        }

        [[nodiscard]] wait_state get_state() const override
        {
                if ( done() ) {
                        return wait_state::READY;
                }
                return wait_state::WAITING;
        }

        [[nodiscard]] bool await_ready() const
        {
                return false;
        }

        template < typename U >
        void await_suspend( const U h )
        {
                h.promise().iface = this;
        }

        auto await_resume()
        {
                if constexpr ( !std::is_void_v< T > ) {
                        return h_.promise().value;
                }
        }

        void tick() override
        {
                if ( !h_ ) {
                        return;
                }
                wait_interface* iface = h_.promise().iface;

                if ( iface != nullptr ) {
                        const wait_state s = iface->get_state();
                        if ( s == wait_state::WAITING ) {
                                iface->tick();
                                return;
                        } else if ( s == wait_state::ERRORED ) {
                                EMLABCPP_ERROR_LOG( "Coroutine errored, killing it" );
                                h_ = owning_handle();
                                return;
                        }
                }
                if ( !h_.done() ) {
                        iface = nullptr;
                        h_();
                }
        }

private:
        owning_handle h_;
};

}  // namespace emlabcpp::coro
