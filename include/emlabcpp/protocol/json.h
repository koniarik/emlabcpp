#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

#ifdef EMLABCPP_USE_MAGIC_ENUM
#include <magic_enum.hpp>
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

}  // namespace emlabcpp

template < emlabcpp::protocol_declarable D >
struct nlohmann::adl_serializer< emlabcpp::protocol_decl< D > >
{
        using decl     = emlabcpp::protocol_decl< D >;
        using prot_ser = emlabcpp::protocol_json_serializer< D >;

        static void to_json( nlohmann::json& j, const decl& )
        {
                j["type"]     = prot_ser::name;
                j["max_size"] = decl::max_size;
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
        static constexpr std::string_view name = "uint8_t";
};
template <>
struct protocol_json_serializer< uint16_t > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "uint16_t";
};
template <>
struct protocol_json_serializer< uint32_t > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "uint32_t";
};
template <>
struct protocol_json_serializer< uint64_t > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "uint64_t";
};
template <>
struct protocol_json_serializer< int8_t > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "int8_t";
};
template <>
struct protocol_json_serializer< int16_t > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "int16_t";
};
template <>
struct protocol_json_serializer< int32_t > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "int32_t";
};
template <>
struct protocol_json_serializer< int64_t > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "int64_t";
};
template < protocol_declarable T >
requires( std::is_enum_v< T > ) struct protocol_json_serializer< T > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "enum";

        static void add_extra( nlohmann::json& j )
        {
#ifdef EMLABCPP_USE_MAGIC_ENUM
                j["values"] = magic_enum::enum_names< T >();
#else
                ignore( j );
#endif
        }
};

template < protocol_declarable D, std::size_t N >
struct protocol_json_serializer< std::array< D, N > > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "array";

        static void add_extra( nlohmann::json& j )
        {
                j["sub_type"]   = protocol_decl< D >{};
                j["item_count"] = N;
        }
};

template < protocol_declarable... Ds >
struct protocol_json_serializer< std::tuple< Ds... > > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "tuple";

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
        static constexpr std::string_view name = "variant";

        static void add_extra( nlohmann::json& j )
        {
                j["sub_types"] =
                    map_f_to_a( std::tuple< Ds... >{}, [&]< typename D >( D ) -> nlohmann::json {
                            return protocol_decl< D >{};
                    } );
        }
};

template < std::size_t N >
struct protocol_json_serializer< std::bitset< N > > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "bitset";

        static void add_extra( nlohmann::json& j )
        {
                j["bits"] = N;
        }
};

template < std::size_t N >
struct protocol_json_serializer< protocol_sizeless_message< N > > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "sizeless_message";
};

template < protocol_declarable D, auto Offset >
struct protocol_json_serializer< protocol_offset< D, Offset > > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "offset";

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
        static constexpr std::string_view name = "quantity";

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
        static constexpr std::string_view name = "bounded";

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
        static constexpr std::string_view name = "sized_buffer";

        static void add_extra( nlohmann::json& j )
        {
                j["counter_type"] = protocol_decl< CounterType >{};
                j["sub_type"]     = protocol_decl< D >{};
        }
};

template < auto V >
struct protocol_json_serializer< tag< V > > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "tag";

        static void add_extra( nlohmann::json& j )
        {
                j["sub_type"] = typename protocol_decl< tag< V > >::sub_decl{};
#ifdef EMLABCPP_USE_MAGIC_ENUM
                if constexpr ( std::is_enum_v< decltype( V ) > ) {
                        j["value"] = magic_enum::enum_name( V );
                } else {
                        j["value"] = V;
                }
#else
                j["value"] = V;
#endif
        }
};

template < protocol_declarable... Ds >
struct protocol_json_serializer< protocol_group< Ds... > > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "group";

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
};

template <>
struct protocol_json_serializer< protocol_mark > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "mark";
};

template <>
struct protocol_json_serializer< protocol_error_record > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "error_record";

        using decl = protocol_decl< protocol_error_record >;

        static void add_extra( nlohmann::json& j )
        {
                j["mark_type"]   = typename decl::mark_decl{};
                j["offset_type"] = typename decl::offset_decl{};
        }
};

template < typename T, std::size_t N >
struct protocol_json_serializer< static_vector< T, N > > : protocol_json_serializer_base
{
        static constexpr std::string_view name = "static_vector";

        static void add_extra( nlohmann::json& j )
        {
                j["counter_type"] = protocol_decl<
                    typename protocol_decl< static_vector< T, N > >::counter_type >{};
                j["sub_type"] = protocol_decl< T >{};
        }
};

}  // namespace emlabcpp

#endif

