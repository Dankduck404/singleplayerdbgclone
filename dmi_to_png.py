"""
dmi_to_png.py — Convert BYOND .dmi files to PNG/GIF
  - Single-frame states  → .png
  - Multi-frame states   → animated .gif
  - Dirs: 1=SOUTH only, 4=S/N/E/W, 8=S/N/E/W+diagonals

Usage:
    py dmi_to_png.py                    # converts all .dmi in "BYOND Icon Folder"
    py dmi_to_png.py path/to/file.dmi   # converts one file
    py dmi_to_png.py --folder some/dir  # converts all .dmi in a folder

Output goes into "Converted Icons/<dmi_name>/<state_name>.png|gif"
"""

import sys
import os
import re
import zlib
import struct
from pathlib import Path
from PIL import Image

# ── DMI metadata parser ────────────────────────────────────────────────────────

def read_dmi_metadata(path: Path) -> tuple[int, int, list[dict]]:
    """
    Parse the zTXt 'Description' chunk from a .dmi (PNG) file.
    Returns (frame_width, frame_height, states) where each state is:
        { name, dirs, frames, delay, loop, rewind, movement, hotspot }
    """
    data = path.read_bytes()

    # Walk PNG chunks to find zTXt with key "Description"
    pos = 8  # skip PNG signature
    desc_raw = None
    while pos < len(data):
        length = struct.unpack(">I", data[pos:pos+4])[0]
        chunk_type = data[pos+4:pos+8]
        chunk_data = data[pos+8:pos+8+length]
        if chunk_type == b"zTXt":
            null_idx = chunk_data.index(0)
            key = chunk_data[:null_idx].decode("latin-1")
            if key == "Description":
                # compression method byte follows null, then compressed data
                desc_raw = zlib.decompress(chunk_data[null_idx+2:]).decode("latin-1")
                break
        pos += 12 + length  # 4 len + 4 type + data + 4 crc

    if desc_raw is None:
        raise ValueError(f"No DMI Description chunk found in {path.name}")

    # Parse the description text
    frame_w = frame_h = 32
    states = []
    current = None

    for line in desc_raw.splitlines():
        line = line.strip()
        if line.startswith("width"):
            frame_w = int(line.split("=")[1].strip())
        elif line.startswith("height"):
            frame_h = int(line.split("=")[1].strip())
        elif line.startswith("state ="):
            if current is not None:
                states.append(current)
            name = re.search(r'"(.*)"', line)
            current = {
                "name": name.group(1) if name else "",
                "dirs": 1,
                "frames": 1,
                "delay": [],
                "loop": 0,
                "rewind": 0,
                "movement": 0,
                "hotspot": None,
            }
        elif current is not None:
            if line.startswith("dirs"):
                current["dirs"] = int(line.split("=")[1].strip())
            elif line.startswith("frames"):
                current["frames"] = int(line.split("=")[1].strip())
            elif line.startswith("delay"):
                vals = line.split("=")[1].strip().split(",")
                current["delay"] = [float(v.strip()) for v in vals]
            elif line.startswith("loop"):
                current["loop"] = int(line.split("=")[1].strip())
            elif line.startswith("rewind"):
                current["rewind"] = int(line.split("=")[1].strip())
            elif line.startswith("movement"):
                current["movement"] = int(line.split("=")[1].strip())

    if current is not None:
        states.append(current)

    return frame_w, frame_h, states


# ── Frame extraction ───────────────────────────────────────────────────────────

DIR_NAMES = {1: ["south"],
             4: ["south", "north", "east", "west"],
             8: ["south", "north", "east", "west",
                 "southeast", "southwest", "northeast", "northwest"]}

