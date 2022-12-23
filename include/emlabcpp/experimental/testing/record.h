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
#include "emlabcpp/experimental/testing/base.h"
#include "emlabcpp/experimental/testing/coroutine.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/match.h"
#include "emlabcpp/static_vector.h"

#pragma once

namespace emlabcpp::testing
{
class record
{
        bool errored_ = false;

public:
        [[nodiscard]] bool errored() const
        {
                return errored_;
        }

        void fail()
        {
                errored_ = true;
        }

        void success() const
        {
        }

        void expect( const bool val )
        {
                val ? success() : fail();
        }
};

}  // namespace emlabcpp::testing
