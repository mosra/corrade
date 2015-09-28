enum class Flag: std::uint32_t;

/** @debugoperatorenum{Flag} */
Corrade::Utility::Debug& operator<<(Corrade::Utility::Debug& debug, Flag value);

class Foo {
    enum class Flag: std::uint32_t;
};

/** @debugoperatorclassenum{Foo,Foo::Flag} */
Corrade::Utility::Debug& operator<<(Corrade::Utility::Debug& debug, Foo::Flag value);
