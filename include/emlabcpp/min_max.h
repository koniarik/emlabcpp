#include <array>
#include <functional>

#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

#pragma once

namespace emlabcpp
{

/// Helper structure representing a range of two variables, one being a minimum
/// and second being a maximum.
template < typename T >
struct min_max : std::array< T, 2 >
{
        using value_type = T;

        [[nodiscard]] constexpr T& min()
        {
                return this->operator[]( 0 );
        }

        [[nodiscard]] constexpr const T& min() const
        {
                return this->operator[]( 0 );
        }

        [[nodiscard]] constexpr T& max()
        {
                return this->operator[]( 1 );
        }

        [[nodiscard]] constexpr const T& max() const
        {
                return this->operator[]( 1 );
        }

        constexpr min_max() = default;

        constexpr min_max( T lh, T rh )
          : std::array< T, 2 >{ std::move( lh ), std::move( rh ) }
        {
        }
};

template < typename T, typename Compare >
constexpr const T& clamp( const T& x, const min_max< T >& mm, Compare&& comp )
{
        return comp( x, mm.min() ) ? mm.min() : comp( mm.max(), x ) ? mm.max() : x;
}

template < typename T >
constexpr const T& clamp( const T& x, const min_max< T >& mm )
{
        return clamp( x, mm, std::less{} );
}

template < typename T, typename... Args >
requires( std::same_as< Args, min_max< T > > && ... )
constexpr min_max< T > intersection( const min_max< T >& head, const Args&... args )
{
        min_max< T > res{ head };
        auto         f = [&]( const min_max< T >& other ) {
                res.min() = std::max( res.min(), other.min() );
                res.max() = std::min( res.max(), other.max() );
        };
        ( f( args ), ... );

        return res;
}

template < typename T >
constexpr min_max< T > expand( const min_max< T >& mm, const T& val )
{
        if ( val < mm.min() ) {
                return { val, mm.max() };
        } else if ( val > mm.max() ) {
                return { mm.min(), val };
        } else {
                return mm;
        }
}

template < typename T >
constexpr bool contains( const min_max< T >& mm, const T& val )
{
        return mm.min() <= val && val <= mm.max();
}

}  // namespace emlabcpp

namespace std
{
template < typename T >
struct tuple_size< emlabcpp::min_max< T > > : std::integral_constant< std::size_t, 2 >
{
};

template < std::size_t I, typename T >
struct tuple_element< I, emlabcpp::min_max< T > >
{
        using type = T;
};
}  // namespace std

#ifdef EMLABCPP_USE_NLOHMANN_JSON

template < typename T >
struct nlohmann::adl_serializer< emlabcpp::min_max< T > >
{
        static void to_json( nlohmann::json& j, const emlabcpp::min_max< T >& mm )
        {
                j["min"] = mm.min();
                j["max"] = mm.max();
        }

        static emlabcpp::min_max< T > from_json( const nlohmann::json& j )
        {
                return emlabcpp::min_max< T >{ j["min"], j["max"] };
        }
};

#endif
