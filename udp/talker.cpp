#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>

#define SERVERPORT "4950"	// the port users will be connecting to
#define BUFFERSIZE 100


int main(int argc, char *argv[]) {
    struct addrinfo hints, *servinfo, *p;

    if (argc != 3) {
        std::cout << "usage: talker hostname message\n";
        return 1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    int response_code;
    response_code = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo);
    if (response_code != 0) {
        std::cerr << "talker: getaddrinto\n";
        return 1;
    }

    int socketfd;
    for (p=servinfo; p != nullptr; p = p->ai_next) {
        socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socketfd == -1) {
            std::cerr << "talker: socket\n";
            continue;
        }
        break;
    }

    if (p == nullptr) {
        std::cerr << "talker: socket creation failed\n";
        return 1;
    }

    // do not have to connect before sending
    int no_bytes;
    no_bytes = sendto(socketfd, argv[2], strlen(argv[2]), 0, p->ai_addr, p->ai_addrlen);
    if (no_bytes == -1) {
        std::cerr << "talker: sendto\n";
        return 1;
    }

    freeaddrinfo(servinfo);
    std::cout << "talker: sent " << no_bytes << " bytes to " << argv[1] << "\n";
    close(socketfd);

    return 0;
}