#pragma once
#include <sys/sem.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <vector>

#define BUILD_FLG  (IPC_CREAT | IPC_EXCL | 0666)
#define GET_FLG  0

//产品类
class Semaphore{
public:
    Semaphore(int semid, int flag)
    {
        _semid = semid;
        _flag = flag;
    }
    void P(int who) {
        std::cout << "Product's P" << std::endl;
        struct sembuf sb;
        sb.sem_num = who;
        sb.sem_op = -1;
        sb.sem_flg = SEM_UNDO;
        int ret = semop(_semid, &sb, 1);
        if(ret == -1){
            perror("semop_P fail");
            return;
        }
    }
    void V(int who) {
        std::cout << "Product's V" << std::endl;
        struct sembuf sb;
        sb.sem_num = who;
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

//建造者接口
class SemaphoreBuilderInterface{
public:
    virtual void GetKey() = 0;
    virtual void GetFlag(int flag) = 0;
    virtual void GetVals(const std::vector<int>& vals) = 0;
    virtual std::shared_ptr<Semaphore> GetSem() = 0;
private:
    virtual std::shared_ptr<Semaphore> Init() = 0;
};

//基于抽象的建造者接口的具体的建造者类
class SemaphoreBuilder: SemaphoreBuilderInterface{
    const char *path = "/tmp";
    const int proj_id = 0x77;
public:
    void GetKey() override{
        key_t k = ::ftok(path, proj_id); // 创建键值对
        if (k == -1){
            perror("ftok fail");
        }
        _k = k;
    }

    void GetFlag(int flag) override{
        _flag = flag;
    }

    void GetVals(const std::vector<int>& vals) override{
        if(vals.size() <= 0 && _flag == BUILD_FLG){
            return;
        }
        _vals = vals;
    }

    std::shared_ptr<Semaphore> GetSem() override{
        int semid = semget(_k, _vals.size(), _flag); // 获取信号量
        if (semid == -1)
        {
            perror("semget fail");
        }
        _semid = semid;
        std::cout << "Builder finish working" << std::endl;
        return Init();
    }
private:    
    int _k;
    int _flag;
    std::vector<int> _vals;
    int _semid;

    std::shared_ptr<Semaphore> Init() override{
        if (_flag == BUILD_FLG)
        {
            for(int i = 0; i < _vals.size(); i++){
                union semun
                {
                    int val;
                    struct semid_ds *buf;
                    unsigned short *array;
                } un;
                un.val = _vals[i];
                int ret = semctl(_semid, i, SETVAL, un);
                if(ret < 0){
                    perror("semctl init fail");
                }
            }
        }
        return std::make_shared<Semaphore>(_semid, _flag);
    }
};

//指挥者类
class Director{
public:
    void Direct(SemaphoreBuilder& sb, int flag = GET_FLG, const std::vector<int>& vals = {}){
        std::cout << "Director working..." << std::endl;
        sb.GetKey();
        sb.GetFlag(flag);
        sb.GetVals(vals);
        sb.GetSem();
    }
private:
};