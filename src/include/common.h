// common utilities
// Author: chengenbao
// Email：genbao.chen@gmail.com


#ifndef UTILITY_H
#define UTILITY_H

#ifdef _WIN32  // windows
#include <windows.h>
#include <winsock.h>
#include <io.h>
#include <stdlib.h>

#pragma comment( lib, "ws2_32.lib" )

#define sleep(x) Sleep(x) 
#define popen(x, y) _popen(x, y)
#define pclose(fp) _pclose(fp)
#define access(fn, mode) _access(fn, mode)
#define SHUTDOWN_CMD "shutdown /s"
#define MSEC_PER_SEC 1000

#elif __linux__ // linux
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>

#define closesocket(socket) close(socket)
#define SHUTDOWN_CMD "shutdown -h now"
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define MSEC_PER_SEC 1

typedef int SOCKET;

#endif

typedef void * (*task_func)(void *);

#include <logger.h>
#include <config_reader.h>
#include <string>
#include <set>

using namespace std;

// 宏定义
#define END_FLAG  "\n\n\n"

// 全局变量定义
extern logger loger;
extern config_reader cfr;

// 常量定义
const int PORT = 8086;
const int BACK_LOG = 256;
const int BUF_SIZE = 256;
const long DEAD_TIME= 1395331200;

// 公用函数定义
bool initialize();
void exit_program();
bool split_string( const string &str, const string &pattern, vector<string> &result);
bool  split_string(const string &str, const set<string> &patterns, vector<string> &result);
string trim_string( const string &str);
bool execute_command(const string command, string &result);
void *handle_socket(void *sock);
void *registe_bootos(void *arg);
bool check_copyright();
bool file_exists(const char * filename);
bool split_string( const string &str, vector<string> &result );

// 多线程包装器
template <typename TYPE, void (TYPE::*run)()>
void* _thread_wrapper(void* param)
{
	TYPE* This = (TYPE*)param;     
	This->run();     
	return NULL;  
}  



#endif
