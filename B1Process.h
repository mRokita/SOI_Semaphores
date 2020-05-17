#ifndef SOI3_B1PROCESS_H
#define SOI3_B1PROCESS_H

#include "Buffer.h"
#include "Process.h"
#include <iostream>

class B1Process : public Process {
    void run() override {
        Buffer* buf = Buffer::getInstance();
        while (Buffer::alive) {
            buf->popEven();
        }
    }
};


#endif //SOI3_B1PROCESS_H
