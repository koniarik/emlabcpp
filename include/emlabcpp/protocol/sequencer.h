#include "emlabcpp/assert.h"
#include "emlabcpp/either.h"
#include "emlabcpp/static_circular_buffer.h"

#include <array>

#pragma once

namespace emlabcpp
{

template < typename Def >
class protocol_sequencer
{
public:
        static constexpr auto        prefix     = Def::prefix;
        static constexpr std::size_t fixed_size = Def::fixed_size;

private:
        static_circular_buffer< uint8_t, Def::message_type::max_size * 2 > buffer_;

public:
        using message_type = typename Def::message_type;

        template < typename Iterator >
        either< std::size_t, message_type > load_data( view< Iterator > dview )
        {
                std::copy( dview.begin(), dview.end(), std::back_inserter( buffer_ ) );

                auto bend = buffer_.end();
                while ( !buffer_.empty() ) {
                        auto [piter, biter] =
                            std::mismatch( prefix.begin(), prefix.end(), buffer_.begin(), bend );

                        // not match, remove one byte from buffer
                        if ( biter == buffer_.begin() ) {
                                buffer_.pop_front();
                                continue;
                        }

                        // partial match - more bytes could be matched
                        if ( piter != prefix.end() && biter != bend ) {
                                buffer_.pop_front();
                                continue;
                        }

                        // partial match - matched maximum bytes available
                        if ( piter != prefix.end() ) {
                                return fixed_size - static_cast< std::size_t >(
                                                        std::distance( prefix.begin(), piter ) );
                        }

                        break;
                }

                if ( buffer_.empty() ) {
                        return fixed_size;
                }

                std::size_t bsize = buffer_.size();

                // This is implied by the fact that we should have full match at the start of buffer
                EMLABCPP_ASSERT( bsize >= prefix.size() );

                if ( bsize < fixed_size ) {
                        return fixed_size - bsize;
                }

                std::size_t desired_size = Def::get_size( buffer_ );
                if ( bsize < desired_size ) {
                        return desired_size - bsize;
                }

                auto opt_msg = message_type::make( view_n( buffer_.begin(), desired_size ) );
                EMLABCPP_ASSERT( opt_msg );

                // clean up only the matched message
                for ( std::size_t i = desired_size; i > 0; i-- ) {
                        buffer_.pop_front();
                }

                return *opt_msg;
        }
};

template < typename Sequencer, typename ReadCallback >
std::optional< typename Sequencer::message_type >
protocol_simple_load( std::size_t read_limit, ReadCallback&& read )
{
        Sequencer                                         seq;
        std::optional< typename Sequencer::message_type > res;
        std::size_t                                       to_read = Sequencer::fixed_size;
        std::size_t                                       count   = 0;
        while ( !res && count < read_limit ) {
                auto data = read( to_read );
                seq.load_data( view{ data } )
                    .match(
                        [&]( std::size_t next_read ) {
                                to_read = next_read;
                                count   = 0;
                        },
                        [&]( auto msg ) {
                                res = msg;
                        } );

                count += 1;
        }
        return res;
}

}  // namespace emlabcpp
