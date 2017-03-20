#include <Corrade/Utility/Debug.h>

#ifndef CORRADE_TESTSUITE_TARGET_XCTEST
int main()
#else
int CORRADE_VISIBILITY_EXPORT corradeTestMain(int, char**);
int corradeTestMain(int, char**)
#endif
{
    Corrade::Utility::Fatal{42}
        << "Cannot make the definite answer to be" << 75.74f
        << Corrade::Utility::Debug::nospace << ". It will always be" << 42
        << Corrade::Utility::Debug::nospace << ".";
}
