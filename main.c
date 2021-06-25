#include "headers/cli.h"
#include "headers/serv.h"

int main(int argc, char *argv[]) {
    if(argc <= 1){
        printf("Choose client or server mode");
        return 0;
    }
    if(strcmp(argv[1], "client") == 0){
        if(argc <= 2){
            printf("Set username");
            return 0;
        }
        if(argc <= 3){
            printf("Set server address");
            return 0;
        }
        return clientModeOn(argv[2], argv[3]);
    }
    if(strcmp(argv[1], "server") == 0){
        return serverMode();
    }
}
