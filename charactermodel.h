#pragma once

#include <QObject>
#include <QTimer>
#include <QString>
#include <QtQml/qqmlregistration.h>
#include "characterconfig.h"

class CharacterModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int currentFrame READ currentFrame NOTIFY frameChanged)
    Q_PROPERTY(QString sourcePath READ sourcePath NOTIFY sourcePathChanged)
    Q_PROPERTY(int frameWidth READ frameWidth NOTIFY sizeChanged)
    Q_PROPERTY(int frameHeight READ frameHeight NOTIFY sizeChanged)
    Q_PROPERTY(int totalFrames READ totalFrames NOTIFY sizeChanged)
    Q_PROPERTY(double positionX READ positionX NOTIFY positionChanged)
    Q_PROPERTY(double positionY READ positionY NOTIFY positionChanged)
    Q_PROPERTY(double posXRatio READ posXRatio WRITE setPosXRatio NOTIFY posXRatioChanged)
    Q_PROPERTY(bool facingLeft READ facingLeft WRITE setFacingLeft NOTIFY facingChanged)

    QML_ELEMENT
public:
    enum State { Waiting, Opening, Stand, Forward };
    Q_ENUM(State)

    explicit CharacterModel(QObject *parent = nullptr);
    ~CharacterModel() override;

    int currentFrame() const { return m_currentFrame; }
    QString sourcePath() const { return m_sourcePath; }
    int frameWidth() const { return m_frameWidth; }
    int frameHeight() const { return m_frameHeight; }
    int totalFrames() const { return m_totalFrames; }
    double positionX() const { return m_posX; }
    double positionY() const { return m_posY; }
    double posXRatio() const { return m_cfgPosX; }
    void setPosXRatio(double r) { m_cfgPosX = r; emit posXRatioChanged(); }
    bool facingLeft() const { return m_facingLeft; }
    void setFacingLeft(bool f) { m_facingLeft = f; emit facingChanged(); }

    void configure(const CharacterConfig &cfg);
    Q_INVOKABLE void playOpening();
    Q_INVOKABLE void playStand();
    Q_INVOKABLE void playForward();
    void reset();
    void updateRootHeight(double h);
    State state() const { return m_state; }

signals:
    void frameChanged();
    void sourcePathChanged();
    void sizeChanged();
    void positionChanged();
    void posXRatioChanged();
    void facingChanged();
    void openingFinished();

private slots:
    void onTick();

private:
    void applyAnim(const AnimParams &p);
    void setPosY();

    QTimer   m_timer;
    State    m_state = Waiting;
    int      m_currentFrame = 0;
    QString  m_sourcePath;
    int      m_frameWidth = 0;
    int      m_frameHeight = 0;
    int      m_totalFrames = 0;
    double   m_posX = 0.5;
    double   m_posY = 0;
    bool     m_facingLeft = false;

    // saved configs
    AnimParams m_opening;
    AnimParams m_stand;
    AnimParams m_forward;
    bool     m_loopAnim = true;
    double    m_cfgPosX = 0.5;

    // root height for y calc (set by FightDirector)
    friend class FightDirector;
    double m_rootHeight = 640;
};
