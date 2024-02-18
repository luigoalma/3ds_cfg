#pragma once
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t base64_size(size_t len);
bool base64_encode(char* out, size_t* outsize, const void* data, size_t len);

#ifdef __cplusplus
}
#endif
