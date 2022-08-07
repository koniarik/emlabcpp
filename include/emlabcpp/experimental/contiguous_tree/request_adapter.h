#include "emlabcpp/experimental/contiguous_tree/base.h"

#pragma once

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
        using type_enum           = contiguous_tree_type_enum;
        using error_enum          = contiguous_request_adapter_errors_enum;
        using const_object_handle = typename Tree::const_object_handle;
        using const_array_handle  = typename Tree::const_array_handle;
        using object_handle       = typename Tree::object_handle;
        using array_handle        = typename Tree::array_handle;

        contiguous_request_adapter( Tree& tree )
          : tree_( tree )
        {
        }

        either< std::reference_wrapper< const value_type >, error_enum >
        get_value( node_id id ) const
        {
                return get_node( id ).bind_left(
                    [&]( const node_type& node )
                        -> either< std::reference_wrapper< const value_type >, error_enum > {
                            const value_type* val_ptr = node.get_value();
                            if ( !val_ptr ) {
                                    return CONTIGUOUS_WRONG_TYPE;
                            }
                            return std::ref( *val_ptr );
                    } );
        }

        either< node_id, error_enum >
        get_child( node_id nid, const std::variant< key_type, child_id >& id_var ) const
        {
                return get_containers( nid ).bind_left(
                    [&]( std::variant< const_object_handle, const_array_handle > var )
                        -> either< node_id, error_enum > {
                            const_object_handle* oh_ptr = std::get_if< 0 >( &var );
                            const_array_handle*  ah_ptr = std::get_if< 1 >( &var );

                            std::optional< node_id > res;
                            if ( oh_ptr ) {
                                    match(
                                        id_var,
                                        [&]( const key_type& k ) {
                                                res = oh_ptr->get_child( k );
                                        },
                                        [&]( child_id chid ) {
                                                res = oh_ptr->get_child( chid );
                                        } );
                            } else if ( ah_ptr && std::holds_alternative< child_id >( id_var ) ) {
                                    auto id = *std::get_if< child_id >( &id_var );
                                    res     = ah_ptr->get_child( id );
                            } else {
                                    return CONTIGUOUS_WRONG_TYPE;
                            }

                            if ( !res ) {
                                    return CONTIGUOUS_CHILD_MISSING;
                            }
                            return *res;
                    } );
        }

        either< child_id, error_enum > get_child_count( node_id nid ) const
        {
                return get_containers( nid ).convert_left(
                    [&]( const std::variant< const_object_handle, const_array_handle > var ) {
                            return visit(
                                [&]( auto handle ) {
                                        return handle.size();
                                },
                                var );
                    } );
        }

        either< key_type, error_enum > get_key( node_id nid, child_id chid ) const
        {
                return get_object_handle( nid ).bind_left(
                    [&]( const_object_handle oh ) -> either< key_type, error_enum > {
                            const key_type* key_ptr = oh.get_key( chid );

                            if ( !key_ptr ) {
                                    return CONTIGUOUS_CHILD_MISSING;
                            }
                            return *key_ptr;
                    } );
        }

        either< type_enum, error_enum > get_type( node_id nid ) const
        {
                return get_node( nid ).convert_left( [&]( const node_type& node ) {
                        return node.get_type();
                } );
        }

        either< node_id, error_enum > insert(
            node_id                                                      parent,
            const key_type&                                              key,
            const std::variant< value_type, contiguous_container_type >& val )
        {
                return get_object_handle( parent ).bind_left( [&]( object_handle oh ) {
                        return construct_node( val ).convert_left( [&]( node_id nid ) {
                                oh.set( key, nid );
                                return nid;
                        } );
                } );
        }

        either< node_id, error_enum >
        insert( node_id parent, const std::variant< value_type, contiguous_container_type >& val )
        {
                return get_array_handle( parent ).bind_left( [&]( array_handle ah ) {
                        return construct_node( val ).convert_left( [&]( node_id nid ) {
                                ah.append( nid );
                                return nid;
                        } );
                } );
        }

