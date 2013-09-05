#ifndef __LOGGER__
#define __LOGGER__
#include <cstdio>

enum level {FINEST, FINNER, FINE, CONFIG, INFO, WARN, SEVERE, ERROR};
class logger
{
	public:
		logger(const char *file_name_prefix, const level l = FINEST);
		logger(FILE *fp, const level l = FINEST);
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
