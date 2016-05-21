/* 
* This is a modification of Beej's example code
* The server will listen on one port then also reply back
** listener.c -- a datagram sockets "server" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MYPORT "5432"    //  The port clients will be connecting to
#define CLIENTPORT "5433" // The port clients will be receiving from
#define MESSAGE " - Don't eat yellow snow "	//Funny message

#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(void)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];// reply[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // Loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Server: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Server: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    printf("Server: waiting to recvfrom...\n");

    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    printf("Server: got packet from client %s\n",
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s));
    printf("Server: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';
    printf("Server: packet contains \"%s\"\n", buf);
    close(sockfd);
    
    // Prepare to launch another socket for sending
    if ((rv = getaddrinfo(NULL, CLIENTPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    
    // Loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("Server: send socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Server: send bind");
            continue;
        }

        break;
    }
    
    // Server bounces the original message along with a funny message concatenated
    strcat(buf,MESSAGE); 
    if ((numbytes= sendto(sockfd, buf, strlen (buf)+1, 0,
            (struct sockaddr *)&their_addr, addr_len)) == -1) {
        perror("Server: sendto");
        exit(1);
    }
   
    freeaddrinfo(servinfo);
    close(sockfd);
    return 0;
}