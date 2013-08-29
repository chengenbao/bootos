#include <heartbeat.h>
#include <common.h>
#include <pthread.h>

void heartbeat::start()
{
	if ( is_started || !interval || !task)
	{
		return;
	}
	pthread_t handler;
	pthread_create(&handler, NULL, heartbeat::run_helper, this);
	pthread_join(handler, NULL);
	is_started = true;
}

void *heartbeat::run_helper(void *arg)
{
	((heartbeat *)arg)->run();

	return NULL;
}

void heartbeat::run()
{
	while(true)
	{
		task(args);
		sleep(interval);
	}
}