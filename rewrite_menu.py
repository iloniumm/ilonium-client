import re

with open('src/tron/gMenus.cpp', 'r') as f:
    content = f.read()

new_code = r'''static int mouseX = 400, mouseY = 300;
static float hoverAlpha[14] = {0};
static bool modState[14] = {0};
static float toggleAnim[14] = {0}; // smooth toggle animation 0.0 = off, 1.0 = on

// =====================================================================
// CUSTOM ROUNDED RECTANGLE (pure OpenGL, no engine wrappers)
// =====================================================================
static void drawRoundedRect(float x, float y, float w, float h, float rad, float r_col, float g_col, float b_col, float a_col) {
    glDisable(GL_TEXTURE_2D);
    glColor4f(r_col, g_col, b_col, a_col);
    // Clamp radius
    if (rad > w * 0.5f) rad = w * 0.5f;
    if (rad > h * 0.5f) rad = h * 0.5f;
    glBegin(GL_POLYGON);
    // Top-left corner
    for (int i = 0; i <= 8; ++i) {
        float angle = M_PI + (M_PI / 2.0f) * (i / 8.0f);
        glVertex2f(x + rad + cosf(angle) * rad, y + rad + sinf(angle) * rad);
    }
    // Top-right corner
    for (int i = 0; i <= 8; ++i) {
        float angle = 1.5f * M_PI + (M_PI / 2.0f) * (i / 8.0f);
        glVertex2f(x + w - rad + cosf(angle) * rad, y + rad + sinf(angle) * rad);
    }
    // Bottom-right corner
    for (int i = 0; i <= 8; ++i) {
        float angle = 0.0f + (M_PI / 2.0f) * (i / 8.0f);
        glVertex2f(x + w - rad + cosf(angle) * rad, y + h - rad + sinf(angle) * rad);
    }
    // Bottom-left corner
    for (int i = 0; i <= 8; ++i) {
        float angle = 0.5f * M_PI + (M_PI / 2.0f) * (i / 8.0f);
        glVertex2f(x + rad + cosf(angle) * rad, y + h - rad + sinf(angle) * rad);
    }
    glEnd();
}

// =====================================================================
// CUSTOM BITMAP FONT RENDERER (bypasses rTextField completely!)
// Reads directly from the font.png bitmap atlas (16 cols x 8 rows)
// =====================================================================
static void drawText(float x, float y, float charW, float charH, const char* text,
                     float r, float g, float b, float a) {
    rFont::s_defaultFont.Select(true);
    glEnable(GL_TEXTURE_2D);
    glColor4f(r, g, b, a);

    const float texW = 1.0f / 16.0f;
    const float texH = 1.0f / 8.0f;
    const float pix = 0.002f; // small inset to avoid bleeding from adjacent glyphs
    float cx = x;

    for (const char* p = text; *p; p++) {
        unsigned char ch = (unsigned char)*p;
        if (ch < 32) continue; // skip control chars
        int col = ch % 16;
        int row = ch / 16;

        float tx0 = col * texW + pix;
        float tx1 = (col + 1) * texW - pix;
        float ty0 = row * texH + pix;
        float ty1 = (row + 1) * texH - pix;

        glBegin(GL_QUADS);
        glTexCoord2f(tx0, ty0); glVertex2f(cx, y);
        glTexCoord2f(tx1, ty0); glVertex2f(cx + charW, y);
        glTexCoord2f(tx1, ty1); glVertex2f(cx + charW, y + charH);
        glTexCoord2f(tx0, ty1); glVertex2f(cx, y + charH);
        glEnd();

        cx += charW * 0.65f; // character spacing (slightly tighter than full width)
    }
    glDisable(GL_TEXTURE_2D);
}

// Convenience: draw text with a given font size (height in pixels)
static void drawLabel(float x, float y, float size, const char* text,
                      float r, float g, float b, float a) {
    float charH = size;
    float charW = size * 0.6f; // aspect ratio of characters
    drawText(x, y, charW, charH, text, r, g, b, a);
}

// =====================================================================
// LERP helper
// =====================================================================
static float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

// =====================================================================
// VISUAL MENU RENDER
// =====================================================================
void cVisualMenu::Render() {
    if (!isOpen && menuAlpha <= 0.01f) return;
    
    // Smooth open/close
    float target = isOpen ? 1.0f : 0.0f;
    menuAlpha += (target - menuAlpha) * 0.12f;
    if (menuAlpha < 0.005f) menuAlpha = 0;

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
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Scale-in animation from center
    float scale = 0.92f + (0.08f * menuAlpha);
    glTranslatef(400, 300, 0);
    glScalef(scale, scale, 1);
    glTranslatef(-400, -300, 0);

    // Mouse in ortho coordinates
    float orthoMX = mouseX * (800.0f / sr_screenWidth);
    float orthoMY = mouseY * (600.0f / sr_screenHeight);
    // Adjust for scale transform
    float mx = (orthoMX - 400) / scale + 400;
    float my = (orthoMY - 300) / scale + 300;

    // === BACKGROUND DIMMING ===
    glColor4f(0, 0, 0, menuAlpha * 0.55f);
    glBegin(GL_QUADS);
        glVertex2f(-500, -500); glVertex2f(1300, -500);
        glVertex2f(1300, 1100); glVertex2f(-500, 1100);
    glEnd();

    // === SIDEBAR (dark vertical bar on left) ===
    drawRoundedRect(55, 55, 75, 490, 20, 0.06f, 0.06f, 0.07f, menuAlpha * 0.97f);

    // === MAIN PANEL ===
    drawRoundedRect(145, 55, 600, 490, 18, 0.09f, 0.09f, 0.10f, menuAlpha * 0.97f);

    // === HEADER BAR inside main panel ===
    drawRoundedRect(145, 55, 600, 50, 18, 0.11f, 0.11f, 0.13f, menuAlpha * 0.95f);
    // Fix: bottom corners of header should be square (overlap with main panel)
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.11f, 0.11f, 0.13f, menuAlpha * 0.95f);
    glBegin(GL_QUADS);
    glVertex2f(145, 85); glVertex2f(745, 85);
    glVertex2f(745, 105); glVertex2f(145, 105);
    glEnd();

    // === CATEGORY TABS ===
    // Active tab indicator (small pill under "Visuals")
    drawRoundedRect(340, 80, 70, 22, 11, 0.18f, 0.15f, 0.28f, menuAlpha * 0.9f);
    drawLabel(348, 83, 16, "Visuals", 1.0f, 1.0f, 1.0f, menuAlpha);
    drawLabel(438, 83, 16, "Hud", 0.45f, 0.45f, 0.50f, menuAlpha);
    drawLabel(498, 83, 16, "Util", 0.45f, 0.45f, 0.50f, menuAlpha);

    // === MOD GRID (2 columns x 7 rows) ===
    const char* modNames[] = {
        "Ambience", "Chat Helper", "Custom HitBox", "Hit Effect",
        "Item Physics", "Menu", "Removals",
        "Aspect Ratio", "Crosshair", "Custom Fog",
        "Hit Particles", "Jump Circle", "Object Info", "Prediction"
    };
    const char* modDescs[] = {
        "Changes world ambience", "Improves chat features", "Custom hitbox display",
        "Hit effect particles", "Realistic item physics", "Client menu settings",
        "Remove game elements",
        "Adjusts screen ratio", "Custom crosshair style", "Adjusts fog settings",
        "Hit particle effects", "Animated jump circles", "Object information",
        "Trajectory prediction"
    };

    for (int i = 0; i < 14; i++) {
        int col = i / 7;
        int row = i % 7;
        float bx = 165 + col * 285;
        float by = 120 + row * 52;

        // Hover detection
        bool hovered = (mx >= bx && mx <= bx + 270 && my >= by && my <= by + 48);
        float hTarget = hovered ? 1.0f : 0.0f;
        hoverAlpha[i] += (hTarget - hoverAlpha[i]) * 0.18f;

        // Toggle animation
        float tTarget = modState[i] ? 1.0f : 0.0f;
        toggleAnim[i] += (tTarget - toggleAnim[i]) * 0.15f;

        // Hover glow background
        if (hoverAlpha[i] > 0.01f) {
            drawRoundedRect(bx - 3, by - 2, 276, 50, 8,
                           0.14f, 0.14f, 0.17f, menuAlpha * 0.6f * hoverAlpha[i]);
        }

        // Mod name
        drawLabel(bx + 5, by + 5, 14, modNames[i], 0.88f, 0.88f, 0.90f, menuAlpha);
        // Mod description
        drawLabel(bx + 5, by + 24, 10, modDescs[i], 0.42f, 0.42f, 0.48f, menuAlpha);

        // Toggle switch
        float toggleT = toggleAnim[i];
        // Track background color (lerp grey -> purple)
        float tr = lerpf(0.18f, 0.55f, toggleT);
        float tg = lerpf(0.18f, 0.35f, toggleT);
        float tb = lerpf(0.22f, 0.90f, toggleT);
        drawRoundedRect(bx + 230, by + 10, 36, 18, 9, tr, tg, tb, menuAlpha);

        // Knob position (lerp left -> right)
        float knobX = lerpf(bx + 233, bx + 249, toggleT);
        drawRoundedRect(knobX, by + 12, 14, 14, 7, 1.0f, 1.0f, 1.0f, menuAlpha);
    }

    // === USER PROFILE AREA ===
    drawRoundedRect(165, 490, 220, 45, 10, 0.12f, 0.12f, 0.14f, menuAlpha * 0.92f);

    // Avatar placeholder (purple circle)
    drawRoundedRect(175, 497, 32, 32, 16, 0.55f, 0.35f, 0.90f, menuAlpha);

    // Username + UID
    drawLabel(215, 500, 16, "Developer", 1.0f, 1.0f, 1.0f, menuAlpha);
    drawLabel(215, 518, 11, "UID: 666", 0.50f, 0.50f, 0.60f, menuAlpha);

    // === SIDEBAR ICONS (colored dots like in Minecraft client) ===
    float sideColors[][3] = {
        {0.2f, 0.5f, 1.0f},   // blue
        {1.0f, 0.85f, 0.1f},  // yellow
        {1.0f, 0.3f, 0.2f},   // red
        {0.3f, 0.9f, 0.4f},   // green
        {0.8f, 0.4f, 1.0f},   // purple
    };
    for (int i = 0; i < 5; i++) {
        float sy = 80 + i * 55;
        drawRoundedRect(72, sy, 40, 40, 12,
                       sideColors[i][0], sideColors[i][1], sideColors[i][2],
                       menuAlpha * 0.85f);
    }
    // "+" button at bottom of sidebar
    drawRoundedRect(72, 80 + 5 * 55, 40, 40, 12, 0.15f, 0.15f, 0.18f, menuAlpha * 0.7f);
    drawLabel(84, 85 + 5 * 55, 22, "+", 0.5f, 0.5f, 0.55f, menuAlpha);

    // === CLOSE / SEARCH / SETTINGS ICONS (top right) ===
    // X button
    drawRoundedRect(715, 62, 24, 24, 6, 0.15f, 0.15f, 0.18f, menuAlpha * 0.8f);
    drawLabel(720, 64, 16, "X", 0.7f, 0.7f, 0.7f, menuAlpha);

    // === CUSTOM CURSOR ===
    glDisable(GL_TEXTURE_2D);
    // Cursor shadow
    glColor4f(0, 0, 0, menuAlpha * 0.4f);
    glBegin(GL_TRIANGLES);
    glVertex2f(mx + 1, my + 1);
    glVertex2f(mx + 13, my + 5);
    glVertex2f(mx + 5, my + 13);
    glEnd();
    // Cursor body (white)
    glColor4f(1, 1, 1, menuAlpha);
    glBegin(GL_TRIANGLES);
    glVertex2f(mx, my);
    glVertex2f(mx + 12, my + 4);
    glVertex2f(mx + 4, my + 12);
    glEnd();

    // === CLEANUP ===
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// =====================================================================
// EVENT HANDLER
// =====================================================================
bool cVisualMenu::HandleEvent(const SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_INSERT) {
            Toggle();
            return true;
        }
        if (isOpen && event.key.keysym.sym == SDLK_ESCAPE) {
            Toggle();
            return true;
        }
    }
    if (isOpen) {
        if (event.type == SDL_MOUSEMOTION) {
            mouseX = event.motion.x;
            mouseY = event.motion.y;
            return true;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            mouseX = event.button.x;
            mouseY = event.button.y;

            // Convert mouse to menu coordinates
            float scale = 0.92f + (0.08f * menuAlpha);
            float orthoMX = mouseX * (800.0f / sr_screenWidth);
            float orthoMY = mouseY * (600.0f / sr_screenHeight);
            float clickX = (orthoMX - 400) / scale + 400;
            float clickY = (orthoMY - 300) / scale + 300;

            // Check X button
            if (clickX >= 715 && clickX <= 739 && clickY >= 62 && clickY <= 86) {
                Toggle();
                return true;
            }

            // Check mod toggle clicks
            for (int i = 0; i < 14; i++) {
                int col = i / 7;
                int row = i % 7;
                float bx = 165 + col * 285;
                float by = 120 + row * 52;
                if (clickX >= bx && clickX <= bx + 270 && clickY >= by && clickY <= by + 48) {
                    modState[i] = !modState[i];
                    return true;
                }
            }
            return true;
        }
        if (event.type == SDL_MOUSEBUTTONUP) return true;
    }
    return false;
}'''

# Find and replace the entire menu rendering + event handler section
pattern = re.compile(r'static int mouseX.*?return false;\n\}', re.DOTALL)
new_file_content = pattern.sub(new_code, content)

with open('src/tron/gMenus.cpp', 'w') as f:
    f.write(new_file_content)

print("OK: replaced menu code")
