#include "emlabcpp/algorithm.h"
#include "emlabcpp/protocol/base.h"

#pragma once

namespace emlabcpp
{

template < auto Key, typename DefType >
struct protocol_reg
{
        using key_type                = decltype( Key );
        static constexpr key_type key = Key;

        using def_type = DefType;

        using item_decl  = protocol_item_decl< def_type >;
        using value_type = typename item_decl::value_type;

        value_type value;
};

template < typename... Regs >
class protocol_register_map
{
        static_assert( are_same_v< typename Regs::key_type... > );

public:
        using registers_tpl = std::tuple< Regs... >;
        using key_type      = typename std::tuple_element_t< 0, registers_tpl >::key_type;

        static constexpr std::size_t max_value_size = std::max( { Regs::item_decl::max_size... } );
        using message_type                          = protocol_message< max_value_size >;

private:
        registers_tpl registers_;

        static constexpr std::size_t get_reg_index( key_type k )
        {
                std::size_t res = std::tuple_size_v< registers_tpl >;
                until_index< sizeof...( Regs ) >( [&]< std::size_t i >() {
                        if ( std::tuple_element_t< i, registers_tpl >::key == k ) {
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
        static constexpr bool contains_key = key_index< Key > != std::tuple_size_v< registers_tpl >;

        template < key_type Key >
        using reg_type = std::tuple_element_t< key_index< Key >, registers_tpl >;

        template < key_type Key >
        using reg_value_type = typename reg_type< Key >::value_type;

        template < key_type Key >
        using item_decl = typename reg_type< Key >::item_decl;

        template < key_type Key >
        using reg_def_type = typename reg_type< Key >::def_type;

        template < key_type Key >
        reg_value_type< Key > get_val() const
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
};

}  // namespace emlabcpp
