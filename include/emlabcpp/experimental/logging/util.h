#include "emlabcpp/experimental/logging.h"

#pragma once

#define EMLACBPP_VA_ARGS_SIZE( ... ) std::tuple_size_v< decltype( std::make_tuple( __VA_ARGS__ ) ) >

namespace emlabcpp::logging
{

template < std::size_t N, std::size_t... Ids >
auto generate_vars_log_tuple(
    std::index_sequence< Ids... >,
    const std::array< std::string_view, N >& names,
    const auto&                              args )
{
        static_assert( 4 * N == sizeof...( Ids ) );

        auto f = [&]< std::size_t i > {
                if constexpr ( i % 4 == 0 ) {
                        return names[i / 4];
                } else if constexpr ( i % 4 == 1 ) {
                        return std::string_view{ " = " };
                } else if constexpr ( i % 4 == 2 ) {
                        return std::get< i / 4 >( args );
                } else {
                        return std::string_view{ ";\t" };
                }
        };

        return std::make_tuple( f.template operator()< Ids >()... );
}

template < std::size_t N >
std::array< std::string_view, N > expand_var_names( std::string_view sv )
{
        std::array< std::string_view, N > names;
        for ( std::size_t i = 0; i < N; i++ ) {
                std::size_t p = sv.find( "," );
                names[i]      = sv.substr( 0, p );
                sv            = sv.substr( p + 1 );
        }
        return names;
}

}  // namespace emlabcpp::logging

#define EMLABCPP_INFO_LOG_VARS( ... )                                                  \
        {                                                                              \
                static constexpr std::size_t N = EMLACBPP_VA_ARGS_SIZE( __VA_ARGS__ ); \
                std::apply(                                                            \
                    [&]( const auto&... items ) {                                      \
                            EMLABCPP_LOG_IMPL(                                         \
                                emlabcpp::INFO_LOGGER, __FILE__, __LINE__, items... ); \
                    },                                                                 \
                    emlabcpp::logging::generate_vars_log_tuple(                        \
                        std::make_index_sequence< 4 * N >{},                           \
                        emlabcpp::logging::expand_var_names< N >( #__VA_ARGS__ ),      \
                        std::forward_as_tuple( __VA_ARGS__ ) ) );                      \
        }                                                                              \
        while ( false )
