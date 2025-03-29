/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///

#pragma once

#include "./base.h"

#include <variant>

namespace emlabcpp
{

template < typename Tree >
class contiguous_request_adapter
{
public:
        using value_type          = typename Tree::value_type;
        using key_type            = typename Tree::key_type;
        using node_id             = typename Tree::node_id;
        using node_type           = typename Tree::node_type;
        using child_id            = typename Tree::child_id;
        using type_enum           = contiguous_tree_type;
        using error_enum          = contiguous_request_adapter_errors;
        using const_object_handle = typename Tree::const_object_handle;
        using const_array_handle  = typename Tree::const_array_handle;
        using object_handle       = typename Tree::object_handle;
        using array_handle        = typename Tree::array_handle;

        explicit contiguous_request_adapter( Tree& tree )
          : tree_( tree )
        {
        }

        [[nodiscard]] std::variant< std::reference_wrapper< value_type const >, error_enum >
        get_value( node_id id ) const
        {
                auto n = get_node( id );
                if ( auto* e = std::get_if< error_enum >( &n ) )
                        return *e;
                auto&             node    = std::get_if< 0 >( &n )->get();
                value_type const* val_ptr = node.get_value();
                if ( val_ptr == nullptr )
                        return error_enum::WRONG_TYPE;
                return std::ref( *val_ptr );
        }

        [[nodiscard]] std::variant< node_id, error_enum >
        get_child( node_id nid, std::variant< key_type, child_id > const& id_var ) const
        {
                auto get_object_child = []( const_object_handle&                      h,
                                            std::variant< key_type, child_id > const& key ) {
                        return visit(
                            [&h]( auto const& k ) {
                                    return h.get_child( k );
                            },
                            key );
                };

                auto get_array_child = []( const_array_handle&                       h,
                                           std::variant< key_type, child_id > const& key )
                    -> std::optional< node_id > {
                        child_id const* const id_ptr = std::get_if< child_id >( &key );
                        if ( id_ptr == nullptr )
                                return std::nullopt;
                        return h.get_child( *id_ptr );
                };

                auto tmp = get_containers( nid );
                if ( auto* e = std::get_if< error_enum >( &tmp ) )
                        return *e;

                const_object_handle* oh_ptr = std::get_if< 0 >( &tmp );
                const_array_handle*  ah_ptr = std::get_if< 1 >( &tmp );

                std::optional< node_id > res;
                if ( oh_ptr != nullptr )
                        res = get_object_child( *oh_ptr, id_var );
                else if ( ah_ptr != nullptr )
                        res = get_array_child( *ah_ptr, id_var );
                else
                        return error_enum::WRONG_TYPE;
                if ( res )
                        return *res;

                return error_enum::CHILD_MISSING;
        }

        [[nodiscard]] std::variant< child_id, error_enum > get_child_count( node_id nid ) const
        {
                using R = std::variant< child_id, error_enum >;
                return match(
                    get_containers( nid ),
                    []( const_object_handle const& oh ) -> R {
                            return oh.size();
                    },
                    []( const_array_handle const& ah ) -> R {
                            return ah.size();
                    },
                    []( error_enum e ) -> R {
                            return e;
                    } );
        }

        [[nodiscard]] std::variant< key_type, error_enum >
        get_key( node_id nid, child_id chid ) const
        {
                auto tmp = get_object_handle( nid );
                if ( auto* e = std::get_if< error_enum >( &tmp ) )
                        return *e;
                auto&           oh      = *std::get_if< const_object_handle >( &tmp );
                key_type const* key_ptr = oh.get_key( chid );

                if ( key_ptr == nullptr )
                        return error_enum::CHILD_MISSING;
                return *key_ptr;
        }

        [[nodiscard]] std::variant< type_enum, error_enum > get_type( node_id nid ) const
        {
                auto tmp = get_node( nid );
                if ( auto* e = std::get_if< error_enum >( &tmp ) )
                        return *e;
                auto& node = *std::get_if< 0 >( &tmp );
                return node.get().get_type();
        }

        [[nodiscard]] std::variant< node_id, error_enum > insert(
            node_id                                                      parent,
            key_type const&                                              key,
            std::variant< value_type, contiguous_container_type > const& val )
        {
                auto obj = get_object_handle( parent );
                if ( auto* e = std::get_if< error_enum >( &obj ) )
                        return *e;
                auto& oh    = *std::get_if< object_handle >( &obj );
                auto  cnode = construct_node( val );
                if ( auto* e = std::get_if< error_enum >( &cnode ) )
                        return *e;
                auto& nid = *std::get_if< node_id >( &cnode );
                oh.set( key, nid );
                return nid;
        }

