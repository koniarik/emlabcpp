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

#include "../match.h"
#include "../protocol/handler.h"
#include "../protocol/streams.h"
#include "../static_function.h"

namespace emlabcpp::rpc
{

struct void_return_type
{
};

struct reactor_error
{
        protocol::error_record record;
};

struct reply_error
{
        std::size_t expected_index;
        std::size_t index;
};

struct error
{
        std::variant< reactor_error, protocol::error_record, reply_error > er;
};

template < typename T >
struct traits;

template < typename... Calls >
struct traits< std::tuple< Calls... > >
{
        template < typename Call >
        struct request_wrapper : protocol::converter_def_type_base
        {
                static constexpr auto id = Call::id;

                using def_type = typename Call::request;
        };

        template < typename Call >
        struct reply_wrapper : protocol::converter_def_type_base
        {
                static constexpr auto id = Call::id;

                static_assert( !std::is_void_v< typename Call::reply > );

                using def_type = typename Call::reply;
        };

        using call_defs = std::tuple< Calls... >;

        using request_group = protocol::tag_group< request_wrapper< Calls >... >;
        using reply_group   = protocol::tag_group< reply_wrapper< Calls >... >;

        using request_type  = typename protocol::traits_for< request_group >::value_type;
        using reply_variant = typename protocol::traits_for< reply_group >::value_type;

        using outter_reply_group = std::variant< reply_group, reactor_error >;
        using reply_type         = typename protocol::traits_for< outter_reply_group >::value_type;

        using request_traits = protocol::traits_for< request_type >;
        using reply_traits   = protocol::traits_for< outter_reply_group >;

        using request_message_type = protocol::message< request_traits::max_size >;
        using reply_message_type   = protocol::message< reply_traits::max_size >;
};

template < typename Wrapper >
using wrapper_traits = traits< typename Wrapper::call_defs >;

template < typename CallDefs >
class reactor
{
public:
        using traits_type          = traits< CallDefs >;
        using reply_variant        = typename traits_type::reply_variant;
        using request_type         = typename traits_type::request_type;
        using request_group        = typename traits_type::request_group;
        using reply_type           = typename traits_type::reply_type;
        using outter_reply_group   = typename traits_type::outter_reply_group;
        using request_message_type = typename traits_type::request_message_type;
        using reply_message_type   = typename traits_type::reply_message_type;

        using request_handler = protocol::handler< request_group >;
        using reply_handler   = protocol::handler< outter_reply_group >;

        template < typename Handler >
        static reply_message_type on_message( request_message_type const& msg, Handler&& h )
        {
                return match(
                    request_handler::extract( msg ),
                    [&]( request_type const& req_var ) {
                            reply_variant rep = visit_index(
                                [&]< std::size_t i >() {
                                        auto val = h( tag< i >{}, *std::get_if< i >( &req_var ) );
                                        return reply_variant( std::in_place_index< i >, val );
                                },
                                req_var );
                            return reply_handler::serialize( rep );
                    },
                    [&]( protocol::error_record const& rec ) {
                            return reply_handler::serialize( reactor_error{ rec } );
                    } );
        }
};

template < typename CallDefs >
static constexpr std::size_t get_call_index( auto id )
{
        return find_if_index< std::tuple_size_v< CallDefs > >( [id]< std::size_t i >() {
                return std::tuple_element_t< i, CallDefs >::id == id;
        } );
}

template < typename Wrapper >
class controller
{

public:
        using traits_type          = wrapper_traits< Wrapper >;
        using call_defs            = typename traits_type::call_defs;
        using reply_variant        = typename traits_type::reply_variant;
        using request_type         = typename traits_type::request_type;
        using reply_type           = typename traits_type::reply_type;
        using request_group        = typename traits_type::request_group;
        using outter_reply_group   = typename traits_type::outter_reply_group;
        using request_message_type = typename traits_type::request_message_type;
        using reply_message_type   = typename traits_type::reply_message_type;

        using request_handler = protocol::handler< request_group >;
        using reply_handler   = protocol::handler< outter_reply_group >;

        template < auto ID >
        static constexpr std::size_t call_index = get_call_index< call_defs >( ID );

        template < auto ID >
        using call_type = std::tuple_element_t< call_index< ID >, call_defs >;

        template < auto ID, typename... Args, typename MsgCallback >
        static std::variant< typename call_type< ID >::reply, error >
        call( MsgCallback&& cb, Args const&... args )
        {
                auto req_msg   = make_call_msg< ID >( args... );
                auto reply_msg = cb( req_msg );

                return on_reply_msg< ID >( reply_msg );
        }

