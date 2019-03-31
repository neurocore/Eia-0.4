#ifndef TIME_H
#define TIME_H

#include <time.h>

// Consts /////////////////////////////////

#define DCLOCKS_SEC   (static_cast<double>(CLOCKS_PER_SEC))
#define DCLOCKS_MSSEC (static_cast<double>(CLOCKS_PER_SEC) / 1000)

// Classes ////////////////////////////////

#ifdef _WIN32

class Timer
{
	clock_t time;

public:
	Timer() { set(); }
	void set() { time = clock(); }
	double get()   { return (clock() - time) / DCLOCKS_SEC; }
	double getms() { return (clock() - time) / DCLOCKS_MSSEC; }
};

#elif defined(__linux__) || defined(__unix__)

	#include <sys/time.h>
	#include <unistd.h>

class Timer
{
	struct timeval time;

public:
	Timer() { set(); }
	void set() { gettimeofday(&time, NULL); }

	double get()
	{
		struct timeval now;
		long seconds, useconds;

		gettimeofday(&now, NULL);

		seconds  = now.tv_sec  - time.tv_sec;
		useconds = now.tv_usec - time.tv_usec;

		return seconds + useconds / 1000000000.0;
	}

	double getms()
	{
		struct timeval now;
		long seconds, useconds;

		gettimeofday(&now, NULL);

		seconds  = now.tv_sec  - time.tv_sec;
		useconds = now.tv_usec - time.tv_usec;

		return seconds * 1000.0 + useconds / 1000000.0;
	}

};

#endif

#endif // TIME_H
