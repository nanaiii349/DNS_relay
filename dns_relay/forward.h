#include"globals.h"
#include"debugOutput.h"
#include"cache.h"

void forward(void* arg); //转发函数，每次转发单独开线程，保证从win来的包全数转发