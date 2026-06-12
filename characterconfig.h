#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>

struct AnimParams {
    int cols = 0;
    int fw = 0;
    int fh = 0;
    int interval = 50;
    QString path;
    bool loop = true;
    // stand-specific feet alignment (Orochi)
    int feetBottom = 0;
    int feetMargin = 0;
};

struct CharacterConfig {
    QString id;
    bool facingLeft = false;
    double posX = 0.5;
    AnimParams opening;
    AnimParams stand;
    AnimParams forward;
};

class CharacterConfigLoader
{
public:
    static CharacterConfig load(const QJsonObject &json, const QString &id);
};
