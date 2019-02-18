// Some random comment

// {{includes}}

// And after

#pragma ACME path .
#pragma ACME local Dir
#pragma ACME noexpand noexpand.h

#include <Dir/Bla.h>
#include "Local.h"
#include <cstring>

void stuffAtTheEnd(Integer = SomeValue);

#pragma ACME comments off
/* to test the below // {includes} get preserved */

#ifdef INCLUDE_ALSO_HEAVY_STUFF
// {{includes}}

#include "Oof.h"
#endif
