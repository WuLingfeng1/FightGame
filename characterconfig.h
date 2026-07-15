// Module
// File: characterconfig.h   Version: 0.1.0   License: AGPLv3
// Created:Linfeng Wu       2026-06-12 15:24:04
// Description:
//
#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>

// 动画参数: 描述一个动作的精灵表(sprite sheet)元数据
struct AnimParams
{
    int cols = 0;               // 帧数(精灵表列数)
    int fw = 0;                 // 单帧宽度(像素)
    int fh = 0;                 // 单帧高度(像素)
    int interval = 50;          // 帧间隔(毫秒), 控制播放速度
    QString path;               // 精灵表图片的 Qt 资源路径
    bool loop = true;           // 是否循环播放(站立/行走为true, 开场为false)
    int pauseFrame = -1;        // 开场暂停帧索引(-1=不暂停)
    int pauseDuration = 0;      // 暂停时长(毫秒)
    int feetBottom = 0;         // 站立时脚底到精灵表顶部的距离(用于脚部对齐)
    int feetMargin = 0;         // 站立时脚底到窗口底部的边距(用于脚部对齐)
    int jumpHeight = 200;       // 跳跃抛物线峰值高度(像素)
    int divFrame = 0;           // 方向帧分割点(对角跳用, 左半=前跳 右半=后跳)
    double jumpDistance = 0.15; // 对角跳水平距离(归一化坐标)
    int offsetXFwd = 0;         // 对角跳前跳段首帧水平偏移
    int offsetXLast = 0;        // 对角跳前跳段末帧水平偏移
    int offsetXBwd = 0;         // 对角跳后跳段首帧水平偏移
    int offsetXBwdLast = 0;     // 对角跳后跳段末帧水平偏移
    double visualScale = 1.0;   // 视觉缩放补偿(帧图缩小放大时用)
    int offsetX = 0;            // 角色在帧内的水平偏移补偿(用于居中对齐)
    double attackPointX = 0;    // 攻击点X偏移(相对于角色中心, 正值=前方)
    int attackPointY = 0;       // 攻击点Y偏移(相对于角色中心, 正值=下方)
    int attackRadius = 60;      // 攻击范围半径
    QString attackFrames;       // 攻击生效帧范围(如 "3-5" 表示帧3到帧5生效)
    int knockbackDistance = 0;  // 击退距离(归一化坐标×1000, 如50=0.05)
    int damage = 0;             // 攻击伤害值
    double dodgeDistance = 0;   // 闪避水平距离(归一化坐标)
    QString dodgeFrames;        // 前闪帧范围(如 "0-11")
    QString dodgeBackFrames;    // 后闪帧范围(如 "24-12", 倒序播放)
    int dodgeStartFrame = 0;    // 闪避瞬移开始帧(Orochi专用)
    int dodgeEndFrame = 0;      // 闪避瞬移结束帧(Orochi专用)
    int dodgeSwitchFrame = 0;   // 闪避offsetX切换帧(Orochi专用)
    int offsetXPost = 0;        // 闪避瞬移后段水平偏移(Orochi专用)
    int offsetXEnd = 0;         // 闪避尾帧目标水平偏移(Orochi专用)
    int blockHoldFrame = 0;     // 防御动画停顿帧(0=不停顿)
};

// 角色配置: 包含一个角色的所有动画参数和初始属性
struct CharacterData
{
    QString id;              // 角色英文标识符, 如 "Orochi"
    bool facingLeft = false; // 初始朝向: false=朝右, true=朝左
    double posX = 0.5;       // 初始水平位置比例(0.0~1.0)
    AnimParams opening;      // 开场动画参数
    AnimParams stand;        // 站立动画参数
    AnimParams forward;      // 前进行走动画参数
    AnimParams backward;     // 后退行走动画参数
    AnimParams jump;         // 直跳动画参数
    AnimParams diagonalJump; // 对角跳动画参数
    AnimParams lightPunch;   // 轻拳攻击动画参数
    AnimParams lightKick;    // 轻腿攻击动画参数
    AnimParams heavyPunch;   // 重拳攻击动画参数
    AnimParams heavyKick;    // 重腿攻击动画参数
    AnimParams heavyStrike;  // 超重击动画参数
    AnimParams hurt;         // 受击动画参数(通用)
    AnimParams hurt1;        // 轻度受击动画参数
    AnimParams hurt2;        // 中度受击动画参数
    AnimParams hurt3;        // 重度受击动画参数
    AnimParams crouch;       // 下蹲动画参数
    AnimParams crouchAttack; // 下蹲攻击动画参数
    AnimParams dodge;        // 闪避动画参数
    AnimParams standBlock;   // 站立防御动画参数
};

// 角色配置加载器: 从 JSON 对象解析出 CharacterData
class CharacterConfig
{
public:
    static CharacterData load(const QJsonObject &json, const QString &id);
};
