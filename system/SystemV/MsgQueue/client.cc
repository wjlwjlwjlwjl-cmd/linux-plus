#include "MsgQueue.hpp"
#include <unistd.h>
int main(){
    Server server;
    while(1){
        std::cout << ">> ";
        std::string msg;
        std::cin >> msg;
        server.Send(msg, CLIENT); 
    }
    return 0;
}