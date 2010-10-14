#pragma once

#include "../globals.h"
#include "allocator.h"
#include "lexer.h"

struct block;

struct compiler {
	/*
	 * Parser stuff
	 */
	int index;
	int count;

	/*
	 * Memory allocator
	 */
	struct allocator allocator;
};

void compiler_setup(void);
struct compiler *compiler_create();
void compiler_destroy(struct compiler *compiler);

static inline void *compiler_alloc(struct compiler *compiler, size_t length)
{
	return allocator_alloc(&compiler->allocator, length);
}
