// Copyright (c) 2021 David Helkowski
// Anti-Corruption License

//#include<CoreFoundation/CoreFoundation.h>
#include<stdlib.h>
#include<dirent.h>
#include<stdio.h>
#include<string.h>
#include<stdint.h>

#include"uclop.h"
#include"ujsonin/ujsonin.h"
#include"cmd_dtxdump.h"
#include"dtxmsg.h"
#include"archiver/types.h"
#include"archiver/archiver.h"
#include"archiver/unarchiver.h"
#include"cfutil.h"

static ucmd *g_cmd = NULL;

void run_dtxdump( ucmd *cmd  ) {
  char *jsonFile = cmd->argv[0];
  char *dataFile = cmd->argv[1];
  
  dmsgList *list = dmsgList__new();
  dump__messages( list, jsonFile, dataFile, 0 );
  
  exit(0);
}

void dump_folder( dmsgList *list, char *folder ) {
  struct dirent *entry;
  DIR *dh = opendir( folder );
  if( !dh ) return;
  
  char hasTo = 0;
  char hasFrom = 0;
  while( ( entry = readdir( dh ) ) != NULL ) {
    char *name = entry->d_name;
    if( name[0] == '.' ) continue;
    
    if( entry->d_type == DT_REG ) {
      //printf("%s/%s\n", folder, name );
      if( !strcmp( name, "to-device.json" ) ) {
        hasTo = 1;
      }
      if( !strcmp( name, "from-device.json" ) ) {
        hasFrom = 1;
      }
    }
    if( entry->d_type == DT_DIR ) {
      //printf("DIR %s\n", name );
      char sub[400];
      sprintf( sub, "%s/%s", folder, name );
      dump_folder( list, sub );
    }
    //printf("%s\n", dir->d_name);
  }
  
  if( hasTo ) {
    char toJson[1000];
    char toBin[1000];
    sprintf(toJson,"%s/to-device.json",folder);
    sprintf(toBin,"%s/to-device.bin",folder);
    printf("%s\n%s\n======================\n",toJson,toBin);
    dump__messages( list, toJson, toBin, 0 );
  }
  
  if( hasFrom ) {
    char fromJson[1000];
    char fromBin[1000];
    sprintf(fromJson,"%s/from-device.json",folder);
    sprintf(fromBin,"%s/from-device.bin",folder);
    printf("%s\n%s\n======================\n",fromJson,fromBin);
    dump__messages( list, fromJson, fromBin, 1 );
  }
  
  closedir(dh);
}

int dmsg__compare( const void *av, const void *bv ) {
  dmsg **ap = (dmsg **) av;
  dmsg **bp = (dmsg **) bv;
  dmsg *a = *ap;
  dmsg *b = *bp;
  return strcmp( a->time, b->time );
}

void run_dtxdumpfolder( ucmd *cmd ) {
  char *folder = cmd->argv[0];
 
  dmsgList *list = dmsgList__new();
  dump_folder( list, folder );
  
  dmsg **flat = dmsgList__toPtrs( list );
  
  qsort( flat, list->count, sizeof( dmsg * ), dmsg__compare );
  
  for( uint16_t i=0;i<list->count;i++ ) {
    dmsg *msg = flat[i];
    if( msg == NULL ) {
      printf("null err\n");
      exit(0);
    }
    decode_message( msg->ptr, msg->len, msg->time, msg->dir );
  }
  
  exit(0);
}

dmsg *dmsg__new( uint8_t *ptr, uint16_t len, char *time, char dir ) {
  dmsg *self = (dmsg *) calloc( sizeof( dmsg ), 1 );
  self->ptr = ( uint8_t * ) malloc( len );
  memcpy( self->ptr, ptr, len );
  self->len = len;
  self->time = time;
  self->dir = dir;
  return self;
}

void dmsg__delete( dmsg *self ) {
  free( self->ptr );
  free( self->time );
  free( self );
}

dmsgList *dmsgList__new() {
  dmsgList *self = (dmsgList *) calloc( sizeof( dmsgList ), 1 );
  return self;
}

dmsg **dmsgList__toPtrs( dmsgList *self ) {
  dmsg **ptrs = ( dmsg ** ) calloc( sizeof( dmsg * ) * self->count, 1 );
  dmsg *msg = self->head;
  uint16_t i = 0;
  while( msg ) {
    ptrs[i++] = msg;
    msg = msg->next;
  }
  return ptrs;
}

