#include <bootos_manager.h>

set <string> bootos_manager::init_block_command() {
  set <string> blk_cmd;
  blk_cmd.insert("reboot");
  blk_cmd.insert("config_imm.sh");
  blk_cmd.insert("shutdown");

  return blk_cmd;
}

map <string, task_func> bootos_manager::init_inner_command() {
  map <string, task_func> inner_command;

  inner_command.insert(pair<string, task_func>("registe_bootos",
                                               registe_bootos));

  return inner_command;
}


map <string, string> bootos_manager::init_registe_status() {
  map <string, string> registe_status;

  registe_status.insert(pair<string, string>("UNREGISTE", "0"));
  registe_status.insert(pair<string, string>("SUCCESS", "1"));
  registe_status.insert(pair<string, string>("FAILED", "2"));

  return registe_status;
}

map <string, string> bootos_manager::init_imm_config_status() {
  map <string, string> imm_config_status;

  imm_config_status.insert(pair<string, string>("UNCONFIG", "0"));
  imm_config_status.insert(pair<string, string>("CONFIGING", "1"));
  imm_config_status.insert(pair<string, string>("SUCCESS", "2"));
  imm_config_status.insert(pair<string, string>("FAILED", "3"));

  return imm_config_status;
}

bool bootos_manager::is_block_command(const string &command) {
  return BLOCK_COMMAND.find(command) != BLOCK_COMMAND.end();
}

bool bootos_manager::is_inner_command(const string &command) {
  return INNER_COMMAND.count(command) != 0;
}

string bootos_manager::get_imm_status() {
  config_pair kv;
  kv.first = "imm_status";

  if (file_exists(IMM_SUCCESS_FLAG_FILE.c_str())) {
    kv.second = IMM_CONFIG_STATUS["SUCCESS"];
  } else if (file_exists(IMM_PID_FILE.c_str())) {
    kv.second = IMM_CONFIG_STATUS["CONFIGING"];
  } else if (file_exists(IMM_FAILED_FLAG_FILE.c_str())) {
    kv.second = IMM_CONFIG_STATUS["FAILED"];
  } else {
    kv.second = IMM_CONFIG_STATUS["UNCONFIG"];
  }

  cfr.add_config(kv);

  return cfr.get_config_value("imm_status");
}

// get the network interface card infomation, mac and ip
Json::Value bootos_manager::get_nic_info() {
  string command = "  ifconfig -a | grep eth | awk '{print $1}'  ";
  string nics;
  Json::Value root;

  execute_command(command, nics);
  vector <string> result;

  split_string(nics, result);

  for (vector<string>::iterator it = result.begin(); it != result.end(); ++it) {
    string nif = *it;
    command = "ifconfig " + nif;
    string ip, mac;
    Json::Value v;

    string nif_info;
    execute_command(command, nif_info);
    vector <string> lines;
    split_string(nif_info, "\n", lines);
    for (vector<string>::iterator line = lines.begin(); line != lines.end();
         ++line) {
      if (line->find("HWaddr") != string::npos) {
        vector <string> tmp;
        split_string(*line, tmp);
        mac = tmp[4];
      } else if (line->find("inet addr") != string::npos) {
        vector <string> tmp;
        split_string(*line, ":", tmp);
        ip = tmp[1];
        tmp.clear();
        split_string(ip, tmp);
        ip = tmp[0];
      }
    }

    v["mac"] = mac;
    v["ip"] = ip;

    root.append(v);
  }

  return root;
}

Json::Value bootos_manager::get_cpu_info() {
  Json::Value v;
  string command =
      "  cat /proc/cpuinfo | grep \"physical id\" | sort | uniq | wc -l ";
  string tmp;

  execute_command(command, tmp);
  int cpu_num = atoi(tmp.c_str());

  command =
      " cat /proc/cpuinfo | grep \"cpu cores\" | uniq | head -1 | awk -F\":\" '{print $2}' ";
  execute_command(command, tmp);
  int cores_per_cpu = 1;

  if (tmp.length() != 0) {
    cores_per_cpu = atoi(tmp.c_str());
  }

  int cores_num = cpu_num * cores_per_cpu;

  v["physical_num"] = cpu_num;
  v["cores_num"] = cores_num;

  return v;
}

string bootos_manager::get_sn_num() {
  string command =
      " dmidecode -t 1 | grep \"Serial Number\" | awk -F: '{print $2}' ";
  string sn;

  execute_command(command, sn);
  sn = trim_string(sn);

  return sn;
}

set <string>
    bootos_manager::BLOCK_COMMAND = bootos_manager::init_block_command();
map <string, task_func>
    bootos_manager::INNER_COMMAND = bootos_manager::init_inner_command();
map <string, string>
    bootos_manager::REGISTE_STATUS = bootos_manager::init_registe_status();
map <string, string>bootos_manager::IMM_CONFIG_STATUS =
    bootos_manager::init_imm_config_status();
const string bootos_manager::IMM_PID_FILE = "/var/run/imm_config.pid";
const string bootos_manager::IMM_SUCCESS_FLAG_FILE = "/var/run/imm_success";
const string bootos_manager::IMM_FAILED_FLAG_FILE = "/var/run/imm_failed";
