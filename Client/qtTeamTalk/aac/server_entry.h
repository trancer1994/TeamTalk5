#pragma once
#include <QString>

namespace aac {

struct ServerEntry {
    QString name;
    QString host;
    int port = 10333;
    QString username;
    bool autoConnect = false;
};

}
