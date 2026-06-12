#pragma once

#include <QObject>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QtQml/qqmlregistration.h>

class CharacterModel;

class FightDirector : public QObject
{
    Q_OBJECT
    Q_PROPERTY(CharacterModel* p1Model READ p1Model CONSTANT)
    Q_PROPERTY(CharacterModel* p2Model READ p2Model CONSTANT)
    Q_PROPERTY(int phase READ phase NOTIFY phaseChanged)
    Q_PROPERTY(double rootHeight READ rootHeight WRITE setRootHeight NOTIFY rootHeightChanged)

    QML_ELEMENT
public:
    enum Phase { Opening_P1, Opening_P2, Fighting };
    Q_ENUM(Phase)

    explicit FightDirector(QObject *parent = nullptr);
    ~FightDirector() override;

    CharacterModel* p1Model() const { return m_p1Model; }
    CharacterModel* p2Model() const { return m_p2Model; }
    int phase() const { return m_phase; }
    double rootHeight() const { return m_rootHeight; }
    void setRootHeight(double h);

    Q_INVOKABLE void start(const QString &p1CharId, const QString &p2CharId);

signals:
    void phaseChanged();
    void rootHeightChanged();

private slots:
    void onP1OpeningFinished();
    void onP2OpeningFinished();

private:
    void loadConfig(const QString &charId, CharacterModel *model);

    CharacterModel *m_p1Model;
    CharacterModel *m_p2Model;
    int m_phase = Fighting;
    double m_rootHeight = 640;
    QJsonObject m_jsonConfig;
};
