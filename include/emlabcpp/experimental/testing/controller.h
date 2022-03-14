#include "emlabcpp/iterators/convert.h"
#include "emlabcpp/match.h"
#include "emlabcpp/protocol/packet_handler.h"
#include "emlabcpp/experimental/testing/protocol.h"

#include <memory_resource>

#pragma once

namespace emlabcpp
{

struct testing_result
{
        testing_test_id                                   tid;
        testing_run_id                                    rid;
        std::pmr::map< testing_key, testing_arg_variant > collected_data;
        bool                                              failed  = false;
        bool                                              errored = false;
};

class testing_controller_interface
{
public:
        using sequencer = testing_reactor_controller_packet::sequencer;

        virtual void                         transmit( std::span< uint8_t > ) = 0;
        virtual static_vector< uint8_t, 64 > read( std::size_t )              = 0;
        virtual void                         on_result( testing_result )      = 0;
        virtual void                         on_error( const protocol_error_record& ){};
        virtual void                         on_error( testing_error_enum ){};
        virtual void                         on_wrong_message( testing_messages_enum ){};

        static constexpr std::size_t read_limit_ = 10;

        template < testing_messages_enum ID, typename... Args >
        void send( const Args&... args )
        {
                using handler = protocol_packet_handler< testing_controller_reactor_packet >;
                auto msg      = handler::serialize(
                    testing_controller_reactor_group::make_val< ID >( args... ) );
                transmit( msg );
        }

        std::optional< testing_reactor_controller_msg > read_message()
        {
                return protocol_simple_load< sequencer >( read_limit_, [&]( std::size_t c ) {
                        return read( c );
                } );
        }

        virtual ~testing_controller_interface() = default;
};

class testing_controller
{
public:
        struct test_info
        {
                testing_name_buffer name;
        };

        static std::optional< testing_controller > make( testing_controller_interface& iface )
        {

                auto opt_name  = load_data< testing_name_buffer, TESTING_SUITE_NAME >( iface );
                auto opt_date  = load_data< testing_name_buffer, TESTING_SUITE_DATE >( iface );
                auto opt_count = load_data< testing_test_id, TESTING_COUNT >( iface );

                if ( !opt_name || !opt_date || !opt_count ) {
                        return {};
                }

                std::pmr::map< testing_test_id, test_info > info;

                for ( testing_test_id i = 0; i < *opt_count; i++ ) {
                        auto opt_name =
                            load_data< testing_name_buffer, TESTING_NAME, testing_test_id >(
                                i, iface );
                        if ( !opt_name ) {
                                return {};
                        }
                        info[i] = test_info{ .name = *opt_name };
                }

                return testing_controller{ *opt_name, *opt_date, std::move( info ) };
        }

        std::string_view suite_name() const
        {
                return { name_.begin(), name_.end() };
        }

        std::string_view suite_date() const
        {
                return { date_.begin(), date_.end() };
        }

        bool has_active_test() const
        {
                return context_.has_value();
        }

        const std::pmr::map< testing_test_id, test_info >& get_tests() const
        {
                return tests_;
        }

        void start_test( testing_test_id tid, testing_controller_interface& iface )
        {
                EMLABCPP_ASSERT( !context_ );

                rid_ += 1;

                context_.emplace( tid, rid_ );

                iface.send< TESTING_LOAD >( tid, rid_ );

                iface.send< TESTING_EXEC >( rid_ );
        }

        void tick( testing_controller_interface& iface )
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
                                iface.on_error( e );
                        } );
        }

private:
        std::pmr::map< testing_test_id, test_info > tests_;
        testing_name_buffer                         name_;
        testing_name_buffer                         date_;
        std::optional< testing_result >             context_;
        testing_run_id                              rid_ = 0;

        testing_controller(
            testing_name_buffer                         name,
            testing_name_buffer                         date,
            std::pmr::map< testing_test_id, test_info > tests )
          : name_( name )
          , date_( date )
          , tests_( std::move( tests ) )
        {
        }

        void handle_message( tag< TESTING_COUNT >, auto, testing_controller_interface& iface )
        {
                iface.on_wrong_message( TESTING_COUNT );
        }
        void handle_message( tag< TESTING_NAME >, auto, testing_controller_interface& iface )
        {
                iface.on_wrong_message( TESTING_NAME );
        }
        void handle_message(
            tag< TESTING_ARG >,
            testing_run_id                rid,
            testing_key                   k,
            testing_controller_interface& iface )
        {
                EMLABCPP_ASSERT( context_->rid == rid );  // TODO better error handling
                testing_arg_variant val;
                iface.send< TESTING_ARG >( rid, k, val );
        }

        void handle_message(
            tag< TESTING_COLLECT >,
            testing_run_id      rid,
            testing_key         key,
            testing_arg_variant avar,
            testing_controller_interface& )
        {
                EMLABCPP_ASSERT( context_ );
                EMLABCPP_ASSERT( context_->rid == rid );  // TODO better error handling

                context_->collected_data[key] = avar;
        }
        void handle_message( tag< TESTING_FINISHED >, auto, testing_controller_interface& iface )
        {
                iface.on_result( *context_ );
                context_.reset();
        }
        void handle_message( tag< TESTING_ERROR >, auto, testing_controller_interface& )
        {
                context_->errored = true;
        }
        void handle_message( tag< TESTING_FAILURE >, auto, testing_controller_interface& )
        {
                context_->failed = true;
        }
        void handle_message( tag< TESTING_SUITE_NAME >, auto, testing_controller_interface& iface )
        {
                iface.on_wrong_message( TESTING_SUITE_NAME );
        }
        void handle_message( tag< TESTING_SUITE_DATE >, auto, testing_controller_interface& iface )
        {
                iface.on_wrong_message( TESTING_SUITE_DATE );
        }
        void handle_message(
            tag< TESTING_INTERNAL_ERROR >,
            testing_error_enum            err,
            testing_controller_interface& iface )
        {
                iface.on_error( err );
        }
        void handle_message(
            tag< TESTING_PROTOCOL_ERROR >,
            protocol_error_record         rec,
            testing_controller_interface& iface )
        {
                // TODO: mark that this comes from reactor
                iface.on_error( rec );
        }

        template < typename T, testing_messages_enum ID, typename... Args >
        static std::optional< T >
        load_data( const Args&... args, testing_controller_interface& iface )
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
                    []< auto WID >( tag< WID >, auto... ){ EMLABCPP_ASSERT( false );
        }
};

handler::extract( *opt_msg )
    .match(
        [&]( auto pack ) {
                apply_on_visit( handle, pack );
        },
        [&]( protocol_error_record rec ) {
                // TODO: error handling - improve :/
                iface.on_error( rec );
        } );
return res;
}  // namespace emlabcpp
}
;

}  // namespace emlabcpp
