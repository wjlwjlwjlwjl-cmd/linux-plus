#include <sys/msg.h>
#include <iostream>
#include <sys/ipc.h>
#include <string>
#include <string.h>
#include "ChainOfResponsibility.hpp"

#define proj_id 0x77
#define filepath "/tmp"
#define SIZE 1024
#define CLIENT 1

#define BUILD_FLG (IPC_CREAT | IPC_EXCL | 0666)
#define GET_FLG 0

struct mymsg {
    long   mtype;       /* Message type. */
    char   mtext[SIZE];    /* Message text. */
};

class MsgQueue{
public:
    void GetKey(){
        _k = ::ftok(filepath, proj_id);
        if(_k == -1){
            perror("GetKey fail: ");
            return;
        }
    }

    void GetMsgQueue(int flag){
        _msqid = ::msgget(_k, flag);
        if(_msqid < 0){
            perror("GetMsgQueue fail: ");
            return;
        }
    }

    void Destroy(){
        int ret = ::msgctl(_msqid, IPC_RMID, nullptr);
        (void)ret;
    }

    void Send(const std::string& text, long type){
        struct mymsg mm;
        mm.mtype = type;
        memset(mm.mtext, 0, text.size());
        strncpy(mm.mtext, text.c_str(), text.size());
        int ret = ::msgsnd(_msqid, &mm, sizeof(mm.mtext), 0);
        if(ret == -1){
            perror("msgsnd fail: ");
        }
    }

    std::string& Recv(std::string& msg, long type){
        struct mymsg mm;
        mm.mtype = type;
        int ret = ::msgrcv(_msqid, &mm, SIZE, type, 0);
        if(ret == -1){
            perror("msgrcv fail");
            return msg;
        }
        mm.mtext[ret] = 0;
        msg = mm.mtext;
        ChainEntry ce(msg);
        return msg;
    }
private:
    key_t _k;
    int _msqid;
};

class Server: public MsgQueue{
public:
    Server(int flag = GET_FLG){
        GetKey();
        GetMsgQueue(flag);
    }
    ~Server(){
        Destroy();
    }
private:
};