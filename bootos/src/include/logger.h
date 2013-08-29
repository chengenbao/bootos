#ifndef __LOGGER__
#define __LOGGER__
#include <cstdio>

enum level {FINEST, FINER, FINE, CONFIG, INFO, WARING, SEVERE};
class logger
{
	public:
		logger(const char *file_name_prefix, level l = FINEST);
		void log(level l, const char *fmt, ...);
		void log_f(FILE *fp, level l, const char *fmt, ...);
		void set_log_level(level l);
		void close();
		~logger();
	private:
		FILE *log_fp;
		level log_level;
};

#endif
