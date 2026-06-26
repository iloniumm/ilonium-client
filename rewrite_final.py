import re

with open('src/tron/gMenus.cpp', 'r') as f:
    content = f.read()

new_code = r'''static int mouseX = 400, mouseY = 300;
static float hoverAlpha[20] = {0};
static bool modState[20] = {0};
static float toggleAnim[20] = {0};
static float slideIn[20] = {0};     // staggered slide-in per item
static float slideInTimer = 0;      // global timer for stagger
static float headerGlow = 0;        // subtle pulsing glow
static float profileSlide = 0;      // profile section slide-in
static bool menuFirstOpen = true;

// =====================================================================
// SMOOTH ROUNDED RECTANGLE (64 segments for silky smooth corners)
// =====================================================================
static void drawSmoothRect(float x, float y, float w, float h, float rad,
                           float r1, float g1, float b1, float a1) {
    glDisable(GL_TEXTURE_2D);
    if (rad > w * 0.5f) rad = w * 0.5f;
    if (rad > h * 0.5f) rad = h * 0.5f;
    
    const int segs = 16; // segments per corner (64 total)
    glColor4f(r1, g1, b1, a1);
    glBegin(GL_POLYGON);
    // Top-left
    for (int i = 0; i <= segs; ++i) {
        float a = M_PI + (M_PI / 2.0f) * ((float)i / segs);
        glVertex2f(x + rad + cosf(a) * rad, y + rad + sinf(a) * rad);
    }
    // Top-right
    for (int i = 0; i <= segs; ++i) {
        float a = 1.5f * M_PI + (M_PI / 2.0f) * ((float)i / segs);
        glVertex2f(x + w - rad + cosf(a) * rad, y + rad + sinf(a) * rad);
    }
    // Bottom-right
    for (int i = 0; i <= segs; ++i) {
        float a = (M_PI / 2.0f) * ((float)i / segs);
        glVertex2f(x + w - rad + cosf(a) * rad, y + h - rad + sinf(a) * rad);
    }
    // Bottom-left
    for (int i = 0; i <= segs; ++i) {
        float a = M_PI * 0.5f + (M_PI / 2.0f) * ((float)i / segs);
        glVertex2f(x + rad + cosf(a) * rad, y + h - rad + sinf(a) * rad);
    }
    glEnd();
}

// Gradient rectangle (top to bottom color blend)
static void drawGradientRect(float x, float y, float w, float h,
                             float r1, float g1, float b1, float a1,
                             float r2, float g2, float b2, float a2) {
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glColor4f(r1, g1, b1, a1); glVertex2f(x, y);
    glColor4f(r1, g1, b1, a1); glVertex2f(x + w, y);
    glColor4f(r2, g2, b2, a2); glVertex2f(x + w, y + h);
    glColor4f(r2, g2, b2, a2); glVertex2f(x, y + h);
    glEnd();
}

// Circle (for avatar frame etc)
static void drawCircle(float cx, float cy, float radius, float r, float g, float b, float a) {
    glDisable(GL_TEXTURE_2D);
    glColor4f(r, g, b, a);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 48; i++) {
        float angle = 2.0f * M_PI * i / 48.0f;
        glVertex2f(cx + cosf(angle) * radius, cy + sinf(angle) * radius);
    }
    glEnd();
}

// =====================================================================
// CUSTOM BITMAP FONT RENDERER
// Reads directly from font.png texture atlas (16 cols x 8 rows)
// =====================================================================
static void drawText(float x, float y, float charW, float charH, const char* text,
                     float r, float g, float b, float a) {
    rFont::s_defaultFont.Select(true);
    glEnable(GL_TEXTURE_2D);
    glColor4f(r, g, b, a);

    const float texW = 1.0f / 16.0f;
    const float texH = 1.0f / 8.0f;
    const float pix = 0.003f; // inset to avoid glyph bleeding
    float cx = x;

    for (const char* p = text; *p; p++) {
        unsigned char ch = (unsigned char)*p;
        if (ch < 32) continue;
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

        cx += charW * 0.52f;  // proper letter spacing
    }
    glDisable(GL_TEXTURE_2D);
}

static void drawLabel(float x, float y, float size, const char* text,
                      float r, float g, float b, float a) {
    drawText(x, y, size * 0.7f, size, text, r, g, b, a);
}

static float lerpf(float a, float b, float t) {
    if (t < 0) t = 0; if (t > 1) t = 1;
    return a + (b - a) * t;
}

// =====================================================================
// RENDER
// =====================================================================
void cVisualMenu::Render() {
    if (!isOpen && menuAlpha <= 0.005f) return;

    // Smooth alpha
    float target = isOpen ? 1.0f : 0.0f;
    menuAlpha += (target - menuAlpha) * 0.10f;
    if (!isOpen && menuAlpha < 0.005f) { menuAlpha = 0; return; }

    // Animation timer
    if (isOpen) {
        if (menuFirstOpen) { slideInTimer = 0; menuFirstOpen = false; }
        slideInTimer += 0.04f;
        if (slideInTimer > 10.0f) slideInTimer = 10.0f;
    } else {
        menuFirstOpen = true;
        slideInTimer = 0;
    }

    // Pulsing glow
    headerGlow += 0.03f;

    sr_ResetRenderState(true);

    // FORCE hide system cursor every frame when menu is open
    SDL_ShowCursor(isOpen ? SDL_DISABLE : SDL_ENABLE);

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

    // Scale-in animation
    float scale = 0.90f + (0.10f * menuAlpha);
    glTranslatef(400, 300, 0);
    glScalef(scale, scale, 1);
    glTranslatef(-400, -300, 0);

    // Mouse coords
    float orthoMX = mouseX * (800.0f / sr_screenWidth);
    float orthoMY = mouseY * (600.0f / sr_screenHeight);
    float mx = (orthoMX - 400) / scale + 400;
    float my = (orthoMY - 300) / scale + 300;

    // =================== BACKGROUND DIM ===================
    glColor4f(0, 0, 0, menuAlpha * 0.60f);
    glBegin(GL_QUADS);
    glVertex2f(-500, -500); glVertex2f(1300, -500);
    glVertex2f(1300, 1100); glVertex2f(-500, 1100);
    glEnd();

    // =================== MAIN PANEL ===================
    // Shadow behind panel
    drawSmoothRect(148, 58, 600, 490, 22, 0.0f, 0.0f, 0.0f, menuAlpha * 0.3f);
    // Panel body
    drawSmoothRect(145, 55, 600, 490, 20, 0.07f, 0.07f, 0.08f, menuAlpha * 0.97f);

    // =================== HEADER with subtle gradient ===================
    drawSmoothRect(145, 55, 600, 48, 20, 0.10f, 0.10f, 0.12f, menuAlpha * 0.97f);
    // Square off bottom of header
    glColor4f(0.10f, 0.10f, 0.12f, menuAlpha * 0.97f);
    glBegin(GL_QUADS);
    glVertex2f(145, 83); glVertex2f(745, 83);
    glVertex2f(745, 103); glVertex2f(145, 103);
    glEnd();
    // Subtle accent line
    float glowPulse = 0.5f + 0.15f * sinf(headerGlow);
    glColor4f(0.45f * glowPulse, 0.30f * glowPulse, 0.85f * glowPulse, menuAlpha * 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(145, 101); glVertex2f(745, 101);
    glVertex2f(745, 103); glVertex2f(145, 103);
    glEnd();

    // =================== CATEGORY TABS ===================
    // Active tab pill
    drawSmoothRect(320, 65, 90, 30, 15, 0.20f, 0.15f, 0.35f, menuAlpha * 0.85f);
    drawLabel(330, 70, 16, "Visuals", 1.0f, 1.0f, 1.0f, menuAlpha);
    drawLabel(435, 70, 16, "Hud", 0.40f, 0.40f, 0.45f, menuAlpha * 0.8f);
    drawLabel(500, 70, 16, "Util", 0.40f, 0.40f, 0.45f, menuAlpha * 0.8f);

    // =================== SETTINGS GRID ===================
    const char* modNames[] = {
        "Custom Fog",    "Rubber Gauge",  "Speed Meter",
        "Brake Meter",   "Show HUD",      "Show Scores",
        "Show Ping",     "Show Fastest",  "Minimap",
        "Alive Counter", "Show Time",     "24h Format",
        "Noclip Mode",   "Camera Lock"
    };
    const char* modDescs[] = {
        "Adjust fog color and density",
        "Display rubber usage gauge",
        "Show speed indicator",
        "Show brake indicator",
        "Toggle entire HUD display",
        "Display score overlay",
        "Show ping to server",
        "Show fastest player speed",
        "Enable minimap radar",
        "Show alive player count",
        "Display current time",
        "Use 24-hour time format",
        "Enable free camera mode",
        "Lock glance camera angle"
    };

    for (int i = 0; i < 14; i++) {
        int col = i / 7;
        int row = i % 7;
        float bx = 165 + col * 285;
        float by = 118 + row * 52;

        // Staggered slide-in animation
        float delay = i * 0.12f;
        float slideT = slideInTimer - delay;
        if (slideT < 0) slideT = 0;
        if (slideT > 1) slideT = 1;
        // Ease-out cubic
        float ease = 1.0f - (1.0f - slideT) * (1.0f - slideT) * (1.0f - slideT);
        slideIn[i] = ease;

        // Slide from right and fade in
        float offsetX = (1.0f - ease) * 60.0f;
        float itemAlpha = menuAlpha * ease;
        if (itemAlpha < 0.01f) continue;

        float ix = bx + offsetX;

        // Hover detection
        bool hovered = (mx >= ix && mx <= ix + 270 && my >= by && my <= by + 48);
        hoverAlpha[i] += ((hovered ? 1.0f : 0.0f) - hoverAlpha[i]) * 0.15f;

        // Toggle animation
        toggleAnim[i] += ((modState[i] ? 1.0f : 0.0f) - toggleAnim[i]) * 0.12f;

        // Hover glow
        if (hoverAlpha[i] > 0.01f) {
            drawSmoothRect(ix - 5, by - 3, 278, 50, 10,
                          0.13f, 0.12f, 0.18f, itemAlpha * 0.55f * hoverAlpha[i]);
        }

        // Mod name
        drawLabel(ix + 5, by + 4, 14, modNames[i], 0.85f, 0.85f, 0.88f, itemAlpha);
        // Description
        drawLabel(ix + 5, by + 24, 10, modDescs[i], 0.38f, 0.38f, 0.44f, itemAlpha * 0.85f);

        // Toggle track
        float tt = toggleAnim[i];
        float tr = lerpf(0.16f, 0.50f, tt);
        float tg = lerpf(0.16f, 0.30f, tt);
        float tb = lerpf(0.20f, 0.85f, tt);
        drawSmoothRect(ix + 232, by + 12, 34, 16, 8, tr, tg, tb, itemAlpha * 0.95f);

        // Toggle knob with shadow
        float knobX = lerpf(ix + 235, ix + 250, tt);
        drawSmoothRect(knobX - 0.5f, by + 13.5f, 13, 13, 6.5f, 0, 0, 0, itemAlpha * 0.2f); // shadow
        drawSmoothRect(knobX, by + 13, 12, 12, 6, 0.95f, 0.95f, 0.97f, itemAlpha);
    }

    // =================== SEPARATOR LINE ===================
    glColor4f(0.15f, 0.15f, 0.18f, menuAlpha * 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(165, 484); glVertex2f(730, 484);
    glVertex2f(730, 485); glVertex2f(165, 485);
    glEnd();

    // =================== PROFILE SECTION ===================
    profileSlide += ((isOpen ? 1.0f : 0.0f) - profileSlide) * 0.08f;
    float pEase = 1.0f - (1.0f - profileSlide) * (1.0f - profileSlide);
    float pAlpha = menuAlpha * pEase;
    float pOffsetY = (1.0f - pEase) * 30.0f;

    // Profile background
    drawSmoothRect(160, 490 + pOffsetY, 250, 48, 12, 0.10f, 0.10f, 0.12f, pAlpha * 0.92f);

    // Avatar image
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
    }

    // Username
    drawLabel(215, 499 + pOffsetY, 16, "Developer", 0.95f, 0.95f, 0.97f, pAlpha);
    // UID with accent color
    drawLabel(215, 518 + pOffsetY, 11, "UID: 666", 0.50f, 0.40f, 0.75f, pAlpha * 0.9f);

    // =================== CLOSE BUTTON ===================
    bool closeHovered = (mx >= 718 && mx <= 740 && my >= 60 && my <= 82);
    float closeAlpha = closeHovered ? 0.9f : 0.5f;
    drawSmoothRect(718, 60, 22, 22, 11, 0.20f, 0.15f, 0.15f, menuAlpha * closeAlpha);
    drawLabel(722, 63, 14, "X", 0.9f, 0.5f, 0.5f, menuAlpha * closeAlpha);

    // =================== CUSTOM CURSOR ===================
    glDisable(GL_TEXTURE_2D);
    // Shadow
    glColor4f(0, 0, 0, menuAlpha * 0.35f);
    glBegin(GL_TRIANGLES);
    glVertex2f(mx + 1.5f, my + 1.5f);
    glVertex2f(mx + 14, my + 5.5f);
    glVertex2f(mx + 5.5f, my + 14);
    glEnd();
    // White body
    glColor4f(1, 1, 1, menuAlpha * 0.95f);
    glBegin(GL_TRIANGLES);
    glVertex2f(mx, my);
    glVertex2f(mx + 13, my + 4);
    glVertex2f(mx + 4, my + 13);
    glEnd();
    // Inner highlight
    glColor4f(0.85f, 0.85f, 0.90f, menuAlpha * 0.5f);
    glBegin(GL_TRIANGLES);
    glVertex2f(mx + 2, my + 3);
    glVertex2f(mx + 10, my + 5);
    glVertex2f(mx + 5, my + 10);
    glEnd();

    // =================== CLEANUP ===================
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

            float scale = 0.90f + (0.10f * menuAlpha);
            float orthoMX = mouseX * (800.0f / sr_screenWidth);
            float orthoMY = mouseY * (600.0f / sr_screenHeight);
            float cx = (orthoMX - 400) / scale + 400;
            float cy = (orthoMY - 300) / scale + 300;

            // Close button
            if (cx >= 718 && cx <= 740 && cy >= 60 && cy <= 82) {
                Toggle();
                return true;
            }

            // Toggle clicks
            for (int i = 0; i < 14; i++) {
                int col = i / 7;
                int row = i % 7;
                float bx = 165 + col * 285;
                float by = 118 + row * 52;
                if (cx >= bx && cx <= bx + 270 && cy >= by && cy <= by + 48) {
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

pattern = re.compile(r'static int mouseX.*?return false;\n\}', re.DOTALL)
new_file_content = pattern.sub(new_code, content)

with open('src/tron/gMenus.cpp', 'w') as f:
    f.write(new_file_content)

print("OK: full rewrite done")
