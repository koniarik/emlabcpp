#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

#ifdef EMLABCPP_USE_MAGIC_ENUM
#include <magic_enum.hpp>
#endif

#ifdef EMLABCPP_USE_DEMANGLING
#include <cxxabi.h>
#endif

#include "emlabcpp/algorithm.h"
#include "emlabcpp/protocol/decl.h"

#pragma once

#ifdef EMLABCPP_USE_NLOHMANN_JSON

namespace emlabcpp
{

template < protocol_declarable >
struct protocol_json_serializer;

struct protocol_json_serializer_base
{
        static void add_extra( nlohmann::json& )
        {
        }
};

}  /// namespace emlabcpp

template < emlabcpp::protocol_declarable D >
struct nlohmann::adl_serializer< emlabcpp::protocol_decl< D > >
{
        using decl     = emlabcpp::protocol_decl< D >;
        using prot_ser = emlabcpp::protocol_json_serializer< D >;

        static void to_json( nlohmann::json& j, const decl& )
        {
                j["type"] = prot_ser::type_name;
#ifdef EMLABCPP_USE_DEMANGLING
                j["pretty_name"] = prot_ser::get_name();
#endif
                j["raw_name"] = typeid( D ).name();
                j["max_size"] = decl::max_size;
                j["min_size"] = decl::min_size;
                prot_ser::add_extra( j );
        }

        static decl from_json( const nlohmann::json& )
        {
                return {};
        }
};

namespace emlabcpp
{

template <>
struct protocol_json_serializer< uint8_t > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "uint8_t";
        static std::string                get_name()
        {
                return std::string{ type_name };
        }
};
template <>
struct protocol_json_serializer< uint16_t > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "uint16_t";
        static std::string                get_name()
        {
                return std::string{ type_name };
        }
};
template <>
struct protocol_json_serializer< uint32_t > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "uint32_t";
        static std::string                get_name()
        {
                return std::string{ type_name };
        }
};
template <>
struct protocol_json_serializer< uint64_t > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "uint64_t";
        static std::string                get_name()
        {
                return std::string{ type_name };
        }
};
template <>
struct protocol_json_serializer< int8_t > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "int8_t";
        static std::string                get_name()
        {
                return std::string{ type_name };
        }
};
template <>
struct protocol_json_serializer< int16_t > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "int16_t";
        static std::string                get_name()
        {
                return std::string{ type_name };
        }
};
template <>
struct protocol_json_serializer< int32_t > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "int32_t";
        static std::string                get_name()
        {
                return std::string{ type_name };
        }
};
template <>
struct protocol_json_serializer< int64_t > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "int64_t";
        static std::string                get_name()
        {
                return std::string{ type_name };
        }
};
template < protocol_declarable T >
requires( std::is_enum_v< T > ) struct protocol_json_serializer< T > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "enum";
        static std::string                get_name()
        {
                return pretty_type_name< T >();
        }

        static void add_extra( nlohmann::json& j )
        {
                j["sub_type"] = protocol_decl< std::underlying_type_t< T > >{};
#ifdef EMLABCPP_USE_MAGIC_ENUM
                j["values"] = magic_enum::enum_names< T >();
#else
                std::ignore = j;
#endif
        }
};

template < protocol_declarable D, std::size_t N >
struct protocol_json_serializer< std::array< D, N > > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "array";
        using sub_ser                               = protocol_json_serializer< D >;

        static std::string get_name()
        {
                return sub_ser::get_name() + "[" + std::to_string( N ) + "]";
        }

        static void add_extra( nlohmann::json& j )
        {
                j["sub_type"]   = protocol_decl< D >{};
                j["item_count"] = N;
        }
};

template < protocol_declarable... Ds >
struct protocol_json_serializer< std::tuple< Ds... > > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "tuple";

        static std::string get_name()
        {
                std::vector< std::string > names{ protocol_json_serializer< Ds >::get_name()... };
                return "(" + joined( names, std::string{ "," } ) + ")";
        }

        static void add_extra( nlohmann::json& j )
        {
                j["sub_types"] =
                    map_f_to_a( std::tuple< Ds... >{}, [&]< typename D >( D ) -> nlohmann::json {
                            return protocol_decl< D >{};
                    } );
        }
};

template < protocol_declarable... Ds >
struct protocol_json_serializer< std::variant< Ds... > > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "variant";

        using decl = protocol_decl< std::variant< Ds... > >;

        static std::string get_name()
        {
                std::vector< std::string > names{ protocol_json_serializer< Ds >::get_name()... };
                return protocol_json_serializer< typename decl::id_type >::get_name() + ",{" +
                       joined( names, std::string{ "|" } ) + "}";
        }

        static void add_extra( nlohmann::json& j )
        {
                j["counter_type"] = typename decl::id_decl{};
                j["sub_types"] =
                    map_f_to_a( std::tuple< Ds... >{}, [&]< typename D >( D ) -> nlohmann::json {
                            return protocol_decl< D >{};
                    } );
        }
};

template < std::size_t N >
struct protocol_json_serializer< std::bitset< N > > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "bitset";

        static std::string get_name()
        {
                return "bitset[" + std::to_string( N ) + "]";
        }

        static void add_extra( nlohmann::json& j )
        {
                j["bits"] = N;
        }
};

