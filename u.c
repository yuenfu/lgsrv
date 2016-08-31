#include <stdio.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include "NetDrv.h"

/* passwd root: most9981 */

/* version histrory
 *
 * 13.08.2013   1.0    fx2 initial coding (first shot)
 * 13.08.2013   1.1    fx2 copy files from /usr/data/tmp /usr/bin
 * 13.08.2013   1.2    fx2 option -fg
 * 13.08.2013   1.3    fx2 create DESTPATH if not exist
 * 13.08.2013   1.4    fx2 use killall and system for restart
 * 15.08.2013   1.5    fx2 show cleaningrecord.stc , active dropbear
 * 19.08.2013   1.6    fx2 Cpu Idle+User+Sys in State page
 * 20.08.2013   1.7    fx2 Cpu usage without history
 * 21.08.2013   1.8    fx2 download map files
 * 29.08.2013   1.9    fx2 show map files (intern js)
 * 06.11.2013   1.10   fx2 syncline if more than 1000000 bytes
 * 14.01.2014   1.11   fx2 download to destpath /usr/tmp  (ramdisk)
 * 17.01.2014   1.12   fx2 first access for /dev/video0
 * 02.11.2014   1.13   fx2 access for /dev/video removed
 * 04.11.2014   1.14   fx2 timer page added
 * 05.11.2014   2.00   fx2 new webpages
 * 06.11.2014   2.01   fx2 upload finished, pages
 * 06.11.2014   2.02   fx2 show version of current running software
 * 19.12.2014   2.03   fx2 load cleaningrecord.scl 1x/minute
 * 08.01.2015   2.04   fx2 json.cgi - multiple bracket pairs '{ / }'
 * 15.01.2015   2.05   fx2 MONOTONIC-CLOCK, new web pages (statistic)
 * 17.01.2015   2.06   fx2 MultiDisable(), empty maps.html
 * 18.01.2015   2.07   fx2 bugfixes - uninitialized mem blkfile[]
 * 18.01.2015   2.08   fx2 bugfixes - vom bugfix
 * 18.01.2015   2.09   fx2 num_blk reset to 0
 * 20.01.2015   2.10   fx2 log-interface, show 3 maps
 * 22.01.2015   2.11   fx2 status.txt + status.html
 * 13.02.2015   2.12   fx2 Access-Control-Allow-Origin: *
 *                         status.txt converted to unix
 *                         add buttons for modes(zz,sb,spot)
 * 17.02.2015   2.13   fx2 accept mode at end of timer-string f.e.',ZZ'
 * 19.02.2015   2.14   fx2 upload/download Motion.xml+Navi.xml +Turbo +Repeat
 * 20.02.2015   2.15   fx2 Nickname CC, bugfix strlen(Navi.xml), Mode-button
 * 20.02.2015   2.16   fx2 Nickname CC on all pages
 * 12.04.2015   2.17   fx2 send mail after work (no config via web !)
 *                         log json=16, mail=2
 * 14.04.2015   2.18   fx2 setup mail via web interface
 * 14.04.2015   2.19   fx2 bug 'Enable=yes' in mail config fixed
 * 14.04.2015   2.20   fx2 allow 50 character len in mail config entries
 * 15.04.2015   2.21   fx2 smtp-password as '***' , enable as checkbox
 * 15.04.2015   2.22   fx2 pop3 function added
 * 17.04.2015   2.23   fx2 suppress bombing same message in log
 * 27.04.2015   2.24   fx2 fixup sourceforge
 * 28.04.2015   2.25   fx2 reconnect json if closed by remote
 * 28.04.2015   2.26   fx2 large file lead to frozen communication
 * 29.04.2015   2.27   aum (audimax) switch to advancend maps
 * 30.04.2015   2.28   fx2 accept additional user in smtp : 'from[,user]'
 * 04.05.2015   2.29   aum runtime fixes maps.html 
 * 13.05.2015   2.30   fx2/aum multiple mail receiver some fixes in map page
 * 15.05.2015   2.30a  aum edit xml values in webbrowser, system reboot
 * 17.05.2015   2.30b  aum error messeges for file loading refined
 * 17.05.2015   2.30c  aum added App.xml to editor
 * 18.05.2015   2.30d  aum added SLAM_control.xml to editor, on failed load restore defaults
 * 19.05.2015   2.30e  aum fixed bug in http.c introduced in 2.30a
 * 28.05.2015   2.31   aum bumping version, compressed www.c
 * 29.05.2015   2.32   fx2 new VAR  CPU:USAGE  for web-pages  (=100-IDLE)
 * 04.08.2015   2.33   fx2 enable core-file in /usr/data/tmp (and use CPU:USAGE)
 * 01.01.2016   2.33   aum fixed 2015 bug in maps.html
 * 04.01.2016   2.33d  bbb (BigBadaBoom) adds cgi-bin functionality and cleandata.html
 * 10.01.2016   2.34   bbb adds pedro patch support, updates javascripts to prevent overload of lg.srv
 * 16.08.2016   2.35   fx2 option -core to enable writing core
 * 17.08.2016   2.36   fx2 allow Smtp-Server like 'mail.abc.de:567#STARTTLS'
 * 17.08.2016   2.37   fx2 changes popen(dd) to fopen for snapshot
 * 17.08.2016   2.38   fx2 more than one timer per day (09:00+10:00)
 * 17.08.2016   2.39   fx2 bugfix: (tueftl,hacking #1445,page73)
 * 17.08.2016   2.39b  fx2 -mp (test only)
 * 17.08.2016   2.40   fx2 back to popen, access device will crash lg.srv
 * 17.08.2016   2.41   fx2 new field MEMORY USAGE in statistic and status page
 * 18.08.2016   2.42   fx2 make possible to save #STARTTLS (bugfix)
 * 19.08.2016   2.43   fx2 self restart on crash
*/

