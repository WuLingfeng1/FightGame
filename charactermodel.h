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
    Q_PROPERTY(int refFrameWidth READ refFrameWidth NOTIFY sizeChanged)       // 参考帧宽(站立), 用于居中
    Q_PROPERTY(int animOffsetX READ animOffsetX NOTIFY sizeChanged)           // 当前动画水平偏移
    Q_PROPERTY(double visualScale READ visualScale NOTIFY sizeChanged)          // 视觉缩放补偿
    Q_PROPERTY(double positionX READ positionX NOTIFY positionChanged)        // 像素X坐标
    Q_PROPERTY(double positionY READ positionY NOTIFY positionChanged)        // 像素Y坐标(底部对齐)
    Q_PROPERTY(double posXRatio READ posXRatio WRITE setPosXRatio NOTIFY posXRatioChanged)  // 水平位置比例
    Q_PROPERTY(bool facingLeft READ facingLeft WRITE setFacingLeft NOTIFY facingChanged)     // 是否朝左
    Q_PROPERTY(bool attacking READ isAttacking NOTIFY stateChanged)          // 是否处于攻击状态
    Q_PROPERTY(bool attackActive READ isAttackActive NOTIFY frameChanged)    // 当前帧是否有攻击判定
    Q_PROPERTY(double hurtboxCenterX READ hurtboxX NOTIFY positionChanged)   // 受击框中心X
    Q_PROPERTY(double hurtboxCenterY READ hurtboxY NOTIFY positionChanged)   // 受击框中心Y
    Q_PROPERTY(double hitboxCenterX READ hitboxX NOTIFY frameChanged)        // 攻击框中心X
    Q_PROPERTY(double hitboxCenterY READ hitboxY NOTIFY frameChanged)        // 攻击框中心Y
    Q_PROPERTY(int hitboxRadius READ hitboxRadius NOTIFY frameChanged)       // 攻击范围半径
    Q_PROPERTY(int state READ stateInt NOTIFY stateChanged)                  // 当前状态(整数)

    QML_ELEMENT
