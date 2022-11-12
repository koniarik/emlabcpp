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

        owning_coroutine_handle( std::coroutine_handle< promise_type > h )
          : h_( h )
        {
        }

        owning_coroutine_handle( const owning_coroutine_handle& ) = delete;
        owning_coroutine_handle( owning_coroutine_handle&& other )
        {
                *this = std::move( other );
        }

        owning_coroutine_handle& operator=( const owning_coroutine_handle& ) = delete;
        owning_coroutine_handle& operator=( owning_coroutine_handle&& other )
        {
                h_       = other.h_;
                other.h_ = std::coroutine_handle< promise_type >{ nullptr };
                return *this;
        }

        void operator()()
        {
                h_();
        }

        operator bool() const
        {
                return bool( h_ );
        }

        const std::coroutine_handle< promise_type >& operator*() const
        {
                return h_;
        }
        const std::coroutine_handle< promise_type >* operator->() const
        {
                return &h_;
        }

        ~owning_coroutine_handle()
        {
                if ( h_ && !h_.done() ) {
                        h_.destroy();
                }
        }

private:
        std::coroutine_handle< promise_type > h_;
};

}  // namespace emlabcpp::coro
