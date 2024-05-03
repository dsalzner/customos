"""
CustomOS
Copyright (C) 2024 D.Salzner <mail@dennissalzner.de>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

import re
import time
import argparse
import math

from PIL import Image
from PIL import ImageFont
from PIL import ImageDraw

FONT_FILE = "SourceCodePro-Light.ttf"
FONT_SIZE = 10
SIZE = "6x14"

parser = argparse.ArgumentParser(description="Convert TrueTypeFont to Rust Array")
parser.add_argument("-f", "--fontFile", help=f"Font to use (default: {FONT_FILE})", default=FONT_FILE, required=False)
parser.add_argument("-p", "--fontSize", help=f"Font size (default: {FONT_SIZE})", default=FONT_SIZE, required=False)
parser.add_argument("-s", "--size", help=f"Size to render to (default: {SIZE})", default=SIZE, required=False)
args = parser.parse_args()

# -- draw all chars in one image
chars = range(ord(' ') + 1, ord('~'))

width,height = map(int, args.size.split("x"))
img = Image.new(mode="1", size=(len(chars) * width, height))
draw = ImageDraw.Draw(img)
font = ImageFont.truetype(args.fontFile, int(args.fontSize))

s = ""
heightOffset = -2
for i,char in enumerate(chars):
  _, _, w, h = draw.textbbox((0, heightOffset), chr(char), font=font)
  if w != width:
    raise Exception("Font is not monospaced/width is not configured correctly")

  s += chr(char)
draw.text((0, heightOffset), s, font=font, fill=1)

img.show()

# -- generate bitmap buffer for each char
bufferLen = 0
entries = []
for i,char in enumerate(chars):
  im1 = img.crop((i*width, 0, (i+1)*width, height))
  im1.save('tmp.xbm')

  d = {}
  d['code'] = char
  with open('tmp.xbm', 'r') as file:
    text = file.read().replace('\n', '')
    print(text)
  d['width'] = re.findall(r'#define im_width (\d*)', text)[0]
  d['height'] = re.findall(r'#define im_height (\d*)', text)[0]
  d['buffer'] = re.findall(r'static char im_bits\[\] = \{(.*)\}', text)[0].replace(" ", "").split(",")

  if chr(char) >= "E" and chr(char) <= "h":
      input("any key to continue")

  entries += [d]

# -- get longest bitmap buffer length
longestBuffer = 0
for entry in entries:
  longestBuffer = max(longestBuffer, len(entry["buffer"]))

fontName = args.fontFile.replace(".ttf", "").lower().replace("-", "_")
arrayName = f"pub const FONT_{fontName.upper()}_{args.fontSize}PT"

# -- output the buffers to C-Array
out = ""
out += f"{arrayName}_BUFFER: &[[u8;{longestBuffer}];{len(entries)}] = &[\n"
for entry in entries:
  # -- fill to longest length with 0
  entry["buffer"] += ["0x00" for i in range(0, longestBuffer - len(entry["buffer"]))]
  # -- write entry
  out += "[" + ", ".join(entry["buffer"]) + "], " + "// " + str(entry["code"]) + " = '" + chr(entry["code"]) + "'\n"
out = out[:-1] + "\n"
out += "];\n"

out = out.replace(", ]", " ]")

print(out)

# -- output width to C-Arrays
out += "\n"
out += f"{arrayName}_WIDTH: &[u16] = &[\n"
for entry in entries:
  out += entry["width"] + ", "
out = out[:-2] + "\n"
out += "];\n"

# -- output height to C-Arrays
out += "\n"
out += f"{arrayName}_HEIGHT: &[u16] = &[\n"
for entry in entries:
  out += entry["height"] + ", "
out = out[:-2] + "\n"
out += "];\n"

with open('font{}_{}pt.rs'.format(fontName, args.fontSize), 'w') as f:
  f.write(out)
