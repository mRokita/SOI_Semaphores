#ifndef SOI3_A1PROCESS_H
#define SOI3_A1PROCESS_H
#include <iostream>
#include "Process.h"
#include <boost/interprocess/sync/named_semaphore.hpp>

class A1Process : public Process {
    void run() override {
        Buffer* buf = Buffer::getInstance();
        int cnum = 0;
        while (buf->alive) {
            buf->pushEven(cnum);
            cnum += 2;
            cnum = cnum % 100;
        }
    }
};
#endif //SOI3_A1PROCESS_H
