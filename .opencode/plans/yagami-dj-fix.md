# Yagami Diagonal Jump Fix Plan

## Goal
Fix Yagami's diagonal jump: height misalignment, size mismatch, frame stuttering, and side-switch bugs.

## Root Cause
Yagami DJ sprite sheet is 384x512 per frame while stand is 251x335. This causes:
- Ground level 177px higher than stand (groundY 68 vs 245)
- Character 1.35x larger visually
- RefFrameWidth (251) doesn't match actual frame width (384)
- Feet position varies 5-280px across frames

## Steps

### Step 1: Resize Yagami DJ sprite sheet
- Script: extract each frame's character, scale to match stand proportions (84% height fill), create 251x335 frames
- Use LANCZOS interpolation, preserve headroom
- Save as `DiagonalJump.png`, backup original to `DiagonalJump_backup.png`
- Verify: no clipping, feet consistent, character fills ~84% of frame

### Step 2: Update characters.json for Yagami diagonalJump
- `fw`: 384 → 251
- `fh`: 512 → 335
- Add `feetBottom` and `feetMargin` (calculated from resized sprite)
- Recalculate `offsetXFwd`, `offsetXLast`, `offsetXBwd`, `offsetXBwdLast` from new sprite centers

### Step 3: Build and test

## Files Modified
- `images/character/Yagami/DiagonalJump.png` (resized)
- `config/characters.json` (Yagami diagonalJump section)

## Verification
- Character size matches stand during diagonal jump
- Feet aligned with stand ground level
- No frame clipping
- Smooth transition from stand to jump and back
- Side-switch works without stutter