public:
    enum State { Waiting, Opening, Stand, Forward, Backward, Jump, DiagonalJump, LightPunch, LightKick, HeavyPunch, HeavyKick, HeavyStrike, Hurt, Crouch, CrouchAttack, Dodge, StandBlock };
    Q_ENUM(State)

    explicit CharacterModel(QObject *parent = nullptr);
    ~CharacterModel() override;

    //内联实现
    int currentFrame() const { return m_currentFrame; }
    QString sourcePath() const { return m_sourcePath; }
    int frameWidth() const { return m_frameWidth; }
    int frameHeight() const { return m_frameHeight; }
    int totalFrames() const { return m_totalFrames; }
    int refFrameWidth() const { return m_refFrameWidth; }
    int animOffsetX() const { return m_animOffsetX; }
    double visualScale() const { return m_visualScale; }
    double positionX() const { return m_posX; }
    double positionY() const { return m_posY; }
    double posXRatio() const { return m_cfgPosX; }
    void setPosXRatio(double r) { m_cfgPosX = r; emit posXRatioChanged(); }
    bool facingLeft() const { return m_facingLeft; }
    void setFacingLeft(bool f) { m_facingLeft = f; emit facingChanged(); }
    void setOpponent(CharacterModel *opp) { m_opponent = opp; } //绑定对手
    void configure(const CharacterData &cfg); //导入角色数据

    Q_INVOKABLE void playOpening(); // 播放开场动画
    Q_INVOKABLE void playStand(); // 切换到站立动画
    Q_INVOKABLE void playForward(); // 切换到前进行走动画
    Q_INVOKABLE void playBackward(); // 切换到后退行走动画
    Q_INVOKABLE void playJump(); // 切换到直跳动画
    Q_INVOKABLE void playDiagonalJump(bool forward); // 切换到对角跳动画(forward=true前跳 false后跳)
    Q_INVOKABLE void playLightPunch(); // 切换到轻拳攻击动画
    Q_INVOKABLE void playLightKick(); // 切换到轻腿攻击动画
    Q_INVOKABLE void playHeavyPunch(); // 切换到重拳攻击动画
    Q_INVOKABLE void playHeavyKick(); // 切换到重腿攻击动画
    Q_INVOKABLE void playHeavyStrike(); // 切换到超重击动画
    Q_INVOKABLE void playHurt(); // 切换到受击动画(通用)
    Q_INVOKABLE void playHurt1(); // 切换到轻度受击动画
    Q_INVOKABLE void playHurt2(); // 切换到中度受击动画
    Q_INVOKABLE void playHurt3(); // 切换到重度受击动画
    Q_INVOKABLE void playCrouch(); // 切换到下蹲动画
    Q_INVOKABLE void playCrouchAttack(); // 切换到下蹲攻击动画
    Q_INVOKABLE void stopCrouch(); // 退出下蹲状态
    Q_INVOKABLE void playDodge(bool forward); // 切换到闪避动画
    Q_INVOKABLE void playStandBlock(); // 切换到站立防御动画
    Q_INVOKABLE void releaseStandBlock(); // 松开防御键, 播放剩余帧后切回站立
    void reset(); // 重置到初始状态
    void updateRootHeight(double h); // 更新窗口高度(用于Y坐标计算)
    State state() const { return m_state; }
    int stateInt() const { return static_cast<int>(m_state); } // 状态整数形式(用于QML)
    Q_INVOKABLE bool isBlocking() const { return m_state == StandBlock; } // 是否处于防御状态

    // 碰撞检测相关
    bool isAttacking() const; // 是否处于攻击状态
    bool isAttackActive() const; // 当前帧是否有攻击判定
    double hitboxX() const; // 攻击框中心X(屏幕坐标)
    double hitboxY() const; // 攻击框中心Y(屏幕坐标)
    int hitboxRadius() const; // 攻击范围半径
    double hurtboxX() const; // 受击框中心X(屏幕坐标)
    double hurtboxY() const; // 受击框中心Y(屏幕坐标)
    int hurtboxW() const { return m_hurtboxW; } // 受击框宽度
    int hurtboxH() const { return m_hurtboxH; } // 受击框高度
    void applyKnockback(); // 被击中时应用击退

signals:
    void frameChanged(); // 帧切换
    void sourcePathChanged(); // 图片路径变更
    void sizeChanged(); // 帧尺寸变更
    void positionChanged(); // 位置变更
    void posXRatioChanged(); // 水平比例变更
    void facingChanged(); // 朝向变更
    void openingFinished(); // 开场动画播放完毕
    void jumpFinished(); // 跳跃动画播放完毕
    void attackFinished(); // 攻击动画播放完毕
    void hurtFinished(); // 受击动画播放完毕
    void dodgeFinished(); // 闪避动画播放完毕
    void stateChanged(); // 状态变更

private slots:
    void onTick(); // 定时器回调: 逐帧推进动画

