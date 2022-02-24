#include "emlabcpp/concepts.h"
#include "emlabcpp/types/base.h"

#ifdef EMLABCPP_USE_STREAMS
#include <ostream>
#endif

#include <string>

#pragma once

namespace emlabcpp
{

// ------------------------------------------------------------------------------------------------
/// has_static_size<T>::value is true in case type T have size deduceable at compile time

template < typename T >
constexpr bool has_static_size_v = static_sized< T >;

// ------------------------------------------------------------------------------------------------
/// mapped<T,F>::type is type returned by instance of F::operator() when applied on items from
/// instance of T. It can differentiate between tuples or containers

template < typename Container, typename UnaryFunction >
struct mapped;

template < gettable_container Container, typename UnaryFunction >
requires( !range_container< Container > ) struct mapped< Container, UnaryFunction >
{
        using type = decltype( std::declval< UnaryFunction >()(
            std::get< 0 >( std::declval< Container >() ) ) );
};

template < range_container Container, typename UnaryFunction >
struct mapped< Container, UnaryFunction >
{
        using type = decltype( std::declval< UnaryFunction >()(
            *std::begin( std::declval< Container >() ) ) );
};

template < typename Container, typename UnaryFunction >
using mapped_t = typename mapped< Container, UnaryFunction >::type;

// ------------------------------------------------------------------------------------------------
//// tag<V> type can be used for tagging f-calls for function overloading
template < auto V >
struct tag
{
        using value_type = decltype( V );

        static constexpr value_type value = V;

        friend constexpr auto operator<=>( const tag&, const tag& ) = default;
};

#ifdef EMLABCPP_USE_STREAMS
template < auto ID >
inline std::ostream& operator<<( std::ostream& os, tag< ID > )
{
        return os << ID;
}
#endif

// ------------------------------------------------------------------------------------------------
//// central function for returning name of type that can demangle if necessary

template < typename T >
inline std::string pretty_type_name()
{
        std::string res;
#ifdef EMLABCPP_USE_DEMANGLING
        int   tmp   = 0;
        char* dname = abi::__cxa_demangle( typeid( T ).name(), nullptr, nullptr, &tmp );
        res         = dname;
        free( dname );
#elseif EMLABCPP_USE_TYPEID
        res = typeid( T ).name();
#endif
        return res;
}

}  // namespace emlabcpp
