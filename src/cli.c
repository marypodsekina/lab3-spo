#include <ncurses.h>
#include "../headers/helper.h"


WINDOW *windows[4];
char* windowsName[4] = {"Lists", "Tasks", "Task details", "Editor"};
int curWind = 0;
Task currentList;
Task currentTask;
int curListNumb = 0;
int totalListNumb = 0;
int curTaskNumb = 0;
int totalTaskNumb = 0;
int curDetailNumb = 0;
char editor[256];
ListItem rootOfTaskList;
ListItem rootOfLists;
int eFlag = 0;
int cSocketFD;

//void updateEditWind(){};
//void updateListWind(){};
//void updateTaskWind(){};
//void updateDetailsWind(){};

void updateListList(){
    ListItem *list = &rootOfLists;
    ListItem *task = &rootOfTaskList;
    while (list->next != NULL){
        ListItem *item = list->next;
        list->next = list->next->next;
        free(item);
    }
    while (task != NULL){
        list = rootOfLists.next;
        int flag = 0;
        while(list != NULL){
            if(strcmp(list->task.taskName, task->task.taskName) == 0){
                flag = 1;
                break;
            }
            list = list->next;
        }
        if(!flag){
            addTask(task->task, &rootOfLists);
        }
        task = task->next;
    }
}

void* cReaderThread(){
    CustomList list;
    while (!eFlag){
        read(cSocketFD, &list, sizeof(CustomList));
        doRequest(list, &rootOfTaskList);
        if(list.type == DELETE_LIST || list.type == DELETE_TASK){
            currentTask.id = -1;
            curTaskNumb = 0;
            curListNumb = 0;
        }
        updateListList();
        updateListWind();
    }
    return NULL;
}

