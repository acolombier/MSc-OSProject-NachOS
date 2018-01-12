#ifndef MALLOC_H
#define MALLOC_H

#include "mem_alloc.h"
#include "userlib.h"

#include "mem_alloc_types.h"

static int __mem_alloc_init_flag=0;

void *malloc(size_t size);

void free(void *p);

void *calloc(size_t nmemb, size_t size);

void *realloc(void *ptr, size_t size);

#endif
