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

using namespace std;

#define SOCKET_ERROR    -1
#define BUFFER_SIZE     100
#define HOST_NAME_SIZE  255
#define MAX_GET         1000
#define MAX_MSG_SZ      1024

int downloadCount = 1;
int actualCount = 0;
bool debugFlag = false;
bool countFlag = false;

int hSocket;
struct hostent* pHostInfo;
struct sockaddr_in Address;
long nHostAddress;
char strHostName[HOST_NAME_SIZE];
int nHostPort;

string host;
string port;
string path;

vector<char *> headerLines;

// Determine if the character is whitespace
bool isWhitespace(char c){
    switch (c){
        case '\r':
        case '\n':
        case ' ':
        case '\0':
            return true;
        default:
            return false;
    }
}

// Strip off whitespace characters from the end of the line
void chomp(char *line){
    int len = strlen(line);
    while (isWhitespace(line[len])){
        line[len--] = '\0';
    }
}

// Read the line one character at a time, looking for the CR
// You dont want to read too far, or you will mess up the content
char * GetLine(int fds){
    char tline[MAX_MSG_SZ];
    char *line;

    int messagesize = 0;
    int amtread = 0;
    while((amtread = read(fds, tline + messagesize, 1)) < MAX_MSG_SZ){
        if (amtread >= 0){
            messagesize += amtread;
        }else{
            perror("Socket Error is:");
            fprintf(stderr, "Read Failed on file descriptor %d messagesize = %d\n", fds, messagesize);
            exit(2);
        }
        if (tline[messagesize - 1] == '\n') break;
    }
    tline[messagesize] = '\0';
    chomp(tline);
    line = (char *)malloc((strlen(tline) + 1) * sizeof(char));
    strcpy(line, tline);
    return line;
}

// Change to upper case and replace with underlines for CGI scripts
void UpcaseAndReplaceDashWithUnderline(char *str){
    int i;
    char *s;

    s = str;
    for (i = 0; s[i] != ':'; i++) {
        if (s[i] >= 'a' && s[i] <= 'z')
            s[i] = 'A' + (s[i] - 'a');

        if (s[i] == '-')
            s[i] = '_';
    }

}

// When calling CGI scripts, you will have to convert header strings
// before inserting them into the environment.  This routine does most
// of the conversion
char *FormatHeader(char *str, const char *prefix){
    char *result = (char *)malloc(strlen(str) + strlen(prefix));
    char* value = strchr(str,':') + 1;
    UpcaseAndReplaceDashWithUnderline(str);
    *(strchr(str,':')) = '\0';
    sprintf(result, "%s%s=%s", prefix, str, value);
    return result;
}

// Get the header lines from a socket
//   envformat = true when getting a request from a web client
//   envformat = false when getting lines from a CGI program

void GetHeaderLines(vector<char *> &headerLines, int skt, bool envformat){
    // Read the headers, look for specific ones that may change our responseCode
    char *line;
    char *tline;

    tline = GetLine(skt);
    while(strlen(tline) != 0){
        if (strstr(tline, "Content-Length") || strstr(tline, "Content-Type")){
            if (envformat)
                line = FormatHeader(tline, "");
            else
                line = strdup(tline);
        }else{
            if (envformat)
                line = FormatHeader(tline, "HTTP_");
            else
            {
                line = (char *)malloc((strlen(tline) + 10) * sizeof(char));
                sprintf(line, "HTTP_%s", tline);
            }
        }
        headerLines.push_back(line);
        free(tline);
        tline = GetLine(skt);
    }
    free(tline);
}

//logs helpful information if desired
void log(string msg){
    if(debugFlag){
        cout << msg << endl;
    }
}

//performs HTTP GET on created socket with given path
//gets the header info and print some info if requested
void download() {
    char *message = (char *)malloc(MAX_GET);
    sprintf(message, "GET %s HTTP/1.1\r\nHost: %s:%d\r\n\r\n", path.c_str(), strHostName, nHostPort);

    log("Requesting " + path + " from " + host);

	write(hSocket, message, strlen(message));

	unsigned nReadAmount;
	char pBuffer[BUFFER_SIZE];

	nReadAmount = read(hSocket, pBuffer, BUFFER_SIZE);

	GetHeaderLines(headerLines, hSocket , false);

    char buffer[MAX_MSG_SZ];
    char contentType[MAX_MSG_SZ];
    char contentLength[MAX_MSG_SZ];

    if(debugFlag){
        printf("\n=======================\n");
        printf("Successfully received page from %s\n", strHostName);
        printf("Received Headers:\n");
        for (int i = 0; i < headerLines.size(); i++) {
            printf("[%d] %s\n",i,headerLines[i]);
            if(strstr(headerLines[i], "Content-Type")) {
                 sscanf(headerLines[i], "Content-Type: %s", contentType);
            }
            if(strstr(headerLines[i], "Content-Length")) {
                 sscanf(headerLines[i], "Content-Length: %s", contentLength);
            }
        }
        printf("Headers are finished, now reading the file\n");
        printf("Content Length: %s\n", contentLength);
        printf("=======================\n");
    }
    headerLines.clear();

    int rval;
    while((rval = read(hSocket, buffer, MAX_MSG_SZ)) > 0) {
        if(!countFlag){
            write(1, buffer, rval);
        }
    }
    if(!countFlag){ cout << endl; }
    actualCount++;

	if(close(hSocket) == SOCKET_ERROR){
        log("Could not close socket");
        return;
    }

    log("Socket closed");
}

//creates socket with given host/port
bool createSocket() {
	hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (hSocket == SOCKET_ERROR) {
		log("Could not create socket");
		return false;
	}

	pHostInfo = gethostbyname(strHostName);

	if(pHostInfo == NULL){
        cout << "Could not connect to host" << endl;
        return false;
    }

	memcpy(&nHostAddress, pHostInfo->h_addr, pHostInfo->h_length);

	Address.sin_addr.s_addr = nHostAddress;
	Address.sin_port = htons(nHostPort);
	Address.sin_family = AF_INET;

    log("Connecting to " + host + " on port " + port);

	if (connect(hSocket, (struct sockaddr*)&Address, sizeof(Address)) == SOCKET_ERROR){
		cout << "Could not connect to host" << endl;
		return false;
	}else{
        log("Successfully connected to " + host);
        return true;
	}
}

int main(int argc, char* argv[]){

    if(argc < 4 || argc > 7){
        cout << "\nUsage: download host port path\n";
        return 0;
    }

    int c;
    extern char *optarg;
    extern int optind;

    while( (c = getopt( argc, argv, "c:d")) != -1){
        switch(c){
            case 'c':
                downloadCount = atoi(optarg);
                countFlag = true;
                break;
            case 'd':
                debugFlag = true;
                break;
            case '?':
                cout << "\nUsage: download host port path\n";
                return 0;
        }
    }

    host = argv[optind];
    port = argv[optind + 1];
    path = argv[optind + 2];
    strcpy(strHostName, argv[optind]);
    nHostPort = atoi(argv[optind + 1]);

    while(downloadCount-- > 0){
        if(createSocket()){
            download();
        }
    }

    if(countFlag){
        cout << "Downloaded the file " << actualCount << " times" << endl;
    }

    return 0;
}



