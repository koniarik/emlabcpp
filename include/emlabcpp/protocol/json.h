///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#pragma once

#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

#ifdef EMLABCPP_USE_DEMANGLING
#include <cxxabi.h>
#endif

#include "emlabcpp/algorithm.h"
#include "emlabcpp/enum.h"
#include "emlabcpp/protocol/traits.h"

#ifdef EMLABCPP_USE_NLOHMANN_JSON

namespace emlabcpp::protocol
{

template < convertible >
struct traits_json_serializer;

template < convertible >
struct backup_traits_json_serializer;

struct traits_json_serializer_base
{
        static void add_extra( nlohmann::json& )
        {
        }
};

template < typename D, typename ProtoSer >
struct proto_traits_adl_serializer
{
        using traits   = traits_for< D >;
        using prot_ser = ProtoSer;

        static void to_json( nlohmann::json& j, const traits& )
        {
                j["type"] = prot_ser::type_name;
#ifdef EMLABCPP_USE_DEMANGLING
                j["pretty_name"] = prot_ser::get_name();
#endif
                j["raw_name"] = typeid( D ).name();
                j["max_size"] = traits::max_size;
                j["min_size"] = traits::min_size;
                prot_ser::add_extra( j );
        }

        static traits from_json( const nlohmann::json& )
        {
                return {};
        }
};

}  // namespace emlabcpp::protocol

template < emlabcpp::protocol::convertible D >
struct nlohmann::adl_serializer< emlabcpp::protocol::proto_traits< D > >
  : emlabcpp::protocol::
        proto_traits_adl_serializer< D, emlabcpp::protocol::traits_json_serializer< D > >
{
};

template < emlabcpp::protocol::convertible D >
struct nlohmann::adl_serializer< emlabcpp::protocol::backup_proto_traits< D > >
  : emlabcpp::protocol::
        proto_traits_adl_serializer< D, emlabcpp::protocol::backup_traits_json_serializer< D > >
{
};

namespace emlabcpp::protocol
{

template <>
struct traits_json_serializer< uint8_t > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "uint8_t";

        static std::string get_name()
        {
                return std::string{ type_name };
        }
};

template <>
struct traits_json_serializer< uint16_t > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "uint16_t";

        static std::string get_name()
        {
                return std::string{ type_name };
        }
};

template <>
struct traits_json_serializer< uint32_t > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "uint32_t";

        static std::string get_name()
        {
                return std::string{ type_name };
        }
};

template <>
struct traits_json_serializer< uint64_t > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "uint64_t";

        static std::string get_name()
        {
                return std::string{ type_name };
        }
};

template <>
struct traits_json_serializer< int8_t > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "int8_t";

        static std::string get_name()
        {
                return std::string{ type_name };
        }
};

template <>
struct traits_json_serializer< int16_t > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "int16_t";

        static std::string get_name()
        {
                return std::string{ type_name };
        }
};

template <>
struct traits_json_serializer< int32_t > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "int32_t";

        static std::string get_name()
        {
                return std::string{ type_name };
        }
};

template <>
struct traits_json_serializer< int64_t > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "int64_t";

        static std::string get_name()
        {
                return std::string{ type_name };
        }
};

template <>
struct traits_json_serializer< float > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "float";

        static std::string get_name()
        {
                return std::string{ type_name };
        }
};

template < convertible T >
requires( std::is_enum_v< T > )
struct traits_json_serializer< T > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "enum";

        static std::string get_name()
        {
                return pretty_type_name< T >();
        }

        static void add_extra( nlohmann::json& j )
        {
                j["sub_type"] = traits_for< std::underlying_type_t< T > >{};
#ifdef EMLABCPP_USE_MAGIC_ENUM
                j["values"] = magic_enum::enum_names< T >();
#endif
        }
};

template < convertible D, std::size_t N >
struct traits_json_serializer< std::array< D, N > > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "array";
        using sub_ser                               = traits_json_serializer< D >;

        static std::string get_name()
        {
                return sub_ser::get_name() + "[" + std::to_string( N ) + "]";
        }

        static void add_extra( nlohmann::json& j )
        {
                j["sub_type"]   = traits_for< D >{};
                j["item_count"] = N;
        }
};

template < convertible... Ds >
struct traits_json_serializer< std::tuple< Ds... > > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "tuple";

        static std::string get_name()
        {
                std::vector< std::string > names{ traits_json_serializer< Ds >::get_name()... };
                return "(" + joined( names, std::string{ "," } ) + ")";
        }

        static void add_extra( nlohmann::json& j )
        {
                if constexpr ( sizeof...( Ds ) != 0 ) {
                        j["sub_types"] = map_f_to_a(
                            std::tuple< Ds... >{}, [&]< typename D >( D ) -> nlohmann::json {
                                    return traits_for< D >{};
                            } );
                }
        }
};

template < convertible... Ds >
struct traits_json_serializer< std::variant< Ds... > > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "variant";

        using traits = traits_for< std::variant< Ds... > >;

        static std::string get_name()
        {
                std::vector< std::string > names{ traits_json_serializer< Ds >::get_name()... };
                return traits_json_serializer< typename traits::id_type >::get_name() + ",{" +
                       joined( names, std::string{ "|" } ) + "}";
        }

        static void add_extra( nlohmann::json& j )
        {
                j["counter_type"] = typename traits::id_traits{};
                j["sub_types"] =
                    map_f_to_a( std::tuple< Ds... >{}, [&]< typename D >( D ) -> nlohmann::json {
                            return traits_for< D >{};
                    } );
        }
};

