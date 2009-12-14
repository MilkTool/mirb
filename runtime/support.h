#pragma once
#include "runtime.h"

void *__stdcall rt_support_alloc_scope(size_t size);

rt_value __cdecl rt_support_interpolate(size_t argc, rt_value argv[]);

rt_value __cdecl rt_support_array(size_t argc, rt_value argv[]);

rt_value __stdcall rt_support_get_const(rt_value obj, rt_value name);
void __stdcall rt_support_set_const(rt_value obj, rt_value name, rt_value value);

rt_value __stdcall rt_support_define_string(const char* string);

