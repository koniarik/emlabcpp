#include "emlabcpp/concepts.h"
#include "emlabcpp/enum.h"
#include "emlabcpp/experimental/decompose.h"
#include "emlabcpp/match.h"

#include <filesystem>
#include <optional>
#include <sstream>
#include <tuple>
#include <variant>

#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

#pragma once

namespace emlabcpp
{

template < typename T, typename PrettyPrinter >
concept pretty_printer_main_printable = requires( const T& item, PrettyPrinter& pp )
{
        pp.main_print( item );
};

class pretty_printer
{
public:
        template < typename T >
        pretty_printer& operator<<( const T& item )
        {
                if constexpr ( pretty_printer_main_printable< T, pretty_printer > ) {
                        main_print( item );
                } else {
                        backup_print( item );
                }
                return *this;
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

#ifdef EMLABCPP_USE_NLOHMANN_JSON
        void main_print( const nlohmann::json& j )
        {
                os_ << j;
        }
#endif

        void main_print( const std::filesystem::path& p )
        {
                os_ << p;
        }

        template < typename T >
        void main_print( const std::reference_wrapper< T >& val )
        {
                *this << val.get();
        }

        template < std::same_as< uint8_t > T >
        void main_print( const T& val )
        {
                os_ << int( val );
        }

        template < typename T >
        requires(
            detail::directly_streamable_for< std::ostream, T > && !std::same_as< T, uint8_t > &&
            !std::is_enum_v< T > ) void main_print( const T& val )
        {
                os_ << val;
        }

        template < typename T >
        requires( std::is_enum_v< T > ) void main_print( const T& val )
        {
                *this << convert_enum( val );
        }

        void main_print( const std::string_view& sview )
        {
                os_ << sview;
        }

        void main_print( const std::string& str )
        {
                os_ << str;
        }

        template < typename... Ts >
        void main_print( const std::variant< Ts... >& var )
        {
                visit(
                    [&]( const auto& val ) {
                            *this << val;
                    },
                    var );
        }

        template < typename... Ts >
        void main_print( const std::tuple< Ts... >& tpl )
        {
                if ( sizeof...( Ts ) == 0 ) {
                        *this << "()";
                        return;
                }
                char delim = '(';
                for_each( tpl, [&]( const auto& item ) {
                        *this << delim << item;
                        delim = ',';
                } );
                *this << ')';
        }

        template < typename T >
        void main_print( const std::optional< T >& val )
        {
                if ( val ) {
                        *this << *val;
                }
                *this << "nothing";
        }

        template < decomposable T >
        void backup_print( const T& item )
        {
                *this << decompose( item );
        }

private:
        std::stringstream os_;
};

}  // namespace emlabcpp
