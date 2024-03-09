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

#include "emlabcpp/experimental/testing/base.h"
#include "emlabcpp/experimental/testing/coroutine.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/match.h"
#include "emlabcpp/static_vector.h"

namespace emlabcpp::testing
{

struct expect_awaiter : public coro::wait_interface
{
        bool success;

        expect_awaiter( bool success )
          : success( success )
        {
        }

        [[nodiscard]] coro::wait_state get_state() const override
        {
                return success ? coro::wait_state::READY : coro::wait_state::ERRORED;
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

class record
{
        test_status status_ = test_status::SUCCESS;

public:
        [[nodiscard]] test_status status() const
        {
                return status_;
        }

        void fail()
        {
                if ( status_ != test_status::ERRORED )
                        status_ = test_status::FAILED;
        }

        void skip()
        {
                if ( status_ == test_status::SUCCESS )
                        status_ = test_status::SKIPPED;
        }

        expect_awaiter expect( const bool val )
        {
                if ( !val )
                        fail();

                return { status_ != test_status::FAILED };
        }
};

}  // namespace emlabcpp::testing
