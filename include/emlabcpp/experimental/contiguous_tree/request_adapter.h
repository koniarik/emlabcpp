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

#include "emlabcpp/experimental/contiguous_tree/base.h"

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

        [[nodiscard]] either< std::reference_wrapper< const value_type >, error_enum >
        get_value( node_id id ) const
        {
                return get_node( id ).bind_left(
                    [&id]( const node_type& node )
                        -> either< std::reference_wrapper< const value_type >, error_enum > {
                            const value_type* val_ptr = node.get_value();
                            if ( val_ptr == nullptr )
                                    return error_enum::WRONG_TYPE;
                            return std::ref( *val_ptr );
                    } );
        }

        [[nodiscard]] either< node_id, error_enum >
        get_child( node_id nid, const std::variant< key_type, child_id >& id_var ) const
        {
                auto get_object_child = []( const_object_handle&                      h,
                                            const std::variant< key_type, child_id >& key ) {
                        return visit(
                            [&h]( const auto& k ) {
                                    return h.get_child( k );
                            },
                            key );
                };

                auto get_array_child = []( const_array_handle&                       h,
                                           const std::variant< key_type, child_id >& key )
                    -> std::optional< node_id > {
                        const child_id* const id_ptr = std::get_if< child_id >( &key );
                        if ( id_ptr == nullptr )
                                return std::nullopt;
                        return h.get_child( *id_ptr );
                };

                return get_containers( nid )
                    .bind_left(
                        [&id_var, &nid, &get_object_child, &get_array_child](
                            std::variant< const_object_handle, const_array_handle > var )
                            -> either< std::optional< node_id >, error_enum > {
                                const_object_handle* oh_ptr = std::get_if< 0 >( &var );
                                const_array_handle*  ah_ptr = std::get_if< 1 >( &var );

                                if ( oh_ptr != nullptr )
                                        return get_object_child( *oh_ptr, id_var );
                                else if ( ah_ptr != nullptr )
                                        return get_array_child( *ah_ptr, id_var );
                                else
                                        return error_enum::WRONG_TYPE;
                        } )
                    .bind_left(
                        [&nid, &id_var](
                            std::optional< node_id > opt_nid ) -> either< node_id, error_enum > {
                                if ( opt_nid )
                                        return *opt_nid;
                                return error_enum::CHILD_MISSING;
                        } );
        }

        [[nodiscard]] either< child_id, error_enum > get_child_count( node_id nid ) const
        {
                return get_containers( nid ).convert_left(
                    []( const std::variant< const_object_handle, const_array_handle > var ) {
                            return visit(
                                []( const auto& handle ) {
                                        return std::size( handle );
                                },
                                var );
                    } );
        }

        [[nodiscard]] either< key_type, error_enum > get_key( node_id nid, child_id chid ) const
        {
                return get_object_handle( nid ).bind_left(
                    [&nid, &chid]( const_object_handle oh ) -> either< key_type, error_enum > {
                            const key_type* key_ptr = oh.get_key( chid );

                            if ( key_ptr == nullptr )
                                    return error_enum::CHILD_MISSING;
                            return *key_ptr;
                    } );
        }

        [[nodiscard]] either< type_enum, error_enum > get_type( node_id nid ) const
        {
                return get_node( nid ).convert_left( []( const node_type& node ) {
                        return node.get_type();
                } );
        }

        [[nodiscard]] either< node_id, error_enum > insert(
            node_id                                                      parent,
            const key_type&                                              key,
            const std::variant< value_type, contiguous_container_type >& val )
        {
                return get_object_handle( parent ).bind_left(
                    [this, &val, &key]( object_handle oh ) {
                            return construct_node( val ).convert_left( [&key, &oh]( node_id nid ) {
                                    oh.set( key, nid );
                                    return nid;
                            } );
                    } );
        }

        [[nodiscard]] either< node_id, error_enum >
        insert( node_id parent, const std::variant< value_type, contiguous_container_type >& val )
        {
                return get_array_handle( parent ).bind_left( [this, &val]( array_handle ah ) {
                        return construct_node( val ).convert_left( [&ah]( node_id nid ) {
                                ah.append( nid );
                                return nid;
                        } );
                } );
        }

