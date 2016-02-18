#include <iostream>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <cmath>
#include <sys/epoll.h>

using namespace std;

#define SOCKET_ERROR -1
#define BUFFER_SIZE 100
#define HOST_NAME_SIZE 255
#define MAX_GET 1000

bool debugFlag = false;

struct hostent* pHostInfo;
struct sockaddr_in Address;
long nHostAddress;
char strHostName[HOST_NAME_SIZE];
int nHostPort;

string host;
string port;
string path;

int num_connections;
vector<int> sockets;
vector<struct timeval> starttimes;
vector<struct timeval> endtimes;

//downloads data received from socket
void download(int socket) {
	unsigned nReadAmount;
	char pBuffer[BUFFER_SIZE];

	nReadAmount = read(socket, pBuffer, BUFFER_SIZE);
	gettimeofday(&endtimes[socket-4], NULL);

	if(close(socket) == SOCKET_ERROR){
        cout << "Could not close socket" << endl;
	}
}

//creates socket with given host/port
void createSocket(int i) {
	sockets[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	gettimeofday(&starttimes[i], NULL);

	if (sockets[i] == SOCKET_ERROR) {
        cout << "Could not create socket" << endl;
		return;
	}

	pHostInfo = gethostbyname(strHostName);

	if(pHostInfo == NULL){
		cout << "Could not connect to host" << endl;
		return;
	}

	memcpy(&nHostAddress, pHostInfo->h_addr, pHostInfo->h_length);

	Address.sin_addr.s_addr = nHostAddress;
	Address.sin_port = htons(nHostPort);
	Address.sin_family = AF_INET;


	if (connect(sockets[i], (struct sockaddr*)&Address, sizeof(Address)) == SOCKET_ERROR){
		cout << "Could not connect to host" << endl;
	}else{		
		char *message = (char *)malloc(MAX_GET);
		sprintf(message, "GET %s HTTP/1.1\r\nHost: %s:%d\r\n\r\n", path.c_str(), strHostName, nHostPort);

		write(sockets[i], message, strlen(message));
	}
}

int main(int argc, char** argv){
	if(argc < 5 || argc > 6){
		cout << "\nUsage: webtest host port path count\n";
		return 0;
	}

	int c;
	extern char *optarg;
	extern int optind;

	while((c = getopt( argc, argv, "d")) != -1){
		switch(c){
			case 'd':
				debugFlag = true;
				break;
			case '?':
				cout << "\nUsage: webtest host port path count\n";
				return 0;
		}
	}

	host = argv[optind];
	port = argv[optind + 1];
	path = argv[optind + 2];
	strcpy(strHostName, argv[optind]);
	nHostPort = atoi(argv[optind + 1]);
	num_connections = atoi(argv[optind + 3]);

	sockets.resize(num_connections);
	starttimes.resize(num_connections);
	endtimes.resize(num_connections);
	
	int epollfd = epoll_create(1);
	
	for(int i = 0; i < num_connections; i++){
		createSocket(i);
		struct epoll_event event;

		event.data.fd = sockets[i];
		event.events = EPOLLIN;

		int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, sockets[i], &event);
	}
	
	for(int i = 0; i < num_connections; i++){
		struct epoll_event event;
		int nr_events = epoll_wait(epollfd, &event, 1, -1);
		download(event.data.fd);
	}

    double totalTime = 0;

    for(int i = 0; i < num_connections; i++){
        double usec = (endtimes[i].tv_sec - starttimes[i].tv_sec)*(double)1000000+(endtimes[i].tv_usec-starttimes[i].tv_usec);
        totalTime += usec;
        if(debugFlag){
            cout << "Request " << i << " took " << usec / 1000000 << " seconds" << endl;
        }
    }

    double mean = totalTime / 1000000 / num_connections;
    cout << "Average time: " << mean << endl;

    double total = 0;
    for(int i = 0; i < num_connections; i++){
        double usec = (endtimes[i].tv_sec - starttimes[i].tv_sec)*(double)1000000+(endtimes[i].tv_usec-starttimes[i].tv_usec);
        total += pow(usec - mean, 2.0);
    }

    cout << "Std deviation: " << sqrt(total / num_connections) / 1000000 << endl;

	return 0;
}
