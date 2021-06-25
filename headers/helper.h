#pragma once

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>
#define MAX 80

typedef int RequestType;
#define ADD_TASK 0
#define CHANGE_TASK 1
#define DELETE_TASK 2
#define CHANGE_LIST 3
#define DELETE_LIST 4
#define HELP 5

typedef struct{
    long id;
    char userName[16];
    char taskName[32];
    char title[32];
    char desc[256];
    time_t creationTime;
    time_t deadlineTime;
}Task;

typedef struct{
    RequestType type;
    Task task;
}CustomList;

typedef struct ListItem{
    Task task;
    struct ListItem* next;
}ListItem;

void doRequest(CustomList taskList, ListItem* root);
void changeTask(Task task, ListItem* list);
void addTask(Task task, ListItem* root);
void deleteTask(Task task, ListItem* cur);
void deleteList(Task task, ListItem* cur);
void changeList(Task task, ListItem* root);

