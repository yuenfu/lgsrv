#include <stdio.h>
#include <signal.h>
#include <stdarg.h>

#define SkLineData		struct _JsonData

#include "NetDrv.h"

extern	JsonVars	json;

static void makeHeader(unsigned char* paramArrayOfByte, int paramInt1,
	unsigned char paramByte,
	short paramShort1, short paramShort2, short paramShort3, int paramInt2)
{
paramArrayOfByte[0] = 13;
paramArrayOfByte[1] = (unsigned char)paramInt1;
paramArrayOfByte[2] = paramByte;
paramArrayOfByte[3] = (unsigned char)paramInt2;
//paramShort1=htons(paramShort1);
//paramShort2=htons(paramShort2);
//paramShort3=htons(paramShort3);
memcpy(paramArrayOfByte+4,&paramShort1,2);
memcpy(paramArrayOfByte+6,&paramShort2,2);
memcpy(paramArrayOfByte+8,&paramShort3,2);
}

static void sendJSONPacket(SkLine *l, char *paramString )
{
int paramInt=strlen(paramString);
static int id=0;
unsigned char arrayOfByte[512];

	memset(arrayOfByte,0,12);
	makeHeader(arrayOfByte, 4, id, (short)0, (short)1, (short)paramInt, 0);
	memcpy(arrayOfByte+12,paramString,paramInt);
	id++;
	if(id == 255)
		id = 0;
#if 0
{
int i;
unsigned char *p;
printf(">> %p\n",l);
for(i=0, p=arrayOfByte; i<paramInt+12; i++, p++)
printf("%02x ",*p);
printf("\n");
}
#endif
	_WritePacket( l, arrayOfByte, paramInt+12 );
}

static	SkLine	*cmd_line=0;
static	SkLine	*session_line=0;

static	void	_sessionClose( SkLine *l, int pt, void *own, void *sys )
{
	if ( l == session_line )
		session_line=0;

	Log(16,"json: session disconnected\r\n");
}

static void _establish4002( SkTimerType tid, void *own );

static	void	_cmdClose( SkLine *l, int pt, void *own, void *sys )
{
	cmd_line=0;

	json.conntime=-1;

	Log(16,"json: cmdline disconnected\r\n");
	if ( session_line )
		skDisconnect( session_line );
	session_line=0;
	_establish4002(0,0);
}

static	SkTimerType	alive_tid=0;

static void _sendAlive( SkTimerType tid, void *own )
{
	alive_tid=0;
	if ( cmd_line )
		sendJSONPacket(cmd_line, "{\"SESSION\":\"ALIVE\"}" );
	alive_tid=skAddTimer( 5000, _sendAlive, 0 );
}

static	int	GetJson( char *in, char **dest, char *property )
{
	char	*p = strstr(in,property);
	char	*pe;
	int		sl;

	if ( !p )
		return 0;
	p += strlen(property)+1;	/* go to value */
	pe = strchr(p,'"');
	if ( *dest )
		free( *dest );
	sl=pe-p;
	*dest = malloc(sl+1);
	memcpy(*dest,p,sl);
	memset((*dest)+sl,0,1);

	return 1;
}

static time_t	cmd_conntime=0;

static	void	_cmdData( SkLine *l, int pt, void *own, void *sys )
{
	SkPacket		*pck=sys;
	char			*data=pck->data;
	int				size=pck->len;
	short			len;
	int				i=0;
	char			*session=0;

	Log(16,"json: cmdline Data received\r\n");

	json.conntime=(int)(time(0) - cmd_conntime);

	while( size > 12 )
	{
		memcpy(&len,data+8,2);
		if ( size < len+12 )
			break;
		data[ len +12 -1 ] = 0;	/* replace last char with 0 */

		Log(16,"json: %s\r\n",data+12);

		if ( !strcmp( data+12, "{\"SESSION\":\"CHECK_POSITIVE\"" ) )
		{
			sendJSONPacket(cmd_line,"{\"DIAGNOSIS\":{\"RECENT\":\"REQUEST\"}}");
			data += len+12;
			size -= (len+12);
			i++;
			continue;
		}
		if ( !strcmp( data+12, "{\"DIAGNOSIS\":{\"RECENT\":\"false\"}" ) )
		{
			data += len+12;
			size -= (len+12);
			i++;
			continue;
		}

		if( GetJson( data+12, &json.robot_state, "\"ROBOT_STATE\":" ) )
		{
			NewState( json.robot_state );
		}
		GetJson( data+12, &json.turbo, "\"TURBO\":" );
		GetJson( data+12, &json.repeat, "\"REPEAT\":" );
		GetJson( data+12, &json.batt, "\"BATT\":" );
		GetJson( data+12, &json.mode, "\"MODE\":" );
		GetJson( data+12, &json.nickname, "\"NICKNAME\":" );
		GetJson( data+12, &json.version, "\"VERSION\":" );
		GetJson( data+12, &session, "\"SESSION\":" );

		data += len+12;
		size -= (len+12);
		i++;
	}

	if ( json.batt )
	{
		char	tmp[128];
		int		batt = atoi(json.batt);
		sprintf(tmp,"%d",batt*20);
		json.battperc=strdup(tmp);
	}

#if 0
	printf("_clientData: %d bytes\r\n",pck->len);

	for( i=0, data=pck->data; i<pck->len; i++ )
	{
		if ( !(i%16) && i )
			printf("\r\n");
		printf("%02x ",data[i]);
	}
	printf("\r\n");
#endif
	return;
}

