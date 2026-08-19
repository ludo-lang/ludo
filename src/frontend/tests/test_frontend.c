#include "ludo_frontend.h"
#include "test.h"

#include <string.h>

LUDO_TEST_MAIN({ LUDO_CHECK(strcmp(ludo_frontend_version(), "") != 0); })
