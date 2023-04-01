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

#include "emlabcpp/experimental/contiguous_tree/base.h"
#include "emlabcpp/experimental/logging.h"
#include "emlabcpp/match.h"
#include "emlabcpp/pmr/aliases.h"
#include "emlabcpp/pmr/pool_resource.h"
#include "emlabcpp/static_vector.h"

#include <map>

#pragma once

namespace emlabcpp
{
template < typename ObjectType >
class contiguous_object_handle
{
        static constexpr bool is_const = std::is_const_v< ObjectType >;

        using object_type    = ObjectType;
        using child_id       = uint32_t;
        using node_id        = std::tuple_element_t< 1, typename object_type::value_type >;
        using key_type       = typename object_type::key_type;
        using const_iterator = typename object_type::const_iterator;
        using iterator =
            std::conditional_t< is_const, const_iterator, typename object_type::iterator >;

public:
        using node_type = typename object_type::node_type;

        explicit contiguous_object_handle( object_type* obj )
          : obj_( obj )
        {
        }

        [[nodiscard]] iterator begin()
        {
                return obj_->begin();
        }
        [[nodiscard]] iterator end()
        {
                return obj_->end();
        }

        [[nodiscard]] const_iterator begin() const
        {
                return obj_->begin();
        }
        [[nodiscard]] const_iterator end() const
        {
                return obj_->end();
        }

        [[nodiscard]] std::optional< node_id > get_child( child_id chid ) const
        {
                if ( obj_->size() < chid ) {
                        return std::nullopt;
                }
                auto iter = obj_->begin();
                std::advance( iter, chid );
                return iter->second;
        }

        [[nodiscard]] std::optional< node_id > get_child( key_type k ) const
        {
                auto iter = obj_->find( k );
                if ( iter == obj_->end() ) {
                        return std::nullopt;
                }
                return iter->second;
        }

        [[nodiscard]] uint32_t size() const
        {
                return static_cast< uint32_t >( obj_->size() );
        }

        [[nodiscard]] const key_type* get_key( child_id chid ) const
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

template < typename ObjectType >
std::ostream& operator<<( std::ostream& os, const contiguous_object_handle< ObjectType >& oh )
{
        for ( const auto& [key, nid] : oh ) {
                os << key << ":" << nid << ",";
        }
        return os;
}

template < typename ArrayType >
class contiguous_array_handle
{
        static constexpr bool is_const = std::is_const_v< ArrayType >;

        using array_type     = ArrayType;
        using child_id       = uint32_t;
        using node_id        = std::tuple_element_t< 1, typename array_type::value_type >;
        using const_iterator = typename array_type::const_iterator;
        using iterator =
            std::conditional_t< is_const, const_iterator, typename array_type::iterator >;

public:
        using node_type = typename array_type::node_type;

        explicit contiguous_array_handle( array_type* arr )
          : arr_( arr )
        {
        }
        [[nodiscard]] iterator begin()
        {
                return arr_->begin();
        }
        [[nodiscard]] iterator end()
        {
                return arr_->end();
        }

        [[nodiscard]] const_iterator begin() const
        {
                return arr_->begin();
        }
        [[nodiscard]] const_iterator end() const
        {
                return arr_->end();
        }

        [[nodiscard]] std::optional< node_id > get_child( child_id chid ) const
        {
                auto iter = arr_->find( chid );
                if ( iter == arr_->end() ) {
                        return std::nullopt;
                }
                return iter->second;
        }

        [[nodiscard]] uint32_t size() const
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
template < typename ArrayType >
std::ostream& operator<<( std::ostream& os, const contiguous_array_handle< ArrayType >& ah )
{
        for ( const auto& [chid, nid] : ah ) {
                os << chid << ":" << nid << ",";
        }
        return os;
}

template < typename Key, typename Value >
class contiguous_node
{
public:
        using key_type            = Key;
        using value_type          = Value;
        using node_id             = uint32_t;
        using child_id            = uint32_t;
        using array_type          = pmr::map< child_id, node_id >;
        using object_type         = pmr::map< key_type, node_id >;
        using content_type        = std::variant< Value, array_type, object_type >;
        using object_handle       = contiguous_object_handle< object_type >;
        using const_object_handle = contiguous_object_handle< const object_type >;
        using array_handle        = contiguous_array_handle< array_type >;
        using const_array_handle  = contiguous_array_handle< const array_type >;

        explicit contiguous_node( content_type cont )
          : content_( std::move( cont ) )
        {
        }

        [[nodiscard]] const Value* get_value() const
        {
                return std::get_if< Value >( &content_ );
        }

        [[nodiscard]] Value* get_value()
        {
                return std::get_if< Value >( &content_ );
        }

        [[nodiscard]] std::
            variant< std::reference_wrapper< const Value >, object_handle, array_handle >
            get_container_handle()
        {
                if ( std::holds_alternative< array_type >( content_ ) ) {
                        return array_handle{ std::get_if< array_type >( &content_ ) };
                }
                if ( std::holds_alternative< object_type >( content_ ) ) {
                        return object_handle{ std::get_if< object_type >( &content_ ) };
                }
                return std::ref( *std::get_if< Value >( &content_ ) );
        }