void createSocket(char* userName, char* host){

    struct sockaddr_in serverAddr;
    cSocketFD = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serverAddr, '0', sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(host);
    serverAddr.sin_port = htons(25080);

    if (connect(cSocketFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) {
        printf("It is impossible to connect with server.\n");
        exit(0);
    }

    pthread_t rec;
    pthread_create(&rec, NULL, cReaderThread, NULL);
    CustomList list;
    strcpy(list.task.userName, userName);
    list.type = HELP;
    write(cSocketFD, &list, sizeof(CustomList));
}

void sendReq(int type, Task task){
    CustomList list;
    list.type = type;
    list.task = task;
    write(cSocketFD, &list, sizeof(CustomList));
}

void updateListWind(){
    wclear(windows[0]);
    curWind == 0 ? box(windows[0], '|', '-') : box(windows[0], 0, 0);
    wprintw(windows[0], windowsName[0]);
    int count = 0;
    ListItem* item = rootOfLists.next;
    while (item != NULL){
        if(count == curListNumb){
            mvwprintw(windows[0], count + 2, 1, ">");
            currentList = item->task;
        }
        mvwprintw(windows[0], count + 2, 2, item->task.taskName);
        item = item->next;
        count++;
    }
    totalListNumb = count - 1;
    wrefresh(windows[0]);
    updateTaskWind();
}

void updateTaskWind(){
    wclear(windows[1]);
    curWind == 1 ? box(windows[1], '|', '-') : box(windows[1], 0, 0);
    wprintw(windows[1], windowsName[1]);
    int count = 0;
    currentTask.id = -1;
    ListItem* item = rootOfTaskList.next;
    while (item != NULL){
        if(strcmp(item->task.taskName, currentList.taskName) == 0){
            if(count == curTaskNumb){
                mvwprintw(windows[1], count + 2, 1, "-->");
                currentTask = item->task;
            }
            mvwprintw(windows[1], count + 2, 2, item->task.title);
            count++;
        }
        item = item->next;
    }
    totalTaskNumb = count;
    wrefresh(windows[1]);
    updateDetailsWind();
}

void updateDetailsWind(){
    wclear(windows[2]);
    curWind == 2 ? box(windows[2], '|', '-') : box(windows[2], 0, 0);
    wprintw(windows[2], windowsName[2]);
    struct tm time;
    char buf[32];
    if(currentTask.id != -1){
        mvwprintw(windows[2], 2, 2, "Title:");
        mvwprintw(windows[2], 2, 15, currentTask.title);

        mvwprintw(windows[2], 3, 2, "Tasks:");
        mvwprintw(windows[2], 3, 15, currentTask.taskName);

        mvwprintw(windows[2], 4, 2, "Task description:");
        mvwprintw(windows[2], 4, 15, currentTask.desc);

        time = *localtime(&currentTask.creationTime);
        sprintf(buf, "Birth:     %d-%02d-%02d", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday);
        mvwprintw(windows[2], 5, 2, buf);

        if(currentTask.deadlineTime != 0){
            time = *localtime(&currentTask.deadlineTime);
            sprintf(buf, "Deadline:    %d-%02d-%02d", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday);
            mvwprintw(windows[2], 6, 2, buf);
        } else{
            mvwprintw(windows[2], 6, 2, "Deadline:    -");
        }

        mvwprintw(windows[2], curDetailNumb + 2, 1, ">");
        if(curDetailNumb == 0){
            strcpy(editor, currentTask.title);
        } else if (curDetailNumb == 1){
            strcpy(editor, currentTask.taskName);
        } else if(curDetailNumb == 2){
            strcpy(editor, currentTask.desc);
        }
    }
    wrefresh(windows[2]);
    updateEditWind();
}

void updateEditWind(){
    wclear(windows[3]);
    curWind == 3 ? box(windows[3], '|', '-') : box(windows[3], 0, 0);
    wprintw(windows[3], windowsName[3]);
    mvwprintw(windows[3], 2, 2, editor);
    wrefresh(windows[3]);
}

int clientModeOn(char* userName, char* host){
    createSocket(userName, host);
    rootOfTaskList.next = NULL;
    rootOfLists.next = NULL;
    const int ww = 20;
    const int wh = 7;
    noecho();
    raw();
    curs_set(0);
    refresh();
    initscr();
    windows[0] = newwin(LINES - wh, ww, 0, 0);
    windows[1] = newwin(LINES - wh, ww, 0, ww);
    windows[2] = newwin(LINES - wh, COLS - ww * 2, 0, ww * 2);
    windows[3] = newwin(wh, COLS, LINES - wh, 0);
    updateListWind();

    while (!eFlag){
        char c = getch();
        if(c == 27){
            eFlag = 1;
        }
        else if(c == 10){
            if (curWind == 2){
                if(currentTask.id >= 0){
                    sendReq(CHANGE_TASK, currentTask);
                } else{
                    sendReq(ADD_TASK, currentTask);
                }
            }
            if (curWind == 3){
                if(curDetailNumb == 0){
                    strcpy(currentTask.title, editor);
                }
                if(curDetailNumb == 1){
                    strcpy(currentTask.taskName, editor);
                }
                if(curDetailNumb == 2){
                    strcpy(currentTask.desc, editor);
                }
                curWind = 2;
                updateDetailsWind();
            }
        }
        else if(c == 9){
            curWind = (curWind + 1) % (currentTask.id == -1 ? 2 : 4);
            if(curWind == 3){
                updateDetailsWind();
            } else{
                updateListWind();
            }
        }
        else if (curWind == 3){
            strncat(editor, &c, 1);
            updateEditWind();
        }
        else if(c == 38){
            if(curWind == 0){
                curListNumb = curListNumb == 0 ? totalListNumb - 1 : curListNumb - 1;
                updateListWind();
            }
            if(curWind == 1){
                curTaskNumb = curTaskNumb == 0 ? totalTaskNumb - 1 : curTaskNumb - 1;
                updateTaskWind();
            }
            if(curWind == 2){
                curDetailNumb = curDetailNumb == 0 ? 2 : curDetailNumb - 1;
                updateDetailsWind();
            }
        }
        else if(c == 40){
            if(curWind == 0){
                curListNumb = (curListNumb + 1) % totalListNumb;
                updateListWind();
            }
            if(curWind == 1){
                curTaskNumb = (curTaskNumb + 1) % totalTaskNumb;
                updateTaskWind();
            }
            if(curWind == 2){
                curDetailNumb = (curDetailNumb + 1) % 3;
                updateDetailsWind();
            }
        }
        else if(c == 'd'){
            if(curWind == 0){
                sendReq(DELETE_LIST, currentList);
            }
            if(curWind == 1){
                sendReq(DELETE_TASK, currentTask);
            }
        }
        else if((c == 'c') && curWind == 1){
            currentTask.id = -2;
            strcpy(currentTask.title, "");
            strcpy(currentTask.desc, "");
            strcpy(currentTask.taskName, "");
            currentTask.creationTime = time(NULL);
            curWind = 2;
            updateDetailsWind();
        }
    }
    endwin();
    return 0;
}



