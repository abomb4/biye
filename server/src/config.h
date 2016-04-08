#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>

/**
 * 配置文件读取类
 */
class Config
{
public:
    static void load(const char *path);
    static std::string get(std::string key);
private:
    static Config s_instance;

    Config();
    std::map<std::string, std::string> config_map;

    void clear();
};

#endif // CONFIG_H