private:
        either< node_id, error_enum >
        construct_node( const std::variant< value_type, contiguous_container_type >& var )
        {
                std::optional< node_id > opt_nid = match(
                    var,
                    [&]( const value_type& val ) {
                            return tree_.make_value_node( val );
                    },
                    [&]( contiguous_container_type type ) -> std::optional< node_id > {
                            if ( type == CONTIGUOUS_CONT_ARRAY ) {
                                    auto opt_res = tree_.make_array_node();
                                    if ( opt_res ) {
                                            return opt_res->first;
                                    }
                                    return std::nullopt;

                            } else if ( type == CONTIGUOUS_CONT_OBJECT ) {
                                    auto opt_res = tree_.make_object_node();
                                    if ( opt_res ) {
                                            return opt_res->first;
                                    }
                                    return std::nullopt;
                            }
                            return std::nullopt;
                    } );
                if ( !opt_nid ) {
                        return CONTIGUOUS_FULL;
                }
                return *opt_nid;
        }

        either< std::reference_wrapper< const node_type >, error_enum >
        get_node( node_id nid ) const
        {
                const node_type* node_ptr = tree_.get_node( nid );

                if ( !node_ptr ) {
                        return CONTIGUOUS_MISSING_NODE;
                }

                return std::ref( *node_ptr );
        }
        either< std::reference_wrapper< node_type >, error_enum > get_node( node_id nid )
        {
                node_type* node_ptr = tree_.get_node( nid );

                if ( !node_ptr ) {
                        return CONTIGUOUS_MISSING_NODE;
                }

                return std::ref( *node_ptr );
        }
        either< std::variant< const_object_handle, const_array_handle >, error_enum >
        get_containers( node_id nid ) const
        {
                return get_containers_impl< const_object_handle, const_array_handle >( this, nid );
        }
        either< std::variant< object_handle, array_handle >, error_enum >
        get_containers( node_id nid )
        {
                return get_containers_impl< object_handle, array_handle >( this, nid );
        }

        template < typename OHandle, typename AHandle, typename Self >
        static either< std::variant< OHandle, AHandle >, error_enum >
        get_containers_impl( Self* self, node_id nid )
        {
                using var_type = std::variant< OHandle, AHandle >;
                return self->get_node( nid ).bind_left(
                    [&]( auto& node_wrapper ) -> either< var_type, error_enum > {
                            auto& node = node_wrapper.get();
                            std::variant< const value_type*, OHandle, AHandle > cont =
                                node.get_container_handle();

                            OHandle* oh_ptr = std::get_if< OHandle >( &cont );
                            AHandle* ah_ptr = std::get_if< AHandle >( &cont );
                            if ( ah_ptr ) {
                                    return var_type{ *ah_ptr };
                            }
                            if ( oh_ptr ) {
                                    return var_type{ *oh_ptr };
                            }
                            return CONTIGUOUS_WRONG_TYPE;
                    } );
        }

        either< const_object_handle, error_enum > get_object_handle( node_id nid ) const
        {
                return get_containers( nid ).bind_left(
                    [&]( std::variant< const_object_handle, const_array_handle > var )
                        -> either< const_object_handle, error_enum > {
                            const_object_handle* oh_ptr = std::get_if< 0 >( &var );
                            if ( oh_ptr ) {
                                    return *oh_ptr;
                            }
                            return CONTIGUOUS_WRONG_TYPE;
                    } );
        };
        either< object_handle, error_enum > get_object_handle( node_id nid )
        {
                return get_containers( nid ).bind_left(
                    [&]( std::variant< object_handle, array_handle > var )
                        -> either< object_handle, error_enum > {
                            object_handle* oh_ptr = std::get_if< 0 >( &var );
                            if ( oh_ptr ) {
                                    return *oh_ptr;
                            }
                            return CONTIGUOUS_WRONG_TYPE;
                    } );
        };
        either< array_handle, error_enum > get_array_handle( node_id nid )
        {
                return get_containers( nid ).bind_left(
                    [&]( std::variant< object_handle, array_handle > var )
                        -> either< array_handle, error_enum > {
                            array_handle* ah_ptr = std::get_if< 1 >( &var );
                            if ( ah_ptr ) {
                                    return *ah_ptr;
                            }
                            return CONTIGUOUS_WRONG_TYPE;
                    } );
        };

        Tree& tree_;
};

}  // namespace emlabcpp
