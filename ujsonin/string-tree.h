// Copyright (C) 2018 David Helkowski

#ifndef __STRING_TREE_H
#define __STRING_TREE_H
#include<stdint.h>

uint32_t fnv1a_len( char *str, int strlen );

struct snode_s {
	char *str;
	int strlen;
	char dataType;
	void *data;
	struct snode_s *next;
};
typedef struct snode_s snode;

snode *snode__new( char *newstr, void *newdata, char dataType, snode *newnext );
void snode__delete( snode *self );
snode *snode__new_len( char *newstr, int strlen, void *newdata, char dataType, snode *newnext );

#define XJR_ARR_MAX 5
typedef struct xjr_arr_s xjr_arr;
struct xjr_arr_s {
	int count;
	int max;
	void **items;
	char *types;
};
xjr_arr *xjr_arr__new();
void xjr_arr__double( xjr_arr *self );
void xjr_arr__delete( xjr_arr *self );

#define XJR_KEY_ARR_MAX 5
typedef struct xjr_key_arr_s xjr_key_arr;
struct xjr_key_arr_s {
	int count;
	int max;
	char **items;
	int *sizes;
};
xjr_key_arr *xjr_key_arr__new();
void xjr_key_arr__double( xjr_key_arr *self );
void xjr_key_arr__delete( xjr_key_arr *self );

struct string_tree_s {
	void *tree;
};
typedef struct string_tree_s string_tree;
snode *string_tree__rawget_len( string_tree *self, char *key, int keylen );
string_tree *string_tree__new();
void string_tree__delete( string_tree *self );
void *string_tree__get_len( string_tree *self, char *key, int keylen, char *dataType );
void string_tree__delkey_len( string_tree *self, char *key, int keylen );

void string_tree__store_len( string_tree *self, char *key, int keylen, void *node, char dataType );

void IntDest(void *); int IntComp(const void *,const void *);
void IntPrint(const void* a); void InfoPrint(void *); void InfoDest(void *);

xjr_key_arr *string_tree__getkeys( string_tree *self );
void string_tree__getkeys_rec( void *snodeV, void *arrV );
#endif
