#pragma once

#include <QObject>
#include <QTimer>
#include <QString>
#include <QtQml/qqmlregistration.h>
#include "characterconfig.h"

// 角色模型: 管理单个角色的动画状态机, 驱动精灵表逐帧播放
// 状态流转: Waiting -> Opening -> Stand <-> Forward / Backward
// 通过 Q_PROPERTY 暴露属性给 QML 渲染层
class CharacterModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int currentFrame READ currentFrame NOTIFY frameChanged)       // 当前播放帧索引
    Q_PROPERTY(QString sourcePath READ sourcePath NOTIFY sourcePathChanged)   // 精灵表图片路径
    Q_PROPERTY(int frameWidth READ frameWidth NOTIFY sizeChanged)             // 单帧宽度
    Q_PROPERTY(int frameHeight READ frameHeight NOTIFY sizeChanged)           // 单帧高度
    Q_PROPERTY(int totalFrames READ totalFrames NOTIFY sizeChanged)           // 总帧数
    Q_PROPERTY(double positionX READ positionX NOTIFY positionChanged)        // 像素X坐标
    Q_PROPERTY(double positionY READ positionY NOTIFY positionChanged)        // 像素Y坐标(底部对齐)
    Q_PROPERTY(double posXRatio READ posXRatio WRITE setPosXRatio NOTIFY posXRatioChanged)  // 水平位置比例
    Q_PROPERTY(bool facingLeft READ facingLeft WRITE setFacingLeft NOTIFY facingChanged)     // 是否朝左

    QML_ELEMENT
public:
    enum State { Waiting, Opening, Stand, Forward, Backward };
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

    void configure(const CharacterConfig &cfg);   // 从配置初始化模型
    Q_INVOKABLE void playOpening();               // 播放开场动画
    Q_INVOKABLE void playStand();                 // 切换到站立动画
    Q_INVOKABLE void playForward();               // 切换到前进行走动画
    Q_INVOKABLE void playBackward();              // 切换到后退行走动画
    void reset();                                 // 重置到初始状态
    void updateRootHeight(double h);              // 更新窗口高度(用于Y坐标计算)
    State state() const { return m_state; }

signals:
    void frameChanged();           // 帧切换
    void sourcePathChanged();      // 图片路径变更
    void sizeChanged();            // 帧尺寸变更
    void positionChanged();        // 位置变更
    void posXRatioChanged();       // 水平比例变更
    void facingChanged();          // 朝向变更
    void openingFinished();        // 开场动画播放完毕

private slots:
    void onTick();                 // 定时器回调: 逐帧推进动画

private:
    void applyAnim(const AnimParams &p);  // 应用动画参数到当前状态
    void setPosY();                       // 根据动画类型计算Y坐标(底部/脚部对齐)

    QTimer   m_timer;              // 动画计时器
    State    m_state = Waiting;    // 当前状态
    int      m_currentFrame = 0;   // 当前帧索引
    QString  m_sourcePath;         // 当前精灵表路径
    int      m_frameWidth = 0;     // 当前帧宽度
    int      m_frameHeight = 0;    // 当前帧高度
    int      m_totalFrames = 0;    // 总帧数
    double   m_posX = 0.5;         // X像素位置
    double   m_posY = 0;           // Y像素位置
    bool     m_facingLeft = false; // 朝向: false=右 true=左

    AnimParams m_opening;          // 开场动画参数副本
    AnimParams m_stand;            // 站立动画参数副本
    AnimParams m_forward;          // 前进动画参数副本
    AnimParams m_backward;         // 后退动画参数副本
    bool     m_loopAnim = true;    // 当前动画是否循环
    double   m_cfgPosX = 0.5;     // 水平位置比例(可运行时修改)

    friend class FightDirector;    // 允许 FightDirector 直接设置 m_rootHeight
    double m_rootHeight = 640;     // 窗口高度(用于Y坐标计算)
};
