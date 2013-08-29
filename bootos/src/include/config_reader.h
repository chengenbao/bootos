#ifndef __CONFIG_READER_H__
#define __CONFIG_READER_H__

#include <map>
#include <string>
#include <utility>
#include <vector>
#include <libxml/parser.h>
#include <libxml/tree.h>

using namespace std;


typedef pair<string, string> config_pair;

class config_reader
{
	public:
		config_reader(string fn = "") : init_filename(fn){}
		bool initialize();
		string get_config_value( string key) 
		{
			return data[key];
		}
		void add_config(const config_pair &kv);
		string to_string();
	private:
		string init_filename;
		map<string, string> data;
		inline void add_element( xmlNode *node, vector<string> &path);
		string to_path( const vector<string> &path);
};

#endif
