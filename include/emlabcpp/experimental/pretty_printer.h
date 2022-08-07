#include "emlabcpp/concepts.h"
#include "emlabcpp/enum.h"
#include "emlabcpp/match.h"

#include <optional>
#include <sstream>
#include <tuple>
#include <variant>

#pragma once

namespace emlabcpp
{

class pretty_printer
{
public:
        template < std::same_as< uint8_t > T >
        pretty_printer& operator<<( const T& val )
        {
                os_ << int( val );
                return *this;
        }

        template < typename T >
        requires(
            detail::directly_streamable_for< std::ostream, T > && !std::same_as< T, uint8_t > &&
            !std::is_enum_v< T > ) pretty_printer&
        operator<<( const T& val )
        {
                os_ << val;
                return *this;
        }
 
        template < typename T >
        requires( std::is_enum_v< T > ) pretty_printer& operator<<( const T& val )
        {
                return *this << convert_enum( val );
        }

        pretty_printer& operator<<( const std::string_view& sview )
        {
                os_ << sview;
                return *this;
        }

        template < typename... Ts >
        pretty_printer& operator<<( const std::variant< Ts... >& var )
        {
                visit(
                    [&]( const auto& val ) {
                            *this << val;
                    },
                    var );
                return *this;
        }

        template < typename... Ts >
        pretty_printer& operator<<( const std::tuple< Ts... >& tpl )
        {
                if ( sizeof...( Ts ) == 0 ) {
                        *this << "()";
                        return *this;
                }
                char delim = '(';
                for_each( tpl, [&]( const auto& item ) {
                        *this << delim << item;
                        delim = ',';
                } );
                return *this << ')';
        }

        template < typename T >
        pretty_printer& operator<<( const std::optional< T >& val )
        {
                if ( val ) {
                        return *this << *val;
                }
                return *this << "nothing";
        }

        operator bool()
        {
                return bool( os_ );
        }

        std::string str()
        {
                return os_.str();
        }

        char fill()
        {
                return os_.fill();
        }
        char fill( char ch )
        {
                return os_.fill( ch );
        }
        std::streamsize width()
        {
                return os_.width();
        }
        std::streamsize width( std::streamsize w )
        {
                return os_.width( w );
        }
        std::ios_base::fmtflags flags()
        {
                return os_.flags();
        }
        std::ios_base::fmtflags flags( std::ios_base::fmtflags f )
        {
                return os_.flags( f );
        }

private:
        std::stringstream os_;
};

}  // namespace emlabcpp
