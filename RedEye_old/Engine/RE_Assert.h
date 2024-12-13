#ifndef __RE_ASSERT__
#define __RE_ASSERT__

// Define Assert to call EA_ASSERT
#include <EAAssert/eaassert.h>
#define RE_ASSERT(expression) EA_ASSERT(expression)

#endif // !__RE_ASSERT__