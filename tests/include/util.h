#include "emlabcpp/static_vector.h"
#include "emlabcpp/zip.h"

#include <deque>
#include <gtest/gtest.h>
#include <mutex>
#include <thread>

namespace emlabcpp
{

struct protocol_test_fixture : public testing::Test
{
        virtual void generate_name( std::ostream& ) const = 0;
};

inline std::ostream& operator<<( std::ostream& os, const protocol_test_fixture& f )
{
        f.generate_name( os );
        return os;
}

template < typename T >
inline std::string pretty_name()
{
        int         tmp  = 0;
        char*       name = abi::__cxa_demangle( typeid( T ).name(), nullptr, nullptr, &tmp );
        std::string res{ name };
        free( name );
        return res;
}

inline void exec_protocol_test_fixture_test(
    const std::vector< std::function< protocol_test_fixture*() > >& factories )
{
        for ( auto [i, factory] : enumerate( factories ) ) {
                std::unique_ptr< protocol_test_fixture > test_ptr{ factory() };
                testing::RegisterTest(
                    "protocol_test_fixture",
                    ( "test " + ::testing::PrintToString( *test_ptr ) ).c_str(),
                    nullptr,
                    nullptr,
                    __FILE__,
                    __LINE__,
                    factory );
        }
}

struct thread_safe_queue
{
        std::deque< std::vector< uint8_t > > buff_;
        std::mutex                           lock_;

public:
        void insert( std::span< uint8_t > inpt )
        {
                std::lock_guard g{ lock_ };
                buff_.emplace_back( inpt.begin(), inpt.end() );
        }

        bool empty()
        {
                std::lock_guard g{ lock_ };
                return buff_.empty();
        }

        static_vector< uint8_t, 64 > pop()
        {
                std::lock_guard g{ lock_ };
                if ( buff_.empty() ) {
                        return {};
                }
                static_vector< uint8_t, 64 > res;
                std::copy( buff_.front().begin(), buff_.front().end(), std::back_inserter( res ) );
                buff_.pop_front();
                return res;
        }
};

}  // namespace emlabcpp
