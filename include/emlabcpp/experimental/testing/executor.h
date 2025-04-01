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
///

#pragma once

#include "./base.h"
#include "./interface.h"

namespace emlabcpp::testing
{

class executor
{
        enum class phase : uint8_t
        {
                INIT,
                SETUP,
                RUN,
                TEARDOWN,
                FINISHED
        };

public:
        executor( run_id const rid, pmr::memory_resource& mem, test_interface& test )
          : rid_( rid )
          , mem_( mem )
          , test_( test )
          , coro_()
        {
        }

        [[nodiscard]] test_status status() const
        {
                return status_;
        }

        [[nodiscard]] run_id get_run_id() const
        {
                return rid_;
        }

        void tick()
        {
                switch ( phas_ ) {
                case phase::INIT:
                        coro_ = test_.setup( mem_ );
                        phas_ = phase::SETUP;
                        break;
                case phase::SETUP: {
                        if ( !coro_.done() ) {
                                coro_.tick();
                                break;
                        }
                        coro_state const s = coro_.get_state();
                        if ( s != coro_state::DONE ) {
                                status_ = status_cnv( coro_.get_state() );
                                phas_   = phase::FINISHED;
                        } else {
                                coro_ = coroutine< void >();
                                coro_ = test_.run( mem_ );
                                phas_ = phase::RUN;
                        }
                        break;
                }
                case phase::RUN:
                        if ( !coro_.done() ) {
                                coro_.tick();
                                break;
                        }
                        status_ = status_cnv( coro_.get_state() );
                        coro_   = coroutine< void >();
                        coro_   = test_.teardown( mem_ );
                        phas_   = phase::TEARDOWN;
                        break;
                case phase::TEARDOWN:
                        if ( !coro_.done() ) {
                                coro_.tick();
                                break;
                        }
                        if ( coro_.get_state() == coro_state::ERRORED )
                                status_ = test_status::ERRORED;
                        coro_ = coroutine< void >();
                        phas_ = phase::FINISHED;
                        break;
                case phase::FINISHED:
                        break;
                }
        }

        [[nodiscard]] bool finished() const
        {
                return phas_ == phase::FINISHED;
        }

private:
        static test_status status_cnv( coro_state s )
        {
                switch ( s ) {
                case coro_state::DONE:
                        return test_status::SUCCESS;
                case coro_state::SKIPPED:
                        return test_status::SKIPPED;
                case coro_state::FAILED:
                        return test_status::FAILED;
                default:
                        break;
                }
                return test_status::ERRORED;
        }

        run_id                rid_;
        test_status           status_ = test_status::ERRORED;
        pmr::memory_resource& mem_;
        test_interface&       test_;
        coroutine< void >     coro_;
        phase                 phas_{ phase::INIT };
};

}  // namespace emlabcpp::testing
