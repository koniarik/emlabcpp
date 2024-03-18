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

#include "emlabcpp/experimental/coro/memory_promise.h"
#include "emlabcpp/experimental/coro/owning_coroutine_handle.h"
#include "emlabcpp/experimental/logging.h"

#include <coroutine>
#include <optional>

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

                ReplyType const& await_resume()
                {
                        // NOLINTNEXTLINE
                        return *prom_->reply;
                }
        };

        struct promise_type : memory_promise< promise_type >
        {
                std::optional< RequestType > request;
                std::optional< ReplyType >   reply;

                static request_reply get_return_object_on_allocation_failure()
                {
                        return {};
                }

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

        request_reply() = default;

        request_reply( handle const cor )
          : h_( cor )
        {
        }

        request_reply( request_reply const& )            = delete;
        request_reply& operator=( request_reply const& ) = delete;

        request_reply( request_reply&& ) noexcept            = default;
        request_reply& operator=( request_reply&& ) noexcept = default;

        RequestType const* get_request()
        {
                if ( !h_ ) {
                        EMLABCPP_ERROR_LOG( "Can't extract request from empty handle" );
                        return nullptr;
                }
                std::optional< RequestType >& opt_val = h_.promise().request;
                if ( !opt_val.has_value() ) {
                        EMLABCPP_ERROR_LOG( "No request in coroutine at ", &h_.promise().request );
                        return nullptr;
                }
                return &*opt_val;
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

        void store_reply( ReplyType const& inpt )
        {
                if ( h_ )
                        h_.promise().reply = inpt;
                else
                        EMLABCPP_ERROR_LOG( "Can't store reply in empty handle" );
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

        [[nodiscard]] void* address() const
        {
                return h_.address();
        }

private:
        owning_handle h_;
};

}  // namespace emlabcpp::coro
