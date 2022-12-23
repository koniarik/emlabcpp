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
        executor( run_id rid, pmr::memory_resource& mem, test_interface& test )
          : rid_( rid )
          , mem_( mem )
          , test_( test )
          , rec_()
          , coro_()
          , phas_( phase::INIT )
        {
        }

        bool errored() const
        {
                return errored_;
        }
        bool failed() const
        {
                return failed_;
        }

        run_id get_run_id() const
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
                        coro_.destroy();
                        coro_ = test_.run( mem_, rec_ );
                        phas_ = phase::RUN;
                        break;
                case phase::RUN:
                        if ( !coro_.done() ) {
                                coro_.tick();
                                break;
                        }
                        coro_.destroy();
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

        bool finished() const
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
        phase                 phas_;
};

}  // namespace emlabcpp::testing
