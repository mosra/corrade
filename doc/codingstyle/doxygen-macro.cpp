/** @brief Set new value */
#ifdef DOXYGEN_GENERATING_OUTPUT
template<class T> void set(T value);
#else
template<class T> type std::enable_if<std::is_convertible<T, int>::value, void>::type set(T value);
#endif
