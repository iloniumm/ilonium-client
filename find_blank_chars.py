import os
from PIL import Image

def find_blanks(img_path):
    blanks = []
    if not os.path.exists(img_path):
        return blanks
    img = Image.open(img_path).convert('L')
    width, height = img.size
    cell_w, cell_h = width // 16, height // 16
    for i in range(256):
        x = (i % 16) * cell_w
        y = (i // 16) * cell_h
        cell = img.crop((x, y, x + cell_w, y + cell_h))
        is_blank = True
        for px in cell.getdata():
            if px > 10: # non-black
                is_blank = False
                break
        if is_blank:
            blanks.append(i)
    return blanks

print("font.png blanks:", find_blanks('textures/font.png'))
print("font_extra.png blanks:", find_blanks('textures/font_extra.png'))
