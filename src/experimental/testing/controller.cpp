
#include "emlabcpp/experimental/testing/controller.h"

#include "emlabcpp/protocol/packet_handler.h"

using namespace emlabcpp;

class emlabcpp::testing_controller_interface_adapter
{
        testing_controller_interface& iface_;

        static constexpr std::size_t read_limit_ = 10;

public:
        testing_controller_interface_adapter( testing_controller_interface& iface )
          : iface_( iface )
        {
        }

        void send( const testing_controller_reactor_variant& var )
        {
                using handler = protocol_packet_handler< testing_controller_reactor_packet >;
                auto msg      = handler::serialize( var );
                iface_.transmit( msg );
        }

        testing_controller_interface* operator->()
        {
                return &iface_;
        }

        testing_controller_interface& operator*()
        {
                return iface_;
        }

        template < testing_messages_enum ID, typename... Args >
        void send( const Args&... args )
        {
                send( testing_controller_reactor_group::make_val< ID >( args... ) );
        }

        std::optional< testing_reactor_controller_msg > read_message()
        {
                using sequencer = testing_reactor_controller_packet::sequencer;
                return protocol_simple_load< sequencer >( read_limit_, [&]( std::size_t c ) {
                        return iface_.receive( c );
                } );
        }
};

namespace
{
template < typename T, testing_messages_enum ID, typename... Args >
std::optional< T > load_data( const Args&... args, testing_controller_interface_adapter& iface )
{
        using handler = protocol_packet_handler< testing_reactor_controller_packet >;
        std::optional< T > res;

        iface.send< ID >( args... );

        auto opt_msg = iface.read_message();
        if ( !opt_msg ) {
                return res;
        }

        auto handle = matcher{
            [&]( tag< ID >, auto item ) {
                    res = item;
            },
            [&]( tag< TESTING_INTERNAL_ERROR >, testing_reactor_error_variant err ) {
                    iface->on_error( testing_internal_reactor_error{ std::move( err ) } );
            },
            [&]( tag< TESTING_PROTOCOL_ERROR >, protocol_error_record rec ) {
                    iface->on_error( testing_reactor_protocol_error{ rec } );
            },
            [&]< auto WID >( tag< WID >, auto... ){
                iface->on_error( testing_controller_message_error{ WID } );
}
};  // namespace

handler::extract( *opt_msg )
    .match(
        [&]( auto pack ) {
                apply_on_visit( handle, pack );
        },
        [&]( protocol_error_record rec ) {
                iface->on_error( testing_controller_protocol_error{ rec } );
        } );
return res;
}
}

std::optional< testing_controller >
testing_controller::make( testing_controller_interface& top_iface, pool_interface* pool )
{
        testing_controller_interface_adapter iface{ top_iface };

        auto opt_name  = load_data< testing_name_buffer, TESTING_SUITE_NAME >( iface );
        auto opt_date  = load_data< testing_name_buffer, TESTING_SUITE_DATE >( iface );
        auto opt_count = load_data< testing_test_id, TESTING_COUNT >( iface );

        if ( !opt_name || !opt_date || !opt_count ) {
                return {};
        }

        pool_map< testing_test_id, test_info > info{ pool };

        for ( testing_test_id i = 0; i < *opt_count; i++ ) {
                auto opt_name =
                    load_data< testing_name_buffer, TESTING_NAME, testing_test_id >( i, iface );
                if ( !opt_name ) {
                        return {};
                }
                info[i] = test_info{ .name = *opt_name };
        }

        return testing_controller{ *opt_name, *opt_date, std::move( info ) };
}
void testing_controller::handle_message(
    tag< TESTING_COUNT >,
    auto,
    testing_controller_interface_adapter iface )
{
        iface->on_error( testing_controller_message_error{ TESTING_COUNT } );
}
void testing_controller::handle_message(
    tag< TESTING_NAME >,
    auto,
    testing_controller_interface_adapter iface )
{
        iface->on_error( testing_controller_message_error{ TESTING_NAME } );
}
void testing_controller::handle_message(
    tag< TESTING_ARG >,
    testing_run_id                       rid,
    testing_key                          k,
    testing_controller_interface_adapter iface )
{
        EMLABCPP_ASSERT( context_->rid == rid );  // TODO better error handling
        auto opt_val = match(
            k,
            [&]( uint32_t v ) -> std::optional< testing_arg_variant > {
                    return iface->on_arg( v );
            },
            [&]( testing_key_buffer v ) -> std::optional< testing_arg_variant > {
                    return iface->on_arg( std::string_view{ v.begin(), v.size() } );
            } );
        if ( opt_val ) {
                iface.send< TESTING_ARG >( rid, k, *opt_val );
        } else {
                iface.send< TESTING_ARG_MISSING >( rid, k );
        }
}

