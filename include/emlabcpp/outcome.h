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
#pragma once

#include "./result.h"
#include "./status.h"

namespace emlabcpp
{

enum class outcome_e : uint8_t
{
        SUCCESS = 0,
        FAILURE = 1,
        ERROR   = 2,
};

/// `outcome` represents tristate resut of some operation, which can succeed, fail or produce an
/// error. `outcome` is designed to be used with SUCCESS, FAILURE, and ERROR global constants. The
/// common semantics should be:
///  - SUCCESS - operation succeeded
///  - FAILURE - operation failed, but only in way that can be expected and is not sign of a problem
///  - ERROR - operaiton errored, in a way that is a sign of a problem
/// For example, for parsing function, FAILURE should be used to mark that the parsed string is not
/// valid, ERROR should be used to mark that unexpected internal error happend (out-of-bound access
/// due to mistake in algorithm)
///
/// Supports comparison: outcome ot = ...; if(ot == outcome::SUCCESS) { ... }
struct [[nodiscard]] outcome : status< outcome, outcome_e, outcome_e::SUCCESS >
{
        using status::status;
        using enum outcome_e;

        constexpr outcome( result const r ) noexcept
          : status( r == result::SUCCESS ? outcome_e::SUCCESS : outcome_e::ERROR )
        {
        }

        constexpr outcome( result_e const r ) noexcept
          : status( r == result::SUCCESS ? outcome_e::SUCCESS : outcome_e::ERROR )
        {
        }

        constexpr operator result() const noexcept
        {
                return value() == outcome_e::SUCCESS ? result::SUCCESS : result::ERROR;
        }
};

}  // namespace emlabcpp
