#!/usr/bin/env python3
"""Content-aware resizing of jump/diagonal jump sprite sheets.
Uniform scale (frame-0 ref), centered X, zero clipping via taller canvas if needed.
"""
from PIL import Image
import json, os, sys

FW_ORIG = 384
FH_ORIG = 512
FW_NEW = 251

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

with open(os.path.join(BASE, "config", "characters.json")) as f:
    CFG = json.load(f)

TASKS = [
    ("Orochi", "diagonalJump", "DiagonalJump.png.backup", "DiagonalJump.png", 335),
    ("Yagami", "jump",            "Jump_original.png",      "Jump.png",         335),
    ("Yagami", "diagonalJump",    "DiagonalJump.png.backup", "DiagonalJump.png", 335),
]


def get_stand_ref(char_dir):
    stand = Image.open(os.path.join(char_dir, "Stand.png"))
    frame0 = stand.crop((0, 0, FW_NEW, 335))
    alpha = frame0.split()[-1]
    bbox = alpha.getbbox()
    if bbox is None:
        return 335, 335, 335
    return bbox[2] - bbox[0], bbox[3] - bbox[1], bbox[3]


def get_frame_data(orig_img, col_idx):
    base_x = col_idx * FW_ORIG
    frame = orig_img.crop((base_x, 0, base_x + FW_ORIG, FH_ORIG))
    alpha = frame.split()[-1]
    bbox = alpha.getbbox()
    if bbox is None:
        return None
    ow = bbox[2] - bbox[0]
    oh = bbox[3] - bbox[1]
    return (base_x + bbox[0], bbox[1], base_x + bbox[2], bbox[3], ow, oh)


def process_animation(char_name, anim_key, backup_fname, output_fname, fh_new):
    char_dir = os.path.join(BASE, "images", "character", char_name)
    cfg = CFG[char_name][anim_key]
    cols = cfg["cols"]

    backup_path = os.path.join(char_dir, backup_fname)
    output_path = os.path.join(char_dir, output_fname)

    if not os.path.exists(backup_path):
        print(f"[{char_name}/{anim_key}] MISSING: {backup_path}", file=sys.stderr)
        return

    target_w, target_h, stand_bottom = get_stand_ref(char_dir)
    print(f"[{char_name}/{anim_key}] cols={cols}  fh={fh_new}  stand_ref w={target_w} h={target_h} bottom={stand_bottom}")

    orig_img = Image.open(backup_path)

    all_data = []
    max_ow, max_oh = 0, 0
    for i in range(cols):
        d = get_frame_data(orig_img, i)
        all_data.append(d)
        if d:
            max_ow = max(max_ow, d[4])
            max_oh = max(max_oh, d[5])

    f0 = all_data[0]
    if f0 is None:
        print("  frame 0 has no alpha content", file=sys.stderr)
        return
    ow0, oh0 = f0[4], f0[5]

    s0 = min(target_h / oh0, target_w / ow0)
    # Cap scale so tallest frame fits in canvas (per-task fh)
    uniform_scale = min(s0, fh_new / max_oh)

    # Feet position: align to stand bottom, capped to canvas
    target_bottom = max(int(max_oh * uniform_scale), stand_bottom)
    target_bottom = min(target_bottom, fh_new)

    print(f"  frame0 orig w={ow0} h={oh0}  max_orig w={max_ow} h={max_oh}  scale={uniform_scale:.4f}  tgt_bot={target_bottom}")

    frames = []
    widths, heights, bottoms = [], [], []

    for i in range(cols):
        d = all_data[i]
        if d is None:
            canvas = Image.new("RGBA", (FW_NEW, fh_new), (0, 0, 0, 0))
            frames.append(canvas)
            widths.append(0); heights.append(0); bottoms.append(0)
            continue

        ol, ot, or_, ob, ow, oh = d

        new_w = int(ow * uniform_scale)
        new_h = int(oh * uniform_scale)
        content = orig_img.crop((ol, ot, or_, ob))
        content = content.resize((new_w, new_h), Image.LANCZOS)

        paste_x = (FW_NEW - new_w) // 2
        paste_y = target_bottom - new_h

        canvas = Image.new("RGBA", (FW_NEW, fh_new), (0, 0, 0, 0))

        # Ensure no clipping: clamp to canvas bounds
        if paste_x < 0:
            paste_x = 0
        if paste_x + new_w > FW_NEW:
            paste_x = FW_NEW - new_w
        if paste_y < 0:
            paste_y = 0
        if paste_y + new_h > fh_new:
            paste_y = fh_new - new_h

        canvas.paste(content, (paste_x, paste_y))
        frames.append(canvas)

        fa = canvas.split()[-1]
        fb = fa.getbbox()
        widths.append(fb[2] - fb[0] if fb else 0)
        heights.append(fb[3] - fb[1] if fb else 0)
        bottoms.append(fb[3] if fb else 0)

    new_img = Image.new("RGBA", (cols * FW_NEW, fh_new))
    for i, frame in enumerate(frames):
        new_img.paste(frame, (i * FW_NEW, 0))
    new_img.save(output_path)

    min_w, max_w = min(widths), max(widths)
    min_h, max_h = min(heights), max(heights)
    min_bot, max_bot = min(bottoms), max(bottoms)
    feet_ok = (min_bot == target_bottom and max_bot == target_bottom)

    top_clip = sum(1 for f in frames if (b := f.split()[-1].getbbox()) and b[1] == 0)
    side_clip = sum(1 for f in frames if (b := f.split()[-1].getbbox()) and (b[0] == 0 or b[2] == FW_NEW))

    print(f"  widths:  {min_w:3d} ~ {max_w:3d}  (target {target_w})")
    print(f"  heights: {min_h:3d} ~ {max_h:3d}  (target {target_h})")
    print(f"  bottoms: {min_bot} ~ {max_bot}  (tgt {target_bottom})  feet={'OK' if feet_ok else 'MISMATCH'}")
    print(f"  top_clip={top_clip}/{cols}  side_clip={side_clip}/{cols}")
    print(f"  saved: {output_fname}  {cols * FW_NEW}x{fh_new}\n")


def main():
    for char_name, anim_key, backup_fname, output_fname, fh in TASKS:
        process_animation(char_name, anim_key, backup_fname, output_fname, fh)
    print("Done.")


if __name__ == "__main__":
    main()
