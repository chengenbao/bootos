#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include <common.h>

class heartbeat
{
public:
	heartbeat() : interval(0), task(NULL), args(NULL), is_started(false)
	{
	}

	heartbeat(const int inter, const task_func func, void *ag) : interval(inter * MSEC_PER_SEC),
		task(func), args(ag), is_started(false)
	{
	}

	// set task for the heartbeat
	void set_task(const task_func func, void *ag)
	{
		task = func;
		args = ag;
	}

	// set interval seconds
	void set_interval(const int inter)
	{
		interval = inter;
	}
	void start();
private:
	static void *run_helper(void *ag);
	void run();
	int interval;
	task_func task;
	void *args;
	bool is_started;
};

#endif