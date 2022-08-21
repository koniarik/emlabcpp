// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
#include "emlabcpp/algorithm.h"
#include "emlabcpp/protocol/base.h"
#include "emlabcpp/protocol/decl.h"

#pragma once

namespace emlabcpp::protocol
{

/// Structure that represents definition of one register in the map. It also contains the value
/// itself.
template < auto Key, convertible D >
struct protocol_reg
{
        using def_type                    = D;
        using key_type                    = decltype( Key );
        static constexpr key_type key     = Key;
        using decl                        = protocol_decl< def_type >;
        using value_type                  = typename decl::value_type;
        static constexpr std::size_t size = decl::max_size;

        value_type value;
};

template < typename UnaryCallable, typename Registers >
concept protocol_register_map_void_returning =
    invocable_returning< UnaryCallable, void, std::tuple_element_t< 0, Registers > >;

/// Register map is abstraction to work with registers of external devices. It stores values of
/// serializable types that can be accessed based on key (usually enum representing address of
/// register in the device).You can access the value based on the key itself, both at compile time
/// and at runtime. You can also iterate over the values and there is handler that allows
/// serialization and deserialization of bytes into the values defined in the map. This includes
/// additional information that can be accessed about the map. This can also be used as simple table
/// of configuration values.
template < endianess_enum Endianess, typename... Regs >
class protocol_register_map
{
        static_assert( are_same_v< typename Regs::key_type... > );

public:
        static constexpr endianess_enum endianess = Endianess;
        using registers_tuple                     = std::tuple< Regs... >;
        using key_type = typename std::tuple_element_t< 0, registers_tuple >::key_type;

        static constexpr std::size_t registers_count = sizeof...( Regs );
        using register_index = bounded< std::size_t, 0, registers_count - 1 >;

        static constexpr std::size_t max_value_size = std::max( { Regs::decl::max_size... } );
        using message_type                          = protocol_message< max_value_size >;

private:
        registers_tuple registers_;

        static constexpr std::size_t get_reg_index( key_type k )
        {
                std::size_t res = std::tuple_size_v< registers_tuple >;
                until_index< sizeof...( Regs ) >( [&]< std::size_t i >() {
                        if ( std::tuple_element_t< i, registers_tuple >::key == k ) {
                                res = i;
                                return true;
                        }
                        return false;
                } );
                return res;
        }

public:
        template < key_type Key >
        static constexpr std::size_t key_index = get_reg_index( Key );

        template < key_type Key >
        static constexpr bool contains_key =
            key_index< Key > != std::tuple_size_v< registers_tuple >;

        template < key_type Key >
        using reg_type = std::tuple_element_t< key_index< Key >, registers_tuple >;

        template < key_type Key >
        using reg_value_type = typename reg_type< Key >::value_type;

        template < key_type Key >
        using decl = typename reg_type< Key >::decl;

        template < key_type Key >
        using reg_def_type = typename reg_type< Key >::def_type;

        protocol_register_map() = default;
        protocol_register_map( typename Regs::value_type... args )
          : registers_( Regs{ args }... )
        {
        }

        [[nodiscard]] constexpr bool contains( key_type key ) const
        {
                return get_reg_index( key ) != std::tuple_size_v< registers_tuple >;
        }

        template < key_type Key >
        [[nodiscard]] reg_value_type< Key > get_val() const
        {
                static_assert( contains_key< Key > );
                const reg_type< Key >& reg = std::get< key_index< Key > >( registers_ );
                return reg.value;
        }

        template < key_type Key >
        void set_val( reg_value_type< Key > val )
        {
                static_assert( contains_key< Key > );
                reg_type< Key >& reg = std::get< key_index< Key > >( registers_ );
                reg.value            = val;
        }

        static constexpr std::size_t register_size( register_index i )
        {
                return select_index( i, [&]< std::size_t j >() {
                        return std::tuple_element_t< j, registers_tuple >::size;
                } );
        }

        static constexpr key_type register_key( register_index i )
        {
                return select_index( i, [&]< std::size_t j >() {
                        return std::tuple_element_t< j, registers_tuple >::key;
                } );
        }

        template < typename UnaryCallable >
        constexpr void setup_register( key_type key, UnaryCallable&& f )
        {
                with_register( key, [&]< typename reg_type >( const reg_type& ) {
                        std::get< reg_type >( registers_ ).value =
                            f.template operator()< reg_type >();
                } );
        }

        template < typename UnaryCallable >
        requires(
            !protocol_register_map_void_returning<
                UnaryCallable,
                registers_tuple > ) constexpr auto with_register( key_type key, UnaryCallable&& f )
            const
        {
                using ret_type = decltype( f( std::get< 0 >( registers_ ) ) );
                ret_type res;
                with_register( key, [&]( const auto& reg ) {
                        res = f( reg );
                } );
                return res;
        }

        template < typename UnaryCallable >
        requires(
            protocol_register_map_void_returning<
                UnaryCallable,
                registers_tuple > ) constexpr void with_register( key_type key, UnaryCallable&& f )
            const
        {
                until_index< registers_count >( [&]< std::size_t j >() {
                        using reg_type = std::tuple_element_t< j, registers_tuple >;
                        if ( reg_type::key != key ) {
                                return false;
                        }
                        f( std::get< j >( registers_ ) );
                        return true;
                } );
        }
};

template < typename Map, typename UnaryCallable >
void protocol_for_each_register( const Map& m, UnaryCallable&& f )
{
        for_each_index< Map::registers_count >( [&]< std::size_t i >() {
                static constexpr auto key = Map::register_key( bounded_constant< i > );
                f.template            operator()< key >( m.template get_val< key >() );
        } );
}

}  // namespace emlabcpp::protocol