void testing_controller::handle_message(
    tag< TESTING_COLLECT >,
    testing_run_id             rid,
    const testing_key&         key,
    const testing_arg_variant& avar,
    testing_controller_interface_adapter )
{
        std::ignore = rid;
        EMLABCPP_ASSERT( context_ );
        EMLABCPP_ASSERT( context_->rid == rid );  // TODO better error handling

        context_->collected_data[key] = avar;
}
void testing_controller::handle_message(
    tag< TESTING_FINISHED >,
    auto,
    testing_controller_interface_adapter iface )
{
        iface->on_result( std::move( *context_ ) );
        context_.reset();
}
void testing_controller::handle_message(
    tag< TESTING_ERROR >,
    auto,
    testing_controller_interface_adapter )
{
        context_->errored = true;
}
void testing_controller::handle_message(
    tag< TESTING_FAILURE >,
    auto,
    testing_controller_interface_adapter )
{
        context_->failed = true;
}
void testing_controller::handle_message(
    tag< TESTING_SUITE_NAME >,
    auto,
    testing_controller_interface_adapter iface )
{
        iface->on_error( testing_controller_message_error{ TESTING_SUITE_NAME } );
}
void testing_controller::handle_message(
    tag< TESTING_SUITE_DATE >,
    auto,
    testing_controller_interface_adapter iface )
{
        iface->on_error( testing_controller_message_error{ TESTING_SUITE_DATE } );
}
void testing_controller::handle_message(
    tag< TESTING_INTERNAL_ERROR >,
    testing_reactor_error_variant        err,
    testing_controller_interface_adapter iface )
{
        iface->on_error( testing_internal_reactor_error{ std::move( err ) } );
}
void testing_controller::handle_message(
    tag< TESTING_PROTOCOL_ERROR >,
    protocol_error_record                rec,
    testing_controller_interface_adapter iface )
{
        iface->on_error( testing_reactor_protocol_error{ rec } );
}

void testing_controller::start_test( testing_test_id tid, testing_controller_interface& top_iface )
{
        testing_controller_interface_adapter iface{ top_iface };
        EMLABCPP_ASSERT( !context_ );

        rid_ += 1;

        context_.emplace( tid, rid_ );

        iface.send< TESTING_LOAD >( tid, rid_ );
        iface.send< TESTING_EXEC >( rid_ );
}

void testing_controller::tick( testing_controller_interface& top_iface )
{
        testing_controller_interface_adapter iface{ top_iface };

        if ( !context_ ) {
                return;
        }

        auto opt_msg = iface.read_message();
        if ( !opt_msg ) {
                return;
        }

        using handler = protocol_packet_handler< testing_reactor_controller_packet >;

        handler::extract( *opt_msg )
            .match(
                [&]( auto var ) {
                        apply_on_visit(
                            [&]( auto... args ) {
                                    handle_message( args..., iface );
                            },
                            var );
                },
                [&]( protocol_error_record e ) {
                        iface->on_error( testing_controller_protocol_error{ e } );
                } );
}
