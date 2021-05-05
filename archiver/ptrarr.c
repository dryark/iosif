// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#include<stdint.h>
#include<stdlib.h>
#include"ptrarr.h"

ptrArr *ptrArr__new() {
  ptrArr *self = (ptrArr *) malloc( sizeof( ptrArr ) );
  self->max = 30;
  self->count = 0;
  self->ptr = (void **) malloc( sizeof( void * ) * 30 );
  self->next = 0;
  return self;
}

ptrArr *ptrArr__add( ptrArr *self, void *item ) {
  if( self->next ) {
    ptrArr *cur = self;
    while( cur->next ) cur = cur->next;
    ptrArr__add( cur, item );
    return cur;
  }
  if( self->count == self->max ) {
    self->next = ptrArr__new();
    ptrArr *next = self->next;
    ptrArr__add( next, item );
    return next;
  }
  self->ptr[ self->count++ ] = item;
  return 0;
}

void *ptrArr__get( ptrArr *self, uint32_t pos ) {
  if( pos < self->count ) return self->ptr[ pos ];
  ptrArr *cur = self;
  while( pos >= cur->count ) {
    pos -= cur->count;
    cur = cur->next;
    if( !cur ) return 0;
  }
  return cur->ptr[ pos ];
}

void ptrArr__del( ptrArr *self ) {
  free( self->ptr );
  if( self->next ) ptrArr__del( self->next );
  free( self );
}

void *ptrArr__iter( ptrArr **cur, uint32_t *pos ) {
  if( *pos < (*cur)->count ) {
    return (*cur)->ptr[ (*pos)++ ];
  }
  while( *pos >= (*cur)->count ) {
    *pos -= (*cur)->count;
    *cur = (*cur)->next;
    if( !*cur ) return 0;
  }
  return (*cur)->ptr[ (*pos)++ ];
}