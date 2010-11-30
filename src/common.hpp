#pragma once
#ifdef _MSC_VER
	#define WIN32 1
	#define MSVC 1
	#pragma warning(disable:4355)
	#pragma warning(disable:4996)
	#pragma warning(disable:4731)
	#pragma warning(disable:4530)
	#pragma warning(disable:4200) // TODO: Remove this
	#define mirb_external(name)
	#define __thread __declspec(thread)
	#define __noreturn __declspec(noreturn)
#else
	#define __noreturn __attribute__((noreturn)) 
	#define mirb_external(name) __asm__(name)
#endif

#ifdef WIN32
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <excpt.h>
#else
	#define __stdcall __attribute__((__stdcall__))
	#define __fastcall __attribute__((__fastcall__))
	#define __cdecl __attribute__((__cdecl__))

	#include <sys/mman.h>
	#include <sys/stat.h>
#endif

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <cassert>

#ifdef VALGRIND
	#ifndef NO_GC
		#define NO_GC
	#endif
#endif

namespace Mirb
{
	typedef uint8_t char_t;

	static const size_t memory_align = 8;

	static inline size_t align(size_t value, size_t alignment)
	{
		alignment -= 1;
		return (value + alignment) & ~alignment;
	};

	static inline size_t align_down(size_t value, size_t alignment)
	{
		return value & ~(alignment - 1);
	};
	
	static inline void __noreturn runtime_abort_with_message(std::string message);
	
	static inline void runtime_abort_with_message(std::string message)
	{
		std::cout << message << "\n";
		std::abort();
	}
	
	#define mirb_stringify(value) #value
	#define mirb_runtime_abort_internal(expression, file, line, message) runtime_abort_with_message(file ":" mirb_stringify(line) ": " + std::string(message))
	#define mirb_runtime_abort(message) mirb_runtime_abort_internal(expression, __FILE__, __LINE__, message)
	#define mirb_runtime_assert(expression) assert(expression)

	#ifdef DEBUG
		#define mirb_debug_assert(expression) assert(expression)
		#define mirb_debug_abort(message) mirb_runtime_abort_internal(expression, __FILE__, __LINE__, message)
	#else
		#define mirb_debug_assert(expression)
		#define mirb_debug_abort(message)
	#endif

	#ifndef __has_builtin
	  #define __has_builtin(x) 0
	#endif

	#if !__has_builtin(__builtin_unreachable)
		static inline void __noreturn __builtin_unreachable();
		static inline void __builtin_unreachable()
		{
			mirb_runtime_abort("Unreachable code");
		}
	#endif
};