template < std::size_t N >
struct protocol_json_serializer< protocol_sizeless_message< N > > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "sizeless_message";
};

template < protocol_declarable D, auto Offset >
struct protocol_json_serializer< protocol_offset< D, Offset > > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "offset";

        static std::string get_name()
        {
                return protocol_json_serializer< D >::get_string() + " + " +
                       std::to_string( Offset );
        }

        static void add_extra( nlohmann::json& j )
        {
                j["sub_type"] = protocol_decl< D >{};
                j["offset"]   = Offset;
        }
};

template < quantity_derived D >
requires( protocol_declarable< D > ) struct protocol_json_serializer< D >
  : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "quantity";

        static std::string get_name()
        {
                return protocol_json_serializer< typename D::value_type >::get_name();
        }

        static void add_extra( nlohmann::json& j )
        {
                j["sub_type"] = protocol_decl< typename D::value_type >{};
                if ( D::get_unit() != "" ) {
                        j["unit"] = D::get_unit();
                }
        }
};

template < protocol_declarable D, D Min, D Max >
struct protocol_json_serializer< bounded< D, Min, Max > > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "bounded";

        static std::string get_name()
        {
                return ">" + protocol_json_serializer< D >::get_name() + "<";
        }

        static void add_extra( nlohmann::json& j )
        {
                j["sub_type"] = protocol_decl< D >{};
                j["min"]      = Min;
                j["max"]      = Max;
        }
};

template < protocol_declarable CounterType, protocol_declarable D >
struct protocol_json_serializer< protocol_sized_buffer< CounterType, D > >
  : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "sized_buffer";

        static std::string get_name()
        {
                return std::string{ type_name };
        }

        static void add_extra( nlohmann::json& j )
        {
                j["counter_type"] = protocol_decl< CounterType >{};
                j["sub_type"]     = protocol_decl< D >{};
        }
};

template < auto V >
struct protocol_json_serializer< tag< V > > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "tag";

        static std::string get_name()
        {
#ifdef EMLABCPP_USE_MAGIC_ENUM
                if constexpr ( std::is_enum_v< decltype( V ) > ) {
                        return "tag<" + std::string{ magic_enum::enum_name( V ) } + ">";
                } else {
                        return "tag<" + std::to_string( V ) + ">";
                }
#else
                return "tag<" + std::to_string( V ) + ">";
#endif
        }

        static void add_extra( nlohmann::json& j )
        {
                j["sub_type"] = typename protocol_decl< tag< V > >::sub_decl{};
                j["value"]    = V;
#ifdef EMLABCPP_USE_MAGIC_ENUM
                if constexpr ( std::is_enum_v< decltype( V ) > ) {
                        j["enumerator"] = magic_enum::enum_name( V );
                }
#endif
        }
};

template < protocol_declarable... Ds >
struct protocol_json_serializer< protocol_group< Ds... > > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "group";

        static std::string get_name()
        {
                std::vector< std::string > names{ protocol_json_serializer< Ds >::get_name()... };
                return "{" + joined( names, std::string{ "|" } ) + "}";
        }

        static void add_extra( nlohmann::json& j )
        {
                j["sub_types"] =
                    map_f_to_a( std::tuple< Ds... >{}, [&]< typename D >( D ) -> nlohmann::json {
                            return protocol_decl< D >{};
                    } );
        }
};

template < protocol_endianess_enum Endianess, protocol_declarable D >
struct protocol_json_serializer< protocol_endianess< Endianess, D > >
  : protocol_json_serializer< D >
{
};

template < std::derived_from< protocol_def_type_base > D >
requires( protocol_declarable< D > ) struct protocol_json_serializer< D >
  : protocol_json_serializer< typename D::def_type >
{
        static std::string get_name()
        {
                return pretty_type_name< D >();
        }
};

template <>
struct protocol_json_serializer< protocol_mark > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "mark";
        static std::string                get_name()
        {
                return std::string{ type_name };
        }
};

template <>
struct protocol_json_serializer< protocol_error_record > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "error_record";

        static std::string get_name()
        {
                return std::string{ type_name };
        }

        using decl = protocol_decl< protocol_error_record >;

        static void add_extra( nlohmann::json& j )
        {
                j["mark_type"]   = protocol_decl< typename decl::mark_type >{};
                j["offset_type"] = protocol_decl< typename decl::offset_type >{};
        }
};

template < typename T, std::size_t N >
struct protocol_json_serializer< static_vector< T, N > > : protocol_json_serializer_base
{
        static constexpr std::string_view type_name = "static_vector";
        using sub_ser                               = protocol_json_serializer< T >;

        static std::string get_name()
        {
                return sub_ser::get_name() + "[" + std::to_string( N ) + "]";
                ;
        }

        static void add_extra( nlohmann::json& j )
        {
                j["counter_type"] = protocol_decl<
                    typename protocol_decl< static_vector< T, N > >::counter_type >{};
                j["sub_type"] = protocol_decl< T >{};
        }
};

}  /// namespace emlabcpp

#endif
