#include "emlabcpp/experimental/logging.h"

#ifdef EMLABCPP_USE_LOGGING

namespace emlabcpp
{
}  // namespace emlabcpp

#elif defined EMLABCPP_USE_NONEABI_LOGGING

namespace emlabcpp
{
logger LOGGER = {};

void log_to_global_logger( std::string_view sv )
{
        LOGGER.write( sv );
}
}  // namespace emlabcpp

#endif
