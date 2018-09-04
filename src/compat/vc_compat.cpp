#include <time.h>

extern "C" {

tm * __cdecl gmtime_r(const time_t *timer, tm *result) {
	__time64_t t64 = *timer;
	if (!_gmtime64_s(result, &t64))
		return result;
	return 0;
}

}