private:
    void applyAnim(const AnimParams &p);
    void setPosY();
    bool tryFinishState(); // 尝试结束当前状态(帧播完时), 返回true表示已处理

    QTimer m_timer; // 动画计时器
    State m_state = Waiting; // 当前状态
    int m_currentFrame = 0; // 当前帧索引
    QString m_sourcePath; // 当前精灵表路径
    int m_frameWidth = 0; // 当前帧宽度
    int m_frameHeight = 0; // 当前帧高度
    int m_totalFrames = 0; // 总帧数
    double m_posX = 0.5; // X像素位置
    double m_posY = 0; // Y像素位置
    bool m_facingLeft = false; // 朝向: false=右 true=左

    AnimParams m_opening; // 开场动画参数副本
    AnimParams m_stand; // 站立动画参数副本
    AnimParams m_forward; // 前进动画参数副本
    AnimParams m_backward; // 后退动画参数副本
    AnimParams m_jump; // 跳跃动画参数副本
    AnimParams m_diagonalJump; // 对角跳动画参数副本
    AnimParams m_lightPunch; // 轻拳攻击动画参数副本
    AnimParams m_lightKick; // 轻腿攻击动画参数副本
    AnimParams m_heavyPunch; // 重拳攻击动画参数副本
    AnimParams m_heavyKick; // 重腿攻击动画参数副本
    AnimParams m_heavyStrike; // 超重击动画参数副本
    AnimParams m_hurt; // 受击动画参数副本
    AnimParams m_hurt1; // 轻度受击动画参数副本
    AnimParams m_hurt2; // 中度受击动画参数副本
    AnimParams m_hurt3; // 重度受击动画参数副本
    AnimParams m_crouch; // 下蹲动画参数副本
    AnimParams m_crouchAttack; // 下蹲攻击动画参数副本
    AnimParams m_dodge; // 闪避动画参数副本
    AnimParams m_standBlock; // 站立防御动画参数副本
    int m_blockHoldFrame = 0;     // 防御动画停顿帧索引
    bool m_blockReleased = false; // 防御键是否已松开
    bool m_crouching = false;  // 是否处于蹲姿
    bool m_loopAnim = true;    // 当前动画是否循环
    double m_cfgPosX = 0.5;     // 水平位置比例(可运行时修改)
    double m_jumpHeight = 200;  // 跳跃峰值高度(从配置加载)
    int m_refFrameWidth = 0;  // 参考帧宽(站立动画), 用于居中计算
    int m_djStartFrame = 0;  // 对角跳: 精灵表中起始帧偏移
    int m_djTotalFrames = 0; // 对角跳: 本段帧数
    bool m_djIsForward = false; // 对角跳: 方向标识
    bool m_djInitialFacingLeft = false; // 对角跳: 起跳时朝向(固定移动方向)
    double m_djStartXRatio = 0.0; // 对角跳: 起跳时X比例
    double m_djDistance = 0.15;   // 对角跳: 水平距离
    int m_djOffsetFirst = 0;  // 对角跳前跳: 首帧偏移
    int m_djOffsetLast = 0;   // 对角跳前跳: 末帧偏移
    int m_animOffsetX = 0;    // 当前动画水平偏移(像素)
    double m_visualScale = 1.0;  // 视觉缩放补偿
    bool m_dodgeForward = true;  // 闪避方向(true=前闪, false=后闪)
    int m_dodgeEndFrame = 0;     // 闪避终止帧
    int m_dodgeSwitchFrame = 0;  // 闪避offsetX切换帧(Orochi专用)
    int m_dodgeOffsetXPost = 0;  // 闪避瞬移后段水平偏移(Orochi专用)
    int m_dodgeOffsetXEnd = 0;   // 闪避尾帧目标水平偏移(Orochi专用)
    bool m_dodgeStandPending = false;  // 延迟playStand标志

    // 碰撞检测相关
    int m_hurtboxW = 120;     // 受击框宽度(像素)
    int m_hurtboxH = 250;     // 受击框高度(像素)
    AnimParams m_currentAnim;      // 当前动画参数引用(用于读取攻击点)
    bool m_hitThisAttack = false;  // 本次攻击是否已命中(防止重复命中)
    int m_knockbackToApply = 0;  // 待应用的击退距离(由攻击者设置)
    double m_knockbackRemaining = 0.0;  // 剩余击退距离(归一化坐标)
    int m_knockbackFrames = 0;       // 剩余击退帧数

    CharacterModel *m_opponent = nullptr;  // 对手角色引用, 用于跳跃朝向更新
    friend class FightDirector;    // 允许 FightDirector 直接设置 m_rootHeight
    double m_rootHeight = 640;     // 窗口高度(用于Y坐标计算)
};
