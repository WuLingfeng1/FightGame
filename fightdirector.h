#pragma once

#include <QObject>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QtQml/qqmlregistration.h>
#include "keybindingconfig.h"

class CharacterModel;

// 战斗导演: 管理整场战斗的流程和两个角色模型
// 阶段控制: Opening_P1(播放P1开场) -> Opening_P2(播放P2开场) -> Fighting(玩家可操控)
class FightDirector : public QObject
{
    Q_OBJECT
    Q_PROPERTY(CharacterModel *p1Model READ p1Model CONSTANT) // P1角色模型
    Q_PROPERTY(CharacterModel *p2Model READ p2Model CONSTANT) // P2角色模型
    Q_PROPERTY(KeyBindingConfig *keyBindings READ keyBindings CONSTANT) // 键位配置
    Q_PROPERTY(int phase READ phase NOTIFY phaseChanged)      // 当前战斗阶段
    Q_PROPERTY(double rootHeight READ rootHeight WRITE setRootHeight NOTIFY rootHeightChanged)
    Q_PROPERTY(double cameraOffset READ cameraOffset NOTIFY cameraOffsetChanged)
    Q_PROPERTY(double maxCameraOffset READ maxCameraOffset CONSTANT)
    Q_PROPERTY(bool previewWin READ previewWin WRITE setPreviewWin NOTIFY previewWinChanged) // 调试: 开场阶段播放胜利动画

    QML_ELEMENT
public:
    enum Phase { Opening_P1, Opening_P2, Fighting };
    Q_ENUM(Phase)

    explicit FightDirector(QObject *parent = nullptr);

    CharacterModel *p1Model() const { return m_p1Model; }
    CharacterModel *p2Model() const { return m_p2Model; }
    KeyBindingConfig *keyBindings() const { return m_keyBindings; }
    int phase() const { return m_phase; }
    double rootHeight() const { return m_rootHeight; }
    double cameraOffset() const { return m_cameraOffset; }
    double maxCameraOffset() const { return kStageWidth - 1.0; }
    bool previewWin() const { return m_previewWin; }
    void setPreviewWin(bool on) { m_previewWin = on; emit previewWinChanged(); }
    void setRootHeight(double h); // QML窗口高度变化时更新

    Q_INVOKABLE void start(const QString &p1CharId, const QString &p2CharId);
    Q_INVOKABLE void resetForNewRound(const QString &p1CharId, const QString &p2CharId); // 回合重置，跳过开场动画
    Q_INVOKABLE void updateCamera();                                                     // KOF97中点跟随镜头
    Q_INVOKABLE bool checkCollision();                                                   // 检测碰撞, 返回是否命中
    Q_INVOKABLE void reloadKeyBindings();                                                // 重新加载键位配置

signals:
    void phaseChanged();
    void rootHeightChanged();
    void cameraOffsetChanged();
    void previewWinChanged();
    void hitDetected(int attacker, int damage); // 命中信号: attacker=1或2, damage=伤害值

private slots:
    void onP1OpeningFinished(); // P1开场完毕 - 触发P2开场
    void onP2OpeningFinished(); // P2开场完毕 - 进入战斗阶段
    void onP1WinFinished();     // 预览模式: P1胜行动画完毕 - 播放P2胜行动画
    void onP2WinFinished();     // 预览模式: P2胜行动画完毕 - 进入战斗阶段

private:
    void loadConfig(const QString &charId, CharacterModel *model); // 从JSON加载角色配置

    CharacterModel *m_p1Model; // P1角色模型
    CharacterModel *m_p2Model; // P2角色模型
    KeyBindingConfig *m_keyBindings; // 键位配置
    static constexpr double kStageWidth = 3.0;
    int m_phase = Fighting;
    double m_rootHeight = 640; // 窗口高度
    double m_cameraOffset = 0; // 镜头左边界在世界坐标中的偏移
    bool m_previewWin = false; // 调试预览: 开场阶段播放胜利动画+语音 (调参完成后已关闭)
    QJsonObject m_jsonConfig;  // 已加载的角色配置JSON
};
