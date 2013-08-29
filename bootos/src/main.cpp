#include <common.h>
#include <iostream>
#include <cstdlib>
#include <heartbeat.h>

using namespace std;

void *say_hello(void *args)
{
	loger.log(INFO, "Hello\n");
	return NULL;
}

int main(int argc, char **argv)
{
	initialize();

	cout << cfr.to_string() << endl;
	heartbeat hb(1, say_hello, NULL);
	hb.start();

	return 0;
}