static void	_establish4000( void );

static	void	_sessionData( SkLine *l, int pt, void *own, void *sys )
{
	SkPacket		*pck=sys;
	char			*data=pck->data;
	int				size=pck->len;
	short			len;
	int				i=0;

	Log(16,"json: session Data received '%s'\r\n",len>12?data+12:"....");
	while( size > 12 )
	{
		memcpy(&len,data+8,2);
		if ( size < len+12 )
			break;
		data[ len +12 -1 ] = 0;	/* replace last char with 0 */

		Log(1,"session: %s\r\n",data+12);

		if ( !strcmp( data+12, "{\"CONNECT\":\"ENABLE\"") )
        {
            _establish4000( );
        }
		data += len+12;
		size -= (len+12);
		i++;
	}

#if 0
	printf("_sessionData: %d bytes\r\n",pck->len);

	for( i=0, data=pck->data; i<pck->len; i++ )
	{
		if ( !(i%16) && i )
			printf("\r\n");
		printf("%02x ",data[i]);
	}
	printf("\r\n");
#endif
	return;
}

static void _asyConn( SkLine *l, int pt, void *own, void *sys )
{
	SkLine	**rl=own;
	int	k=(int)sys;
	rl[0] = l+1;
	if ( k == ASY_CONNECTED )
		rl[1]=l;
	else
		rl[1]=0;
}

static SkLine	*RawConnect( char *service, char *host )
{
	SkLine *l[2]={0,0};

	Log(16,"json: connect '%s:%s'\r\n",host,service);
	skAsyConnect( host, service, 500, _asyConn, l );

	while( l[0] == 0 )
		skTimeoutStep(100);

	return l[1];
}

#ifdef JSON_SIMUL
static void json_simu( void )
{
	char		data[]="____________{\"CONNECT_INIT\":[{\"ROBOT_STATE\":\"CHARGING\"},{\"TURBO\":\"true\"},{\"REPEAT\":\"false\"},{\"BATT\":\"4\"},{\"MODE\":\"ZZ\"},{\"RESERVATION\":{\"RESP_RSVSTATE\":\"false\"}},{\"NICKNAME\":\"HOMBOT\"},{\"VERSION\":\"11128\"},{\"BLACKBOX\":{\"RECENT_ABS\":\"false\"}},{\"DIAGNOSIS\":{\"RECENT\":\"false\"}},{\"VOICEMODE\":\"MALE\"}]}";
	SkPacket	pck;
	short		len;

	pck.data = data;
	pck.len = strlen(data);
	len = pck.len - 12;
	data[8] = len%256;
	data[9] = len/256;
	_clientData( 0, 0, 0, &pck );
}
#endif

static void _establish4002( SkTimerType tid, void *own )
{
	SkLine	*l;

	if ( session_line )
		return;

#ifdef JSON_REDIRECT
	l=RawConnect("4002","192.168.28.30");
#else
	l=RawConnect("4002","127.0.0.1");
#endif
	if (!l )
	{
		skAddTimer(1000,_establish4002,0);
		return;
	}
	session_line=l;

	skAddHandler( l, SK_H_READABLE, _HReadable, 0 );
	skAddHandler( l, SK_H_PACKET, _sessionData, 0 );
	skAddHandler( l, SK_H_CLOSE, _sessionClose, 0 );

	sendJSONPacket(l, "{\"CONNECT\":\"REQUEST\"}" );

	Log(16,"json: session connected\r\n");
}

static void _establish4000( void )
{
	SkLine	*l;

	l=RawConnect("4000","192.168.28.30");

	if (!l )
		return;
	cmd_line=l;

	json.conntime=0;
	cmd_conntime=time(0);

	skAddHandler( l, SK_H_READABLE, _HReadable, 0 );
	skAddHandler( l, SK_H_PACKET, _cmdData, 0 );
	skAddHandler( l, SK_H_CLOSE, _cmdClose, 0 );

	if ( !alive_tid )
		alive_tid=skAddTimer( 2000, _sendAlive, 0 );

	printf("json: cmd connected\r\n");
}

int	jsonSend( char *command )
{
	int		clen=command ? strlen(command) : 0;
	SkLine	*l=0;

	if ( clen > 512 )
		return -1;

#ifdef JSON_SIMUL
	json_simu();
	return 0;
#endif

	if ( clen )
		sendJSONPacket(l, command );

	return 0;
}

void jsonInit( void )
{
	_establish4002(0,0);
}
