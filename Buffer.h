#include <boost/interprocess/managed_shared_memory.hpp>
#include <cstring>
#include <boost/interprocess/sync/named_semaphore.hpp>
#include "semaphores.h"
#include <deque>

#ifndef SOI3_BUFFER_H
#define SOI3_BUFFER_H

using namespace boost::interprocess;
class Buffer {
public:
    static inline managed_shared_memory segment = managed_shared_memory (open_or_create, "soi3_buffer_6", 128536);
    short queue[10000];
    int begin = 0;
    int end = 0;
    int evenCount = 0;
    int oddCount = 0;
    static inline bool alive = true;

    static Buffer* getInstance(){
        std::pair<Buffer *, std::size_t> sharedMemory = segment.find<Buffer>(unique_instance);
        if(sharedMemory.first == nullptr){
            std::cout << "Init buffer..." << std::endl;
            return segment.construct<Buffer>(unique_instance)();
        } else {
            std::cout << "Retrieve buffer..." << std::endl;
            return sharedMemory.first;
        }
    }

    static void destroy(){
        alive = false;
        segment.destroy<Buffer>(unique_instance);
    }

    void pushEven(short num){
        boost::interprocess::named_semaphore a1Sem(boost::interprocess::open_only_t(), Semaphore::A1);
        a1Sem.wait();
        push(num);
    }

    void pushOdd(short num){
        boost::interprocess::named_semaphore a2Sem(boost::interprocess::open_only_t(), Semaphore::A2);
        a2Sem.wait();
        push(num);
    }

    void popEven(){
        boost::interprocess::named_semaphore b1Sem(boost::interprocess::open_only_t(), Semaphore::B1);
        b1Sem.wait();
        pop(0);
    }

    void popOdd(){
        boost::interprocess::named_semaphore b2Sem(boost::interprocess::open_only_t(), Semaphore::B2);
        b2Sem.wait();
        pop(1);
    }

private:
    void push(short num){
        boost::interprocess::named_semaphore sem(boost::interprocess::open_only_t(), Semaphore::PUSH_OP);
        sem.wait();
        queue[end] = num;
        end ++;
        num % 2 ? oddCount ++ : evenCount ++;
        afterOperation();
        sem.post();
    }

    void pop(short odd){
        boost::interprocess::named_semaphore sem(boost::interprocess::open_only_t(), Semaphore::PUSH_OP);
        sem.wait();
        if(queue[begin] % 2 == odd){
            begin ++;
            odd == 1 ? oddCount -- : evenCount --;
        }
        afterOperation();
        sem.post();
    }

    void afterOperation(){
        boost::interprocess::named_semaphore a1Sem(boost::interprocess::open_only_t(), Semaphore::A1);
        boost::interprocess::named_semaphore a2Sem(boost::interprocess::open_only_t(), Semaphore::A2);
        boost::interprocess::named_semaphore b1Sem(boost::interprocess::open_only_t(), Semaphore::B1);
        boost::interprocess::named_semaphore b2Sem(boost::interprocess::open_only_t(), Semaphore::B2);

        for(int i=begin; i<end; ++i){
            std::cout << queue[i] << " ";
        }
        std::cout << std::endl;
        if(evenCount < 10) a1Sem.post();
        if(oddCount < evenCount) a2Sem.post();
        if(evenCount >= 3) b1Sem.post();
        if(oddCount >= 7) b2Sem.post();
        sleep(1);
        std::cout << oddCount << "-" << evenCount << std::endl;
    }

};
#endif //SOI3_BUFFER_H
