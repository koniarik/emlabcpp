#include "emlabcpp/concepts.h"

#include <gtest/gtest.h>
#include <ostream>

using namespace emlabcpp;

static_assert( arithmetic_like< float > );
static_assert( arithmetic_like< int > );
static_assert( !arithmetic_like< std::string > );

static_assert( gettable_container< std::array< int, 4 > > );
static_assert( gettable_container< std::tuple< int, float > > );
static_assert( !gettable_container< std::vector< int > > );

static_assert( range_container< std::array< int, 4 > > );
static_assert( !range_container< std::tuple< int, float > > );
static_assert( range_container< std::vector< int > > );

static_assert( static_sized< std::array< int, 4 > > );
static_assert( static_sized< std::tuple< int, float > > );
static_assert( !static_sized< std::vector< int > > );

static_assert( ostreamlike< std::ostream > );
