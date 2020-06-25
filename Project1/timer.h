#ifndef TIMER_H
#define TIMER_H

typedef struct {
	unsigned long mark;
} timer;

void timer_start(timer* t);
float timer_peek(timer* t);
float timer_reset(timer* t);

#endif