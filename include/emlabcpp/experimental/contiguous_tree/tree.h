#include "emlabcpp/allocator/pool.h"
#include "emlabcpp/experimental/contiguous_tree/base.h"
#include "emlabcpp/experimental/static_function.h"
#include "emlabcpp/match.h"
#include "emlabcpp/static_vector.h"

#include <map>

#pragma once

namespace emlabcpp
{
template < typename ObjectType >
class contiguous_object_handle
{
        using object_type = ObjectType;
        using child_id    = uint32_t;
        using node_id     = std::tuple_element_t< 1, typename object_type::value_type >;
        using key_type    = typename object_type::key_type;

public:
        using node_type = typename object_type::node_type;

        contiguous_object_handle( object_type* obj )
          : obj_( obj )
        {
        }
        std::optional< node_id > get_child( child_id chid ) const
        {
                if ( obj_->size() < chid ) {
                        return std::nullopt;
                }
                auto iter = obj_->begin();
                std::advance( iter, chid );
                return iter->second;
        }

        std::optional< node_id > get_child( key_type k ) const
        {
                auto iter = obj_->find( k );
                if ( iter == obj_->end() ) {
                        return std::nullopt;
                }
                return iter->second;
        }

        uint32_t size() const
        {
                return static_cast< uint32_t >( obj_->size() );
        }

        const key_type* get_key( child_id chid ) const
        {
                if ( chid >= obj_->size() ) {
                        return nullptr;
                }

                auto iter = obj_->begin();
                std::advance( iter, chid );
                return &iter->first;
        }

        void set( const key_type& k, node_id nid )
        {
                obj_->emplace( k, nid );
        }

private:
        object_type* obj_;
};

template < typename ArrayType >
class contiguous_array_handle
{
        using array_type = ArrayType;
        using child_id   = uint32_t;
        using node_id    = std::tuple_element_t< 1, typename array_type::value_type >;

public:
        using node_type = typename array_type::node_type;

        contiguous_array_handle( array_type* arr )
          : arr_( arr )
        {
        }
        std::optional< node_id > get_child( child_id chid ) const
        {
                auto iter = arr_->find( chid );
                if ( iter == arr_->end() ) {
                        return std::nullopt;
                }
                return iter->second;
        }

        uint32_t size() const
        {
                return static_cast< uint32_t >( arr_->size() );
        }

        void append( node_id nid )
        {
                arr_->emplace( arr_->size(), nid );
        }

private:
        array_type* arr_;
};

template < typename Key, typename Value >
class contiguous_node
{
public:
        using key_type            = Key;
        using value_type          = Value;
        using node_id             = uint32_t;
        using child_id            = uint32_t;
        using array_type          = pool_map< child_id, node_id >;
        using object_type         = pool_map< key_type, node_id >;
        using content_type        = std::variant< Value, array_type, object_type >;
        using object_handle       = contiguous_object_handle< object_type >;
        using const_object_handle = contiguous_object_handle< const object_type >;
        using array_handle        = contiguous_array_handle< array_type >;
        using const_array_handle  = contiguous_array_handle< const array_type >;

        contiguous_node( content_type cont )
          : content_( std::move( cont ) )
        {
        }

        const Value* get_value() const
        {
                return std::get_if< Value >( &content_ );
        }

        Value* get_value()
        {
                return std::get_if< Value >( &content_ );
        }

        std::variant< const Value*, object_handle, array_handle > get_container_handle()
        {
                if ( std::holds_alternative< array_type >( content_ ) ) {
                        return array_handle{ std::get_if< array_type >( &content_ ) };
                }
                if ( std::holds_alternative< object_type >( content_ ) ) {
                        return object_handle{ std::get_if< object_type >( &content_ ) };
                }
                return std::get_if< Value >( &content_ );
        }

        std::variant< const Value*, const_object_handle, const_array_handle >
        get_container_handle() const
        {
                if ( std::holds_alternative< array_type >( content_ ) ) {
                        return const_array_handle{ std::get_if< array_type >( &content_ ) };
                }
                if ( std::holds_alternative< object_type >( content_ ) ) {
                        return const_object_handle{ std::get_if< object_type >( &content_ ) };
                }
                return std::get_if< Value >( &content_ );
        }

        contiguous_tree_type_enum get_type() const
        {
                return match(
                    content_,
                    []( const Value& ) {
                            return CONTIGUOUS_TREE_VALUE;
                    },
                    []( const array_type& ) {
                            return CONTIGUOUS_TREE_ARRAY;
                    },
                    []( const object_type& ) {
                            return CONTIGUOUS_TREE_OBJECT;
                    } );
        }

private:
        content_type content_;
};

template < typename Key, typename Value >
class contiguous_tree
{
public:
        using key_type            = Key;
        using value_type          = Value;
        using node_id             = uint32_t;
        using child_id            = uint32_t;
        using array_type          = pool_map< child_id, node_id >;
        using object_type         = pool_map< key_type, node_id >;
        using node_type           = contiguous_node< Key, Value >;
        using container           = pool_map< node_id, node_type >;
        using content_type        = typename node_type::content_type;
        using object_handle       = typename node_type::object_handle;
        using const_object_handle = typename node_type::const_object_handle;
        using array_handle        = typename node_type::array_handle;
        using const_array_handle  = typename node_type::const_array_handle;

        // TODO: find better way to decided this
        static constexpr std::size_t required_pool_size = 110;

        template < std::size_t Count >
        using pool_type = pool_resource< required_pool_size, Count >;

        contiguous_tree( pool_interface* mem_pool )
          : data_( mem_pool )
          , mem_pool_( mem_pool )
        {
        }

        const node_type* get_node( node_id nid ) const
        {
                auto iter = data_.find( nid );
                if ( iter == data_.end() ) {
                        return nullptr;
                }
                return &iter->second;
        }
        node_type* get_node( node_id nid )
        {
                auto iter = data_.find( nid );
                if ( iter == data_.end() ) {
                        return nullptr;
                }
                return &iter->second;
        }

        bool empty() const
        {
                return data_.empty();
        }

        std::optional< node_id > make_value_node( value_type val )
        {
                std::optional opt_val = make_node( std::move( val ) );
                if ( !opt_val ) {
                        return std::nullopt;
                }
                auto [nid, iter] = *opt_val;
                return nid;
        }

        std::optional< std::pair< node_id, array_handle > > make_array_node()
        {
                std::optional opt_val = make_node( array_type{ mem_pool_ } );
                if ( !opt_val ) {
                        return std::nullopt;
                }
                auto [nid, iter] = *opt_val;
                auto var         = iter->second.get_container_handle();
                return std::make_pair( nid, std::get< array_handle >( var ) );
        }

        std::optional< std::pair< node_id, object_handle > > make_object_node()
        {
                std::optional opt_val = make_node( object_type{ mem_pool_ } );
                if ( !opt_val ) {
                        return std::nullopt;
                }
                auto [nid, iter] = *opt_val;
                auto var         = iter->second.get_container_handle();
                return std::make_pair( nid, std::get< object_handle >( var ) );
        }

private:
        using iterator = typename container::iterator;
        std::optional< std::pair< node_id, iterator > > make_node( content_type cont )
        {
                node_id nid = static_cast< node_id >( data_.size() );
                try {
                        auto [iter, inserted] =
                            data_.emplace( nid, node_type{ std::move( cont ) } );
                        return std::make_pair( nid, iter );
                }
                catch ( std::bad_alloc& ) {
                        return std::nullopt;
                }
        }

        container       data_;
        pool_interface* mem_pool_;
};

}  // namespace emlabcpp