        template < auto ID, typename... Args >
        static auto make_call_msg( Args const&... args )
        {

                typename call_type< ID >::request req{ args... };
                auto                              req_msg = request_handler::serialize(
                    request_type{ std::in_place_index< call_index< ID > >, req } );
                return req_msg;
        }

        template < auto ID >
        static std::variant< typename call_type< ID >::reply, error >
        on_reply_msg( auto const& reply_msg )
        {
                using rt = typename call_type< ID >::reply;

                auto tmp = reply_handler::extract( reply_msg );
                if ( auto* err = std::get_if< protocol::error_record >( &tmp ) )
                        return error{ *err };

                auto const& var     = *std::get_if< 0 >( &tmp );
                auto*       err_ptr = std::get_if< reactor_error >( &var );
                if ( err_ptr != nullptr )
                        return error{ *err_ptr };

                auto& reply_var = std::get< 0 >( var );
                auto* ptr       = std::get_if< call_index< ID > >( &reply_var );
                if ( ptr == nullptr ) {
                        return error{ reply_error{
                            .expected_index = call_index< ID >, .index = reply_var.index() } };
                }
                return *ptr;
        }
};

template < auto ID, auto Method >
struct derive
{
        static constexpr auto id = ID;

        using sig = signature_of< decltype( Method ) >;

        static constexpr bool void_returning = std::is_void_v< typename sig::return_type >;

        using request = typename sig::args_type;
        using reply =
            std::conditional_t< void_returning, void_return_type, typename sig::return_type >;

        static constexpr reply handle( auto& obj, auto const&... args )
        {
                if constexpr ( void_returning ) {
                        ( obj.*Method )( args... );
                        return void_return_type{};
                } else {
                        return ( obj.*Method )( args... );
                }
        }
};

template < typename Class, typename... Bindings >
class class_wrapper
{
public:
        using bindings_tuple       = std::tuple< Bindings... >;
        using call_defs            = bindings_tuple;
        using reactor_type         = reactor< bindings_tuple >;
        using request_message_type = typename reactor_type::request_message_type;
        using reply_message_type   = typename reactor_type::reply_message_type;

        class_wrapper( Class& obj )
          : obj_( obj ) {};

        reply_message_type on_message( request_message_type const& msg )
        {
                return reactor_type::on_message( msg, *this );
        }

        template < std::size_t I, typename Request >
        auto operator()( tag< I >, Request const& req )
        {
                using call_type = std::tuple_element_t< I, bindings_tuple >;

                return std::apply(
                    [&]( auto const&... args ) -> typename call_type::reply {
                            return call_type::handle( obj_, args... );
                    },
                    req );
        }

private:
        Class& obj_;
};

template < auto ID, typename Signature, std::size_t CallableSize = 32 >
struct bind
{
        static constexpr auto id = ID;
        using sfunction          = static_function< Signature, CallableSize >;
        using sig                = signature_of< Signature >;

        static constexpr bool void_returning = std::is_void_v< typename sig::return_type >;

        using request = typename sig::args_type;
        using reply =
            std::conditional_t< void_returning, void_return_type, typename sig::return_type >;
};

template < typename... Bindings >
class bind_wrapper
{
        using def_type             = std::tuple< Bindings... >;
        using callbacks            = std::tuple< typename Bindings::sfunction... >;
        using reactor_type         = reactor< def_type >;
        using request_message_type = typename reactor_type::request_message_type;
        using reply_message_type   = typename reactor_type::reply_message_type;

        template < auto ID >
        static constexpr std::size_t call_index = get_call_index< def_type >( ID );

public:
        reply_message_type on_message( request_message_type const& msg )
        {
                return reactor_type::on_message( msg, *this );
        }

        template < auto ID, typename Callable >
        void bind( Callable cb )
        {
                static constexpr std::size_t i = call_index< ID >;

                std::get< i >( cbs_ ) = cb;
        }

        template < std::size_t I, typename Request >
        auto operator()( tag< I >, Request const& req )
        {
                using call_type = std::tuple_element_t< I, def_type >;

                return std::apply(
                    [&]( auto const&... args ) -> typename call_type::reply {
                            if constexpr ( call_type::void_returning ) {
                                    std::get< I >( cbs_ )( args... );
                                    return void_return_type{};
                            } else {
                                    return std::get< I >( cbs_ )( args... );
                            }
                    },
                    req );
        }

private:
        callbacks cbs_;
};

}  // namespace emlabcpp::rpc
