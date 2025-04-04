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

#include "emlabcpp/algorithm.h"
#include "emlabcpp/enumerate.h"
#include "emlabcpp/static_vector.h"

#include <deque>
#include <gtest/gtest.h>
#include <mutex>
#include <span>
#include <thread>

namespace emlabcpp
{

template < typename T >
std::string to_string( T const& val )
{
        std::stringstream ss;
        ss << val;
        return ss.str();
}

struct protocol_test_fixture : public ::testing::Test
{
        virtual void generate_name( std::ostream& ) const = 0;
};

inline std::ostream& operator<<( std::ostream& os, protocol_test_fixture const& f )
{
        f.generate_name( os );
        return os;
}

template < typename T >
inline std::string pretty_name()
{
        std::unique_ptr< char, decltype( std::free )& > name{ nullptr, std::free };

        int tmp = 0;
        name.reset( abi::__cxa_demangle( typeid( T ).name(), nullptr, nullptr, &tmp ) );
        return std::string{ name.get() };
}

inline void exec_protocol_test_fixture_test(
    std::vector< std::function< protocol_test_fixture*() > > const& factories )
{
        for ( auto [i, factory] : enumerate( factories ) ) {
                std::unique_ptr< protocol_test_fixture > const test_ptr{ factory() };
                ::testing::RegisterTest(
                    "protocol_test_fixture",
                    ( "test " + ::testing::PrintToString( *test_ptr ) ).c_str(),
                    nullptr,
                    nullptr,
                    __FILE__,
                    __LINE__,
                    factory );
        }
}

void protocol_def_tests();
void protocol_sophisticated_tests();
void protocol_register_map_tests();

struct thread_safe_queue
{
        std::deque< std::vector< uint8_t > > buff_;
        std::mutex                           lock_;

public:
        void insert( std::span< uint8_t > inpt )
        {
                std::lock_guard const g{ lock_ };
                buff_.emplace_back( inpt.begin(), inpt.end() );
        }

        bool empty()
        {
                std::lock_guard const g{ lock_ };
                return buff_.empty();
        }

        static_vector< uint8_t, 64 > pop()
        {
                std::lock_guard const g{ lock_ };
                if ( buff_.empty() )
                        return {};
                static_vector< uint8_t, 64 > res;
                copy( buff_.front(), std::back_inserter( res ) );
                buff_.pop_front();
                return res;
        }
};

}  // namespace emlabcpp