char *cstr = "lg.srv, V2.43 compiled 19.08.2016, by fx2";

int	debug = 0;		// increasing debug output (0 to 9)
int		vmeth=0;

static	SkLine	*listen_line=0;
static	SkLine	*cl_array[30];

static	int		noWrongSize=0;
static	int		maxlog=48;

	JsonVars	json;
	TimerVars	timer;
	MailVars	mail;

void	SetMaxLog( int nr )
{
	maxlog=nr;
}

static	void	_clientClose( SkLine *l, int pt, void *own, void *sys )
{
	int			i;
	int			numcl=0;

	free( l->data );
	l->data = 0;

	for( i=0; i < 30; i++ )
	{
		if ( cl_array[i] == l )
			cl_array[i] = 0;
		if ( cl_array[i] )
			numcl++;
	}
}

static	void	_clientData( SkLine *l, int pt, void *own, void *sys )
{
	SkPacket	*pck = sys;
	int			i;
	unsigned char	*data=(unsigned char*)pck->data;
#if 0
	unsigned int	ui;
	unsigned int	dst;
	unsigned short	us;
	unsigned long	ul;
	unsigned short	instr;
#endif

	if ( l->data->mode == MODE_UNKNOWN )
	{
//		if (( data[1] == 0x00 ) && ( data[2] == 0x00 ))
//			l->data->mode = MODE_LG;
//		else if ( !strncmp((char*)data,"GET /",5) )
		if ( !strncmp((char*)data,"GET /",5) )
			l->data->mode = MODE_HTTP;
		else if ( !strncmp((char*)data,"POST ",5) )
		{
			l->data->mode = MODE_HTTP;
			l->data->post_mode = 1;
		}
		else
			l->data->mode = MODE_LOG;
	}

	if ( l->data->mode == MODE_HTTP )
	{
		if ( !l->data->post_mode && !strncmp((char*)data,"POST ",5) )
			l->data->post_mode = 1;
		HttpPck( l, 0, own, sys );
		return;
	}
	if ( l->data->mode == MODE_LOG )
	{
		DoLogData( l, pck->data, pck->len );
		return;
	}

	HttpStatAddClData();

	Log(8,"_clientData: %d bytes (%d)\r\n",pck->len,l->data->cl_id);

	if ( pck->len > 20000 )
	{
		Log(8," # request too long ! - ignoring (wait for fault)\r\n");
		noWrongSize++;
		skDisconnect( l );
		return;
	}

	for( i=0; i<pck->len && (i<maxlog); i++ )
	{
		if ( !(i%16) && i )
			Log(8,"\r\n");
		Log(8,"%02x ",data[i]);
	}
	Log(8,"\r\n");

	l->data->num_wrong_frames++;
	return;
}

