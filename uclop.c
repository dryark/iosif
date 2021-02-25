// Copyright (c) 2021 David Helkowski
// github.com/nanoscopic/uclop
// Anti-Corruption License

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

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

uopt *uopt__new( char *name, char *descr, int type, int req ) {
    uopt *self = ( uopt * ) calloc( sizeof( uopt ), 1 );
    self->type = type;
    self->name = name;
    self->descr = descr;
    self->required = req;
    return self;
}

void ucmd__addopt( ucmd *self, uopt *opt ) {
    if( !self->head ) {
        self->head = self->tail = opt;
        return;
    }
    self->tail->next = opt;
    self->tail = opt;
}

ucmd *uclop__addcmd( uclop *self, char *cmd_name, char *cmd_descr, void ( *default_handler )( ucmd * ), uopt *cmd_opts[] ) {
    ucmd *cmd = ( ucmd * ) calloc( sizeof( ucmd ), 1 );
    cmd->name = cmd_name;
    cmd->descr = cmd_descr;
    cmd->handler = default_handler;
    if( cmd_opts ) for( int i=0;i<50;i++ ) {
        uopt *opt = cmd_opts[i];
        if( opt == NULL ) break;
        ucmd__addopt( cmd, opt );
    }
    if( !self->head ) {
        self->head = self->tail = cmd;
        return cmd;
    }
    self->tail->next = cmd;
    self->tail = cmd;
    return cmd;
}

uclop *uclop__new( void ( *default_handler)( ucmd * ), uopt *default_opts[] ) {
    uclop *self = ( uclop * ) calloc( sizeof( uclop ), 1 );
    char *cmdDescr = "Show Usage";
    uclop__addcmd( self, "default", cmdDescr, default_handler, default_opts );
    return self;
}

void ucmd__run( ucmd *cmd ) {
    cmd->handler( cmd );
}

void ucmd__usage_inline( ucmd *self ) {
    uopt *opt = self->head;
    while( opt ) {
        if( opt->required ) printf(" %s val", opt->name );
        else {
            if( opt->type == 2 ) printf(" [%s]", opt->name );
            else printf(" [%s val]", opt->name );
        }
        opt = opt->next;
    }
    if( self->extrahelp ) printf(" %s", self->extrahelp );
    printf("\n");
}

void ucmd__usage( ucmd *self ) {
    uopt *opt = self->head;
    while( opt ) {
        printf("  %s %s\n", opt->name, opt->required ? "REQUIRED" : "" );
        printf("    %s\n", opt->descr );
        opt = opt->next;
    }
}

void uopt__usage( uopt *opt ) {
    printf("  %s %s\n", opt->name, opt->required ? "REQUIRED" : "" );
    printf("    %s\n", opt->descr );
}

void uclop__usage( uclop *self, char *prog ) {
    printf("Usage:\n");
    ucmd *cmd = self->head;
    while( cmd ) {
        int isDefault = ( cmd == self->head );
        char *cmdName = isDefault ? "" : cmd->name;
        printf("%s%s%s", prog, isDefault ? "" : " ", cmdName );
        ucmd__usage_inline( cmd );
        printf("  %s\n", cmd->descr );
        ucmd__usage( cmd );
        cmd = cmd->next;
    }
}

uopt *ucmd__find_opt( ucmd *self, char *name ) {
    uopt *opt = self->head;
    while( opt ) {
        if( !strcmp( name, opt->name ) ) return opt;
        opt = opt->next;
    }
    return NULL;
}

char *ucmd__get( ucmd *self, char *name ) {
    uopt *opt = self->head;
    while( opt ) {
        if( !strcmp( name, opt->name ) ) return opt->val;
        opt = opt->next;
    }
    return NULL;
}

void ucmd__ensure_required( ucmd *self ) {
    uopt *opt = self->head;
    int missing = 0;
    while( opt ) {
        if( opt->required && !opt->val ) {
            missing = 1;
            fprintf(stderr,"Required option %s missing\n", opt->name);
            uopt__usage( opt );
        }
        opt = opt->next;
    }
    if( missing ) exit(1);
}

void uclop__run( uclop *self, int argc, char *argv[] ) {
    argc--;
    
    // Pull cmd name out of args
    char *cmdStr = "default";
    int start = 1;
    if( argc >= 1 && argv[1][0] != '-' ) {
        start++;
        cmdStr = argv[1];
    }
    
    // Find the cmd object using the name
    ucmd *cmd = self->head;
    while( cmd ) {
        if( !strcmp( cmdStr, cmd->name ) ) break;
        cmd = cmd->next;
    }
    if( !cmd ) {
        uclop__usage( self, argv[0] );
        return;
    }
    
    // Store arguments
    int i;
    for( i=start;i<=argc;i++ ) {
        char *arg = argv[i];
        if( arg[0] == '-' ) {
            uopt *opt = ucmd__find_opt( cmd, arg );
            if( !opt ) {
                fprintf( stderr, "Unknown option %s\n", arg );
                exit(1);
            }
            if( opt->type == 2 ) opt->val = "true";
            else opt->val = argv[++i];
        }
        else break;
    }
    cmd->argc = argc - i + 1;
    cmd->argv = &argv[i];
    
    // Check that all required options are present
    ucmd__ensure_required( cmd );
  
    if( !cmd->handler ) uclop__usage( self, argv[0] );
    else ucmd__run( cmd );
}