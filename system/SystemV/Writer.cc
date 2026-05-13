#include "SemV2.hpp"
#include <ctime>

int main(){
    srand(time(0));
    SemaphoreBuilder sb;
    Director d;
    d.Direct(sb, BUILD_FLG, {0});
    pid_t pid = fork();
    if(pid == 0){
        d.Direct(sb);
        auto semc = sb.GetSem();
        int cnt = 30;
        while(cnt--){
            semc->P(0);
            int ms = rand() % 401;
            printf("C");
            fflush(stdout);
            usleep(ms);
            ms = rand() % 400;
            printf("C ");
            fflush(stdout);
            usleep(ms);
            semc->V(0);
        }
        exit(0);
    }
    int cnt = 50;
    auto semp = sb.GetSem();
    while(cnt--){
        semp->P(0);
        int ms = rand() % 200;
        printf("P");
        fflush(stdout);
        usleep(ms);
        ms = rand() % 201;
        printf("P ");
        fflush(stdout);
        usleep(ms);
        semp->V(0);
    }
    return 0;
}