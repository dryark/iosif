// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

#ifndef __PLIST_H
#define __PLIST_H
#include<stdint.h>
#include"types.h"
#include"../bytearr.h"

uint16_t tDICT__plist( tOFFS *offs, bytearr *ba, tDICT *self );
void bpList__encodeTrailer( bytearr *ba, uint32_t numObs, uint32_t offsetsPos, uint32_t rootNum, uint8_t offBytes );
uint16_t tBASE__plist( tOFFS *offs, bytearr *ba, tBASE *ob );
uint8_t *tBASE__tobin( tBASE *self, uint32_t *len );
uint8_t *tBASE__archivebin( tBASE *self, uint32_t *len );
uint8_t *encodeSize( uint32_t val, int *len );
#endif