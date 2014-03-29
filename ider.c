
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "redir.h"

#define APPNAME "amtterm"
#define BUFSIZE 512

static void usage(FILE *fp)
{
    fprintf(fp,
            "\n"
	    "This is " APPNAME ", release " VERSION ", I'll establish\n"
	    "serial-over-lan (sol) connections to your Intel AMT boxes.\n"
            "\n"
            "usage: " APPNAME " [options] host [port]\n"
            "options:\n"
            "   -h            print this text\n"
            "   -v            verbose (default)\n"
            "   -q            quiet\n"
            "   -u user       username (default: admin)\n"
            "   -p pass       password (default: $AMT_PASSWORD)\n"
			"   -c path       path to *.iso image of redirected CD\n"
			"   -f path       path to *.img image of redirected FD\n"
            "\n"
            "By default port 16994 is used.\n"
	    "If no password is given " APPNAME " will ask for one.\n"
            "\n"
            "-- \n"
            "Based on amtterm by Gerd Hoffmann.\n"
			"(c) 2014 Václav Fanfule <fanfuvac@fit.cvut.cz>\n"
	    "\n");
}

static int redir_loop(struct redir *r)
{
    //unsigned char buf[BUFSIZE+1];
    struct timeval tv;
    int i;
    fd_set set;
	printf("%i\n",r->state);
    //int i=0;
	for(i=0;;i++) {
		if (r->state == REDIR_CLOSED ||
			r->state == REDIR_ERROR){
			printf("%i\n",r->state);
			//break;
		} 
		FD_ZERO(&set);
		//if (r->state == REDIR_RUN_IDER)
		//	FD_SET(STDIN_FILENO,&set);
		FD_SET(r->sock,&set);
		tv.tv_sec  = HEARTBEAT_INTERVAL * 4 / 1000;
		tv.tv_usec = 0;
		switch (select(r->sock+1,&set,NULL,NULL,&tv)) {
		case -1:
			perror("select");
			return -1;
		case 0:
			fprintf(stderr,"select: timeout\n");
			return -1;
		}



		if (FD_ISSET(r->sock,&set)) {
			if (-1 == redir_data(r)){
				printf("why??\n");
				return -1;
			}
		}
    }
    return 0;
}
//***********************************************************************//
int main(int argc, char *argv[])
{
    struct redir r;
    char *h;
    //int c;

    memset(&r, 0, sizeof(r));
    r.verbose = 1;
	r.trace=1;
    memcpy(r.type, "IDER", 4);
    strcpy(r.user, "admin");
	
    r.cb_data  = &r;
    //r.cb_recv  = recv_tty;
    //r.cb_state = state_tty;

    if (NULL != (h = getenv("AMT_PASSWORD")))
	snprintf(r.pass, sizeof(r.pass), "%s", h);
	int c;
    for (;;) {
        if (-1 == (c = getopt(argc, argv, "hvqc:f:u:p:")))
            break;
        switch (c) {
	case 'v':
	    r.verbose = 1;
	    break;
	case 'q':
	    r.verbose = 0;
	    break;
	case 'u':
	    snprintf(r.user, sizeof(r.user), "%s", optarg);
	    break;
	case 'p':
	    snprintf(r.pass, sizeof(r.pass), "%s", optarg);
	    memset(optarg,'*',strlen(optarg)); //rm passwd from ps list 
	    break;
	case 'c':
	    snprintf(r.cd, sizeof(r.cd), "%s", optarg);
	    break;
	case 'f':
	    snprintf(r.fd, sizeof(r.fd), "%s", optarg);
	    break;	
		
	case 'h':
		usage(stdout);
		exit(0);
	default:
		usage(stderr);
		exit(1);
	}
    }
	//snprintf(r.host, sizeof(r.host), "%s", "10.0.0.100");
    if (optind < argc)
	snprintf(r.host, sizeof(r.host), "%s", argv[optind]);
    /*if (optind+1 < argc)
	snprintf(r.port, sizeof(r.port), "%s", argv[optind+1]);*/
    if (0 == strlen(r.host)) {
	usage(stderr);
	exit(1);
    }

    //tty_save();
    if (0 == strlen(r.pass)) {
	//tty_noecho();
	fprintf(stderr, "AMT password for host %s: ", r.host);
	fgets(r.pass, sizeof(r.pass), stdin);
	fprintf(stderr, "\n");
	if (NULL != (h = strchr(r.pass, '\r')))
	    *h = 0;
	if (NULL != (h = strchr(r.pass, '\n')))
	    *h = 0;
    }

    snprintf(r.err, sizeof(r.err), "%s", "stdout");
    //r.err="stdout";
    if (-1 == redir_connect(&r)) {
	//tty_restore();
	exit(1);
    }

    //tty_raw();
    redir_start(&r);
    redir_loop(&r);
    //tty_restore();

    exit(0);
    //tty_restore();

    exit(0);
}