
#include "emlabcpp/experimental/testing/controller.h"

#include "emlabcpp/protocol/packet_handler.h"

using namespace emlabcpp;

namespace
{
template < typename T, testing_messages_enum ID, typename... Args >
std::optional< T > load_data( const Args&... args, testing_controller_interface& iface )
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
            [&]< auto WID >( tag< WID >, auto... ){ EMLABCPP_ASSERT( false );
}
};  // namespace

handler::extract( *opt_msg )
    .match(
        [&]( auto pack ) {
                apply_on_visit( handle, pack );
        },
        [&]( protocol_error_record rec ) {
                iface.on_error( testing_controller_protocol_error{ rec } );
        } );
return res;
}
}

std::optional< testing_controller >
testing_controller::make( testing_controller_interface& iface, pool_interface* pool )
{

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
    testing_controller_interface& iface )
{
        iface.on_error( testing_controller_message_error{ TESTING_COUNT } );
}
void testing_controller::handle_message(
    tag< TESTING_NAME >,
    auto,
    testing_controller_interface& iface )
{
        iface.on_error( testing_controller_message_error{ TESTING_NAME } );
}
void testing_controller::handle_message(
    tag< TESTING_ARG >,
    testing_run_id                rid,
    testing_key                   k,
    testing_controller_interface& iface )
{
        EMLABCPP_ASSERT( context_->rid == rid );  // TODO better error handling
        auto opt_val = match(
            k,
            [&]( uint32_t v ) -> std::optional< testing_arg_variant > {
                    return iface.on_arg( v );
            },
            [&]( testing_key_buffer v ) -> std::optional< testing_arg_variant > {
                    return iface.on_arg( std::string_view{ v.begin(), v.size() } );
            } );
        if ( opt_val ) {
                iface.send< TESTING_ARG >( rid, k, *opt_val );
        } else {
                iface.send< TESTING_ARG_MISSING >( rid, k );
        }
}

void testing_controller::handle_message(
    tag< TESTING_COLLECT >,
    testing_run_id      rid,
    testing_key         key,
    testing_arg_variant avar,
    testing_controller_interface& )
{
        std::ignore = rid;
        EMLABCPP_ASSERT( context_ );
        EMLABCPP_ASSERT( context_->rid == rid );  // TODO better error handling

        context_->collected_data[key] = avar;
}
void testing_controller::handle_message(
    tag< TESTING_FINISHED >,
    auto,
    testing_controller_interface& iface )
{
        iface.on_result( *context_ );
        context_.reset();
}
void testing_controller::handle_message( tag< TESTING_ERROR >, auto, testing_controller_interface& )
{
        context_->errored = true;
}
void testing_controller::handle_message(
    tag< TESTING_FAILURE >,
    auto,
    testing_controller_interface& )
{
        context_->failed = true;
}
void testing_controller::handle_message(
    tag< TESTING_SUITE_NAME >,
    auto,
    testing_controller_interface& iface )
{
        iface.on_error( testing_controller_message_error{ TESTING_SUITE_NAME } );
}
void testing_controller::handle_message(
    tag< TESTING_SUITE_DATE >,
    auto,
    testing_controller_interface& iface )
{
        iface.on_error( testing_controller_message_error{ TESTING_SUITE_DATE } );
}
void testing_controller::handle_message(
    tag< TESTING_INTERNAL_ERROR >,
    testing_error_enum            err,
    testing_controller_interface& iface )
{
        iface.on_error( testing_internal_reactor_error{ err } );
}
void testing_controller::handle_message(
    tag< TESTING_PROTOCOL_ERROR >,
    protocol_error_record         rec,
    testing_controller_interface& iface )
{
        iface.on_error( testing_reactor_protocol_error{ rec } );
}

void testing_controller::start_test( testing_test_id tid, testing_controller_interface& iface )
{
        EMLABCPP_ASSERT( !context_ );

        rid_ += 1;

        context_.emplace( tid, rid_ );

        iface.send< TESTING_LOAD >( tid, rid_ );

        iface.send< TESTING_EXEC >( rid_ );
}

void testing_controller::tick( testing_controller_interface& iface )
{
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
                        iface.on_error( testing_controller_protocol_error{ e } );
                } );
}
