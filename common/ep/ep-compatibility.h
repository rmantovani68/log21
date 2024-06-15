/*
* ep-compatibility.h
*/
#ifdef __linux__
void ep_usleep(int64_t usec) ;
#else
void ep_usleep(__int64 usec) ;
#endif