#include <cassert>

#if defined( EMLABCPP_ASSERT_FUNC )

#define EMLABCPP_ASSERT( cond ) EMLABCPP_ASSERT_FUNC( cond )

#elif defined( EMLABCPP_ASSERT_NATIVE )

#define EMLABCPP_ASSERT( cond ) assert( cond )

#else

#define EMLABCPP_ASSERT( cond ) static_cast< void >( 0 )

#endif
