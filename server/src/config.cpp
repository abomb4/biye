#include "config.h"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include "fxutils/stringutils.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

Config Config::s_instance;

void Config::clear() {
    config_map.clear();
}

string Config::get(string key) {
    if (s_instance.config_map.count(key) > 0)
        return s_instance.config_map[key];
    else
        return "";
}

void Config::load(const char *path) {
    s_instance.clear();

    std::ifstream infile;
    infile.open(path);
    if (infile.is_open()) {
        cout << "loading config file " << path << endl;
        // read config file
        string buff;
        while (std::getline(infile, buff)) {
            if (buff[0] == '#' || buff.length() == 0)
                continue;
            unsigned int pos_eq = buff.find_first_of("=");
            if (pos_eq < 0 || pos_eq == buff.size() - 1) { // bad config line
                cerr << "Bad config parse: " << buff << endl;
                exit(-1);
            }

            // deal with line, load to map
            string key = buff.substr(0, pos_eq);
            key = StringUtils::trim(key);
            string value = buff.substr(pos_eq + 1, buff.size());
            value = StringUtils::trim(value);
            s_instance.config_map.insert(std::pair<string, string>(key, value));
            cout << "\tkey: " << key << ", value: " << value << endl;
        }
        cout << "config file loaded." << endl;
    } else {
        cerr << "read config fail!" << endl;
        exit(-1);
    }
}

Config::Config()
{
}
