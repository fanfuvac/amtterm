
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
			printf("aaa\n");
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

		//if (FD_ISSET(STDIN_FILENO,&set)) {
			/* stdin has data */
			//rc = read(STDIN_FILENO,buf,BUFSIZE);
			/*switch (rc) {
				case -1:
				perror("read(stdin)");
				return -1;
				case 0:
				fprintf(stderr,"EOF from stdin\n");
				return -1;
				default:
				if (buf[0] == 0x1d) {
					if (r->verbose)
					fprintf(stderr, "\n" APPNAME ": saw ^], exiting\n");
					redir_sol_stop(r);
						}
						for (i = 0; i < rc; i++) {
							// meet BIOS expectations 
							if (buf[i] == 0x0a)
								buf[i] = 0x0d;
				}
				if (-1 == redir_sol_send(r, buf, rc))
					return -1;
				break;
				}*/
		//}

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
    memcpy(r.type, "IDER", 4);
    strcpy(r.user, "admin");
	
    r.cb_data  = &r;
    //r.cb_recv  = recv_tty;
    //r.cb_state = state_tty;

    if (NULL != (h = getenv("AMT_PASSWORD")))
	snprintf(r.pass, sizeof(r.pass), "%s", h);

   /*for (;;) {
        if (-1 == (c = getopt(argc, argv, "hvqu:p:")))
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

        case 'h':
            usage(stdout);
            exit(0);
        default:
            usage(stderr);
            exit(1);
        }
    }*/
	//snprintf(r.host, sizeof(r.host), "%s", "10.0.0.100");
    if (optind < argc)
	snprintf(r.host, sizeof(r.host), "%s", argv[optind]);
    /*if (optind+1 < argc)
	snprintf(r.port, sizeof(r.port), "%s", argv[optind+1]);*/
    if (0 == strlen(r.host)) {
	//usage(stderr);
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