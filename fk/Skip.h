#ifndef SKIP_H
#define SKIP_H

typedef struct _skipnode
{
    void         *data ;
    struct _skipnode *bwd ;
    struct _skipnode *fwd[1] ;
} *skipnode ;

typedef struct _skiplist
{
    int (*cmp)(const void*,const void*) ;
    int      n ;
    short     level ;
    struct _skipnode head[1] ;
} *skiplist ;

extern skiplist fkSkipNew( int (*c)(const void*,const void*) ) ;

extern skipnode fkSkipInsert( skiplist l , void *data , void *key ) ;
extern void fkSkipDelete( skiplist l , const void *data , const void *key ) ;
extern void fkSkipKill( skiplist l ) ;

extern skipnode fkSkipSearch( skiplist l , const void *key ) ;
extern int fkSkipCount( skiplist l ) ;

#define fkSkipFirst(l) ( (l) ? (l)->head->fwd[0] : NULL )
#define fkSkipNext(p) ((p)->fwd[0])
#define fkSkipData(type,node) ((type)((node)?((node)->data):(void*)0))
#define fkSkipCompare(list,x,y) ((list)->cmp((x),(y)))

#endif
