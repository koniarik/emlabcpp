#include <emlabcpp/types.h>
#include <emlabcpp/view.h>
#include <gtest/gtest.h>
#include <list>

using namespace emlabcpp;

static_assert( std::same_as< mapped_t< std::vector< int >, std::identity >, const int& > );
static_assert( std::same_as< mapped_t< std::list< int >, std::identity >, const int& > );
static_assert( std::same_as< mapped_t< view< const int* >, std::identity >, const int& > );
// TODO: decide this:
// static_assert(
//    std::same_as< mapped_t< view< const int ( * )[128] >, std::identity >, const int& > );
//
