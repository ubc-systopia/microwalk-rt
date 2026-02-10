#pragma once

#include <stdint.h>
#include <stdio.h>

#include "Microwalk/PinTracer/FilterEntry.h"

extern void init_target(const char *script_name);

extern void init_target_run();

extern void target_run(const char *buffer, size_t buffer_size);

extern void cleanup_target_run(void);

extern void cleanup_target(void);

extern void init_target_filter(void);

extern void cleanup_target_filter(void);

#define FilterTypeByteCodeHandler   (FilterTypeWhiteList | FilterTypeControlFlow | FilterTypeJump)
#define FilterTypeMainCall          (FilterTypeWhiteList | FilterTypeControlFlow | FilterTypeCall)
#define FilterTypeBinaryLookup      (FilterTypeWhiteList | FilterTypeDataAccess  | FilterTypeRead)

extern int PinNotifyTestcaseStart(int t);

extern int PinNotifyTestcaseEnd(void);

extern int PinNotifyStackPointer(uint64_t spMin, uint64_t spMax);

extern int PinNotifyAllocation(uint64_t address, uint64_t size);

extern int PinNotifyFilter(FilterEntry *addr, size_t length);

void mw_exit_error(const char *format, ...);
