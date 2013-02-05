#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#ifndef A
#define A

#include <iostream>
#include <thread>
#include <process.h>
#include "logger.hpp"

void thread_a(void*);
void thread_b(void*);
void thread_c(void*);

#endif
