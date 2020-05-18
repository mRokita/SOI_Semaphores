#include <iostream>
#include <cstring>
#include <wait.h>
#include "Buffer.h"
#include "A1Process.h"
#include "A2Process.h"
#include "semaphores.h"
#include "B1Process.h"
#include "B2Process.h"

const char* semaphores[] = {Semaphore::QUEUE_OP, Semaphore::B1, Semaphore::B2, Semaphore::A1, Semaphore::A2};

void createSemaphores(){
    for(const char* sem: semaphores) {
        boost::interprocess::named_semaphore(boost::interprocess::create_only_t(), sem, 1);
    }
    boost::interprocess::named_semaphore(boost::interprocess::open_only_t(), Semaphore::QUEUE_OP).post();
    boost::interprocess::named_semaphore(boost::interprocess::open_only_t(), Semaphore::A1).post();
}

void destroySemaphores(){
    for(const char* sem: semaphores) {
        boost::interprocess::named_semaphore::remove(sem);
    }
}

void cleanup(){
    std::cout << "Cleanup..." << std::endl;
    destroySemaphores();
    Buffer::destroy();
}

void sigintHandler(int s){
    std::cout << std::endl << "SIGINT" << std::endl;
    cleanup();
    exit(0);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, sigintHandler);

    std::cout << "Spawning root process..." << std::endl;
    Buffer::getInstance();
    createSemaphores();
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
            sleep(100);
        }
    }
    return 0;
}
