#include <emlabcpp/algorithm.h>
#include <emlabcpp/allocator/pool.h>
#include <emlabcpp/static_vector.h>

#pragma once

namespace emlabcpp
{

template < typename Coroutine, std::size_t N >
class round_robin_executor
{
public:
        using container   = static_vector< Coroutine, N >;
        using output_type = typename Coroutine::output_type;
        using input_type  = typename Coroutine::input_type;

        [[nodiscard]] bool register_coroutine( Coroutine cor )
        {
                if ( cont_.full() ) {
                        return false;
                }

                cont_.push_back( std::move( cor ) );

                return true;
        }

        [[nodiscard]] bool empty() const
        {
                return cont_.empty();
        }

        [[nodiscard]] bool finished() const
        {
                return all_of( cont_, [&]( const Coroutine& cor ) -> bool {
                        return cor.done();
                } );
        }

        Coroutine run( pool_interface* )
        {
                std::size_t i = 0;

                EMLABCPP_LOG( "Run of executor started" );

                while ( !finished() ) {

                        Coroutine& cor = cont_[i];
                        i              = ( i + 1 ) % cont_.size();

                        if ( cor.done() ) {
                                drop_coroutine( i );
                                continue;
                        }

                        const output_type* out = cor.get_output();

                        if ( out == nullptr ) {
                                EMLABCPP_LOG( "output from coroutine is nullptr!" );
                                co_return;
                        }

                        input_type resp = co_yield *out;
                        cor.store_input( resp );

                        if ( !cor.tick() ) {
                                EMLABCPP_LOG(
                                    "Coroutine " << cor.address()
                                                 << " failed to tick, bailing out" );
                                co_return;
                        }
                }
        }

private:
        void drop_coroutine( std::size_t i )
        {
                using std::swap;

                swap( cont_[i], cont_.back() );

                cont_.pop_back();
        }

        container cont_;
};

}  // namespace emlabcpp
