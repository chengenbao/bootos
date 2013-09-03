#ifndef BOOTOS_MANAGER_H
#include <common.h>
#include <string>
#include <set>
#include <map>

using namespace std;

class bootos_manager
{
public:
	static const set<string> BLOCK_COMMAND;
	static const map<string, task_func> INNER_COMMAND;
	static const map<string, int> REGISTE_STATUS;
	static const map<string, int> IMM_CONFIG_STATUS;
	static const string IMM_PID_FILE;
	static const string IMM_SUCCESS_FLAG_FILE;
	static const string IMM_FAILED_FLAG_FILE;

	static bool is_block_command(const string &command);
	static bool is_inner_command(const string &commnad);

	static set<string> init_block_command();
	static map<string, task_func> init_inner_command();
	static map<string, int> init_registe_status();
	static map<string, int> init_imm_config_status();
};

#endif