#include <bootos_manager.h>

set<string> bootos_manager::init_block_command()
{
	set<string> blk_cmd;
	blk_cmd.insert("reboot");
	blk_cmd.insert("config_imm.sh");
	blk_cmd.insert("shutdown");

	return blk_cmd;
}

map<string, task_func> bootos_manager::init_inner_command()
{
	map<string, task_func> inner_command;

	inner_command.insert(pair<string, task_func>("registe_bootos", registe_bootos));

	return inner_command;
}


map<string, int> bootos_manager::init_registe_status()
{
	map<string, int> registe_status;

	registe_status.insert(pair<string, int>("UNREGISTE", 0));
	registe_status.insert(pair<string, int>("SUCCESS", 1));
	registe_status.insert(pair<string, int>("FAILED", 2));

	return registe_status;
}

map<string, int> bootos_manager::init_imm_config_status()
{
	map<string, int> imm_config_status;

	imm_config_status.insert(pair<string, int>("UNCONFIG", 0));
	imm_config_status.insert(pair<string, int>("CONFIGING", 1));
	imm_config_status.insert(pair<string, int>("SUCCESS", 2));
	imm_config_status.insert(pair<string, int>("FAILED", 3));

	return imm_config_status;
}

bool bootos_manager::is_block_command(const string &command)
{
	return BLOCK_COMMAND.find(command) != BLOCK_COMMAND.end();
}

bool bootos_manager::is_inner_command(const string &command)
{
	return INNER_COMMAND.count(command) != 0;
}

const set<string> bootos_manager::BLOCK_COMMAND = bootos_manager::init_block_command();
const map<string, task_func> bootos_manager::INNER_COMMAND = bootos_manager::init_inner_command();
const map<string, int> bootos_manager::REGISTE_STATUS = bootos_manager::init_registe_status();
const map<string, int> bootos_manager::IMM_CONFIG_STATUS = bootos_manager::init_imm_config_status();
const string bootos_manager::IMM_PID_FILE = "/var/run/imm_config.pid";
const string bootos_manager::IMM_SUCCESS_FLAG_FILE = "/var/run/imm_success";
const string bootos_manager::IMM_FAILED_FLAG_FILE = "/var/run/imm_failed";