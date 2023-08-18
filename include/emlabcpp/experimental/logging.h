///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#pragma once

#include <string_view>

namespace emlabcpp
{
consteval std::string_view stem_of( const std::string_view file )
{
        const std::string_view res{ file };
        const std::size_t      pos = res.find_last_of( '/' );
        if ( pos == std::string_view::npos ) {
                return res;
        }
        return res.substr( pos + 1 );
}

}  // namespace emlabcpp

#ifdef EMLABCPP_USE_LOGGING

#include "emlabcpp/experimental/logging/eabi_logger.h"

namespace emlabcpp
{

extern eabi_logger DEBUG_LOGGER;
extern eabi_logger INFO_LOGGER;
extern eabi_logger ERROR_LOGGER;

}  // namespace emlabcpp

#define EMLABCPP_DEBUG_LOG( ... ) \
        EMLABCPP_EABI_LOG_IMPL( emlabcpp::DEBUG_LOGGER, __FILE__, __LINE__, __VA_ARGS__ )
#define EMLABCPP_INFO_LOG( ... ) \
        EMLABCPP_EABI_LOG_IMPL( emlabcpp::INFO_LOGGER, __FILE__, __LINE__, __VA_ARGS__ )
#define EMLABCPP_ERROR_LOG( ... ) \
        EMLABCPP_EABI_LOG_IMPL( emlabcpp::ERROR_LOGGER, __FILE__, __LINE__, __VA_ARGS__ )

#elif defined EMLABCPP_USE_NONEABI_LOGGING

#include "emlabcpp/experimental/logging/noneabi_logger.h"

namespace emlabcpp
{
extern noneabi_logger DEBUG_LOGGER;
extern noneabi_logger INFO_LOGGER;
extern noneabi_logger ERROR_LOGGER;
}  // namespace emlabcpp

#define EMLABCPP_DEBUG_LOG( ... ) EMLABCPP_NONEABI_LOG_IMPL( emlabcpp::DEBUG_LOGGER, __VA_ARGS__ )
#define EMLABCPP_INFO_LOG( ... ) EMLABCPP_NONEABI_LOG_IMPL( emlabcpp::INFO_LOGGER, __VA_ARGS__ )
#define EMLABCPP_ERROR_LOG( ... ) EMLABCPP_NONEABI_LOG_IMPL( emlabcpp::ERROR_LOGGER, __VA_ARGS__ )

#else

#define EMLABCPP_INFO_LOG( ... )
#define EMLABCPP_DEBUG_LOG( ... )
#define EMLABCPP_ERROR_LOG( ... )

#endif
