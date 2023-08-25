#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>


#define SERVERPORT "4950"	// the port users will be connecting to
#define BUFFERSIZE 100


void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        auto tmp = (struct sockaddr_in*)sa;
        return &tmp->sin_addr;
    }
    auto tmp = (struct sockaddr_in6*)sa;
    return &tmp->sin6_addr;

}
// The server
int main(int argc, char *argv[]) {
    struct addrinfo hints, *p, *servinfo;

    memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

    int response_code;
    response_code = getaddrinfo(NULL, SERVERPORT, &hints, &servinfo);
    if (response_code != 0) {
        std::cerr << "listener: getaddrinto\n";
        return 1;
    }

    int socketfd;
    for (p=servinfo; p!=nullptr; p=p->ai_next) {
        socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socketfd == -1) {
            std::cerr << "listener: socket\n";
            continue;
        }

        if (bind(socketfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(socketfd);
            std::cerr << "listener: bind fail\n";
            continue;
        }
        break;
    }

    if (p == nullptr) {
        std::cerr << "listener: socket creation failed\n";
        return 1;
    }

    freeaddrinfo(servinfo);

    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof(their_addr);

    struct sockaddr *c = (struct sockaddr *)&their_addr;
    char buffer[BUFFERSIZE];
    int no_bytes;
    no_bytes = recvfrom(socketfd, buffer, BUFFERSIZE-1, 0, c, &addr_len);
    if (no_bytes == -1) {
        std::cerr << "listener: recvfrom\n";
        return 1;
    }

    char s[INET6_ADDRSTRLEN]; // string to store ip in presentation format
    inet_ntop(their_addr.ss_family, get_in_addr(c), s, sizeof(s));

    // char s[INET6_ADDRSTRLEN];
    // inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof s))
    // std::cout << "listener: got packet from " << s << std::endl;


    buffer[BUFFERSIZE - 1] = '\0';
    std::cout << "listener: packet from '" << s << "'\n";  
    std::cout << "listener: packet contains '" << buffer << "'\n"; 

    close(socketfd);
    return 0;


}
