#include "Sem.hpp"
#include <ctime>

int main(){
    srand(time(0));
    SemaphoreBuilder sb;
    auto semp = sb.SetVar(1)->Build(BUILD_FLG);
    pid_t pid = fork();
    if(pid == 0){
        auto semc = sb.Build(GET_FLG);
        int cnt = 30;
        while(cnt--){
            semc->P();
            int ms = rand() % 401;
            printf("C");
            fflush(stdout);
            usleep(ms);
            ms = rand() % 400;
            printf("C ");
            fflush(stdout);
            usleep(ms);
            semc->V();
        }
        exit(0);
    }
    int cnt = 50;
    while(cnt--){
        semp->P();
        int ms = rand() % 200;
        printf("P");
        fflush(stdout);
        usleep(ms);
        ms = rand() % 201;
        printf("P ");
        fflush(stdout);
        usleep(ms);
        semp->V();
    }
    return 0;
}