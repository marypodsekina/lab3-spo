#include "../headers/serv.h"
pthread_mutex_t sSocketLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sBufferLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sWriterCV = PTHREAD_COND_INITIALIZER;
pthread_cond_t sReaderCV = PTHREAD_COND_INITIALIZER;
int sEndFlag = 0;
int connections = 0;
atomic_long taskId = 0;
atomic_int messages = 0;

CustomList sBuffer;
ListItem sListRoot;

struct WrittenData{
    int fd;
    char user[32];
    int eFlag;
};

void* sWriterThread(void * params){

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    struct WrittenData* wData = (struct WrittenData*) params;

    while (!sEndFlag){
        pthread_cond_wait(&sWriterCV, &mutex);
        if(strcmp(wData->user, sBuffer.task.userName) == 0 && !wData->eFlag){
            pthread_mutex_lock(&sSocketLock);
            write(wData->fd, &sBuffer, sizeof(CustomList));
            pthread_mutex_unlock(&sSocketLock);
        }
        messages++;
        if(messages == connections){
            pthread_cond_signal(&sReaderCV);
        }
    }
    return NULL;
}

void* sReaderThread(void * voidParams){

    char user[32];
    int fd = *(int*) voidParams;
    CustomList tasks;
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    connections++;
    printf("Server have new client: %s. Now it has %i connections.\n", user, connections);

    read(fd, &tasks, sizeof(CustomList));
    strcpy(user, tasks.task.userName);

    pthread_mutex_lock(&sBufferLock);

    ListItem* currentItem = sListRoot.next;
    while (currentItem != NULL){
        if(strcmp(currentItem->task.userName, user) == 0){
            tasks.task = currentItem->task;
            tasks.type = ADD_TASK;
            pthread_mutex_lock(&sSocketLock);
            write(fd, &tasks, sizeof(CustomList));
            pthread_mutex_unlock(&sSocketLock);
        }
        currentItem = currentItem->next;
    }

    pthread_t writerThread;
    struct WrittenData data;
    data.fd = fd;
    data.eFlag = 0;
    strcpy(data.user, user);
    pthread_create(&writerThread, NULL, sWriterThread, &data);
    pthread_mutex_unlock(&sBufferLock);

    while(recv(fd, &tasks, sizeof(CustomList), 0) > 0) {

        strcpy(tasks.task.userName, user);
        if(tasks.type == ADD_TASK){
            tasks.task.creationTime = time(NULL);
            tasks.task.id = taskId++;
        }
        pthread_mutex_lock(&sBufferLock);
        doRequest(tasks, &sListRoot);
        messages = 0;
        sBuffer = tasks;
        pthread_cond_broadcast(&sWriterCV);
        pthread_cond_wait(&sReaderCV, &lock);
        pthread_mutex_unlock(&sBufferLock);
    }
    data.eFlag = 1;
    pthread_mutex_lock(&sBufferLock);
    connections--;
    printf("User with username %s has disconnected. Now server has %i connections\n", user, connections);
    pthread_cancel(writerThread);
    pthread_mutex_unlock(&sBufferLock);
    return NULL;
}

void* clientListenerThread(void * voidParams){
    int listenfd, connfd;
    struct sockaddr_in servAddr;
    pthread_t cReader;
    sListRoot.next = NULL;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        printf("It is impossible to create a socket.\n");
        return NULL;
    } else {
        printf("Socket created.\n");
    }

    memset(&servAddr, '0', sizeof(servAddr));

    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(25080);

    if ((bind(listenfd, (struct sockaddr*)&servAddr, sizeof(servAddr))) != 0) {
        printf("It is impossible to bind socket.\n");
        return NULL;
    } else {
        printf("Socket binded.\n");
    }

    while (!sEndFlag){
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        if (connfd < 0) {
            printf("It is impossible to accept server\n");
            return NULL;
        } else{
            pthread_create(&cReader, NULL, sReaderThread, (void *) &connfd);
        }
    }
    close(listenfd);
    return NULL;
}

int serverModeOn(){
    pthread_t listenerThread;
    pthread_create(&listenerThread, NULL, clientListenerThread, NULL);
    while (!sEndFlag){
        char c = getchar();
        if(c=='e'){
            sEndFlag = 1;
        }
    }
    return 0;
}