void	_clientConnected( SkLine *cl, int pt, void *own, void *sys )
{
	int		i;

	for( i=0; i < 30; i++ )
	{
		if ( cl_array[i] == 0 )
			break;
	}
	if ( i == 30 )
	{
		/* too many clients : disconnect */
		Log(1,"too many clients - disconnect client\r\n");
		skDisconnect( cl );
		return;
	}
	cl_array[i] = cl;

	skAddHandler( cl, SK_H_READABLE, _HReadable, 0 );
	skAddHandler( cl, SK_H_PACKET, _clientData, 0 );
	skAddHandler( cl, SK_H_CLOSE, _clientClose, 0 );

	cl->data = malloc( sizeof(struct _ClientData) );
	memset(cl->data,0, sizeof(struct _ClientData) );

	cl->data->cl_id = i;
	Log(1,"new connection : clients (%d)\r\n",i);
}

static	char	**g_av=0;
static	int		g_ac=0;
static  char	*mem_a=0;

float _GetMemUsage( void )
{
	char *mem_b = sbrk(0);
	return (float)(mem_b - mem_a)/1024/1024;
}

static pid_t		pid=0;

static void _stop( int snr )
{
	if( pid )
		kill(pid,SIGTERM);
	exit(0);
}

static void	_StartChild( void );

static void _sigchld_( int snr )
{
	pid_t	wpid;
	int		st;

	while( ( wpid = waitpid(-1, &st, WNOHANG) ) != 0 )
	{
		if ( wpid == pid )
		{
			pid = 0;
			_StartChild();
			break;
		}
	}
}

static	char	*defport="6260";
static	char	*port=0;
static	int		enacore=0;

static void	_StartChild( void )
{
	sigset_t			nmask, omask;
	struct sigaction	sa;

	sa.sa_handler = _sigchld_;
	sa.sa_flags = SA_RESTART;
	sigaction(SIGCHLD,&sa,NULL);

	sigemptyset(&nmask);
	sigaddset(&nmask,SIGCHLD);
	sigprocmask(SIG_BLOCK,&nmask,&omask);

	pid = fork();
	if ( pid == -1 )
	{
		fprintf(stderr,"no more porcesses\n");
		exit(127);
	}
	if ( !pid )	/* child */
	{
		char	*av[]={ 0, "-fg", 0, 0, 0, 0, 0 };
		int		dpa=2;

		av[0]=*g_av;
		if ( port != defport )
		{
			*(av+dpa) = "-port";
			*(av+dpa+1) = port;
			dpa+=2;
		}
		if ( enacore )
		{
			*(av+dpa) = "-core";
			dpa++;
		}
		execvp( *g_av, av );
		perror( *g_av );
		_exit(127);
	}
	sigprocmask(SIG_SETMASK,&omask,NULL);
}

