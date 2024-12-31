#!/usr/bin/env python3

from PIL import Image
import sys

img = Image.open(sys.argv[1])

width, height = img.size

output_file = open("frame.asm", "w", encoding="utf-8")
lines = [
    "start:\n",
    "   draw frame\n",
    "   sleep 10\n",
    "   hlt\n",
    "\n",
    "frame:\n",
    "\n",

]

for y in range(height):
    for x in range(width):
        rgb = img.getpixel((x, y))
        r, g, b = rgb
        lines.append(f"db {hex(r)}\n")
    
        
output_file.writelines(lines)
output_file.close()
print("done")
