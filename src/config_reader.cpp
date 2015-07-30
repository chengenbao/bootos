#include <config_reader.h>
#include <common.h>
#include <algorithm>
#include <cstdio>

using namespace std;

bool config_reader::initialize()
{
	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	vector<string> path;

	if ( init_filename == "")
		init_filename = "config.xml";

	doc = xmlReadFile(init_filename.c_str(), NULL, 0);

	if ( doc == NULL )
	{
		loger.log(ERROR, "can not parse configuation file: %s\n", init_filename.c_str());
		//abort();
	}

	root_element = xmlDocGetRootElement(doc);
	add_element( root_element, path);
	xmlFreeDoc(doc);
	xmlCleanupParser();

	return true;
}

void config_reader::add_element(xmlNode *node, vector<string> &path)
{
	xmlNode *cur_node = NULL;

	for ( cur_node = node; cur_node != NULL ; cur_node = cur_node->next)
	{
		if ( cur_node->type == XML_ELEMENT_NODE )
		{
			path.push_back( (const char *)cur_node->name );
		}
		else if ( cur_node->type ==  XML_TEXT_NODE )
		{
			string key = to_path(path);
			string value = (const char *) cur_node->content;

			value = trim_string( value );

			if ( value.compare("") != 0 )
			{
				loger.log( INFO, "add config pair: %s --> %s \n", key.c_str(), value.c_str() );
				data[key] = value;
			}
		}

		add_element( cur_node->children, path );
		if (cur_node->type == XML_ELEMENT_NODE && path.size() )
			path.pop_back();
	}
}

string config_reader::to_path( const vector<string> &path )
{
	string result;

	for (size_t i = 0; i != path.size(); ++i)
	{
		if ( i )
		{
			result += ".";
		}

		result += path[i];
	}

	return result;
}

void config_reader::add_config( const config_pair &pair)
{
	data[pair.first] = pair.second;
}

string config_reader::to_string()
{
	string tmp;

	for(map<string,string>::iterator it = data.begin(); it != data.end(); ++it)
	{
		tmp += it->first;
		tmp += ":";
		tmp += it->second;
		tmp += "\n";
	}

	return tmp;
}

void  config_reader::add_config(const string &key, const string &value)
{
    data[key] = value;
}
