import re

with open('src/tron/gMenus.cpp', 'r') as f:
    content = f.read()

content = content.replace("float adjMouseX = (mouseX - 400) / scale + 400;", "float orthoX = mouseX * (800.0f / sr_screenWidth);\n    float adjMouseX = (orthoX - 400) / scale + 400;")
content = content.replace("float adjMouseY = (mouseY - 300) / scale + 300;", "float orthoY = mouseY * (600.0f / sr_screenHeight);\n    float adjMouseY = (orthoY - 300) / scale + 300;")

with open('src/tron/gMenus.cpp', 'w') as f:
    f.write(content)
