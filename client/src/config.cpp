#include "config.h"

#include <QFile>
#include <QTextStream>
#include <fstream>

Config Config::s_instance;

void Config::clear() {
    config_map.clear();
}

QString Config::get(QString key) {
    if (s_instance.config_map.count(key) > 0)
        return s_instance.config_map[key];
    else
        return "";
}

bool Config::load(const char *path) {
    s_instance.clear();

    QFile infile;
    infile.setFileName(path);

    infile.open(QIODevice::ReadOnly);
    if (infile.isOpen()) {
        // read config file
        QTextStream in(&infile);
        QString buff;
        while (!in.atEnd()) {
            buff = in.readLine();
            if (buff[0] == '#' || buff.length() == 0)
                continue;
            int pos_eq = buff.indexOf("=");
            if (pos_eq < 0 || pos_eq == buff.size() - 1) { // bad config line
                return false;
            }

            // deal with line, load to map
            QString key = buff.mid(0, pos_eq);
            key = key.trimmed();
            QString value = buff.mid(pos_eq + 1, buff.size() - pos_eq - 1);
            value = value.trimmed();
            s_instance.config_map.insert(key, value);
        }
        return true;
    } else {
        return false;
    }
    return false;
}

Config::Config()
{
}