void dmsgList__add( dmsgList *self, dmsg *msg ) {
  self->count++;
  
  if( !self->head ) {
    self->head = self->tail = msg;
    return;
  }
  self->tail->next = msg;
  self->tail = msg;
}

void dump__messages( dmsgList *list, char *jsonFile, char *binFile, char dir ) {
  printf("json File: %s\n", jsonFile );
  printf("bin File: %s\n", binFile );
  
  int flen;
  char *fdata = slurp_file( (char *) jsonFile, &flen );
  if( !fdata ) {
    printf("Could not read %s\n", jsonFile );
    return;
  }
  
  int dlen;
  char *ddata = slurp_file( (char *) binFile, &dlen );
  if( !ddata ) {
    printf("Could not read %s\n", binFile );
  }
  
  //printf("json len: %i\n", flen);
  //printf("data len: %i\n", dlen );
  
  int ferr;
  
  int jsonStart = 0;
  int jsonEnd = 0;
  int pos = 0;
  int prevOffset = 0;
  char *timeDup = NULL;
  char *prevTime = NULL;
  while(1) {
    //printf("Json start:%i\n", pos );
    jsonStart = pos;
    for( ;pos<flen;pos++ ) {
      if( fdata[ pos ] == '\n' ) break; 
    }
    jsonEnd = pos-1;
    pos++;
    
    int blockLen = jsonEnd - jsonStart + 1;
    
    //printf("Json to parse:%.*s\n", blockLen, &fdata[ jsonStart ] );
    
    int offPos = jsonStart;
    for(;offPos<flen;offPos++ ) {
      if( !strncmp( &fdata[offPos], "OffsetInDump", 12 ) ) {
        break;
      }
    }
    offPos += 14;
    int offEndPos = offPos;
    for(;offEndPos<flen;offEndPos++ ) {
      if( fdata[offEndPos] < '0' || fdata[offEndPos] > '9' ) break;
    }
    
    int tOffPos = jsonStart;
    for(;tOffPos<flen;tOffPos++ ) {
      if( !strncmp( &fdata[tOffPos], "TimeReceived", 12 ) ) {
        break;
      }
    }
    tOffPos += 15;
    int tOffEndPos = tOffPos;
    for(;tOffEndPos<flen;tOffEndPos++ ) {
      char let = fdata[tOffEndPos];
      if( let == '"' ) break;
    }
    char timeStr[50];
    sprintf(timeStr, "%.*s", tOffEndPos-tOffPos, &fdata[tOffPos] );
    //printf("Time: %s\n", timeStr );
    timeDup = strdup( timeStr );
    
    //printf("Offset: %.*s\n", offEndPos-offPos, &fdata[offPos] );
    
    //node_hash *root = parse( &fdata[ start ], blockLen - 1, NULL, &ferr );
    //printf("Parsed\n");
    
    /*node_str *offsetN = (node_str *) node_hash__get( root, "OffsetInDump", 12 );
    char *offsetStr = offsetN->str;
    char offsetStr2[10];
    memcpy( offsetStr2, offsetStr, offsetN->len );
    offsetStr2[ offsetN->len -1 ] = 0x00;
    int offset = atoi( offsetStr2 );
    
    int msgLen = offset - prevOffset;**/
    char offsetStr[10];
    sprintf(offsetStr, "%.*s", offEndPos-offPos, &fdata[offPos] );
    int offset = atoi( offsetStr );
    
    int msgLen = offset-prevOffset+1;
    
    if( offset != 0 ) {
      //printf("Message - From: %i to %i; len=%i\n", prevOffset, offset, msgLen );
      dmsg *msg = dmsg__new( (uint8_t *) &ddata[ prevOffset ], msgLen, prevTime, dir );
      dmsgList__add( list, msg );
      //decode_message( (uint8_t *) &ddata[ prevOffset ], msgLen, prevTime );
    }
    
    prevOffset = offset;
    prevTime = timeDup;
    
    if( pos == flen ) break;
  }
  dmsg *msg = dmsg__new( (uint8_t *) &ddata[ prevOffset ], dlen - prevOffset, prevTime, dir );
  dmsgList__add( list, msg );
  //decode_message( (uint8_t *) &ddata[ prevOffset ], dlen - prevOffset, prevTime );
      
  //node_hash *root = parse( fdata, flen, NULL, &ferr );
}

