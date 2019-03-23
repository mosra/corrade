// Some random comment

/* An {{includes}} that has nothing to add will get ignored */
// {{includes}}

// {{includes}}

// And after

#pragma ACME path .
#pragma ACME local Dir
#pragma ACME noexpand noexpand.h

#include <Dir/Bla.h>
#include "Local.h"
#include <cstring>

void stuffAtTheEnd(Integer = SomeValue);

/* this ifdef stays here even though the file doesn't have an include guard */
#ifdef _WIN32
#include <windows.h>
#endif

#pragma ACME comments off
/* to test the below // {includes} get preserved */

#ifdef INCLUDE_ALSO_HEAVY_STUFF
// {{includes}}

#include "Oof.h"
#endif
