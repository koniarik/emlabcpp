#include "emlabcpp/concepts.h"
#include "emlabcpp/enum.h"
#include "emlabcpp/experimental/decompose.h"
#include "emlabcpp/match.h"
#include "emlabcpp/types.h"
#include "emlabcpp/view.h"

#include <filesystem>
#include <optional>
#include <span>
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

template < typename StreamType >
class pretty_printer
{
public:
        pretty_printer() = default;

        explicit pretty_printer( StreamType st )
          : os_( st )
        {
        }

        template < typename T >
        pretty_printer& operator<<( const T& item ) &
        {
                if constexpr ( pretty_printer_main_printable< T, pretty_printer > ) {
                        main_print( item );
                } else {
                        backup_print( item );
                }
                return *this;
        }

        [[nodiscard]] explicit operator bool() const
        {
                return bool( os_ );
        }

        [[nodiscard]] std::string str()
        {
                return os_.str();
        }

        [[nodiscard]] char fill() const
        {
                return os_.fill();
        }
        char fill( char ch )
        {
                return os_.fill( ch );
        }

        std::ios_base::fmtflags setf( std::ios_base::fmtflags flags, std::ios_base::fmtflags mask )
        {
                return os_.setf( flags, mask );
        }

        [[nodiscard]] std::streamsize width() const
        {
                return os_.width();
        }
        std::streamsize width( std::streamsize w )
        {
                return os_.width( w );
        }
        [[nodiscard]] std::ios_base::fmtflags flags() const
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
                os_ << j.dump();
        }
#endif

        void main_print( const std::filesystem::path& p )
        {
                main_print( p.string() );
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

        void main_print( const char* const c )
        {
                main_print( std::string_view{ c } );
        }

        void main_print( const std::string_view& sview )
        {
                os_ << sview;
        }

        void main_print( const std::string& str )
        {
                main_print( std::string_view{ str } );
        }

        template < typename... Ts >
        void main_print( const std::variant< Ts... >& var )
        {
                visit(
                    [this]( const auto& val ) {
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
                *this << pretty_type_name< T >() << decompose( item );
        }

private:
        StreamType os_;
};

}  // namespace emlabcpp
