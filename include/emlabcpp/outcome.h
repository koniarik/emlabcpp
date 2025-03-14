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

#include "emlabcpp/result.h"

namespace emlabcpp
{

/// Unique type representing failure, should not be used directly
struct failure_type
{
        static constexpr uint8_t id = 3;
};

/// Global constant of failure_type for direct usage
inline failure_type FAILURE;

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
/// To check the state of `outcome`, preferred way is to use operator== or operator!= as follows:
/// `if( out == FAILURE ){ ...`
class [[nodiscard]] outcome
{
public:
        /// Constructs outcome with SUCCESS
        constexpr outcome( success_type )
          : state_( success_type::id )
        {
        }

        /// Constructs outcome with FAILURE
        constexpr outcome( failure_type )
          : state_( failure_type::id )
        {
        }

        /// Constructs outcome with ERROR
        constexpr outcome( error_type )
          : state_( error_type::id )
        {
        }

        /// Constructs `outcome` out of `result`, result with SUCCESS becames outcome with SUCCESS,
        /// and result with ERROR becames outcome with ERROR.
        constexpr outcome( result res )
          : state_( res.get_state() )
        {
        }

        /// Returns current state of the outcome as numerical id, this corresponds to
        /// success_type::id, failure_type::id, error_type::id respectively
        [[nodiscard]] constexpr uint8_t get_state() const
        {
                return state_;
        }

        /// Convers `outcome` to `result`, SUCCESS/ERROR type are converted as is, FAILURE is
        /// converted to SUCCESS.
        constexpr result has_errored_result() const
        {
                if ( state_ == error_type::id )
                        return ERROR;
                return SUCCESS;
        }

private:
        uint8_t state_;
};

/// Compare two outcomes for equality, uses get_state()
constexpr bool operator==( const outcome& lh, const outcome& rh )
{
        return lh.get_state() == rh.get_state();
}

/// Compare two outcomes for inequality, uses get_state()
constexpr bool operator!=( const outcome& lh, const outcome& rh )
{
        return !( lh == rh );
}

/// Filter concept that allows only success_type, failure_type, or error_type
template < typename T >
concept outcome_native_constant = std::same_as< T, success_type > ||
                                  std::same_as< T, failure_type > || std::same_as< T, error_type >;

/// Compares outcome with any of the constant types
template < outcome_native_constant T >
constexpr bool operator==( const outcome& outc, T )
{
        return outc.get_state() == T::id;
}

/// Compares outcome with any of the constant types
constexpr bool operator==( outcome_native_constant auto c, const outcome& outc )
{
        return outc == c;
}

/// Compares outcome for inequality with any of the constant types
constexpr bool operator!=( const outcome& outc, outcome_native_constant auto c )
{
        return !( outc == c );
}

/// Compares outcome for inequality with any of the constant types
constexpr bool operator!=( outcome_native_constant auto c, const outcome& outc )
{
        return !( outc == c );
}

}  // namespace emlabcpp
