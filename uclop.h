// Copyright (c) 2021 David Helkowski
// github.com/nanoscopic/uclop
// Anti-Corruption License
#ifndef __UCLOP_H
#define __UCLOP_H
#define UOPT(a,b) uopt__new(a,b,1,0)
#define UOPT_REQUIRED(a,b) uopt__new(a,b,1,1)
#define UOPT_FLAG(a,b) uopt__new(a,b,2,0)
#define UOPT_FLAG_REQUIRED(a,b) uopt__new(a,b,2,1)

typedef struct uopts_s uopt;
struct uopts_s {
    int type;
    int required;
    char *name;
    char *descr;
    char *def; // default value
    char *val;
    uopt *next;
};

typedef struct ucmd_s ucmd;

struct ucmd_s {
    char *name;
    char *descr;
    void ( *handler )( ucmd * );
    ucmd *next; 
    uopt *head; uopt *tail;
    int argc;
    char **argv;
    char *extrahelp;
};

typedef struct uclop_s {
    ucmd *head;
    ucmd *tail;
    char *default_cmd;
} uclop;

uopt *uopt__new( char *name, char *descr, int type, int req );

void ucmd__addopt( ucmd *self, uopt *opt );

ucmd *uclop__addcmd( uclop *self, char *cmd_name, char *cmd_descr,
  void ( *default_handler )( ucmd * ), uopt *cmd_opts[] );

uclop *uclop__new( void ( *default_handler)( ucmd * ), uopt *default_opts[] );

void ucmd__run( ucmd *cmd );

void ucmd__usage_inline( ucmd *self );

void ucmd__usage( ucmd *self );

void uopt__usage( uopt *opt );

void uclop__usage( uclop *self, char *prog );

uopt *ucmd__find_opt( ucmd *self, char *name );

char *ucmd__get( ucmd *self, char *name );

void ucmd__ensure_required( ucmd *self );

void uclop__run( uclop *self, int argc, char *argv[] );
#endif