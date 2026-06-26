import os
import subprocess

# Paths
MALE_SRC = "/home/scarlet/Desktop/New Folder"
FEMALE_SRC = "/home/scarlet/Downloads/women/misc/female"
NEUTRAL_SRC = "/home/scarlet/Downloads/hit_maker2.mp3"

DEST_DIR = "/home/scarlet/Documents/antigravity/retrocycles_mod/sound/announcer"

# Target directories
os.makedirs(os.path.join(DEST_DIR, "male"), exist_ok=True)
os.makedirs(os.path.join(DEST_DIR, "female"), exist_ok=True)
os.makedirs(os.path.join(DEST_DIR, "neutral"), exist_ok=True)

# Male mappings
male_map = {
    "Vo_announcer_killing_spree_announcer_1stblood_01.mp3.mpeg": "firstblood.wav",
    "Vo_announcer_killing_spree_announcer_kill_dominate_01.mp3.mpeg": "dominating.wav",
    "Vo_announcer_killing_spree_announcer_kill_double_01.mp3.mpeg": "doublekill.wav",
    "Vo_announcer_killing_spree_announcer_kill_godlike_01.mp3.mpeg": "godlike.wav",
    "Vo_announcer_killing_spree_announcer_kill_holy_01.mp3.mpeg": "holyshit.wav",
    "Vo_announcer_killing_spree_announcer_kill_mega_01.mp3.mpeg": "megakill.wav",
    "Vo_announcer_killing_spree_announcer_kill_monster_01.mp3.mpeg": "monsterkill.wav",
    "Vo_announcer_killing_spree_announcer_kill_rampage_01.mp3.mpeg": "rampage.wav",
    "Vo_announcer_killing_spree_announcer_kill_spree_01.mp3.mpeg": "killingspree.wav",
    "Vo_announcer_killing_spree_announcer_kill_triple_01.mp3.mpeg": "triplekill.wav",
    "Vo_announcer_killing_spree_announcer_kill_ultra_01.mp3.mpeg": "ultrakill.wav",
    "Vo_announcer_killing_spree_announcer_kill_unstop_01.mp3.mpeg": "unstoppable.wav",
    "Vo_announcer_killing_spree_announcer_kill_wicked_01.mp3.mpeg": "wickedsick.wav",
    "Vo_announcer_killing_spree_announcer_ownage_01.mp3.mpeg": "ownage.wav"
}

def convert_to_wav(src_path, dest_path):
    print(f"Converting {src_path} -> {dest_path}")
    # Convert to 22050Hz 16-bit mono WAV to be 100% compatible with the game's audio specs
    cmd = [
        "ffmpeg", "-y", "-i", src_path,
        "-acodec", "pcm_s16le", "-ar", "22050", "-ac", "1",
        dest_path
    ]
    subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

# 1. Convert male pack
for src_name, dest_name in male_map.items():
    src = os.path.join(MALE_SRC, src_name)
    dest = os.path.join(DEST_DIR, "male", dest_name)
    if os.path.exists(src):
        convert_to_wav(src, dest)
    else:
        print(f"Warning: {src} not found!")

# 2. Convert neutral hitmaker
convert_to_wav(NEUTRAL_SRC, os.path.join(DEST_DIR, "neutral", "hit.wav"))

# 3. Convert female pack
# Let's map female files
female_map = {
    "doublekill.wav": os.path.join(FEMALE_SRC, "multikill.wav"),
    "triplekill.wav": os.path.join(FEMALE_SRC, "multikill.wav"),
    "ultrakill.wav": os.path.join(FEMALE_SRC, "multikill.wav"),
    "rampage.wav": os.path.join(FEMALE_SRC, "rampage.wav"),
    "killingspree.wav": os.path.join(FEMALE_SRC, "killingspree.wav"),
    "dominating.wav": os.path.join(FEMALE_SRC, "dominating.wav"),
    "megakill.wav": os.path.join(FEMALE_SRC, "monsterkill.wav"),
    "unstoppable.wav": os.path.join(FEMALE_SRC, "monsterkill.wav"),
    "wickedsick.wav": os.path.join(FEMALE_SRC, "monsterkill.wav"),
    "monsterkill.wav": os.path.join(FEMALE_SRC, "monsterkill.wav"),
    "godlike.wav": os.path.join(FEMALE_SRC, "godlike.wav"),
    "holyshit.wav": os.path.join(FEMALE_SRC, "holyshit.wav"),
    "ownage.wav": os.path.join(FEMALE_SRC, "humiliation.wav")
}

# Firstblood fallback to male version
convert_to_wav(os.path.join(MALE_SRC, "Vo_announcer_killing_spree_announcer_1stblood_01.mp3.mpeg"),
               os.path.join(DEST_DIR, "female", "firstblood.wav"))

for dest_name, src_path in female_map.items():
    dest = os.path.join(DEST_DIR, "female", dest_name)
    if os.path.exists(src_path):
        convert_to_wav(src_path, dest)
    else:
        print(f"Warning: {src_path} not found!")

print("All conversions completed successfully.")
