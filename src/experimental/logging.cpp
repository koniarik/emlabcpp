#include "emlabcpp/experimental/logging.h"

#ifdef EMLABCPP_USE_LOGGING

namespace emlabcpp
{
gpos_logger INFO_LOGGER{ { set_stdout{ true }, set_stderr{ false }, INFO_LOGGER_COLORS } };
gpos_logger DEBUG_LOGGER{ { set_stdout{ false }, set_stderr{ false }, DEBUG_LOGGER_COLORS } };
gpos_logger ERROR_LOGGER{ { set_stdout{ false }, set_stderr{ false }, ERROR_LOGGER_COLORS } };
}  // namespace emlabcpp

#elif defined EMLABCPP_USE_NONEABI_LOGGING
#endif
