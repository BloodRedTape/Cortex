#include <windows.h>
#include "time.hpp"

UnixTime UnixTime::CurrentTime() {
   //Get the number of seconds since January 1, 1970 12:00am UTC
   //Code released into public domain; no attribution required.

   const u64 UNIX_TIME_START = 0x019DB1DED53E8000; //January 1, 1970 (start of Unix epoch) in "ticks"
   const u64 TICKS_PER_SECOND = 10000000; //a tick is 100ns

   FILETIME ft;
   GetSystemTimeAsFileTime(&ft); //returns ticks in UTC

   //Copy the low and high parts of FILETIME into a LARGE_INTEGER
   //This is so we can access the full 64-bits as an Int64 without causing an alignment fault
   LARGE_INTEGER li;
   li.LowPart  = ft.dwLowDateTime;
   li.HighPart = ft.dwHighDateTime;
 
   //Convert ticks since 1/1/1970 into seconds
   return UnixTime{(li.QuadPart - UNIX_TIME_START) / TICKS_PER_SECOND};
}