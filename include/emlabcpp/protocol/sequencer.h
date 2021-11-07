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
        static_circular_buffer< uint8_t, Def::buffer_size > buffer_;

public:
        using message_type = typename Def::message_type;

        template < typename Iterator >
        either< std::size_t, message_type > load_data( view< Iterator > dview )
        {
                std::copy( dview.begin(), dview.end(), std::back_inserter( buffer_ ) );

                while ( !buffer_.empty() ) {
                        auto [piter, biter] = std::mismatch(
                            prefix.begin(), prefix.end(), buffer_.begin(), buffer_.end() );

                        // not match, remove one byte from buffer
                        if ( biter == buffer_.begin() ) {
                                buffer_.pop_front();
                                continue;
                        }

                        // partial match - more bytes could be matched
                        if ( piter != prefix.end() && biter != buffer_.end() ) {
                                buffer_.pop_front();
                                continue;
                        }

                        // partial match - matched maximum bytes available
                        if ( piter != prefix.end() ) {
                                return fixed_size - std::distance( prefix.begin(), piter );
                        }

                        break;
                }

                if ( buffer_.empty() ) {
                        return fixed_size;
                }

                // This is implied by the fact that we should have full match at the start of buffer
                EMLABCPP_ASSERT( buffer_.size() >= prefix.size() );

                if ( buffer_.size() < fixed_size ) {
                        return fixed_size - buffer_.size();
                }

                std::size_t desired_size = Def::get_size( buffer_ );
                if ( buffer_.size() < desired_size ) {
                        return desired_size - buffer_.size();
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

}  // namespace emlabcpp
