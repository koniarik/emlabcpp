#include "emlabcpp/protocol/handler.h"

#pragma once

namespace emlabcpp::rpc
{

template < typename T >
struct traits;

template < typename... Calls >
struct traits< std::tuple< Calls... > >
{
        using request_type  = std::variant< typename Calls::request... >;
        using reply_variant = std::variant< typename Calls::reply... >;
        // TODO: well, the second thing is more like "rpc_reactor_error"
        using reply_type = std::variant< reply_variant, protocol::error_record >;

        using request_traits = protocol::traits_for< request_type >;
        using reply_traits   = protocol::traits_for< reply_type >;

        using request_message_type = protocol::message< request_traits::max_size >;
        using reply_message_type   = protocol::message< reply_traits::max_size >;
};

template < typename Def >
class reactor
{
public:
        using traits_type          = traits< Def >;
        using reply_variant        = typename traits_type::reply_variant;
        using request_type         = typename traits_type::request_type;
        using reply_type           = typename traits_type::reply_type;
        using request_message_type = typename traits_type::request_message_type;
        using reply_message_type   = typename traits_type::reply_message_type;

        using request_handler = protocol::handler< request_type >;
        using reply_handler   = protocol::handler< reply_type >;

        template < typename Handler >
        static reply_message_type on_message( const request_message_type& msg, Handler&& h )
        {
                return request_handler::extract( msg )
                    .convert_left( [&]( const request_type& req_var ) {
                            reply_variant rep = visit( h, req_var );
                            return reply_handler::serialize( rep );
                    } )
                    .convert_right( [&]( const protocol::error_record& rec ) {
                            return reply_handler::serialize( rec );
                    } )
                    .join();
        }
};

template < typename Def >
class controller : public traits< Def >
{
        static constexpr std::size_t get_call_index( auto id )
        {
                return find_if_index< std::tuple_size_v< Def > >( [id]< std::size_t i >() {
                        return std::tuple_element_t< i, Def >::id == id;
                } );
        }

public:
        using traits_type          = traits< Def >;
        using reply_variant        = typename traits_type::reply_variant;
        using request_type         = typename traits_type::request_type;
        using reply_type           = typename traits_type::reply_type;
        using request_message_type = typename traits_type::request_message_type;
        using reply_message_type   = typename traits_type::reply_message_type;

        using request_handler = protocol::handler< request_type >;
        using reply_handler   = protocol::handler< reply_type >;

        template < auto ID >
        static constexpr std::size_t call_index = get_call_index( ID );

        template < auto ID >
        using call_type = std::tuple_element_t< call_index< ID >, Def >;

        // TODO: we need better error type /o\.. error_record here can belong to both: reactor and
        // controller
        template < auto ID, typename MsgCallback >
        static either< typename call_type< ID >::reply, protocol::error_record >
        call( const typename call_type< ID >::request& req, MsgCallback&& cb )
        {
                using rt = typename call_type< ID >::reply;

                auto req_msg   = request_handler::serialize( req );
                auto reply_msg = cb( req_msg );
                return reply_handler::extract( reply_msg )
                    .bind_left( [&]( const auto& var ) -> either< rt, protocol::error_record > {
                            // TODO: extract car properly and decompose errors properly

                            auto* ptr = std::get_if< rt >( std::get_if< 0 >( &var ) );
                            if ( ptr ) {
                                    return *ptr;
                            } else {
                                    return protocol::error_record{};
                            }
                    } );
        }
};

template < typename Signature >
struct signature_of;

template < typename ReturnType, typename Class, typename... Args >
struct signature_of< ReturnType ( Class::* )( Args... ) >
{
        using return_type = ReturnType;
        using class_type  = Class;
        using args_tuple  = std::tuple< Args... >;
};

template < auto ID, auto Method >
struct derive
{
        static constexpr auto id = ID;

        static constexpr auto method = Method;

        using sig = signature_of< decltype( Method ) >;

        using request = typename sig::args_tuple;
        using reply   = typename sig::return_type;
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
          : obj_( obj ){};

        reply_message_type on_message( const request_message_type& msg )
        {
                return reactor_type::on_message( msg, *this );
        }

        template < typename Request >
        auto operator()( const Request& req )
        {
                static constexpr auto i =
                    find_if_index< std::tuple_size_v< bindings_tuple > >( []< std::size_t i > {
                            return std::same_as<
                                Request,
                                typename std::tuple_element_t< i, bindings_tuple >::request >;
                    } );
                using call_type = std::tuple_element_t< i, bindings_tuple >;

                return std::apply(
                    [&]( const auto&... args ) {
                            static constexpr auto m = call_type::method;
                            return ( obj_.*m )( args... );
                    },
                    req );
        }

private:
        Class& obj_;
};

}  // namespace emlabcpp::rpc
