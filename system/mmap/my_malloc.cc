#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <string.h>

void* my_malloc(int size){
    if(size <= 0){
        perror("incorrect size");
        return nullptr;
    }
    void* ret_ptr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(ret_ptr == MAP_FAILED){
        perror("mmap fail");
        return nullptr;
    }
    return ret_ptr;
}

void my_free(void* ptr, int size){
    if(ptr == nullptr || size <= 0){
        return;
    }
    ::munmap(ptr, size);
}

int main(){
    char* sentence = (char*)my_malloc(4096);
    memset(sentence, 'a', 4095);
    sentence[4095] = 0; 
    std::cout << sentence << std::endl;
    my_free(sentence, 4096);
    return 0;
}