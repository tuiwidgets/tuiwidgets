#! /usr/bin/python3

import sys
import os
import json

import PIL.Image
import PIL.ImageFont
import PIL.ImageDraw
import PIL.ImageColor

import freetype

conf_dict = {
    'font_path': '/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf',
    'font_bold_path': '/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf',
    'font_italic_path': '/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Oblique.ttf',
    'font_bold_italic_path': '/usr/share/fonts/truetype/dejavu/DejaVuSansMono-BoldOblique.ttf',
    'font_fallback_path': '/usr/share/fonts/opentype/unifont/unifont.otf'
                                if os.path.exists('/usr/share/fonts/opentype/unifont/unifont.otf')
                                else '/usr/share/fonts/truetype/unifont/unifont.ttf',
}

conf_path = os.path.join(os.path.abspath(os.path.dirname(__file__)), 'tpi2png.conf')
if os.path.exists(conf_path):
    with open(conf_path, 'r') as f:
        conf_dict.update(json.load(f))

input_file = sys.argv[1]
output_file = sys.argv[2]

with open(input_file, 'r') as fp:
    tpi = json.load(fp)

face = freetype.Face(conf_dict['font_path'])

font = {}
font[False, False] = PIL.ImageFont.truetype(conf_dict['font_path'], 15)
font[True, False] = PIL.ImageFont.truetype(conf_dict['font_bold_path'], 15)
font[False, True] = PIL.ImageFont.truetype(conf_dict['font_italic_path'], 15)
font[True, True] = PIL.ImageFont.truetype(conf_dict['font_bold_italic_path'], 15)

font_fallback = PIL.ImageFont.truetype(conf_dict['font_fallback_path'], 150)

#print(font[False, False].getbbox('M'))
_, _, cell_width, cell_height = font[False, False].getbbox(' ')
_, _, fallback_width, fallback_height = font_fallback.getbbox(' ')

cell_width, cell_height = cell_width, cell_height + 3

cols = tpi['width']
rows = tpi['height']

img = PIL.Image.new('RGBA', (cols * cell_width, rows * cell_height))
draw = PIL.ImageDraw.Draw(img)

scratch = PIL.Image.new('RGBA', (fallback_width, fallback_height))
scratch_draw = PIL.ImageDraw.Draw(scratch)

warned = set()

supported_attrs = set(['t', 'x', 'y', 'fg', 'bg', 'deco', 'bold', 'italic', 'underline', 'inverse', 'cleared'])

for cell in tpi['cells']:

    x = set([unsupported for unsupported in cell.keys() if unsupported not in supported_attrs])
    if x - warned:
        print('Unsupported cell attributes: ', x - warned)
        warned |= x

    fg = PIL.ImageColor.getrgb(cell.get('fg', '#fff'))
    bg = PIL.ImageColor.getrgb(cell.get('bg', '#000'))
    deco = None
    if cell.get('deco'):
        deco = PIL.ImageColor.getrgb(cell['deco'])
        
    bold = cell.get('bold', False)
    italic = cell.get('italic', False)
    underline = cell.get('underline', False)
        
    if cell.get("inverse", False):
        fg, bg = bg, fg

    x = cell['x'] * cell_width
    y = cell['y'] * cell_height
    width = cell.get('width', 1) * cell_width

    draw.rectangle((x, y,
                   x + width, y + cell_height),
                   fill=bg)
                   
                   
    if face.get_char_index(cell['t']):
        draw.text((x, y), cell['t'],
                   font=font[bold, italic], fill=fg)
    else:
        scratch.paste((0, 0, 0, 0), box=(0, 0, fallback_width, fallback_height))
        scratch_draw.text((0, 0), cell['t'], font=font_fallback, fill=fg)
        scaled = scratch.resize((cell_width, cell_height))
        img.alpha_composite(scaled, (x, y))
                   
    if underline:
        draw.line((x, y + cell_height - 1, x + width, y + cell_height - 1), fill=deco or fg)

img.save(output_file, format='png')
