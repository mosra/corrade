#include "Utility/Resource.h"
#include "Utility/utilities.h"

static int staticResourceInitializer() {
    RESOURCE_INITIALIZE(ResourceTestData)
    return 1;
} AUTOMATIC_INITIALIZER(staticResourceInitializer)
