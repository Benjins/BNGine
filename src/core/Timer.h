#ifndef TIMER_H
#define TIMER_H

#pragma once

#include <time.h>

#if !defined(__APPLE__) && !defined(_WIN32) && !defined(_WIN64)
#include <sys/time.h>
#endif 

struct Timer{
	clock_t startTime;

	Timer(){
		Reset();
	}

	void Reset(){
#if !defined(__APPLE__) && !defined(_WIN32) && !defined(_WIN64)
		timeval start;
		gettimeofday(&start, NULL);
		startTime = start.tv_sec*1000 + start.tv_usec/1000;
#else
		startTime = clock();
#endif 
	}

	double GetTimeSince(){
		int divisor = CLOCKS_PER_SEC;
		clock_t currTime;
#if !defined(__APPLE__) && !defined(_WIN32) && !defined(_WIN64)
		divisor = 1000;
		timeval start;
		gettimeofday(&start, NULL);
		currTime = start.tv_sec*1000 + start.tv_usec/1000;
#else
		currTime = clock();
#endif
		double deltaTime = ((double)currTime - startTime)/divisor;

		return deltaTime;
	}

};

#endif