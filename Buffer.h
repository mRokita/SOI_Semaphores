#include <boost/interprocess/managed_shared_memory.hpp>
#include <cstring>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <deque>

#ifndef SOI3_BUFFER_H
#define SOI3_BUFFER_H

const int QUEUE_MAX_SIZE = 100000;
const char* SEGMENT_NAME = "SOI4_BUFFER";

using namespace boost::interprocess;
class Buffer {
public:
    static inline managed_shared_memory segment = managed_shared_memory (open_or_create, SEGMENT_NAME, 1000000);
    short queue[QUEUE_MAX_SIZE];
    int begin = 0;
    int end = 0;
    int evenCount = 0;
    int oddCount = 0;
    bool alive = true;
    interprocess_mutex mutex;
    interprocess_condition A1Condition;
    interprocess_condition A2Condition;
    interprocess_condition B1Condition;
    interprocess_condition B2Condition;



    static Buffer* getInstance(bool ref = true){
        std::pair<Buffer *, std::size_t> sharedMemory = segment.find<Buffer>(unique_instance);
        Buffer* inst;
        if(sharedMemory.first == nullptr){
            std::cout << "Init buffer..." << std::endl;
            inst = segment.construct<Buffer>(unique_instance)();
        } else {
            inst = sharedMemory.first;
        }
        if (ref) inst->references ++;
        return inst;
    }

    static void destroy(){
        getInstance(false)->cleanup();
    }

    void cleanup(){
        alive = false;
        references --;
        if(references == 0 ) {
            std::cout << "Received SIGINT..." << std::endl;
            shared_memory_object::remove(SEGMENT_NAME);
            mutex.unlock();
            A1Condition.notify_all();
            A2Condition.notify_all();
            B1Condition.notify_all();
            B2Condition.notify_all();
            std::cout << "Destroyed shared memory segment" << std::endl;
        }
    }

    void pushEven(short num){
        scoped_lock<interprocess_mutex> lock(mutex);
        if (evenCount >= 10) {
            A1Condition.wait(lock);
        }
        push(num);
    }

    void pushOdd(short num){
        scoped_lock<interprocess_mutex> lock(mutex);
        if (evenCount >= oddCount){
            A2Condition.wait(lock);
        }
        push(num);
    }

    void popEven(){
        scoped_lock<interprocess_mutex> lock(mutex);
        if (evenCount < 3) {
            B1Condition.wait(lock);
        }
        pop(0);
    }

    void popOdd(){
        scoped_lock<interprocess_mutex> lock(mutex);
        if (oddCount < 7){
            B2Condition.wait(lock);
        }
        pop(1);
    }

private:
    void push(short num){
        if(!alive) return;
        queue[end] = num;
        end ++;
        num % 2 ? oddCount ++ : evenCount ++;
        if(end >= QUEUE_MAX_SIZE){
            std::cerr << "QUEUE MAX SIZE EXCEEDED" << std::endl;
            destroy();
        }
        afterOperation();
    }

    void pop(short odd){
        if(!alive) return;
        if(queue[begin] % 2 == odd){
            begin ++;
            odd == 1 ? oddCount -- : evenCount --;
        }
        afterOperation();
    }

    void afterOperation(){
        for(int i=begin; i<end; ++i){
            std::cout << queue[i] << " ";
        }
        std::cout << std::endl;
        if (evenCount < 10) A1Condition.notify_one();
        if (oddCount < evenCount) A2Condition.notify_one();
        if (evenCount >= 3) B1Condition.notify_one();
        if (oddCount >= 7) B2Condition.notify_one();
        sleep(0.1);
        std::cout << oddCount << "-" << evenCount << std::endl;
    }
private:
    int references = 0;
};
#endif //SOI3_BUFFER_H
