#include <common.h>
#include <iostream>
#include <cstdlib>
#include <heartbeat.h>
#include <commander.h>

using namespace std;


int main(int argc, char **argv) {
  if (initialize()) {
    registe_bootos(NULL);

    heartbeat hb(5, send_heart_beat, NULL);
    hb.start();

    commander cmd;
    cmd.start();

    while (true)
      sleep(1000);
  }

  return 0;
}
