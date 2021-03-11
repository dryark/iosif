#ifndef __UNARCHIVER_H
#define __UNARCHIVER_H
#include<stdint.h>
#include"archiver.h"

struct ptrArr_t {
  uint16_t max;
  uint16_t count;
  void **ptr;
  struct ptrArr_t *next;
};
typedef struct ptrArr_t ptrArr;

typedef struct {
  uint8_t scan;
  uint8_t bytesPerOffset;
  uint8_t bytesPerRef;
  uint32_t maxObNum;
  uint32_t rootObNum;
  uint32_t offsetsPos;
  
  //tARR *obs;
  //tBASE **obsArr;
  ptrArr *obs;
  uint8_t *data;
  int len;
} bpList;

bpList *bpList__new( uint8_t *data, int len );
tBASE *dearchive( uint8_t *data, uint32_t len );
#endif