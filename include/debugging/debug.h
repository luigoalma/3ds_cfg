#pragma once
#include <stddef.h>

#ifdef DEBUG_ENABLED
#ifdef __cplusplus
extern "C" {
#endif

void DebugOut_Init() __attribute__((noinline));
void DebugOut_Exit() __attribute__((noinline));
void DebugOut_Write(const void* buf, size_t size) __attribute__((noinline));
void DebugOut_WriteCmdBuf() __attribute__((noinline));

#ifdef __cplusplus
}
#endif
#else
#define DebugOut_Init() do{}while(0)
#define DebugOut_Exit() do{}while(0)
#define DebugOut_Write(x,y) do{}while(0)
#define DebugOut_WriteCmdBuf() do{}while(0)
#endif
