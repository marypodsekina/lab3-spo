#include "../headers/helper.h"

void changeTask(Task task, ListItem* list){
    ListItem *item = list->next;
    while (item != NULL){
        if(item->task.id == task.id){
            strcpy(item->task.taskName, task.taskName);
            strcpy(item->task.title, task.title);
            strcpy(item->task.desc, task.desc);
            item->task.deadlineTime = task.deadlineTime;
            break;
        }
        item = item->next;
    }
}

void addTask(Task task, ListItem* root){
    ListItem* newItem = malloc(sizeof(ListItem));
    newItem->task = task;
    if(root->next == NULL){
        newItem->next = NULL;
        root->next = newItem;
    } else{
        newItem->next = root->next;
        root->next = newItem;
    }
}

void deleteTask(Task task, ListItem* cur){
    while (cur->next != NULL){
        if(cur->next->task.id == task.id){
            ListItem * item = cur->next;
            cur->next = cur->next->next;
            free(item);
            break;
        }
        cur = cur->next;
    }
}

void deleteList(Task task, ListItem* cur){
    while (cur->next != NULL){
        if(strcmp(cur->next->task.taskName, task.taskName) == 0 && strcmp(cur->next->task.userName, task.userName) == 0){
            ListItem * item = cur->next;
            cur->next = cur->next->next;
            free(item);
        } else{
            cur = cur->next;
        }
    }
}

void changeList(Task task, ListItem* root){
    ListItem *cur = root->next;
    while (cur != NULL){
        if(strcmp(cur->task.taskName, task.taskName) == 0 && strcmp(cur->task.userName, task.userName) == 0){
            strcpy(cur->task.taskName, task.title);
        }
        cur = cur->next;
    }
}

void doRequest(CustomList taskList, ListItem* root){

    switch (taskList.type) {
        case CHANGE_TASK:{
            changeTask(taskList.task, root);
            return;
        }
        case ADD_TASK:{
            addTask(taskList.task, root);
            return;
        }
        case DELETE_TASK:{
            deleteTask(taskList.task, root);
            return;
        }
        case DELETE_LIST:{
            deleteList(taskList.task, root);
            return;
        }
        case CHANGE_LIST:{
            changeList(taskList.task, root);
            return;
        }
        default: return;
    }
}
