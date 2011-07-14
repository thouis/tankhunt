/* main.c - read arguments, init, and start everything going */

#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>

#include "defs.h"
#include "packets.h"

void init_display();
int create_socket();
void input_loop();

char *display_name = (char *) NULL;
char *server_name = DEFAULT_SERVER;
int port_num = DEFAULT_PORT;
char *player_id = "noname";

/* print out the usage of the program */
void
    usage(option, name)
char *option;
char *name;
{
    fprintf(stderr, "%s : invalid command line option '%s'\n", name, option);
    fprintf(stderr, "\nUsage:\n");
    fprintf(stderr, "\n%s [-display display] [-server server] [-port port] [-name name]\n", name);
    exit(1);
}

/*
 * parse the arguments to the program 
 */  
void
    parse_args(argc, argv)
int  argc;
char **argv;
{
    int i;
    
    for (i = 1; i < argc; i++) 
    {
	if (!strcmp(argv[i], "-server")) 
	{
	    i++;
	    if (i == argc) usage("-server", argv[0]);
	    server_name = argv[i];
	    continue;
	}
	if (!strcmp(argv[i], "-port")) 
	{
	    i++;
	    if (i == argc) usage("-port", argv[0]);
	    port_num = atoi(argv[i]);
	    continue;
	}
	if (!strcmp(argv[i], "-display")) 
	{
	    i++;
	    if (i == argc) usage("-display", argv[0]);
	    display_name = argv[i];
	    continue;
	}
	if (!strcmp(argv[i], "-name")) 
	{
	    i++;
	    if (i == argc) usage("-name", argv[0]);
	    player_id = argv[i];
	    continue;
	}
	usage(argv[i], argv[0]);
    }
}

/*
 * parse arguments, connect to the display, connect to the server, and
 * start playing.
 */
int
    main(argc, argv)
int argc;
char **argv;
{
    int fd;

    parse_args(argc, argv);

    init_display(display_name);

    fd = create_socket(server_name, port_num); 

    input_loop(player_id, fd); /* never returns */
}
