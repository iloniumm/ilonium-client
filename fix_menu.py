import re

with open('src/tron/gMenus.cpp', 'r') as f:
    content = f.read()

# 1. Fix character spacing - was 0.52, need much wider
content = content.replace(
    "        cx += charW * 0.52f;  // proper letter spacing",
    "        cx += charW * 1.15f;  // proper letter spacing with gaps"
)

# 2. Fix charW ratio - was 0.7, make narrower quads so spacing works
content = content.replace(
    "    drawText(x, y, size * 0.7f, size, text, r, g, b, a);",
    "    drawText(x, y, size * 0.55f, size, text, r, g, b, a);"
)

# 3. Remove the pulsing purple line completely - replace with static subtle line
content = content.replace(
    """    // Subtle accent line
    float glowPulse = 0.5f + 0.15f * sinf(headerGlow);
    glColor4f(0.45f * glowPulse, 0.30f * glowPulse, 0.85f * glowPulse, menuAlpha * 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(145, 101); glVertex2f(745, 101);
    glVertex2f(745, 103); glVertex2f(145, 103);
    glEnd();""",
    """    // Static subtle accent line
    glColor4f(0.30f, 0.22f, 0.55f, menuAlpha * 0.35f);
    glBegin(GL_QUADS);
    glVertex2f(145, 102); glVertex2f(745, 102);
    glVertex2f(745, 103); glVertex2f(145, 103);
    glEnd();"""
)

# 4. Remove avatar texture loading entirely, replace with styled initial
content = content.replace(
    """    // Avatar image
    static rFileTexture* avatarTex = NULL;
    static bool avatarLoaded = false;
    if (!avatarLoaded) {
        avatarTex = new rFileTexture(0, "textures/avatar.png", false, false, true);
        avatarLoaded = true;
    }

    if (avatarTex) {
        // Avatar circular clip (draw circle first, then texture on top)
        drawCircle(190, 514 + pOffsetY, 18, 0.55f, 0.35f, 0.90f, pAlpha * 0.6f); // glow ring
        
        avatarTex->Select();
        glEnable(GL_TEXTURE_2D);
        glColor4f(1, 1, 1, pAlpha);
        // Draw avatar as square over the circle area
        float ax = 174, ay = 498 + pOffsetY, as = 32;
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(ax, ay);
        glTexCoord2f(1, 0); glVertex2f(ax + as, ay);
        glTexCoord2f(1, 1); glVertex2f(ax + as, ay + as);
        glTexCoord2f(0, 1); glVertex2f(ax, ay + as);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }""",
    """    // Styled avatar (gradient circle with initial letter)
    // Outer glow ring
    drawCircle(190, 514 + pOffsetY, 19, 0.45f, 0.28f, 0.78f, pAlpha * 0.4f);
    // Avatar gradient circle
    drawCircle(190, 514 + pOffsetY, 16, 0.50f, 0.32f, 0.85f, pAlpha * 0.95f);
    // Inner lighter circle
    drawCircle(190, 514 + pOffsetY, 12, 0.58f, 0.40f, 0.92f, pAlpha * 0.6f);
    // Letter "D" in the center
    drawLabel(183, 506 + pOffsetY, 18, "D", 1.0f, 1.0f, 1.0f, pAlpha * 0.95f);"""
)

# 5. Remove the headerGlow increment (no longer needed)
content = content.replace(
    "    // Pulsing glow\n    headerGlow += 0.03f;\n",
    ""
)

with open('src/tron/gMenus.cpp', 'w') as f:
    f.write(content)

print("OK: all fixes applied")