def extract_states(dmi_path: Path, out_dir: Path):
    frame_w, frame_h, states = read_dmi_metadata(dmi_path)

    sheet = Image.open(dmi_path)
    sheet_w, sheet_h = sheet.size
    cols = sheet_w // frame_w  # frames per row in the sprite sheet

    out_dir.mkdir(parents=True, exist_ok=True)

    frame_index = 0  # absolute frame counter across the whole sprite sheet

    for state in states:
        dirs    = state["dirs"]
        frames  = state["frames"]
        delay   = state["delay"]   # delay per frame in ticks (1 tick = 100ms in BYOND)
        name    = state["name"] or "unnamed"
        # Sanitise for filesystem
        safe_name = re.sub(r'[\\/:*?"<>|]', "_", name) or "unnamed"

        dir_names = DIR_NAMES.get(dirs, ["south"])
        total_frames = dirs * frames

        # Gather all PIL frames for this state
        # Layout: for each animation frame, all dirs are stored consecutively
        # Actually BYOND stores: frame0_dir0, frame0_dir1 ... frame1_dir0 ...
        # i.e. dirs cycle faster than frames
        pil_frames_by_dir = {d: [] for d in dir_names}

        for anim_frame in range(frames):
            for di, dir_name in enumerate(dir_names):
                abs_idx = frame_index + anim_frame * dirs + di
                row = abs_idx // cols
                col = abs_idx % cols
                x0 = col * frame_w
                y0 = row * frame_h
                crop = sheet.crop((x0, y0, x0 + frame_w, y0 + frame_h))
                pil_frames_by_dir[dir_name].append(crop)

        frame_index += total_frames

        # Build durations list (ms). BYOND tick = 100ms.
        def build_durations(n_frames):
            if delay and len(delay) >= n_frames:
                return [int(d * 100) for d in delay[:n_frames]]
            return [100] * n_frames

        for dir_name, dir_frames in pil_frames_by_dir.items():
            if dirs == 1:
                suffix = ""  # only one direction, skip dir suffix
            else:
                suffix = f"_{dir_name}"

            if frames == 1:
                # Static PNG
                out_path = out_dir / f"{safe_name}{suffix}.png"
                dir_frames[0].save(str(out_path), format="PNG")
            else:
                # Animated GIF
                out_path = out_dir / f"{safe_name}{suffix}.gif"
                durations = build_durations(frames)
                dir_frames[0].save(
                    str(out_path),
                    format="GIF",
                    save_all=True,
                    append_images=dir_frames[1:],
                    duration=durations,
                    loop=state["loop"],
                    disposal=2,
                )

    return len(states)


# ── Main ───────────────────────────────────────────────────────────────────────

def convert_file(dmi_path: Path, base_out: Path, skip_existing: bool = False):
    out_dir = base_out / dmi_path.stem
    # Skip if output folder already exists and has files (--skip-existing mode)
    if skip_existing and out_dir.exists() and any(out_dir.iterdir()):
        return
    try:
        count = extract_states(dmi_path, out_dir)
        print(f"  OK  {dmi_path.name}  ({count} states)  → {out_dir}")
    except Exception as e:
        print(f"  ERR {dmi_path.name}: {e}")


def main():
    args = sys.argv[1:]

    skip_existing = "--skip-existing" in args
    args = [a for a in args if a != "--skip-existing"]

    base_out = Path("Converted Icons")

    if "--folder" in args:
        idx = args.index("--folder")
        folder = Path(args[idx + 1])
        dmi_files = list(folder.glob("*.dmi"))
    elif args and not args[0].startswith("-"):
        dmi_files = [Path(a) for a in args]
    else:
        # Default: "BYOND Icon Folder" next to this script
        folder = Path(__file__).parent / "BYOND Icon Folder"
        dmi_files = list(folder.glob("*.dmi"))

    if not dmi_files:
        print("No .dmi files found.")
        return

    print(f"Converting {len(dmi_files)} DMI file(s) → {base_out}/")
    for f in sorted(dmi_files):
        convert_file(f, base_out, skip_existing)
    print("Done.")


if __name__ == "__main__":
    main()
