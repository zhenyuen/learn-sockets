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
#define MAX_CONNECTIONS 10


void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
    struct addrinfo hints, *serveinfo, *p;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int rv;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &serveinfo)) != 0) {
        std::cout << "getaddrinfo: " << gai_strerror(rv) << "\n";
        return 1;
    }

    int sockfd;
    for (p=serveinfo; p != nullptr; p = p -> ai_next) {
        if ((sockfd = socket(serveinfo->ai_family, serveinfo->ai_socktype, serveinfo->ai_protocol)) == -1) {
            perror("Server: socket");
            continue;
        }
        
        int yes = 1;
        // Can handle errors when port is occupied, but ignore for now
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("Server: setsockopt");
            exit(1);
        }
        ////////

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Server: bind");
            continue;
        }
        break;
    }

    freeaddrinfo(serveinfo);

    if (p == nullptr) {
        perror("Server: failed to bind");
        exit(1);
    }

    if (listen(sockfd, MAX_CONNECTIONS) == -1) {
        perror("Server: listen");
        exit(1);
    }

    /// Code for killing zombie child processes
    struct sigaction sa;
    sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
    ///

    std::cout << "Server: waiting for connections...\n";

    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    int new_fd;
    char s[INET6_ADDRSTRLEN];
    while (1) {
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
        
        if (!fork()) {
            // child processes inherit the file descriptors from the parent.
            close(sockfd); // prevent any incoming connections
            if (send(new_fd, "Hello", 13, 0) == -1) {
                perror("server: send");

            }
            close(new_fd);
            exit(0); // This causes the child process to exit, not the parent
        }
        close(new_fd);
    }
    return 0;
}