        [[nodiscard]] std::variant< node_id, error_enum >
        insert( node_id parent, std::variant< value_type, contiguous_container_type > const& val )
        {
                auto tmp = get_array_handle( parent );
                if ( auto* e = std::get_if< error_enum >( &tmp ) )
                        return *e;
                auto& ah  = *std::get_if< array_handle >( &tmp );
                auto  nid = construct_node( val );
                if ( auto* e = std::get_if< error_enum >( &nid ) )
                        return *e;
                auto& val_nid = *std::get_if< node_id >( &nid );
                ah.append( val_nid );
                return nid;
        }

private:
        [[nodiscard]] std::variant< node_id, error_enum >
        construct_node( std::variant< value_type, contiguous_container_type > const& var )
        {
                std::optional< node_id > opt_nid = match(
                    var,
                    [this]( value_type const& val ) {
                            return tree_.make_value_node( val );
                    },
                    [this]( contiguous_container_type const type ) -> std::optional< node_id > {
                            if ( type == contiguous_container_type::ARRAY ) {
                                    auto opt_res = tree_.make_array_node();
                                    if ( opt_res )
                                            return opt_res->first;
                                    return std::nullopt;

                            } else if ( type == contiguous_container_type::OBJECT ) {
                                    auto opt_res = tree_.make_object_node();
                                    if ( opt_res )
                                            return opt_res->first;
                                    return std::nullopt;
                            } else {
                                    return std::nullopt;
                            }
                    } );
                if ( !opt_nid )
                        return error_enum::FULL;
                return *opt_nid;
        }

        [[nodiscard]] std::variant< std::reference_wrapper< node_type const >, error_enum >
        get_node( node_id nid ) const
        {
                return get_node_impl< node_type const >( this, nid );
        }

        [[nodiscard]] std::variant< std::reference_wrapper< node_type >, error_enum >
        get_node( node_id nid )
        {
                return get_node_impl< node_type >( this, nid );
        }

        template < typename Node, typename Self >
        [[nodiscard]] static std::variant< std::reference_wrapper< Node >, error_enum >
        get_node_impl( Self* self, node_id nid )
        {
                Node* node_ptr = self->tree_.get_node( nid );

                if ( node_ptr == nullptr )
                        return error_enum::MISSING_NODE;

                return std::ref( *node_ptr );
        }

        [[nodiscard]] std::variant< const_object_handle, const_array_handle, error_enum >
        get_containers( node_id nid ) const
        {
                return get_containers_impl< const_object_handle, const_array_handle >( this, nid );
        }

        [[nodiscard]] std::variant< object_handle, array_handle, error_enum >
        get_containers( node_id nid )
        {
                return get_containers_impl< object_handle, array_handle >( this, nid );
        }

        template < typename OHandle, typename AHandle, typename Self >
        [[nodiscard]] static std::variant< OHandle, AHandle, error_enum >
        get_containers_impl( Self* self, node_id nid )
        {
                auto tmp = self->get_node( nid );
                if ( auto* e_ptr = std::get_if< error_enum >( &tmp ) )
                        return *e_ptr;
                auto& node_wrapper = *std::get_if< 0 >( &tmp );
                auto& node         = node_wrapper.get();
                std::variant< std::reference_wrapper< value_type const >, OHandle, AHandle > cont =
                    node.get_container_handle();

                auto* ah_ptr = std::get_if< AHandle >( &cont );
                if ( ah_ptr != nullptr )
                        return *ah_ptr;
                auto* oh_ptr = std::get_if< OHandle >( &cont );
                if ( oh_ptr != nullptr )
                        return *oh_ptr;
                return error_enum::WRONG_TYPE;
        }

        [[nodiscard]] std::variant< const_object_handle, error_enum >
        get_object_handle( node_id nid ) const
        {
                return get_handle_impl< const_object_handle >( this, nid );
        };

        [[nodiscard]] std::variant< object_handle, error_enum > get_object_handle( node_id nid )
        {
                return get_handle_impl< object_handle >( this, nid );
        };

        [[nodiscard]] std::variant< const_array_handle, error_enum >
        get_array_handle( node_id nid ) const
        {
                return get_handle_impl< const_array_handle >( this, nid );
        };

        [[nodiscard]] std::variant< array_handle, error_enum > get_array_handle( node_id nid )
        {
                return get_handle_impl< array_handle >( this, nid );
        };

        template < typename Handle, typename Self >
        [[nodiscard]] static std::variant< Handle, error_enum >
        get_handle_impl( Self* self, node_id nid )
        {
                auto var = self->get_containers( nid );
                if ( auto* e_ptr = std::get_if< error_enum >( &var ) )
                        return *e_ptr;

                auto* h_ptr = std::get_if< Handle >( &var );
                if ( h_ptr != nullptr )
                        return *h_ptr;
                return error_enum::WRONG_TYPE;
        }

        Tree& tree_;
};

}  // namespace emlabcpp
