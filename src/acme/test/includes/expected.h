// Some random comment

#include <cstdint>
#include <cstring>

// And after

enum EnumInRelativePath {};
/* This has to be added *after* Relative.h for correct definition order */
int function(EnumInRelativePath);
typedef int Integer;
enum: std::int32_t { SomeValue = 42 };

void stuffAtTheEnd(Integer = SomeValue);
