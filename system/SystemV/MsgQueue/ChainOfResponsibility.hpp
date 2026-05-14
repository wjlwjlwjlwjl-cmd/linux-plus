#include <filesystem>
#include <memory>
#include <unistd.h>
#include <iostream>
#include <format>
#include <filesystem>
#include <fstream>

#define MAX_LINE 2

class HandlerText{
public:
    virtual void Execuate(const std::string& raw_string) = 0;

    void Enable(){
        _enable = true;
    }

    void Disable(){
        _enable = false;
    }

    void SetNextHandler(std::shared_ptr<HandlerText> next_hdl){
        _next_handler = next_hdl;
    }
protected:
    std::shared_ptr<HandlerText> _next_handler;
    bool _enable; //当前节点是否允许处理
};

class HandlerTextFormat: public HandlerText{
public:
    void Execuate(const std::string& raw_string) override{
        int pid = getpid();
        std::string ret = std::format("[{}]: {}", pid, raw_string);
        std::cout << ret << std::endl;
        if(_next_handler){
            _next_handler->Execuate(raw_string);
        }
        else{
            std::cout << "责任链在TextFormat完毕" << std::endl;
        }
    }
};
class HandlerTextSave: public HandlerText{
public:
    void Execuate(const std::string& raw_string) override{
        namespace fs = std::filesystem;
        try{
            fs::path p = "./tmp";
            if(!fs::exists(p)){
                fs::create_directories(p);
            }
            chdir("./tmp");
            int pid = getpid();
            std::ofstream out(std::to_string(pid), std::ios::app);
            out << raw_string << '\n';
            out.close();
            chdir("./..");
        }
        catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
        if(_next_handler){
            _next_handler->Execuate(raw_string);
        }
        else{
            std::cout << "责任链在HandlerTextSave完毕" << std::endl;
        }
    }
};
class HandlerTextBackupFile: public HandlerText{
public:
    void Execuate(const std::string& raw_string) override{
        try{
            chdir("./tmp");
            int pid = getpid();
            std::string filename = std::to_string(pid);
            std::ifstream in(filename);
            if(!in.is_open()){
                std::cout << "open file fail: " << std::endl;;
                return;
            }
            int cnt = 0;
            std::string buff;
            while(getline(in, buff)){ cnt += 1;}
            in.close();
            if(cnt >= MAX_LINE){
                std::string tgz = std::to_string(pid) + ".tgz";
                std::string raw_file = std::to_string(pid);
                std::cout << std::format("tgz: {}, raw: {}", tgz, raw_file) << std::endl;
                execlp("tar", "tar", "cvf", tgz.c_str(), raw_file.c_str(), nullptr);
                std::cout << "tar log fail" << std::endl;
            }
            else{
                std::cout << "lines in backup file: " << cnt << std::endl;
                std::cout << "(无需压缩)" << std::endl;
                std::cout << "(责任链完整结束)" << std::endl;
                chdir("./..");
            }
        }
        catch(const std::exception& e){
            std::cout << e.what() << std::endl;
        }
    }
};

class ChainEntry{
public:
    ChainEntry(const std::string& raw_file){
        std::shared_ptr<HandlerTextFormat> formatter = std::make_shared<HandlerTextFormat>();
        std::shared_ptr<HandlerTextSave> saver = std::make_shared<HandlerTextSave>();
        std::shared_ptr<HandlerTextBackupFile> backer = std::make_shared<HandlerTextBackupFile>();
        formatter->Enable();
        saver->Enable();
        backer->Enable();
        formatter->SetNextHandler(saver);
        saver->SetNextHandler(backer);
        formatter->Execuate(raw_file);
    }
private:
};