#include <common.h>
#include <iostream>
#include <cstdlib>
#include <heartbeat.h>
#include <commander.h>

using namespace std;

void *say_hello(void *args)
{
	loger.log(INFO, "Hello\n");
	return NULL;
}

int main(int argc, char **argv)
{
	if ( initialize() )
	{
		heartbeat hb(5, say_hello, NULL);
		hb.start();
		commander cmd;
		cmd.start();

		while(true)
			sleep(1000);
	}
	
	return 0;
}