/* main.c - main and initialization routines */

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "defs.h"
#include "struct.h"
#include "data.h"
#include "sintab.h"

void update();
void input_loop();
void gen_walls();
void init_sintab();

static u_short port_num = DEFAULT_PORT;
static int density = DEFAULT_DENSITY;

/* print out the usage of the program */
static void
    usage(name)
char *name;
{
    fprintf(stderr, "Unknown option.\n");
    fprintf(stderr, "Usage - %s [-port port] [-density 0-100] [-size <size of arena>]\n", name);
    exit (1);
}

/* open up the UDP port we'll be using */
static int
    open_port(port_num)
u_short port_num;
{
    struct sockaddr_in server_addr;
    int socket_fd;

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
	fprintf(stderr, "Could not open socket.\n");
	perror("socket");
	exit (1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port_num);

    if (bind(socket_fd, (struct sockaddr *) &server_addr, 
	     sizeof(server_addr)) < 0)
    {
	fprintf(stderr, "Could not bind socket.\n");
	perror("bind");
	exit (1);
    }

    return (socket_fd);
}

/*
 * parse the arguments to the program 
 */  
void
    parse_args(argc, argv)
int argc;
char **argv;
{
    int i;
    
    for (i = 1; i < argc; i++) 
    {
        if (!strcmp(argv[i], "-port")) 
        {
            i++;
            if (i == argc) usage(argv[0]);
            port_num = atoi(argv[i]);
            continue;
        }
        if (!strcmp(argv[i], "-density")) 
        {
            i++;
            if (i == argc) usage(argv[0]);
	    density = atoi(argv[i]);
	    if ((density < 0) || (density > 100))
		usage(argv[0]);
            continue;
        }
        if (!strcmp(argv[i], "-size"))
        {
            i++;
            if (i == argc) usage(argv[0]);
	    arena_size = atoi(argv[i]) * BOX_SIZE;
	    half_arena = arena_size / 2;
	    arena_box_dim = arena_size / BOX_SIZE + 2 * ARENA_OFFSET;
	    if (arena_box_dim > MAX_ARENA)
	    {
		fprintf(stderr, 
			"Too large of an arena.  Must be %d or less.\n",
			MAX_ARENA - (2 * ARENA_OFFSET));
		exit (1);
	    }
	    if ((density < 0) || (density > 100))
		usage(argv[0]);
            continue;
        }
	usage(argv[0]);
    }
}

/* 
 * call parse_args, init the pieces of the program that need initting,
 * start talking to the outside world.
 */
int 
    main(argc, argv)
int argc;
char **argv;
{
    struct itimerval tick;
    int i;
    int val, len = 4;

    srandom(time(NULL));
    
    /* parse arguments */
    parse_args(argc, argv);

    /* initialize the fast SIN and COS routines */
    init_sintab();

    /* build the maze */
    gen_walls(density);

    /* accept connections */
    port_fd = open_port(port_num);

    /* set up the update alarm */
#ifdef SYSV
    sigset(SIGALRM, update);
#else
    signal(SIGALRM, update);
#endif

    tick.it_interval.tv_sec = 0;
    tick.it_interval.tv_usec = UPDATE;
    tick.it_value.tv_sec = 0;
    tick.it_value.tv_usec = UPDATE;
    setitimer(ITIMER_REAL, &tick, (struct itimerval *) NULL);

    /* never exits */
    input_loop(); 
}
