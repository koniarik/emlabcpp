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

#include "emlabcpp/experimental/testing/base.h"
#include "emlabcpp/experimental/testing/interface.h"

#pragma once

namespace emlabcpp::testing
{

class executor
{
        enum class phase
        {
                INIT,
                SETUP,
                RUN,
                TEARDOWN,
                FINISHED
        };

public:
        executor( const run_id rid, pmr::memory_resource& mem, test_interface& test )
          : rid_( rid )
          , mem_( mem )
          , test_( test )
          , rec_()
          , coro_()
        {
        }

        [[nodiscard]] bool errored() const
        {
                return errored_;
        }
        [[nodiscard]] bool failed() const
        {
                return failed_;
        }

        [[nodiscard]] run_id get_run_id() const
        {
                return rid_;
        }

        void tick()
        {
                switch ( phas_ ) {
                case phase::INIT:
                        coro_ = test_.setup( mem_, rec_ );
                        phas_ = phase::SETUP;
                        break;
                case phase::SETUP:
                        if ( !coro_.done() ) {
                                coro_.tick();
                                break;
                        }
                        if ( rec_.errored() ) {
                                failed_ = true;
                        }
                        coro_ = test_coroutine();
                        coro_ = test_.run( mem_, rec_ );
                        phas_ = phase::RUN;
                        break;
                case phase::RUN:
                        if ( !coro_.done() ) {
                                coro_.tick();
                                break;
                        }
                        coro_ = test_coroutine();
                        coro_ = test_.teardown( mem_, rec_ );
                        phas_ = phase::TEARDOWN;
                        break;
                case phase::TEARDOWN:
                        if ( !coro_.done() ) {
                                coro_.tick();
                                break;
                        }
                        if ( rec_.errored() ) {
                                errored_ = true;
                        }
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
        run_id                rid_;
        bool                  errored_ = false;
        bool                  failed_  = false;
        pmr::memory_resource& mem_;
        test_interface&       test_;
        record                rec_;
        test_coroutine        coro_;
        phase                 phas_{ phase::INIT };
};

}  // namespace emlabcpp::testing
