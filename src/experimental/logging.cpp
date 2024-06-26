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

#include "emlabcpp/experimental/logging.h"

#include "emlabcpp/experimental/logging/color.h"
#include "emlabcpp/experimental/logging/eabi_logger.h"

#ifdef EMLABCPP_USE_LOGGING

namespace emlabcpp
{
eabi_logger INFO_LOGGER{ { set_stdout{ true }, set_stderr{ false }, INFO_LOGGER_COLORS } };
eabi_logger DEBUG_LOGGER{ { set_stdout{ false }, set_stderr{ false }, DEBUG_LOGGER_COLORS } };
eabi_logger ERROR_LOGGER{ { set_stdout{ false }, set_stderr{ true }, ERROR_LOGGER_COLORS } };
}  // namespace emlabcpp

#elif defined EMLABCPP_USE_NONEABI_LOGGING
#endif
