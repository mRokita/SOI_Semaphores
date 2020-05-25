#include <iostream>
#include <wait.h>
#include "Buffer.h"
#include "A1Process.h"
#include "A2Process.h"
#include "B1Process.h"
#include "B2Process.h"

void cleanup(){
    Buffer::destroy();
}

void sigintHandler(int s){
    cleanup();
    exit(0);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, sigintHandler);

    std::cout << "Spawning root process..." << std::endl;
    Buffer* buf = Buffer::getInstance();
    std::vector<std::unique_ptr<Process>> children;
    children.emplace_back(new A1Process);
    children.emplace_back(new A2Process);
    children.emplace_back(new B1Process);
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
        while(buf->alive){
            sleep(100);
        }
    }
    return 0;
}
