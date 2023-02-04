
#include "emlabcpp/experimental/coro/memory_promise.h"
#include "emlabcpp/experimental/coro/owning_coroutine_handle.h"
#include "emlabcpp/experimental/testing/convert.h"
#include "emlabcpp/experimental/testing/reactor_interface_adapter.h"

#include <coroutine>

#pragma once

namespace emlabcpp::testing
{

enum class await_state : uint8_t
{
        WAITING,
        READY,
        ERRORED
};

class test_awaiter_interface
{
public:
        [[nodiscard]] virtual await_state get_state() const = 0;
        virtual void                      tick()
        {
        }
        virtual ~test_awaiter_interface() = default;
};

class test_coroutine : public test_awaiter_interface
{
public:
        struct promise_type : coro::memory_promise< promise_type >
        {

                [[nodiscard]] test_coroutine get_return_object()
                {
                        return test_coroutine{ handle::from_promise( *this ) };
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

                void return_void() const
                {
                }

                test_awaiter_interface* iface = nullptr;
        };

        using handle        = std::coroutine_handle< promise_type >;
        using owning_handle = coro::owning_coroutine_handle< promise_type >;

        test_coroutine() = default;

        explicit test_coroutine( const handle& cor )
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

        [[nodiscard]] await_state get_state() const override
        {
                if ( done() ) {
                        return await_state::READY;
                }
                return await_state::WAITING;
        }

        [[nodiscard]] bool await_ready() const
        {
                return false;
        }

        void await_suspend( std::coroutine_handle< typename test_coroutine::promise_type > h )
        {
                h.promise().iface = this;
        }

        void await_resume()
        {
        }

        // TODO: this is shady API as fuck
        void tick()
        {
                if ( !h_ ) {
                        return;
                }
                test_awaiter_interface* iface = h_.promise().iface;

                if ( iface != nullptr ) {
                        await_state s = iface->get_state();
                        if ( s == await_state::WAITING ) {
                                iface->tick();
                                return;
                        } else if ( s == await_state::ERRORED ) {
                                EMLABCPP_LOG( "Coroutine errored, killing it" );
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

}  // namespace emlabcpp::testing
