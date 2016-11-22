
#include <stdlib.h>
#include <malloc.h>
#include "Skip.h"

#define MAXLEVEL 16

skiplist fkSkipNew( int (*cmp)(const void*,const void*) )
{
	skiplist l ;
	int i ;
	l = (skiplist) malloc( sizeof(struct _skiplist) 
			+ MAXLEVEL * sizeof(skipnode) ) ;
	if ( l ) 
	{
		l->level = 0 ;
		l->n = 0 ;
		l->cmp = cmp ;
		l->head->bwd = 0 ;
		l->head->data = 0 ;
		for ( i = 0 ; i < MAXLEVEL ; i++ )
			l->head->fwd[i] = 0 ;
	}
	return l ;
}

static short rndLevel(void)
{
	short l ;
	unsigned long r = rand() ;

	l=0 ;
	while ( r && (!(r&3)) )
		{ l++ ; r >>= 2 ; }
	return l<MAXLEVEL ? l : MAXLEVEL-1 ;
}

skipnode fkSkipInsert( skiplist l , void *data , void *key )
{
	short k ;
	int s ;
	skipnode  p,q,new_node ;
	skipnode  update[MAXLEVEL] ;

	p = l->head ;
	k = l->level ;
	do {
		for ( ; ( q = p->fwd[k] ) 
			&& ( (s=l->cmp(q->data,key))<0 || (s==0 && q->data < data)) ;
			 p = q )
		;
		update[k] = p ;
	} while ( --k >= 0 ) ;

	k = rndLevel() ;
	if ( k > l->level ) 
	{
		k = ++l->level ;
		update[k] = l->head ;
	}
	new_node = q = (skipnode)malloc(
		sizeof( struct _skipnode ) +k*sizeof(skipnode) ) ;

	if ( !q )
		return 0 ;

	q->data = data ;
	do {
		p = update[k] ;
		q->fwd[k] = p->fwd[k] ;
		p->fwd[k] = q ;
	} while ( --k >= 0 ) ;

	q->bwd = ( p != l->head ) ? p : 0 ;
	if ( q->fwd[0] ) 
		q->fwd[0]->bwd = q ;

	l->n++ ;
	return new_node ;
}

skipnode fkSkipSearch( skiplist l , const void *key )
{
	short k ;
	skipnode  p,q ;

	if ( !l )
		return 0 ;

	p = l->head ;
	k = l->level ;

	q=0 ;
	do {
		for ( ; ( q = p->fwd[k] ) 
			&& ( l->cmp(q->data,key)<0 ) ;
			 p = q )
		;
	} while ( --k >= 0 ) ;
	
	return ( q && (0==l->cmp(q->data,key))) ? q : 0 ;	
}

void fkSkipDelete( skiplist l , const void *data , const void *key )
{
	int s ;
	short k,m ;
	skipnode update[MAXLEVEL] ;
	skipnode p,q ;

	p = l->head ;
	m = k = l->level ;

	do {
		for ( ; ( q = p->fwd[k] ) 
			&& ( (s=l->cmp(q->data,key))<0 || (s==0 && q->data < data)) ;
			 p = q )
		;
		update[k] = p ;
	} while ( --k >= 0 ) ;
	if ( !q || q->data != data )
		return ;

	for ( k=0 ; k<=m && ( p = update[k] )->fwd[k] == q ; k++ )
		p->fwd[k] = q->fwd[k] ;		

	if ( q->fwd[0] )
		q->fwd[0]->bwd = q->bwd ;
	free(q) ;

	while ( ! l->head->fwd[m] && ( m > 0 ))
		--m ;
	l->level = m ;
	l->n-- ;
}

int fkSkipCount(skiplist l )
{
	return l->n ;
}

void fkSkipKill( skiplist l )
{
	skipnode n , q ;

	for ( n = l->head->fwd[0] ; n ; n = q )
	{
		q = n->fwd[0] ;
		free(n) ;
	}
	free(l) ;
}
