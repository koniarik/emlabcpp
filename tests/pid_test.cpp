///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
/// associated documentation files (the "Software"), to deal in the Software without restriction,
/// including without limitation the rights to use, copy, modify, merge, publish, distribute,
/// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or substantial
/// portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
/// NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
/// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
/// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#include "emlabcpp/pid.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

using tpid = pid< float >;

TEST( PID, base )
{
        tpid::config conf;
        tpid         my_pid{ float{ 0 }, conf };

        EXPECT_EQ( my_pid.get_output(), 0 );
}

TEST( PID, simple )
{
        tpid::config conf{
            .coefficients{
                .p = 0.2f,
                .i = 0.01f,
                .d = 0.01f,
            },
            .limits{ 0.f, 100.f },
        };

        tpid my_pid{ float{ 0 }, conf };

        float val     = 0.f;
        float desired = 100.f;

        for ( std::size_t i = 0; i < 1000; i++ ) {
                my_pid.update( static_cast< float >( i ), val, desired );
                val = my_pid.get_output();
        }

        EXPECT_NEAR( my_pid.get_output(), desired, 0.1f );
}
