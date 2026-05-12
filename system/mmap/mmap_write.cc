#include <iostream>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <format>
#include <string>

int main(int argc, char* argv[]){
    std::string filename = "log.txt";
    std::cout << filename << std::endl;
    int fd = ::open(filename.c_str(), O_CREAT | O_RDWR);
    if(fd < 0){
        perror("open file fail");
        return 1;
    }
    ::ftruncate(fd, 4096);
    char* mmap_ptr = (char*)mmap(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(mmap_ptr == MAP_FAILED){
        perror("mmap映射失败");
        return 1;
    }
    for(int i = 0; i < 4096; i++){
        mmap_ptr[i] = 'a' + i % 26;
        sleep(1);
    }
    ::munmap(mmap_ptr, 4096);
    ::close(fd);
    return 0;
}