#!/usr/bin/env python3
from PIL import Image
import os

fw_original = 384
fh_original = 512
fw_new = 251
fh_new = 335

tasks = [
    ("images/character/Orochi/LightPunch.png", 12),
    ("images/character/Yagami/StandAttack.png", 34),
]

for src_path, cols in tasks:
    bak_path = src_path.replace(".png", "_original.png")

    if not os.path.exists(bak_path):
        os.rename(src_path, bak_path)
        print(f"Backed up: {bak_path}")

    img = Image.open(bak_path)
    frames = []
    for i in range(cols):
        x = i * fw_original
        frame = img.crop((x, 0, x + fw_original, fh_original))
        frame = frame.resize((fw_new, fh_new), Image.LANCZOS)
        frames.append(frame)

    new_w = cols * fw_new
    new_img = Image.new("RGBA", (new_w, fh_new))
    for i, frame in enumerate(frames):
        new_img.paste(frame, (i * fw_new, 0))

    new_img.save(src_path)
    print(f"Resized {src_path}: {new_w}x{fh_new}, {cols} frames @ {fw_new}x{fh_new}")
