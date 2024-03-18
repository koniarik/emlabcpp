#pragma once

#include "emlabcpp/algorithm.h"

#include <cstdint>

namespace emlabcpp
{

/// Unique type representing success, should not be used directly
struct success_type
{
        static constexpr uint8_t id = 1;
};

/// Global constant of success_type for direct usage
inline success_type SUCCESS;

/// Unique type representing error, should not be used directly
struct error_type
{
        static constexpr uint8_t id = 2;
};

/// Global constant of error_type for direct usage
inline error_type ERROR;

/// `result` represents an result of some operation, as an alternative to returning just `bool` with
/// true/false value. Result has only two states, SUCCESS or ERROR, to check which state it is in
/// use operator== or operator !=: `if( res == SUCCESS) { ...`, alternativelly, one can directly
/// access the state value via `get_state()`, which returns numerical representation.
class [[nodiscard]] result
{
public:
        /// constructs result with SUCCESS
        constexpr result( success_type )
          : state_( success_type::id )
        {
        }

        /// construct result with ERROR
        constexpr result( error_type )
          : state_( error_type::id )
        {
        }

        /// returns current state of the result as numerical id, this corresponds to
        /// success_type::id or error_type::id respectively
        [[nodiscard]] constexpr uint8_t get_state() const
        {
                return state_;
        }

        static result from_bool( bool has_succeeded )
        {
                if ( has_succeeded )
                        return SUCCESS;
                return ERROR;
        }

private:
        uint8_t state_;
};

/// Compare two results for equality, uses get_state()
constexpr bool operator==( result const& lh, result const& rh )
{
        return lh.get_state() == rh.get_state();
}

/// Compare two results for inequality, uses get_state()
constexpr bool operator!=( result const& lh, result const& rh )
{
        return !( lh == rh );
}

/// Filter concept that allows only success_type or error_type
template < typename T >
concept result_native_constant = std::same_as< T, success_type > || std::same_as< T, error_type >;

/// Compares result with any of the constant types
template < result_native_constant T >
constexpr bool operator==( result const& res, T )
{
        return res.get_state() == T::id;
}

/// Compares result with any of the constant types
constexpr bool operator==( result_native_constant auto c, result const& res )
{
        return res == c;
}

/// Compares result for inequality with any of the constant types
constexpr bool operator!=( result const& res, result_native_constant auto c )
{
        return !( res == c );
}

/// Compares result for inequality with any of the constant types
constexpr bool operator!=( result_native_constant auto c, result const& res )
{
        return !( res == c );
}

/// Matches types that could be considered a result constant, matches anything, not just
/// error_type/success_type. The goal is _some_ of the result infrastructure to be compatible with
/// other similar types.
template < typename T >
concept result_constant = requires() {
        { T::id } -> std::convertible_to< uint8_t >;
};

/// Matches types that could be considered a result value, that is result similar types can be
/// treated the same way.
template < typename T >
concept result_value = requires( T item ) {
        { item.get_state() } -> std::convertible_to< uint8_t >;
};

/// Matches types that are relevant in context of working with result-like objects, be it result
/// values or result constants. Designed to allow code that can be used with similar types to
/// result.
template < typename T >
concept result_like = result_constant< T > || result_value< T >;

/// Returns the worst result like state out of given arguments. Firs item given to function is used
/// as result type, rest just has to have compatible API.
template < result_like T, result_like... Ts >
T worst_of( T const& item, Ts const&... other )
{
        return accumulate(
            std::array{ item, T{ other }... }, T{ SUCCESS }, [&]( T init, auto item ) -> T {
                    if ( init.get_state() > item.get_state() )
                            return init;
                    return item;
            } );
}

}  // namespace emlabcpp