void dtxmsg__dump( dtxmsg *msg ) {
  //printf("  Fragment: %i/%i\n", msg->fragId+1, msg->fragCount );
  //printf("  Length: %i\n", msg->length );
  printf("id=\"%i.%i.%i\" ", msg->channelCode, msg->id, msg->conversationIndex );
  if( msg->expectsReply ) printf("wantreply=\"1\" " );
}

void dtxmsg__dumpack( dtxmsg *msg ) {
  //printf("Ack: ");
  //printf("  Fragment: %i/%i\n", msg->fragId+1, msg->fragCount );
  //printf("  Length: %i\n", msg->length );
  printf("id=\"%i.%i.%i\" ", msg->channelCode, msg->id, msg->conversationIndex );
  //if( msg->expectsReply ) printf("  Expects Reply\n" );
}

void payloadHeader__dump( dtxpayload *header ) {
  if( !header->type && !header->auxlen && !header->totlen && !header->flags ) {
    return;
  }
  
  //printf("Payload - ");
  printf("itype=\"%i\" ", header->type );
  //printf("  Auxlen: %i\n", header->auxlen );
  //printf("  Totlen: %i\n", header->totlen );
  if( header->flags ) printf("flags=\"%i\" ", header->flags );
}

void decode_message( uint8_t *buffer, int len, char *time, char dir ) {
  dtxmsg msg;
  int pos = 0;
  
  printf("<msg ");
  memcpy( &msg, buffer, sizeof(dtxmsg) );
  
  pos += sizeof( dtxmsg );
  
  if( msg.magic != 0x1F3D5B79 ) { fprintf(stderr, "invalid magic: %x\n", msg.magic); return; }
  
  uint32_t dataLen = msg.length;
  
  dtxpayload payloadHeader;
  memcpy( &payloadHeader, &buffer[pos], sizeof( payloadHeader ) );
  
  if( !payloadHeader.type && !payloadHeader.totlen ) {
    if( dir == DIR_TO ) printf("type=\"ack\" ");
    else printf("type=\"rack\" ");
    
    dtxmsg__dumpack( &msg );
    
    printf(" />\n\n");
    return;
  }
  else {
    printf("type=\"msg\" ");
    if( dir == DIR_TO ) printf("dir=\"out\" ");
    else printf("dir=\"in\" ");
    
    dtxmsg__dump( &msg );
  }
  
  payloadHeader__dump( &payloadHeader );
  
  printf(">\n");
  
  pos += sizeof( payloadHeader );
  
  if( payloadHeader.auxlen ) {
    tARR *auxArr = (tARR *) deserialize2t( &buffer[pos], payloadHeader.auxlen );
    if( auxArr == NULL ) {
      printf("Aux: decode failure\n");
    }
    else {
      //printf("  <aux><![CDATA[");
      //if( auxArr->count == 1 ) {
        tBASE *arg = auxArr->head;
        for( int i=0;i<auxArr->count;i++ ) {
          printf("  <arg><![CDATA[");
          tBASE__dump_nocr( arg, 1 );
          printf("]]></arg>\n");
          arg = arg->next;
        }
        //tBASE__dump( (tBASE *) auxArr->head, 1 );
      //}
      //else {
      //  tBASE__dump( (tBASE *) auxArr, 1 );
      //}
      //printf("]]></aux>\n");
    }
    pos += payloadHeader.auxlen;
  }
  
  if( payloadHeader.totlen ) {
    int msgLen = payloadHeader.totlen - payloadHeader.auxlen;
    //printf("Raw msg:(%d)%.*s\n", msgLen, msgLen, &buffer[pos] );
    
    if( msgLen ) {
      tBASE *tMsg = dearchive( (uint8_t *) &buffer[pos], msgLen );
      if( tMsg == NULL ) {
        if( buffer[pos] == '<' ) {
          printf("  <msg xml=\"1\"><![CDATA[%.*s", msgLen, &buffer[pos] );
          printf("]]></msg>\n");
        }
        else {
          printf("  <msg raw=\"1\"><![CDATA[");
          for( int i=0;i<msgLen;i++ ) {
            char let = buffer[pos+i];
            printf("%02x", let );
          }
          printf("]]></msg>\n");
        }
      }
      else {
        printf("  <msg><![CDATA[");
        tBASE__dump( tMsg, 1 );
        printf("]]></msg>\n");
      }
    }
  }
  
  printf("</msg>\n");
  //uint32_t auxlen = payloadHeader->auxlen;
}