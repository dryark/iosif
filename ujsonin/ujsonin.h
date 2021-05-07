#ifndef __UJSONIN_H
#define __UJSONIN_H
#include"string-tree.h"
typedef struct jnode_s jnode;

#define NODEBASE char type; jnode *parent;
// type 1=hash, 2=str

#define SAFE(x) if(pos>=len) { endstate=x; goto Done; }
#define SAFEGET(x) if(pos>=len) { endstate=x; goto Done; } let=data[pos++];
#define SPACES for( int j=0;j<depth;j++ ) printf("  ");

struct jnode_s { NODEBASE };

typedef struct node_hash_s { NODEBASE
    string_tree *tree;
} node_hash;

typedef struct node_str_s { NODEBASE
    char *str;
    int len;
} node_str;

typedef struct node_arr_s { NODEBASE
    jnode *head;
    jnode *tail;
    int count;
} node_arr;

typedef struct parser_state_s {
    int state;
} parser_state;

node_hash *parse( char *data, int len, parser_state *beginState, int *err );
jnode *node_hash__get( node_hash *self, char *key, int keyLen );
void jnode__dump( jnode *self, int depth );
char *slurp_file( char *filename, int *outlen );
void ujsonin_init();
void jnode__dump_env( jnode *self );
void node_hash__dump_to_makefile( node_hash *self, char *prefix );
void node_hash__delete( node_hash *self );
node_hash *parse_with_default( char *file, char *def, char **d1, char **d2 );
#endif