private:
        [[nodiscard]] either< node_id, error_enum >
        construct_node( const std::variant< value_type, contiguous_container_type >& var )
        {
                std::optional< node_id > opt_nid = match(
                    var,
                    [this]( const value_type& val ) {
                            return tree_.make_value_node( val );
                    },
                    [this]( const contiguous_container_type type ) -> std::optional< node_id > {
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

        [[nodiscard]] either< std::reference_wrapper< const node_type >, error_enum >
        get_node( node_id nid ) const
        {
                return get_node_impl< const node_type >( this, nid );
        }

        [[nodiscard]] either< std::reference_wrapper< node_type >, error_enum >
        get_node( node_id nid )
        {
                return get_node_impl< node_type >( this, nid );
        }

        template < typename Node, typename Self >
        [[nodiscard]] static either< std::reference_wrapper< Node >, error_enum >
        get_node_impl( Self* self, node_id nid )
        {
                Node* node_ptr = self->tree_.get_node( nid );

                if ( node_ptr == nullptr )
                        return error_enum::MISSING_NODE;

                return std::ref( *node_ptr );
        }

        [[nodiscard]] either< std::variant< const_object_handle, const_array_handle >, error_enum >
        get_containers( node_id nid ) const
        {
                return get_containers_impl< const_object_handle, const_array_handle >( this, nid );
        }

        [[nodiscard]] either< std::variant< object_handle, array_handle >, error_enum >
        get_containers( node_id nid )
        {
                return get_containers_impl< object_handle, array_handle >( this, nid );
        }

        template < typename OHandle, typename AHandle, typename Self >
        [[nodiscard]] static either< std::variant< OHandle, AHandle >, error_enum >
        get_containers_impl( Self* self, node_id nid )
        {
                using var_type = std::variant< OHandle, AHandle >;
                return self->get_node( nid ).bind_left(
                    [&nid]( auto& node_wrapper ) -> either< var_type, error_enum > {
                            auto& node = node_wrapper.get();
                            std::variant<
                                std::reference_wrapper< const value_type >,
                                OHandle,
                                AHandle >
                                cont = node.get_container_handle();

                            auto* ah_ptr = std::get_if< AHandle >( &cont );
                            if ( ah_ptr != nullptr )
                                    return var_type{ *ah_ptr };
                            auto* oh_ptr = std::get_if< OHandle >( &cont );
                            if ( oh_ptr != nullptr )
                                    return var_type{ *oh_ptr };
                            return error_enum::WRONG_TYPE;
                    } );
        }

        [[nodiscard]] either< const_object_handle, error_enum >
        get_object_handle( node_id nid ) const
        {
                return get_handle_impl< const_object_handle >( this, nid );
        };

        [[nodiscard]] either< object_handle, error_enum > get_object_handle( node_id nid )
        {
                return get_handle_impl< object_handle >( this, nid );
        };

        [[nodiscard]] either< const_array_handle, error_enum > get_array_handle( node_id nid ) const
        {
                return get_handle_impl< const_array_handle >( this, nid );
        };

        [[nodiscard]] either< array_handle, error_enum > get_array_handle( node_id nid )
        {
                return get_handle_impl< array_handle >( this, nid );
        };

        template < typename Handle, typename Self >
        [[nodiscard]] static either< Handle, error_enum > get_handle_impl( Self* self, node_id nid )
        {
                return self->get_containers( nid ).bind_left(
                    [&nid]( auto& var ) -> either< Handle, error_enum > {
                            auto* h_ptr = std::get_if< Handle >( &var );
                            if ( h_ptr != nullptr )
                                    return *h_ptr;
                            return error_enum::WRONG_TYPE;
                    } );
        }

        Tree& tree_;
};

}  // namespace emlabcpp
