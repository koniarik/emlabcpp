#include <emlabcpp/algorithm.h>
#include <emlabcpp/allocator/pool.h>
#include <emlabcpp/static_circular_buffer.h>

#pragma once

namespace emlabcpp
{
template < typename Coroutine, std::size_t N >
Coroutine round_robin_run( pool_interface*, std::array< Coroutine, N > coros )
{
        std::size_t i = 0;
        EMLABCPP_LOG( "Run of executor started" );

        while ( true ) {

                Coroutine& cor = coros[i];
                i              = ( i + 1 ) % N;

                if ( cor.done() ) {
                        if ( i == 0 && all_of( coros ) ) {
                                co_return;
                        }
                        continue;
                }

                const auto* out = cor.get_request();

                if ( out == nullptr ) {
                        EMLABCPP_LOG( "reply from coroutine is nullptr!" );
                        co_return;
                }

                auto resp = co_yield *out;
                cor.store_reply( resp );

                if ( !cor.tick() ) {
                        EMLABCPP_LOG(
                            "Coroutine " << cor.address() << " failed to tick, bailing out" );
                        co_return;
                }
        }
}

}  // namespace emlabcpp
