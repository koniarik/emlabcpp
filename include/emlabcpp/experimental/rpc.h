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
public:
        using traits_type          = traits< Def >;
        using reply_variant        = typename traits_type::reply_variant;
        using request_type         = typename traits_type::request_type;
        using reply_type           = typename traits_type::reply_type;
        using request_message_type = typename traits_type::request_message_type;
        using reply_message_type   = typename traits_type::reply_message_type;

        using request_handler = protocol::handler< request_type >;
        using reply_handler   = protocol::handler< reply_type >;
        // TODO: we need better error type /o\.. error_record here can belong to both: reactor and
        // controller
        template < element_of< Def > T, typename MsgCallback >
        static either< typename T::reply, protocol::error_record >
        call( const typename T::request& req, MsgCallback&& cb )
        {

                auto req_msg   = request_handler::serialize( req );
                auto reply_msg = cb( req_msg );
                return reply_handler::extract( reply_msg )
                    .bind_left(
                        [&]( const auto& var )
                            -> either< typename T::reply, protocol::error_record > {
                                // TODO: extract car properly and decompose errors properly

                                auto* ptr =
                                    std::get_if< typename T::reply >( std::get_if< 0 >( &var ) );
                                if ( ptr ) {
                                        return *ptr;
                                } else {
                                        return protocol::error_record{};
                                }
                        } );
        }
};

}  // namespace emlabcpp::rpc
