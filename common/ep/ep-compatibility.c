/*
* ep-compatibility.c
*/
#ifdef __linux__
#include <stdint.h>
#include <unistd.h>

void ep_usleep(int64_t usec) 
{
    usleep(usec);
}
#else
#include <windows.h>

void ep_usleep(__int64 usec) 
{ 
    HANDLE timer; 
    LARGE_INTEGER ft; 

    ft.QuadPart = -(10*usec); // Convert to 100 nanosecond interval, negative value indicates relative time

    timer = CreateWaitableTimer(NULL, TRUE, NULL); 
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0); 
    WaitForSingleObject(timer, INFINITE); 
    CloseHandle(timer); 
}
#endif
