///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
/// associated documentation files (the "Software"), to deal in the Software without restriction,
/// including without limitation the rights to use, copy, modify, merge, publish, distribute,
/// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or substantial
/// portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
/// NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
/// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
/// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

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
