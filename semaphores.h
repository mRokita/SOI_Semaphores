#ifndef SOI3_SEMAPHORES_H
#define SOI3_SEMAPHORES_H
#include <string>

class Semaphore{
public:
    static inline const char* const PRINT = "ssssssspop";
    static inline const char* const PUSH_OP = "sssssssoi_push_op_sem";
    static inline const char* const POP_OP = "sssssssoi_pop_op_sem";
    static inline const char* const A1 = "sssssssoi_a1_sem";
    static inline const char* const A2 = "sssssssoi_a2_sem";
    static inline const char* const B1 = "sssssssoi_b1_sem";
    static inline const char* const B2 = "sssssssoi_b2_sem";
};
#endif //SOI3_SEMAPHORES_H
