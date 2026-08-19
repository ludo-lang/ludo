#include "ludo_interp.h"
#include "test.h"

#include <string.h>

LUDO_TEST_MAIN({ LUDO_CHECK(strcmp(ludo_interp_version(), "") != 0); })
