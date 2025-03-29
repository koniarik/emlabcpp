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

#include "../assert.h"
#include "../quantity.h"
#include "../static_circular_buffer.h"

#include <array>

namespace emlabcpp::protocol
{

struct sequencer_read_request : quantity< sequencer_read_request, std::size_t >
{
        using quantity::quantity;
};

template < typename Def >
class sequencer
{
public:
        static constexpr auto        prefix     = Def::prefix;
        static constexpr std::size_t fixed_size = Def::fixed_size;

private:
        static_circular_buffer< std::byte, Def::message_type::capacity * 2 > buffer_;

public:
        using message_type = typename Def::message_type;

        template < typename Container >
        void insert( Container&& dview )
        {
                copy( std::forward< Container >( dview ), std::back_inserter( buffer_ ) );
        }

        std::variant< sequencer_read_request, message_type > get_message()
        {
                auto bend = buffer_.end();
                while ( !buffer_.empty() ) {
                        auto [piter, biter] =
                            std::mismatch( prefix.begin(), prefix.end(), buffer_.begin(), bend );

                        /// not match, remove one byte from buffer
                        if ( biter == buffer_.begin() ) {
                                buffer_.pop_front();
                                continue;
                        }

                        /// partial match - more bytes could be matched
                        if ( piter != prefix.end() && biter != bend ) {
                                buffer_.pop_front();
                                continue;
                        }

                        /// partial match - matched maximum bytes available
                        if ( piter != prefix.end() ) {
                                return sequencer_read_request{
                                    fixed_size - static_cast< std::size_t >(
                                                     std::distance( prefix.begin(), piter ) ) };
                        }

                        break;
                }

                if ( buffer_.empty() )
                        return sequencer_read_request{ fixed_size };

                std::size_t const bsize = buffer_.size();

                /// This is implied by the fact that we should have full match at the start of
                /// buffer
                EMLABCPP_ASSERT( bsize >= prefix.size() );

                if ( bsize < fixed_size )
                        return sequencer_read_request{ fixed_size - bsize };

                std::size_t const desired_size = Def::get_size( buffer_ );
                if ( bsize < desired_size )
                        return sequencer_read_request{ desired_size - bsize };

                message_type res( desired_size );
                copy( view_n( buffer_.begin(), desired_size ), res.begin() );

                /// clean up only the matched message
                for ( std::size_t i = desired_size; i > 0; i-- )
                        buffer_.pop_front();

                return res;
        }
};

template < typename Sequencer, typename ReadCallback >
std::optional< typename Sequencer::message_type >
sequencer_simple_load( std::size_t const read_limit, ReadCallback&& read )
{
        Sequencer                                         seq;
        std::optional< typename Sequencer::message_type > res;
        std::size_t                                       to_read = Sequencer::fixed_size;
        std::size_t                                       count   = 0;
        while ( !res && count < read_limit ) {
                std::optional data = read( to_read );
                if ( !data )
                        return res;
                seq.insert( *data );
                seq.get_message().match(
                    [&to_read, &count]( std::size_t const next_read ) {
                            to_read = next_read;
                            count   = 0;
                    },
                    [&res]( auto msg ) {
                            res = msg;
                    } );

                count += 1;
        }
        return res;
}

}  // namespace emlabcpp::protocol
