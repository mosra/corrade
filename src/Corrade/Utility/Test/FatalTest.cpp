#include <Corrade/Utility/Debug.h>

int main() {
    Corrade::Utility::Fatal{42}
        << "Cannot make the definite answer to be" << 75.74f
        << Corrade::Utility::Debug::nospace << ". It will always be" << 42
        << Corrade::Utility::Debug::nospace << ".";
}
