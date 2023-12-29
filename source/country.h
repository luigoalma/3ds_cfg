#pragma once
#include <3ds/types.h>

#ifdef __cplusplus
extern "C" {
#endif

Result CountryStrToEnum(const char* str, CFG_CountryCode* code);
Result CountryEnumToStr(CFG_CountryCode code, char* str);

#ifdef __cplusplus
}
#endif
