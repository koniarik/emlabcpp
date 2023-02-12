#include <emlabcpp/algorithm.h>
#include <emlabcpp/pmr/memory_resource.h>
#include <emlabcpp/static_circular_buffer.h>

#pragma once

namespace emlabcpp::coro
{
template < typename Container >
typename Container::value_type round_robin_run( pmr::memory_resource&, Container coros )
{
        std::size_t i = 0;
        EMLABCPP_INFO_LOG( "Run of executor started" );

        while ( true ) {

                auto& cor = coros[i];
                i         = ( i + 1 ) % std::size( coros );

                if ( cor.done() ) {
                        if ( i == 0 && all_of( coros ) ) {
                                co_return;
                        }
                        continue;
                }

                const auto* out = cor.get_request();

                if ( out == nullptr ) {
                        EMLABCPP_ERROR_LOG( "reply from coroutine is nullptr!" );
                        co_return;
                }

                auto resp = co_yield *out;
                cor.store_reply( resp );

                if ( !cor.tick() ) {
                        EMLABCPP_ERROR_LOG(
                            "Coroutine ", cor.address(), " failed to tick, bailing out" );
                        co_return;
                }
        }
}

}  // namespace emlabcpp::coro
