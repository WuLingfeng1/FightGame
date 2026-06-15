# Boundary Fix Plan

## Goal
Fix left and right boundaries so characters don't overflow the visible screen area.

## Changes

### 1. fightdirector.h:57
`kStageWidth = 2.0` → `kStageWidth = 1.86`

### 2. fightdirector.cpp:100
`minPos = 0.08` → `minPos = 0.14`

### 3. FightScreen.qml:50 (P1 moveTimer left)
`0.08` → `0.14`

### 4. FightScreen.qml:49 (P1 moveTimer right)
`director.maxCameraOffset + 1.0` → `director.maxCameraOffset + 0.86`

### 5. FightScreen.qml:180 (P2 moveTimer left)
`0.08` → `0.14`

### 6. FightScreen.qml:179 (P2 moveTimer right)
`director.maxCameraOffset + 1.0` → `director.maxCameraOffset + 0.86`

## Verification
- Build and run
- Walk left: character left edge stays at screen left edge
- Walk right: character right edge stays at screen right edge
- Camera doesn't scroll past stage boundaries
