#ifndef SOI3_A2PROCESS_H
#define SOI3_A2PROCESS_H

#include "Process.h"
#include <iostream>

class A2Process : public Process {
public:
    void run() override {
        Buffer* buf = Buffer::getInstance();
        int cnum = 1;
        while (Buffer::alive) {
            buf->pushOdd(cnum);
            cnum += 2;
            cnum = cnum % 100;
        }
    }
};
#endif //SOI3_A2PROCESS_H
