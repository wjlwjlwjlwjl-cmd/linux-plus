#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <stdint.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <format>
#include <string.h>
#include <signal.h>

enum class State{
    NEW,
    CONNECTING,
    CONNECTED,
    CLOSED,
    DISCONNECTED
};

class ConnectionClient{
public:
    ConnectionClient(const std::string& server_ip, uint16_t port, int max_retries = 3, int retry_time = 2)
        : _server_ip(server_ip)
        , _port(port)
        , _state(State::NEW)
        , _max_retries(max_retries)
        , _retry_time(retry_time)
        , _sock(-1)
    {}

    State GetState(){
        return _state;
    }

    void Connect(){
        _state = State::CONNECTING;
        _sock = socket(AF_INET, SOCK_STREAM, 0);
        if(_sock == -1){
            perror("create sock fail: ");
            _state = State::CLOSED;
            close(_sock);            
            _sock = -1;
            return;
        }
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(_port);
        addr.sin_addr.s_addr = inet_addr(_server_ip.c_str());
        int ret = connect(_sock, (sockaddr*)&addr, sizeof(addr));
        if(ret == -1){
            perror("connect fail: ");
            _state = State::CLOSED;
            close(_sock);            
            _sock = -1;
            return;
        }
        _state = State::CONNECTED;
        std::cout << "connect success! " << std::endl;
    }

    void Reconnect(){
        _state = State::CONNECTING;
        int cnt = 0;
        while(cnt < _max_retries){
            cnt++;
            _sock = socket(AF_INET, SOCK_STREAM, 0);
            if(_sock == -1){
                perror("[retry] create sock fail: ");
                sleep(_retry_time);
                continue;
            }
            struct sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(_port);
            addr.sin_addr.s_addr = inet_addr(_server_ip.c_str());
            int ret = connect(_sock, (sockaddr*)&addr, sizeof(addr));
            if(ret == -1){
                perror(std::format("[retry] connect fail {}: ", cnt).c_str());
                _state = State::CLOSED;
                close(_sock);            
                _sock = -1;
                sleep(_retry_time);
            }
            else{
                _state = State::CONNECTED;
                std::cout << "reconnect success! " << std::endl;
                return;
            }
        }
        _state = State::DISCONNECTED;
        std::cout << "[retry] no more retry, fail" << std::endl;
    }

    void Process(){
        if(_sock == -1){
            _state = State::CLOSED;
            return;
        }
        std::string msg = "hello server";
        char buf[1024];
        int n = send(_sock, msg.c_str(), msg.size(), MSG_NOSIGNAL);
        if(n < 0){perror("send fail: "); _state = State::CLOSED; return;}
        n = recv(_sock, buf, sizeof(buf) - 1, 0);
        buf[n] = 0;
        if(n < 0){perror("recv fail: "); _state = State::CLOSED; return;}
        std::cout << "[client]: " << buf << std::endl;
    }

    void Disconnect(){
        if(_sock > 0){
            close(_sock);
            _sock = -1;
        }
        std::cout << "disconnect success" << std::endl;
    }
private:
    int _sock;    
    std::string _server_ip;
    uint16_t _port;
    State _state;
    int _max_retries;
    int _retry_time;
};

class TCPClient{
public:
    TCPClient(const std::string& server_ip, uint16_t port)
        : _cc(server_ip, port)
    {}

    void Run(){
        while(1){
            switch(_cc.GetState()){
            case State::NEW:
                _cc.Connect();
                break;
            case State::CONNECTED:
                _cc.Process();
                break;
            case State::DISCONNECTED:
                return;
            case State::CLOSED:
                _cc.Reconnect();
                break;
            default:
                std::cout << "unknown state" << std::endl;
                exit(1);
            }
            sleep(1);
        }
    }
private:
    ConnectionClient _cc;
};

int main(){
    signal(SIGPIPE, SIG_IGN);
    TCPClient client("127.0.0.1", 8080);
    client.Run();
    return 0;
}