template < std::size_t N >
struct traits_json_serializer< std::bitset< N > > : traits_json_serializer_base
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
struct traits_json_serializer< sizeless_message< N > > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "sizeless_message";
};

template < std::size_t N >
struct traits_json_serializer< message< N > > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "message";
};

template < convertible D, auto Offset >
struct traits_json_serializer< value_offset< D, Offset > > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "offset";

        static std::string get_name()
        {
                return traits_json_serializer< D >::get_string() + " + " + std::to_string( Offset );
        }

        static void add_extra( nlohmann::json& j )
        {
                j["sub_type"] = traits_for< D >{};
                j["offset"]   = Offset;
        }
};

template < quantity_derived D >
requires( convertible< D > )
struct traits_json_serializer< D > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "quantity";

        static std::string get_name()
        {
                return traits_json_serializer< typename D::value_type >::get_name();
        }

        static void add_extra( nlohmann::json& j )
        {
                j["sub_type"] = traits_for< typename D::value_type >{};
                if ( D::get_unit() != "" )
                        j["unit"] = D::get_unit();
        }
};

template < convertible D, D Min, D Max >
struct traits_json_serializer< bounded< D, Min, Max > > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "bounded";

        static std::string get_name()
        {
                return ">" + traits_json_serializer< D >::get_name() + "<";
        }

        static void add_extra( nlohmann::json& j )
        {
                j["sub_type"] = traits_for< D >{};
                j["min"]      = Min;
                j["max"]      = Max;
        }
};

template < convertible CounterType, convertible D >
struct traits_json_serializer< sized_buffer< CounterType, D > > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "sized_buffer";

        static std::string get_name()
        {
                return std::string{ type_name };
        }

        static void add_extra( nlohmann::json& j )
        {
                j["counter_type"] = traits_for< CounterType >{};
                j["sub_type"]     = traits_for< D >{};
        }
};

template < auto V >
struct traits_json_serializer< tag< V > > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "tag";

        static std::string get_name()
        {
                if constexpr ( std::is_enum_v< decltype( V ) > )
                        return "tag<" + std::string{ convert_enum( V ) } + ">";
                else
                        return "tag<" + std::to_string( V ) + ">";
        }

        static void add_extra( nlohmann::json& j )
        {
                j["sub_type"] = typename traits_for< tag< V > >::sub_traits{};
                j["value"]    = V;
#ifdef EMLABCPP_USE_MAGIC_ENUM
                if constexpr ( std::is_enum_v< decltype( V ) > )
                        j["enumerator"] = magic_enum::enum_name( V );
#endif
        }
};

template < convertible... Ds >
struct traits_json_serializer< group< Ds... > > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "group";

        static std::string get_name()
        {
                std::vector< std::string > names{ traits_json_serializer< Ds >::get_name()... };
                return "{" + joined( names, std::string{ "|" } ) + "}";
        }

        static void add_extra( nlohmann::json& j )
        {
                j["sub_types"] =
                    map_f_to_a( std::tuple< Ds... >{}, [&]< typename D >( D ) -> nlohmann::json {
                            return traits_for< D >{};
                    } );
        }
};

template < convertible... Ds >
struct traits_json_serializer< tag_group< Ds... > >
  : traits_json_serializer< typename traits_for< tag_group< Ds... > >::sub_type >
{
        static constexpr std::string_view type_name = "group";
};

template < std::endian Endianess, convertible D >
struct traits_json_serializer< endianess_wrapper< Endianess, D > > : traits_json_serializer< D >
{
};

template < std::derived_from< converter_def_type_base > D >
requires( convertible< D > )
struct traits_json_serializer< D > : traits_json_serializer< typename D::def_type >
{
        static std::string get_name()
        {
                return pretty_type_name< D >();
        }
};

template <>
struct traits_json_serializer< mark > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "mark";

        static std::string get_name()
        {
                return std::string{ type_name };
        }
};

template <>
struct traits_json_serializer< error_record > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "error_record";

        static std::string get_name()
        {
                return std::string{ type_name };
        }

        using traits = traits_for< error_record >;

        static void add_extra( nlohmann::json& j )
        {
                j["mark_type"]   = traits_for< typename traits::mark_type >{};
                j["offset_type"] = traits_for< typename traits::offset_type >{};
        }
};

template < typename T, std::size_t N >
struct traits_json_serializer< static_vector< T, N > > : traits_json_serializer_base
{
        static constexpr std::string_view type_name = "static_vector";
        using sub_ser                               = traits_json_serializer< T >;

        static std::string get_name()
        {
                return sub_ser::get_name() + "[" + std::to_string( N ) + "]";
                ;
        }

        static void add_extra( nlohmann::json& j )
        {
                j["counter_type"] =
                    traits_for< typename traits_for< static_vector< T, N > >::counter_type >{};
                j["sub_type"] = traits_for< T >{};
        }
};

template < convertible D >
struct backup_traits_json_serializer
  : traits_json_serializer< typename traits_for< D >::tuple_type >
{

        static std::string get_name()
        {
                return pretty_type_name< D >();
        }
};

}  // namespace emlabcpp::protocol

#endif
