#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QMap>

/**
 * 配置文件读取类
 */
class Config
{
public:
    static bool load(const char *path);
    static QString get(QString key);
private:
    static Config s_instance;

    Config();
    QMap<QString, QString> config_map;

    void clear();
};

#endif // CONFIG_H
