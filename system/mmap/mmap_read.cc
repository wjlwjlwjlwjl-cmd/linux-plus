#include <iostream> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(){
    const char* filename = "log.txt";
    int fd = ::open(filename, O_RDONLY);
    if(fd < 0){
        perror("open file fail");
        return 1;
    }
    struct stat fs;
    stat(filename, &fs);
    char* mmap_ptr = (char*)mmap(nullptr, fs.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if(mmap_ptr == MAP_FAILED){
        perror("mmap fail");
        return 1;
    }
    std::cout << mmap_ptr << std::endl;
    ::munmap(mmap_ptr, fs.st_size);
    ::close(fd);
    return 0;
}