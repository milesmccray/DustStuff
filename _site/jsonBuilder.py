import json
import os

masterScripts = []
masterSprites = []

# Grab a list of paths from path.txt
# Generated in PowerShell from this command: Get-ChildItem -Recurse -File downloads | Resolve-Path -Relative > paths.txt
with open('my-site/paths.txt', 'r', encoding='utf-16') as f:
    paths = f.read().splitlines()
    paths = [p for p in paths if p.strip()]  # drop any blank lines
    # Normalize all backslashes to forward slashes for web/Jekyll use
    paths = [p.replace('\\', '/') for p in paths]

image_map = {}

# Pass 1: build a lookup of "base filename, no extension" -> full image path
for path in paths:
    if "/__IMAGES/" in path:
        filename = os.path.basename(path)
        key, _ext = os.path.splitext(filename)
        image_map[key] = path

# Pass 2: build the actual JSON entries
for path in paths:
    if "/Scripts/" in path:
        masterScripts.append({
            "title": "",
            "description": "",
            "category": "",
            "tags": [],
            "path": path,
        })

    if "/Sprites/" in path and "/__IMAGES" not in path:
        filename = os.path.basename(path)
        key, _ext = os.path.splitext(filename)

        masterSprites.append({
            "title": "",
            "author": "",
            "category": "",
            "path": path,
            "imagePath": image_map.get(key, "")
        })

# Dump Files
with open('my-site/_data/sprites2.json', 'w') as f:
    json.dump(masterSprites, f, indent=4)

with open('my-site/_data/scripts2.json', 'w') as f:
    json.dump(masterScripts, f, indent=4)