int main( int argc, char ** argv )
{
	int		i;
	int		fg=0;
	char	*cmdname;

	mem_a=sbrk(0);

	cmdname=strrchr(*argv,'/');
	if ( cmdname )
		cmdname ++;
	else
		cmdname = *argv;

	g_av=argv;
	g_ac=argc;

	memset( cl_array, 0, sizeof(cl_array) );

	for( i=1; i < argc; i++ )
	{
		if ( (i<argc-1) && !strcmp(argv[i],"-port") )
		{
			port = argv[++i];
		}
		else if ( !strcmp(argv[i],"-debug") )
		{
			debug = 1;
		}
		else if ( !strncmp(argv[i],"-debug=",7) )
		{
			debug = atoi(argv[i]+7);
		}
		else if ( !strcmp(argv[i],"-fg") )
		{
			fg = 1;
		}
		else if ( !strcmp(argv[i],"-core") )
		{
			enacore = 1;
		}
		else if ( !strcmp(argv[i],"-version") )
		{
			printf("%s\n",cstr);
			exit(0);
		}
		else
		{
			printf("usage: %s [-port <nr>] [-debug=mask] [-fg]\n", *argv);
			printf("  debug-mask :  1 : quiet debug\n");
			printf("  debug-mask :  2 : send mail\n");
			printf("  debug-mask :  4 : timer\n");
			printf("  debug-mask :  8 : http traffic\n");
			printf("  debug-mask : 16 : json traffic\n");
			return 1;
		}
	}

	if ( debug )
		fg=1;
	if ( !port )
		port=defport;

	if ( !fg && fork() )
		return 0;

	signal( SIGCHLD, SIG_IGN );
	signal( SIGPIPE, SIG_IGN );
	signal( SIGHUP, SIG_IGN );
	signal( SIGINT, _stop );
	signal( SIGTERM, _stop );

	if ( !fg )
	{
		_StartChild();
		skMainLoop();
		return 0;
	}

	memset(&json,0,sizeof(JsonVars));
	memset(&timer,0,sizeof(TimerVars));
	memset(&mail,0,sizeof(MailVars));

	ReadTimerFromFile();
	ReadMailConfigFromFile();
	HttpLoadCleaningRecord(0,0);

	if ( enacore )
	{
		struct rlimit rli;
		mkdir("/usr/data/tmp",511);	/* 0777 */
		chdir("/usr/data/tmp");
		rli.rlim_cur = 16*1048576;	/* 16MB */
		setrlimit(RLIMIT_CORE,&rli);
	}

	system("ifconfig lo up");

	listen_line = _lgListen( atol(port) );

	StartTimer();

	jsonSend(0);

	skMainLoop();

	return( 0 );
}

static	void	WriteStat( SkLine *l, int ashtml, char *fmt, ... )
{
	va_list		args;
	char		out[2048];

	va_start( args, fmt );
	vsprintf( out, fmt, args );
	va_end( args );

	_WriteString(l,out);
	if ( ashtml )
		_WritePacket(l,(unsigned char*)"<BR>",4);
	else
		_WritePacket(l,(unsigned char*)"\r\n",2);
}

void	_ShowStatistic( SkLine *l, int ashtml )
{
	int		i;
	int		numcl=0;

	for( i=0; i < 30; i++ )
	{
		if ( cl_array[i] != 0 )
			numcl++;
	}
	WriteStat(l,ashtml,"Clients connected (all)   : %d",numcl);
	for( i=0; i<30; i++ )
	{

		if ( cl_array[i] == 0 )
			continue;
		switch( cl_array[i]->data->mode )
		{
		case MODE_UNKNOWN :
			WriteStat(l,ashtml," [%d] - protocol             : ?",i);
			break;
		case MODE_LOG :
			WriteStat(l,ashtml," [%d] - protocol             : LOG",i);
			break;
		case MODE_HTTP :
			WriteStat(l,ashtml," [%d] - protocol             : HTTP",i);
			break;
		}
	}
	WriteStat(l,ashtml,"noWrongSize (>20kBytes)   : %d",noWrongSize);
	WriteStat(l,ashtml,"MaxLog (def:48)           : %d",maxlog);
}

void	_ResetStatistik( void )
{
	int		i;

	noWrongSize=0;
	for( i=0; i < 30; i++ )
	{
		if ( cl_array[i] != 0 )
		{
			cl_array[i]->data->num_wrong_frames=0;
		}
	}
}

void	_RestartMe( void )
{
	unsigned short	portnr=0;

	if ( listen_line )
	{
		portnr=listen_line->port;
		skDisconnect( listen_line );
	}
	listen_line=0;

	execvp( "/usr/bin/lg.srv", g_av );

	listen_line = _lgListen( portnr ? portnr : 6260 );
}
