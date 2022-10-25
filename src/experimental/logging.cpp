#include "emlabcpp/experimental/logging.h"

#ifdef EMLABCPP_USE_LOGGING

namespace emlabcpp
{
}  // namespace emlabcpp

#elif defined EMLABCPP_USE_NONEABI_LOGGING

namespace emlabcpp
{
logger LOGGER = {};
}  // namespace emlabcpp

#endif
