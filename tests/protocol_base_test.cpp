// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
#include "emlabcpp/protocol/message.h"
#include "emlabcpp/view.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

// NOLINTNEXTLINE
TEST( Protocol, message_conversion )
{
        std::array< uint8_t, 4 > data   = { 53, 3, 64, 85 };
        std::array< uint8_t, 4 > suffix = { 0, 0, 0, 0 };
        protocol_message< 4 >    m{ data };

        auto buff = std::array< uint8_t, 8 >( m );

        EXPECT_EQ( view_n( buff.begin(), 4 ), view{ data } );

        EXPECT_EQ( view_n( buff.begin() + 4, 4 ), view{ suffix } );
}
