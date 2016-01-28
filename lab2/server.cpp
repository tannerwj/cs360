#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <string>
#include <vector>

#include <fstream>
#include <sstream>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         10000
#define QUEUE_SIZE          5
#define MAXPATH 1000

using namespace std;

int hSocket,hServerSocket;  /* handle to socket */
struct hostent* pHostInfo;   /* holds info about a machine */
struct sockaddr_in Address; /* Internet socket address stuct */
int nAddressSize=sizeof(struct sockaddr_in);
int nHostPort;

char* client_request;
string directory_root;
char _directory_root[MAXPATH];


string getContentType (string filename){
    string file_ext = filename.substr(filename.size() - 4, 4);

    string type = "";

    if(file_ext == "html"){
        type = "text/html";
    }else if(file_ext == ".txt"){
        type = "text/plain";
    }else if(file_ext == ".jpg"){
        type = "image/jpg";
    }else if(file_ext == ".gif"){
        type = "image/gif";
    }

    return type;
}

long getContentLength (string filename){
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

bool exists (const string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

bool isDir (const string& name){
    struct stat s;
    return stat(name.c_str(), &s) == 0 && s.st_mode & S_IFDIR;
}

string createIndex(string root, string path){
    string index = "<h1>" + path + " Index </h1><ul>";
    int len;
    DIR *dirp;
    struct dirent *dp;

    dirp = opendir((root + path).c_str());
    if (path != "/") path += "/";
    while ((dp = readdir(dirp)) != NULL){
        string file = string(dp->d_name);
        if(file != "." && file != ".."){
            index += "<li><a href='"+ path + file +"'>" + file + "</li>";
        }
    }

    (void)closedir(dirp);
    return index + "</ul>";
}

void readSocket (int hSocket){
    char pBuffer[BUFFER_SIZE];

    memset(pBuffer, 0, sizeof(pBuffer));
    int rval = read(hSocket,pBuffer,BUFFER_SIZE);

    char path[MAXPATH];
    rval = sscanf(pBuffer,"GET %s HTTP/1.1",path);

    cout << "Client requested " << path << endl;

    memset(pBuffer, 0, sizeof(pBuffer));

    if (!exists(directory_root + path)) {
        cout << "File does not exist" << endl;
        sprintf(pBuffer,"HTTP/1.1 404 Not Found\r\nContent-Type:text/html\r\nContent-Length:7\r\n\r\nNo Dice");
        write(hSocket, pBuffer, strlen(pBuffer));
    }else if (isDir(directory_root + path)){

        if(exists(directory_root + path + "/index.html")){
            sprintf(path,"%s/index.html", path);
            cout << "Client requested a folder with an index " << path << endl;

            string contentType = getContentType(path);
            long contentLength = getContentLength(directory_root + path);

            sprintf(pBuffer,"HTTP/1.1 200 OK\r\n\Content-Type: %s\r\n\Content-Length: %ld\r\n\r\n", contentType.c_str(), contentLength);
            write(hSocket, pBuffer, strlen(pBuffer));

            printf(pBuffer);

            char fullpath[MAXPATH];
            sprintf(fullpath,"%s%s",_directory_root, path);

            FILE *fp = fopen(fullpath,"r");
            char *buffer = (char *)malloc(contentLength + 1);
            fread(buffer, contentLength, 1, fp);
            write(hSocket, buffer, contentLength);
            cout << "Sent " << path << " to client" << endl;

            free(buffer);
            fclose(fp);
        }else{
            cout << "Client requested a folder without an index" << endl;
            string index = createIndex(directory_root, path);
            sprintf(pBuffer,"HTTP/1.1 200 OK\r\n\Content-Type: text/html\r\n\Content-Length: %ld\r\n\r\n", index.length());

            write(hSocket, pBuffer, strlen(pBuffer));
            write(hSocket, index.c_str(), strlen(index.c_str()));
        }
    }else{
        string contentType = getContentType(path);
        long contentLength = getContentLength(directory_root + path);

        sprintf(pBuffer,"HTTP/1.1 200 OK\r\n\Content-Type: %s\r\n\Content-Length: %ld\r\n\r\n", contentType.c_str(), contentLength);
        write(hSocket, pBuffer, strlen(pBuffer));

        char fullpath[MAXPATH];
        sprintf(fullpath,"%s%s",_directory_root, path);

        FILE *fp = fopen(fullpath,"r");
        char *buffer = (char *)malloc(contentLength + 1);
        fread(buffer, contentLength, 1, fp);
        write(hSocket, buffer, contentLength);
        cout << "Sent " << path << " to client" << endl;

        free(buffer);
        fclose(fp);
    }

    #ifdef notdef
    linger lin;
    unsigned int y=sizeof(lin);
    lin.l_onoff=1;
    lin.l_linger=10;
    setsockopt(hSocket,SOL_SOCKET, SO_LINGER,&lin,sizeof(lin));
    shutdown(hSocket, SHUT_RDWR);
    #endif

    printf("\nClosing the socket");

    if(close(hSocket) == SOCKET_ERROR){
        printf("\nCould not close socket\n");
        return;
    }
}


int main(int argc, char* argv[]){
    if(argc < 3){
        printf("\nUsage: server host-port serving-dir\n");
        return 0;
    }else{
        nHostPort = atoi(argv[1]);
        directory_root = argv[2];
        strcpy(_directory_root, argv[2]);
    }

    printf("\nStarting server");

    printf("\nMaking socket");
    hServerSocket=socket(AF_INET,SOCK_STREAM,0);

    if(hServerSocket == SOCKET_ERROR){
        printf("\nCould not make a socket\n");
        return 0;
    }

    Address.sin_addr.s_addr=INADDR_ANY;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

    printf("\nBinding to port %d\n",nHostPort);
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) == SOCKET_ERROR){
        printf("\nCould not connect to host\n");
        return 0;
    }

    getsockname( hServerSocket, (struct sockaddr *) &Address,(socklen_t *)&nAddressSize);
    printf("Opened socket as fd (%d) on port (%d) for stream i/o\n",hServerSocket, ntohs(Address.sin_port));

    printf("\nMaking a listen queue of %d elements",QUEUE_SIZE);

    if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR) {
        printf("\nCould not listen\n");
        return 0;
    }

    int optval = 1;
    setsockopt (hServerSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    for(;;){
        printf("\nWaiting for a connection\n");
        hSocket = accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);
        readSocket(hSocket);
    }
}
