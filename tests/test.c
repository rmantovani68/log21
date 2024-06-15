/* 
* crt_localtime.cpp
* compile with: /W3
* This program uses _time64 to get the current time
* and then uses localtime64() to convert this time to a structure
* representing the local time. The program converts the result
* from a 24-hour clock to a 12-hour clock and determines the
* proper extension (AM or PM).
*/

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

int main( void )
{
    struct tm *newtime;
    char am_pm[] = "AM";
    char target[256];
    __time64_t long_time;
	struct timeval tv;

	gettimeofday(&tv,NULL);


    _time64( &long_time );             // Get time as 64-bit integer.
                                       // Convert to local time.
    newtime = _localtime64( &long_time ); // C4996
    // Note: _localtime64 deprecated; consider _localetime64_s

    if( newtime->tm_hour > 12 )        // Set up extension.
        strcpy_s( am_pm, sizeof(am_pm), "PM" );
    if( newtime->tm_hour > 12 )        // Convert from 24-hour
        newtime->tm_hour -= 12;        //   to 12-hour clock.
    if( newtime->tm_hour == 0 )        // Set hour to 12 if midnight.
        newtime->tm_hour = 12;

    char buff[30];
    asctime_s( buff, sizeof(buff), newtime );
    printf( "%.19s %s\n", buff, am_pm );
	printf("%.2d:%.2d:%.2d.%3d\n", newtime->tm_hour, newtime->tm_min, newtime->tm_sec,(int)tv.tv_usec/1000);
}
