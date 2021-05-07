#ifndef __RED_BLACK_TREE_H
#define __RED_BLACK_TREE_H
#include<stdlib.h>

typedef struct rb_red_blk_node {
  void* key;
  void* info;
  int red; /* if red=0 then the node is black */
  struct rb_red_blk_node* left;
  struct rb_red_blk_node* right;
  struct rb_red_blk_node* parent;
} rb_red_blk_node;


/* Compare(a,b) should return 1 if *a > *b, -1 if *a < *b, and 0 otherwise */
/* Destroy(a) takes a pointer to whatever key might be and frees it accordingly */
typedef struct rb_red_blk_tree {
  int (*Compare)(const void* a, const void* b); 
  void (*DestroyKey)(void* a);
  void (*DestroyInfo)(void* a);
  void (*PrintKey)(const void* a);
  void (*PrintInfo)(void* a);
  /*  A sentinel is used for root and for nil.  These sentinels are */
  /*  created when RBTreeCreate is caled.  root->left should always */
  /*  point to the node which is the root of the tree.  nil points to a */
  /*  node which should always be black but has aribtrary children and */
  /*  parent and no key or info.  The point of using these sentinels is so */
  /*  that the root and nil nodes do not require special cases in the code */
  rb_red_blk_node* root;             
  rb_red_blk_node* nil;              
} rb_red_blk_tree;

rb_red_blk_tree* RBTreeCreate(int  (*CompFunc)(const void*, const void*),
			     void (*DestFunc)(void*), 
			     void (*InfoDestFunc)(void*), 
			     void (*PrintFunc)(const void*),
			     void (*PrintInfo)(void*));
rb_red_blk_node * RBTreeInsert(rb_red_blk_tree*, void* key, void* info);
void RBTreePrint(rb_red_blk_tree*);
void RBDelete(rb_red_blk_tree* , rb_red_blk_node* );
void RBTreeDestroy(rb_red_blk_tree*);
rb_red_blk_node* TreePredecessor(rb_red_blk_tree*,rb_red_blk_node*);
rb_red_blk_node* TreeSuccessor(rb_red_blk_tree*,rb_red_blk_node*);
rb_red_blk_node* RBExactQuery(rb_red_blk_tree*, void*);
void NullFunction(void*);
void TreeForEach(rb_red_blk_tree *tree, void (*Func)(void*), rb_red_blk_node *x);
void TreeForEach1p(rb_red_blk_tree *tree, void (*Func)(void*,void*), void *param, rb_red_blk_node *x);
#endif