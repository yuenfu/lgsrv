#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <malloc.h>
#include <net/if.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
#include <unistd.h>

#include "NetDrv.h"
#include "Skip.h"

typedef struct _Param
{
	char	*key;
	char	*value;
} Param;

static skiplist	sl=0;

static int _cmp( const void *p, const void *q )
{
	const Param *param=p;
	const char  *s2 = q;

	if ( !p || !param->key )
		return 1;

	return strcmp(param->key,s2);
}

void _addParam( char *key, char *value )
{
	skipnode N;
	Param    *param;

	if ( !sl )
		return;

	N=fkSkipSearch(sl,key);
	if ( N )
	{
		param=fkSkipData(Param*,N);
		free( param->value );
	}
	else
	{
		param=malloc(sizeof(Param));
		memset(param,0,sizeof(Param));
		param->key=strdup(key);
		fkSkipInsert(sl,param,param->key);
	}
	param->value=strdup(value);
}

static void WriteParamsToFile( void )
{
	FILE	 *fp;
	skipnode N, N2;
	Param	 *param;

	if ( !sl )
		return;

	fp=fopen( "/usr/data/htdocs/param.txt", "w" );
	if ( !fp )
		return;

	for( N=fkSkipFirst(sl); N; N=N2 )
	{
		N2=fkSkipNext(N);
		param=fkSkipData(Param*,N);
		fprintf(fp,"%s=%s\n",param->key,param->value);
	}
	fclose(fp);
}

void RunFreeParams(char *param)
{
	char	*p, *p2;
	char	*pin=param;
	int		dosave=0;

	while(1)
	{
		p=strchr(pin,'&');
		if ( p )
			*p=0;
		_Cnv26(pin);
		p2=strchr(pin,'=');
		if ( !p2 )
		{
			if ( !strcasecmp(pin,"SAVE") )
				dosave=1;
			if ( p )
			{
				pin=p+1;
				continue;
			}
			break;
		}
		*p2=0;
		if ( !strcasecmp(pin,"SAVE") )
		{
			dosave=atoi(p2+1);
		}
		else if ( strlen(p2+1) )
		{
			_addParam(pin,p2+1);
		}
		if ( p )
		{
			pin=p+1;
			continue;
		}
		break;
	}

	if( dosave )
		WriteParamsToFile();
}

static void LoadParamsFromFile( void )
{
	FILE		*fp;
	char		buff[1024];
	char		*p;
	int			len;

	if ( !sl )
		return;

	fp=fopen( "/usr/data/htdocs/param.txt", "r" );
	if ( !fp )
		return;
	while( fgets( buff, 1024, fp ) )
	{
		len=strlen(buff);
		if ( !len < 2 )
			continue;
		p=strchr(buff+len-2,'\r'); if(p)*p=0;
		p=strchr(buff+len-2,'\n'); if(p)*p=0;

		p=strchr(buff,'=');
		if ( !p )
			continue;

		_addParam(buff,p+1);
	}
	fclose(fp);
}

void	InitParams( void )
{
	sl=fkSkipNew( _cmp );
	LoadParamsFromFile( );
}
