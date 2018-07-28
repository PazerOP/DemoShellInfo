#pragma once
#include "Helpers/LargeIntegerCreator.h"
#include "Helpers/PropVariantCreator.h"
#include "Helpers/PropVariantSafe.h"

#include <propvarutil.h>

#define PP_CAT(a, b) PP_CAT_I(a, b)
#define PP_CAT_I(a, b) PP_CAT_II(~, a ## b)
#define PP_CAT_II(p, res) res

#define UNIQUE_NAME(base) PP_CAT(base, __COUNTER__)

#define LOCK_GUARD(mutex) std::lock_guard<decltype(mutex)> UNIQUE_NAME(__lock_guard)(mutex)