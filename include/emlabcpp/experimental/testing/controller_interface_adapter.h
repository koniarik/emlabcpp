#include "emlabcpp/experimental/testing/controller_interface.h"
#include "emlabcpp/static_function.h"

#pragma once

namespace emlabcpp::testing
{

class controller_interface_adapter
{
        controller_interface& iface_;

        static constexpr std::size_t read_limit_ = 10;

        static_function< bool( const reactor_controller_variant& ), 32 > reply_cb_;
        controller_transmit_callback                                     send_cb_;

public:
        explicit controller_interface_adapter(
            controller_interface&        iface,
            controller_transmit_callback send_cb )
          : iface_( iface )
          , send_cb_( send_cb )
        {
        }

        void send( const controller_reactor_variant& var )
        {
                using h  = protocol::handler< controller_reactor_group >;
                auto msg = h::serialize( var );
                send_cb_( msg );
        }

        controller_interface* operator->()
        {
                return &iface_;
        }

        controller_interface& operator*()
        {
                return iface_;
        }

        void set_reply_cb( static_function< bool( const reactor_controller_variant& ), 32 > cb )
        {
                reply_cb_ = cb;
        }

        bool on_msg_with_cb( const reactor_controller_variant& var )
        {
                return reply_cb_( var );
        }

        void report_error( const error_variant& var )
        {
                log_error( var );
                iface_.on_error( var );
        }
#ifdef EMLABCPP_USE_LOGGING
        void log_error( const error_variant& var )
        {
                match(
                    var,
                    []( const reactor_protocol_error& e ) {
                            EMLABCPP_LOG( "Protocol error reported from reactor: " << e );
                    },
                    []( const controller_protocol_error& e ) {
                            EMLABCPP_LOG( "Protocol error reported from controller: " << e );
                    },
                    []( const internal_reactor_error& e ) {
                            EMLABCPP_LOG( "Internal error from reactor: " << e );
                    },
                    []( const controller_internal_error& e ) {
                            EMLABCPP_LOG( "Wrong message arrived to controller: " << e );
                    } );
                const auto* const internal_ptr = std::get_if< internal_reactor_error >( &var );

                if ( internal_ptr != nullptr ) {
                        match(
                            internal_ptr->val,
                            [this]( const wrong_type_error& ) {},
                            []( const auto& ) {} );
                }
        }
#else
        void log_error( const error_variant& )
        {
        }
#endif
};

}  // namespace emlabcpp::testing
