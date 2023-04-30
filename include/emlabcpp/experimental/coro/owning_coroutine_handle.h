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
