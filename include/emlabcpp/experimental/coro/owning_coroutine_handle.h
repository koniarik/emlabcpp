#include <coroutine>
#include <utility>

#pragma once

namespace emlabcpp::coro
{

template < typename PromiseType >
class owning_coroutine_handle
{
public:
        using promise_type = PromiseType;

        owning_coroutine_handle() = default;

        explicit owning_coroutine_handle( std::coroutine_handle< promise_type > h )
          : h_( h )
        {
        }

        owning_coroutine_handle( const owning_coroutine_handle& ) = delete;
        owning_coroutine_handle( owning_coroutine_handle&& other ) noexcept
        {
                *this = std::move( other );
        }

        owning_coroutine_handle& operator=( const owning_coroutine_handle& ) = delete;
        owning_coroutine_handle& operator=( owning_coroutine_handle&& other ) noexcept
        {
                std::swap( h_, other.h_ );
                return *this;
        }

        void operator()() const
        {
                h_();
        }

        [[nodiscard]] constexpr explicit operator bool() const
        {
                return static_cast< bool >( h_ );
        }

        [[nodiscard]] constexpr bool done() const
        {
                return h_.done();
        }
        [[nodiscard]] constexpr void* address() const
        {
                return h_.address();
        }

        constexpr promise_type& promise()
        {
                return h_.promise();
        }

        [[nodiscard]] constexpr const promise_type& promise() const
        {
                return h_.promise();
        }

        ~owning_coroutine_handle()
        {
                if ( h_ ) {
                        h_.destroy();
                }
        }

private:
        std::coroutine_handle< promise_type > h_;
};

}  // namespace emlabcpp::coro
