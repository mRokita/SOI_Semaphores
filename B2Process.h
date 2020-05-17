#ifndef SOI3_B2PROCESS_H
#define SOI3_B2PROCESS_H

#include <iostream>
#include "Buffer.h"
#include "Process.h"

class B2Process : public Process {
    void run() override {
        Buffer* buf = Buffer::getInstance();
        while (Buffer::alive) {
            buf->popOdd();
        }
    }
};


#endif //SOI3_B2PROCESS_H
