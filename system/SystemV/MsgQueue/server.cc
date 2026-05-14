#include "MsgQueue.hpp"
#include <unistd.h>
int main(){
    Server server(BUILD_FLG);
    int cnt = 5;
    while(cnt--){
        std::string msg;
        server.Recv(msg, CLIENT);
        std::cout << msg << std::endl;
    }
}