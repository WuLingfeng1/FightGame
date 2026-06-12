#!/usr/bin/env python3
from PIL import Image
import os

FW_ORIGINAL = 384
FH_ORIGINAL = 512
FW_NEW = 251
FH_NEW = 335

configs = [
    ("Orochi", "Backward.png", 16),
    ("Yagami", "BackWard.png", 9),
]

for name, filename, cols in configs:
    src_path = f"images/character/{name}/{filename}"
    bak_path = f"images/character/{name}/{filename.replace('.png', '_original.png')}"

    if not os.path.exists(bak_path) and os.path.exists(src_path):
        os.rename(src_path, bak_path)
        print(f"[{name}] Backed up to {bak_path}")

    img = Image.open(bak_path)
    frames = []
    for i in range(cols):
        x = i * FW_ORIGINAL
        frame = img.crop((x, 0, x + FW_ORIGINAL, FH_ORIGINAL))
        frame = frame.resize((FW_NEW, FH_NEW), Image.LANCZOS)
        frames.append(frame)

    new_w = cols * FW_NEW
    new_img = Image.new("RGBA", (new_w, FH_NEW))
    for i, frame in enumerate(frames):
        new_img.paste(frame, (i * FW_NEW, 0))

    new_img.save(src_path)
    print(f"[{name}] Saved {filename}: {new_w}x{FH_NEW}, {cols} frames of {FW_NEW}x{FH_NEW}")
