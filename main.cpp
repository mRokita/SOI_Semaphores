#include <iostream>
#include <cstring>
#include <wait.h>
#include "Buffer.h"
#include "A1Process.h"
#include "A2Process.h"
#include "semaphores.h"
#include "B1Process.h"
#include "B2Process.h"

void createSemaphores(){
    boost::interprocess::named_semaphore(boost::interprocess::create_only_t(), Semaphore::PRINT, 0);
    boost::interprocess::named_semaphore(boost::interprocess::create_only_t(), Semaphore::PUSH_OP, 1);
    boost::interprocess::named_semaphore(boost::interprocess::create_only_t(), Semaphore::POP_OP, 1);
    boost::interprocess::named_semaphore(boost::interprocess::create_only_t(), Semaphore::B1, 0);
    boost::interprocess::named_semaphore(boost::interprocess::create_only_t(), Semaphore::B2, 0);
    boost::interprocess::named_semaphore(boost::interprocess::create_only_t(), Semaphore::A1, 1);
    boost::interprocess::named_semaphore(boost::interprocess::create_only_t(), Semaphore::A2, 0);
}

void destroySemaphores(){
    boost::interprocess::named_semaphore::remove(Semaphore::PRINT);
    boost::interprocess::named_semaphore::remove(Semaphore::PUSH_OP);
    boost::interprocess::named_semaphore::remove(Semaphore::POP_OP);
    boost::interprocess::named_semaphore::remove(Semaphore::B1);
    boost::interprocess::named_semaphore::remove(Semaphore::B2);
    boost::interprocess::named_semaphore::remove(Semaphore::A1);
    boost::interprocess::named_semaphore::remove(Semaphore::A2);
}

void cleanup(){
    std::cout << "Cleanup..." << std::endl;
    destroySemaphores();
    Buffer::destroy();
}

void sigintHandler(int s){
    printf("Caught signal %d\n",s);
    cleanup();
    exit(0);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, sigintHandler);

    std::cout << "Spawning root process..." << std::endl;
    createSemaphores();
    Buffer::getInstance();
    std::vector<std::unique_ptr<Process>> children;
    children.emplace_back(new A1Process);
    children.emplace_back(new A2Process);
    children.emplace_back(new B1Process);
    children.emplace_back(new B2Process);
    int pid;
    std::cout << "Root process spawned!" << std::endl;
    for(auto& p: children){
        pid = fork();
        if (pid < 0){
            std::cerr << "Fork error!!!" << std::endl;
        } else if (pid == 0) {
            p->run();
            break; // child process
        }
    }
    if(pid != 0){
        while(Buffer::alive){
            sleep(1);
        }
    }
    return 0;
}
