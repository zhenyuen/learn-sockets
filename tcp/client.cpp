#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
// #include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <signal.h>


#define PORT "3490"
#define MAXDATASIZE 100


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr ,"Usage: client hostname\n");
        return 1;
    }

    struct addrinfo hints, *serveinfo, *p;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int rv;
	if ((rv = getaddrinfo(argv[1], PORT, &hints, &serveinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
    

    int sockfd;
    for (p=serveinfo; p != nullptr; p = p -> ai_next) {
        if ((sockfd = socket(serveinfo->ai_family, serveinfo->ai_socktype, serveinfo->ai_protocol)) == -1) {
            perror("Client: socket");
            continue;
        }
    

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Client: connect");
            continue;
        }
        break;
    }


    if (p == nullptr) {
        perror("Client: failed to connect\n");
        exit(2);
    }

    int numbytes;
    char s[INET6_ADDRSTRLEN], buf[MAXDATASIZE];

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	
    printf("client: connecting to %s\n", s);
    
    freeaddrinfo(serveinfo);

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}

	buf[numbytes] = '\0';

	printf("client: received '%s'\n",buf);

	close(sockfd);
    return 0;
}