/* socket.c - create a socket to talk to the server */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

/*
 * create a UDP port and aim it at the server, return the fd ofthe socket
 */
int
    create_socket(server_name, port_num)
char * server_name;
int port_num;
{
    struct sockaddr_in serv_addr, cli_addr;
    struct hostent *hostptr;
    int fd;
    
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
	fprintf(stderr, "Could not open socket.\n");
	perror("socket");
	exit (1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_num);
    if ( (hostptr = (struct hostent *) gethostbyname(server_name)) == NULL) 
    {
	perror("gethostbyname");
	exit (1);
    }
    
    serv_addr.sin_addr.s_addr = * (long *) hostptr->h_addr;

    bzero((char *) &cli_addr, sizeof(cli_addr));
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    cli_addr.sin_port = htons(0);
    
    if (bind(fd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) 
    {
	perror("bind");
	exit (1);
    }


    if (connect(fd, (struct sockaddr *) &serv_addr, 
		sizeof(struct sockaddr)) < 0)
    {
	perror("connect");
	exit (1);
    }

    return (fd);
}
