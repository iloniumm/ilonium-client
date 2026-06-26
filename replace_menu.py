import re

with open('src/tron/gMenus.cpp', 'r') as f:
    content = f.read()

new_content = """static void drawRoundedRect(float x, float y, float w, float h, float rad, float r_col, float g_col, float b_col, float a_col) {
    glDisable(GL_TEXTURE_2D);
    glColor4f(r_col, g_col, b_col, a_col);
    glBegin(GL_POLYGON);
    for (int i = 0; i <= 10; ++i) {
        float angle = M_PI + (M_PI / 2.0f) * (i / 10.0f);
        glVertex2f(x + rad + cos(angle) * rad, y + rad + sin(angle) * rad);
    }
    for (int i = 0; i <= 10; ++i) {
        float angle = 1.5f * M_PI + (M_PI / 2.0f) * (i / 10.0f);
        glVertex2f(x + w - rad + cos(angle) * rad, y + rad + sin(angle) * rad);
    }
    for (int i = 0; i <= 10; ++i) {
        float angle = 0.0f + (M_PI / 2.0f) * (i / 10.0f);
        glVertex2f(x + w - rad + cos(angle) * rad, y + h - rad + sin(angle) * rad);
    }
    for (int i = 0; i <= 10; ++i) {
        float angle = 0.5f * M_PI + (M_PI / 2.0f) * (i / 10.0f);
        glVertex2f(x + rad + cos(angle) * rad, y + h - rad + sin(angle) * rad);
    }
    glEnd();
}

static void RenderTextCustom(float x, float y, float size, const char* text, float r, float g, float b, float a) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(size, -size, 1);
    glColor4f(r, g, b, a);
    rFont::s_defaultFont.Select(true);
    rTextField c(0, 0, 1, 1);
    c.SetWidth(1000); // Prevent wrapping
    c.StringOutput(text);
    glPopMatrix();
}

void cVisualMenu::Render() {
    if (!isOpen && menuAlpha <= 0.01f) return;
    
    // Animation logic
    if (isOpen) {
        menuAlpha += (1.0f - menuAlpha) * 0.15f;
    } else {
        menuAlpha += (0.0f - menuAlpha) * 0.15f;
    }

    sr_ResetRenderState(true);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 800, 600, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Apply scale animation
    float scale = 0.95f + (0.05f * menuAlpha);
    glTranslatef(400, 300, 0);
    glScalef(scale, scale, 1);
    glTranslatef(-400, -300, 0);

    // Background Shadow/Overlay (Darken the game behind the menu)
    glDisable(GL_TEXTURE_2D);
    glColor4f(0, 0, 0, menuAlpha * 0.5f);
    glBegin(GL_QUADS);
        glVertex2f(-1000, -1000);
        glVertex2f(2000, -1000);
        glVertex2f(2000, 2000);
        glVertex2f(-1000, 2000);
    glEnd();

    // 1. Sidebar Panel (Like Minecraft reference)
    drawRoundedRect(50, 50, 80, 500, 30, 0.08f, 0.08f, 0.08f, menuAlpha * 0.95f);
    
    // 2. Main Mod Panel
    drawRoundedRect(150, 50, 600, 500, 20, 0.1f, 0.1f, 0.12f, menuAlpha * 0.95f);

    // 3. Category Buttons (Visuals, Hud, Util)
    drawRoundedRect(350, 70, 200, 30, 15, 0.15f, 0.15f, 0.18f, menuAlpha * 0.9f);
    RenderTextCustom(380, 80, 10, "Visuals", 1, 1, 1, menuAlpha);
    RenderTextCustom(460, 80, 10, "Hud", 0.6f, 0.6f, 0.6f, menuAlpha);
    RenderTextCustom(510, 80, 10, "Util", 0.6f, 0.6f, 0.6f, menuAlpha);

    // 4. Mod Toggles Grid (Mockup for now)
    const char* mods[] = { "Ambience", "Chat Helper", "Custom HitBox", "Hit Effect", "Item Physics", "Menu", "Removals", "Aspect Ratio", "Crosshair", "FogBlur", "Hit Particles", "Jump Circle", "Object Info", "Swing Animation" };
    for (int i=0; i<14; i++) {
        int col = i / 7;
        int row = i % 7;
        float x = 180 + col * 280;
        float y = 130 + row * 50;
        
        // Mod name
        RenderTextCustom(x, y, 10, mods[i], 0.9f, 0.9f, 0.9f, menuAlpha);
        // Mod description
        RenderTextCustom(x, y + 15, 6, "Description text here...", 0.5f, 0.5f, 0.5f, menuAlpha);
        
        // Toggle Switch background
        drawRoundedRect(x + 220, y + 5, 30, 15, 7.5f, 0.2f, 0.2f, 0.25f, menuAlpha);
        // Toggle Knob (Active for some)
        if (i % 3 == 0) {
            drawRoundedRect(x + 220, y + 5, 30, 15, 7.5f, 0.6f, 0.5f, 1.0f, menuAlpha); // Purple active
            drawRoundedRect(x + 235, y + 6, 13, 13, 6.5f, 1, 1, 1, menuAlpha); // Knob right
        } else {
            drawRoundedRect(x + 222, y + 6, 13, 13, 6.5f, 0.5f, 0.5f, 0.5f, menuAlpha); // Knob left
        }
    }

    // 5. User Profile Area (Bottom Left of main panel)
    drawRoundedRect(170, 480, 200, 50, 10, 0.15f, 0.15f, 0.18f, menuAlpha * 0.9f);

    // Render Avatar
    static rFileTexture* avatarTex = NULL;
    if (!avatarTex) {
        avatarTex = new rFileTexture(0, "textures/avatar.png", false, false, true);
    }

    if (avatarTex) {
        avatarTex->Select();
        glEnable(GL_TEXTURE_2D);
        glColor4f(1, 1, 1, menuAlpha);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(180, 490);
        glTexCoord2f(1, 0); glVertex2f(210, 490);
        glTexCoord2f(1, 1); glVertex2f(210, 520);
        glTexCoord2f(0, 1); glVertex2f(180, 520);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }

    // Fix texture binding for fonts
    rFont::s_defaultFont.Select(true);

    // Draw Developer Text
    RenderTextCustom(220, 495, 12, "Developer", 1, 1, 1, menuAlpha);
    RenderTextCustom(220, 510, 8, "UID: 666", 0.6f, 0.6f, 0.7f, menuAlpha);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}"""

pattern = re.compile(r'static void DrawRoundedRect.*?glMatrixMode\(GL_MODELVIEW\);\n}', re.DOTALL)
new_file_content = pattern.sub(new_content, content)

with open('src/tron/gMenus.cpp', 'w') as f:
    f.write(new_file_content)