        [[nodiscard]] std::variant<
            std::reference_wrapper< const Value >,
            const_object_handle,
            const_array_handle >
        get_container_handle() const
        {
                if ( std::holds_alternative< array_type >( content_ ) ) {
                        return const_array_handle{ std::get_if< array_type >( &content_ ) };
                }
                if ( std::holds_alternative< object_type >( content_ ) ) {
                        return const_object_handle{ std::get_if< object_type >( &content_ ) };
                }
                return std::ref( *std::get_if< Value >( &content_ ) );
        }

        [[nodiscard]] contiguous_tree_type get_type() const
        {
                return match(
                    content_,
                    []( const Value& ) {
                            return contiguous_tree_type::VALUE;
                    },
                    []( const array_type& ) {
                            return contiguous_tree_type::ARRAY;
                    },
                    []( const object_type& ) {
                            return contiguous_tree_type::OBJECT;
                    } );
        }

private:
        content_type content_;
};

template < typename Key, typename Value >
std::ostream& operator<<( std::ostream& os, const contiguous_node< Key, Value >& node )
{
        visit(
            [&os]( const auto& val ) {
                    os << val;
            },
            node.get_container_handle() );
        return os;
}

template < typename Key, typename Value >
class contiguous_tree
{
public:
        using key_type            = Key;
        using value_type          = Value;
        using node_id             = uint32_t;
        using child_id            = uint32_t;
        using array_type          = pmr::map< child_id, node_id >;
        using object_type         = pmr::map< key_type, node_id >;
        using node_type           = contiguous_node< Key, Value >;
        using container           = pmr::map< node_id, node_type >;
        using content_type        = typename node_type::content_type;
        using object_handle       = typename node_type::object_handle;
        using const_object_handle = typename node_type::const_object_handle;
        using array_handle        = typename node_type::array_handle;
        using const_array_handle  = typename node_type::const_array_handle;

        using iterator       = typename container::iterator;
        using const_iterator = typename container::const_iterator;

        // TODO: find better way to decided this
        static constexpr std::size_t required_pool_size = 110;

        template < uint16_t Count >
        using pool_type = pmr::pool_resource< required_pool_size, Count >;

        explicit contiguous_tree( pmr::memory_resource& mem_res )
          : data_( mem_res )
          , mem_res_( mem_res )
        {
        }

        void clear()
        {
                data_.clear();
        }

        [[nodiscard]] const node_type* get_node( node_id nid ) const
        {
                auto iter = data_.find( nid );
                if ( iter == data_.end() ) {
                        return nullptr;
                }
                return &iter->second;
        }
        [[nodiscard]] node_type* get_node( node_id nid )
        {
                auto iter = data_.find( nid );
                if ( iter == data_.end() ) {
                        return nullptr;
                }
                return &iter->second;
        }

        [[nodiscard]] iterator begin()
        {
                return data_.begin();
        }
        [[nodiscard]] iterator end()
        {
                return data_.end();
        }

        [[nodiscard]] const_iterator begin() const
        {
                return data_.begin();
        }
        [[nodiscard]] const_iterator end() const
        {
                return data_.end();
        }

        [[nodiscard]] std::size_t size() const
        {
                return data_.size();
        }

        [[nodiscard]] bool empty() const
        {
                return data_.empty();
        }

        std::optional< node_id > make_value_node( value_type val )
        {
                std::optional opt_val = make_node( std::move( val ) );
                if ( !opt_val ) {
                        EMLABCPP_ERROR_LOG( "Failed to make value node in tree" );
                        return std::nullopt;
                }
                auto [nid, iter] = *opt_val;
                return nid;
        }

        std::optional< std::pair< node_id, array_handle > > make_array_node()
        {
                std::optional opt_val = make_node( array_type{ mem_res_.get() } );
                if ( !opt_val ) {
                        EMLABCPP_ERROR_LOG( "Failed to make array node in tree" );
                        return std::nullopt;
                }
                auto [nid, iter] = *opt_val;
                auto var         = iter->second.get_container_handle();
                return std::make_pair( nid, std::get< array_handle >( var ) );
        }

        std::optional< std::pair< node_id, object_handle > > make_object_node()
        {
                std::optional opt_val = make_node( object_type{ mem_res_.get() } );
                if ( !opt_val ) {
                        EMLABCPP_ERROR_LOG( "Failed to make object node in tree" );
                        return std::nullopt;
                }
                auto [nid, iter] = *opt_val;
                auto var         = iter->second.get_container_handle();
                return std::make_pair( nid, std::get< object_handle >( var ) );
        }

private:
        std::optional< std::pair< node_id, iterator > > make_node( content_type cont )
        {
                if ( mem_res_.get().is_full() ) {
                        return std::nullopt;
                }
                auto nid              = static_cast< node_id >( data_.size() );
                auto [iter, inserted] = data_.emplace( nid, node_type{ std::move( cont ) } );
                return std::make_pair( nid, iter );
        }

        container                                      data_;
        std::reference_wrapper< pmr::memory_resource > mem_res_;
};

template < typename Key, typename Value >
std::ostream& operator<<( std::ostream& os, const contiguous_tree< Key, Value >& tree )
{
        for ( const auto& [nid, node] : tree ) {
                os << nid << ":" << node << "\n";
        }
        return os;
}

}  // namespace emlabcpp
