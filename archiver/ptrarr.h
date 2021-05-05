// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#ifndef __PTRARR_H
#define __PTRARR_H
#include<stdint.h>

struct ptrArr_t {
  uint16_t max;
  uint16_t count;
  void **ptr;
  struct ptrArr_t *next;
};
typedef struct ptrArr_t ptrArr;

ptrArr *ptrArr__new();

ptrArr *ptrArr__add( ptrArr *self, void *item );

void *ptrArr__get( ptrArr *self, uint32_t pos );

void ptrArr__del( ptrArr *self );

void *ptrArr__iter( ptrArr **cur, uint32_t *pos );

#ifdef UDEBUG
#define duprintf(...) printf(__VA_ARGS__)
#else
#define duprintf(...) (void)0
#endif

#endif