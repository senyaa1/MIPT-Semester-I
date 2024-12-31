#!/usr/bin/env python3

from PIL import Image
from pwn import *

import sys
import os

width, height = 480, 360

dir_path = sys.argv[1]

output_file = open("video.bin", "wb")

listing = os.listdir(dir_path)
listing.sort()


entrysize = 6 * len(listing) + 1

counter = 0
for entry in listing:
    counter += 1
    addr = entrysize + (counter * width * height)
    output_file.write(b"\x4b\x14" + p32(addr))

output_file.write(b'\x0e')


counter = 0
for entry in listing:
    counter += 1
    print("assembling: " + entry)
    full_path = os.path.join(dir_path, entry)

    img = Image.open(full_path)
    buffer = bytearray()
    for y in range(height):
        for x in range(width):
            rgb = img.getpixel((x, y))
            r, g, b = rgb
            buffer.append(r)
    output_file.write(buffer)
    img.close()
        
output_file.close()

print("done")
