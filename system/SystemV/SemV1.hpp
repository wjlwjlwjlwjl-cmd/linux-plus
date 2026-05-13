#pragma once
#include <sys/sem.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <iostream>
#include <memory>

#define BUILD_FLG  (IPC_CREAT | IPC_EXCL | 0666)
#define GET_FLG  0

class Semaphore
{
public:
    Semaphore(int semid, int flag)
    {
        _semid = semid;
        _flag = flag;
    }
    void P() {
        struct sembuf sb;
        sb.sem_num = 0;
        sb.sem_op = -1;
        sb.sem_flg = SEM_UNDO;
        int ret = semop(_semid, &sb, 1);
        if(ret == -1){
            perror("semop_P fail");
            return;
        }
    }
    void V() {
        struct sembuf sb;
        sb.sem_num = 0;
        sb.sem_op = 1;
        sb.sem_flg = SEM_UNDO;
        int ret = semop(_semid, &sb, 1);
        if(ret == -1){
            perror("semop_V fail");
            return;
        }
    }
    ~Semaphore()
    {
        if(_flag == GET_FLG){
            return;
        }
        int ret = semctl(_semid, 0, IPC_RMID);
        if(ret == -1){
            perror("semctl_rm fail");
        }
    }

private:
    int _semid;
    int _flag;
};


class SemaphoreBuilder
{
    const char *path = "/tmp";
    const int proj_id = 0x77;

public:

    SemaphoreBuilder() {}

    SemaphoreBuilder* SetVar(int val)
    {
        _val = val;
        return this;
    }

    std::shared_ptr<Semaphore> Build(int flag)
    {
        key_t k = ::ftok(path, proj_id); // 创建键值对
        if (k == -1)
        {
            perror("ftok fail");
            return nullptr;
        }

        int semid = semget(k, 1, flag); // 获取信号量
        if (semid == -1)
        {
            perror("semget fail");
            return nullptr;
        }

        if (flag == BUILD_FLG)
        {
            union semun
            {
                int val;
                struct semid_ds *buf;
                unsigned short *array;
            } un;
            un.val = _val;
            int ret = semctl(semid, 0, SETVAL, un);
            if(ret < 0){
                perror("semctl init fail");
                return nullptr;
            }
        }

        return std::make_shared<Semaphore>(semid, flag);
    }
    ~SemaphoreBuilder() {}

private:
    int _val;
};

//产品类

//建造者接口

//基于抽象的建造者接口的具体的建造者类

//指挥者类