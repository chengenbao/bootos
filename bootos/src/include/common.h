// common utilities
// Author: chengenbao
// Email：genbao.chen@gmail.com


#ifndef UTILITY_H
#define UTILITY_H

#ifdef _WIN32  // windows
#include <windows.h>
#include <winsock.h>

#pragma comment( lib, "ws2_32.lib" )

#define sleep(x) Sleep(x) 
#define popen(x, y) _popen(x, y)
#define pclose(fp) _pclose(fp)

#elif __linux__ // linux

#define closesocket(socket) close(socket)

#endif

typedef void * (*task_func)(void *);

#include <logger.h>
#include <config_reader.h>
#include <string>
#include <set>

using namespace std;

// 全局变量定义
extern logger loger;
extern config_reader cfr;

// 常量定义
const int MSEC_PER_SEC = 1000;
const int PORT = 8087;
const int BACK_LOG = 256;

// 公用函数定义
void initialize();
void exit_program();
bool split_string( const string &str, const string &pattern, vector<string> &result);
bool  split_string(const string &str, const set<string> &patterns, vector<string> &result);
string trim_string( const string &str);
bool execute_command(const string command, string &result);
void *handle_socket(void *sock);

// 多线程包装器
template <typename TYPE, void (TYPE::*run)()>
void* _thread_wrapper(void* param)
{
	TYPE* This = (TYPE*)param;     
	This->run();     
	return NULL;  
}  

#endif