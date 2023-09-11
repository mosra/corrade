/* This file should be included after a correspnding wrapped #include <cassert>
   in the library */

/* Exclude all assertions by default. The libraries should then whitelist a
   subset using #pragma ACME forget. */
#pragma ACME enable CORRADE_ASSERT
#pragma ACME enable CORRADE_DEBUG_ASSERT
#pragma ACME enable CORRADE_CONSTEXPR_ASSERT
#pragma ACME enable CORRADE_CONSTEXPR_DEBUG_ASSERT
#pragma ACME enable CORRADE_ASSERT_OUTPUT
#pragma ACME enable CORRADE_DEBUG_ASSERT_OUTPUT
#pragma ACME enable CORRADE_ASSERT_UNREACHABLE
#pragma ACME enable CORRADE_DEBUG_ASSERT_UNREACHABLE
#pragma ACME enable CORRADE_INTERNAL_ASSERT
#pragma ACME enable CORRADE_INTERNAL_DEBUG_ASSERT
#pragma ACME enable CORRADE_INTERNAL_CONSTEXPR_ASSERT
#pragma ACME enable CORRADE_INTERNAL_CONSTEXPR_DEBUG_ASSERT
#pragma ACME enable CORRADE_INTERNAL_ASSERT_OUTPUT
#pragma ACME enable CORRADE_INTERNAL_DEBUG_ASSERT_OUTPUT
#pragma ACME enable CORRADE_INTERNAL_ASSERT_EXPRESSION
#pragma ACME enable CORRADE_INTERNAL_DEBUG_ASSERT_EXPRESSION
#pragma ACME enable CORRADE_INTERNAL_ASSERT_UNREACHABLE
#pragma ACME enable CORRADE_INTERNAL_DEBUG_ASSERT_UNREACHABLE
