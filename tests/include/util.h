#include "emlabcpp/zip.h"

#include <gtest/gtest.h>

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

}  // namespace emlabcpp
