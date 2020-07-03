#include "timer.h"
#include <sys/timeb.h>
#include <time.h>

unsigned long get_current_ms() {
	struct timeb start;
	ftime(&start);
	unsigned long ms = (unsigned long)(1000l * start.time + start.millitm);

	return ms;
}

void timer_start(timer* t) {
	t->mark = get_current_ms();
}

float timer_peek(timer* t) {
	unsigned long current_ms = get_current_ms();
	int diff = current_ms - t->mark;

	return ((float)diff) / 1000.0f;
}

float timer_reset(timer* t) {
	unsigned long current_ms = get_current_ms();
	int diff = current_ms - t->mark;
	t->mark = current_ms;

	return ((float)diff) / 1000.0f;
}