
#include "./operations_counter.h"

namespace emlabcpp
{

std::size_t operations_counter::move_count    = 0;  // NOLINT
std::size_t operations_counter::copy_count    = 0;  // NOLINT
std::size_t operations_counter::destroy_count = 0;  // NOLINT
std::size_t operations_counter::default_count = 0;  // NOLINT

}  // namespace emlabcpp
