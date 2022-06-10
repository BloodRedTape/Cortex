#include "time.hpp"
#include <ctime>

UnixTime UnixTime::CurrentTime() {
   return UnixTime{(u64)time(nullptr)};
}