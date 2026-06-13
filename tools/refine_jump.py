#!/usr/bin/env python3
from PIL import Image
import json, os

FW_NEW = 251
FH_NEW = 335
FW_ORIG = 384
FH_ORIG = 512

base = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
with open(os.path.join(base, "config", "characters.json")) as f:
    cfg = json.load(f)

for name in ["Orochi", "Yagami"]:
    jcfg = cfg[name]["jump"]
    cols = jcfg["cols"]
    char_dir = os.path.join(base, "images", "character", name)
    orig_path = os.path.join(char_dir, "Jump_original.png")
    dst_path = os.path.join(char_dir, "Jump.png")

    if not os.path.exists(orig_path):
        print(f"[{name}] ERROR: {orig_path} not found")
        continue

    stand_img = Image.open(os.path.join(char_dir, "Stand.png"))
    sr = stand_img.crop((0, 0, FW_NEW, FH_NEW)).split()[-1].getbbox()
    target_h = sr[3] - sr[1] if sr else FH_NEW
    target_w = sr[2] - sr[0] if sr else FW_NEW

    orig_img = Image.open(orig_path)
    frames = []
    overflow_count = 0

    for i in range(cols):
        x = i * FW_ORIG
        oframe = orig_img.crop((x, 0, x + FW_ORIG, FH_ORIG))
        obox = oframe.split()[-1].getbbox()
        if obox is None:
            oframe = oframe.resize((FW_NEW, FH_NEW), Image.LANCZOS)
            frames.append(oframe)
            continue

        ol, ot, or_, ob = obox
        ow = or_ - ol
        oh = ob - ot

        scale = min(target_h / oh, target_w / ow)

        content = oframe.crop((ol, ot, or_, ob))
        new_w = int(ow * scale)
        new_h = int(oh * scale)

        content = content.resize((new_w, new_h), Image.LANCZOS)

        canvas = Image.new("RGBA", (FW_NEW, FH_NEW), (0, 0, 0, 0))
        paste_x = (FW_NEW - new_w) // 2
        paste_y = FH_NEW - new_h
        canvas.paste(content, (paste_x, paste_y))
        frames.append(canvas)

        if paste_y < 0:
            overflow_count += 1

    new_img = Image.new("RGBA", (cols * FW_NEW, FH_NEW))
    for i, frame in enumerate(frames):
        new_img.paste(frame, (i * FW_NEW, 0))
    new_img.save(dst_path)

    feet_ok = True
    widths = []
    scales = []
    for i in range(cols):
        fa = frames[i].split()[-1]
        fb = fa.getbbox()
        if fb:
            if fb[3] != FH_NEW:
                feet_ok = False
            widths.append(fb[2] - fb[0])
        else:
            feet_ok = False
            widths.append(0)

    jf0 = frames[0].split()[-1].getbbox()
    jw, jh = jf0[2] - jf0[0], jf0[3] - jf0[1]
    min_w, max_w = min(widths), max(widths)
    ratio = max_w / min_w if min_w > 0 else 0

    print(f"[{name}]")
    print(f"  Stand ref: {target_w}x{target_h}")
    print(f"  Jump f0:   {jw}x{jh}  (w{jw-target_w:+d} h{jh-target_h:+d})")
    print(f"  Width:     {min_w} ~ {max_w}  (ratio {ratio:.2f}x)")
    print(f"  Feet:      {'all='+str(FH_NEW) if feet_ok else 'MISMATCH'}")
    if overflow_count > 0:
        print(f"  Overflow:  {overflow_count}/{cols} frames (clip at y=0, no crop)")
    print(f"  Saved {cols*FW_NEW}x{FH_NEW}\n")
