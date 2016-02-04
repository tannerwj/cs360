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

#include <semaphore.h>
#include <pthread.h>
#include <queue>
#include <sys/signal.h>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         10000
#define QUEUE_SIZE          20
#define MAXPATH             1000

using namespace std;

int hServerSocket;  /* handle to socket */
struct hostent* pHostInfo;   /* holds info about a machine */
struct sockaddr_in Address; /* Internet socket address stuct */
int nAddressSize = sizeof(struct sockaddr_in);
int nHostPort;

char* client_request;
int num_threads;
string directory_root;
char _directory_root[MAXPATH];

queue<int> connections;

sem_t work_mutex;
sem_t work_to_do;
sem_t space_on_q;

struct thread_info{
    int thread_id;
};

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

void sendFile (char * path, int socket, long length){
    char fullpath[MAXPATH];
    sprintf(fullpath,"%s%s",_directory_root, path);
    FILE *fp = fopen(fullpath,"r");
    char *buffer = (char *)malloc(length + 1);
    fread(buffer, length, 1, fp);
    write(socket, buffer, length);
    cout << "Sent " << path << " to client" << endl;
    free(buffer);
    fclose(fp);
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

            sprintf(pBuffer,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", contentType.c_str(), contentLength);
            write(hSocket, pBuffer, strlen(pBuffer));

            sendFile(path, hSocket, contentLength);
        }else{
            cout << "Client requested a folder without an index" << endl;
            string index = createIndex(directory_root, path);
            sprintf(pBuffer,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n", index.length());

            write(hSocket, pBuffer, strlen(pBuffer));
            write(hSocket, index.c_str(), strlen(index.c_str()));
        }
    }else{
        string contentType = getContentType(path);
        long contentLength = getContentLength(directory_root + path);

        sprintf(pBuffer,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", contentType.c_str(), contentLength);
        write(hSocket, pBuffer, strlen(pBuffer));

        sendFile(path, hSocket, contentLength);
    }

    #ifdef notdef
    linger lin;
    unsigned int y=sizeof(lin);
    lin.l_onoff=1;
    lin.l_linger=10;
    setsockopt(hSocket,SOL_SOCKET, SO_LINGER,&lin,sizeof(lin));
    shutdown(hSocket, SHUT_RDWR);
    #endif

    printf("Closing the socket\n");

    if(close(hSocket) == SOCKET_ERROR){
        printf("\nCould not close socket\n");
        return;
    }
}

void* handler (void* data){
    struct thread_info* info = (struct thread_info*) data;
    int id = info->thread_id;

    for(;;){
        sem_wait(&work_to_do);
        sem_wait(&work_mutex);

        int hSocket = connections.front();
        connections.pop();

        printf("\nThread #%d received connection\n", id);
        readSocket(hSocket);

        sem_post(&work_mutex);
        sem_post(&space_on_q);
    }
}

int main(int argc, char* argv[]){
    if(argc < 4){
        printf("\nUsage: server host-port num-threads root-dir\n");
        return 0;
    }else{
        nHostPort = atoi(argv[1]);
        num_threads = atoi(argv[2]);
        directory_root = argv[3];
        strcpy(_directory_root, argv[3]);
    }
    cout << "Setting up server" << endl;

    sem_init(&work_mutex, 0, 1);
    sem_init(&work_to_do, 0, 0);
    sem_init(&space_on_q, 0, 100);

    pthread_t threads[num_threads];
    struct thread_info all_thread_info[num_threads];

    printf("Creating thread pool with %d threads\n", num_threads);

    for( int i = 0; i < num_threads; i++ ){
        sem_wait(&work_mutex);
        all_thread_info[i].thread_id = i;
        pthread_create(&threads[i], NULL, handler, (void*) &all_thread_info[i]);
        sem_post(&work_mutex);
    }

    signal(SIGPIPE, SIG_IGN);

    hServerSocket=socket(AF_INET,SOCK_STREAM,0);

    if(hServerSocket == SOCKET_ERROR){
        printf("\nCould not make a socket\n");
        return 0;
    }

    Address.sin_addr.s_addr=INADDR_ANY;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

    printf("Starting server on port %d\n",nHostPort);
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) == SOCKET_ERROR){
        printf("\nCould not connect to host\n");
        return 0;
    }

    getsockname( hServerSocket, (struct sockaddr *) &Address,(socklen_t *)&nAddressSize);

    printf("Making a listen queue of length %d\n", QUEUE_SIZE);

    if(listen(hServerSocket, QUEUE_SIZE) == SOCKET_ERROR) {
        printf("\nCould not listen\n");
        return 0;
    }

    int optval = 1;
    setsockopt (hServerSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    for(;;){
        printf("\nWaiting for a connection\n");
        int hSocket = accept(hServerSocket, (struct sockaddr*)&Address, (socklen_t *)&nAddressSize);

        if(hSocket < 0){
            cout << "Connection error";
            return 0;
        }

        sem_wait(&space_on_q);
        sem_wait(&work_mutex);

        printf("\nSocket made, pushing on queue\n");
        connections.push(hSocket);

        sem_post(&work_mutex);
        sem_post(&work_to_do);
    }
}
