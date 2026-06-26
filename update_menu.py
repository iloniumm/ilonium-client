import re

with open('src/tron/gMenus.cpp', 'r') as f:
    content = f.read()

new_content = """static int mouseX = 400, mouseY = 300;
static float hoverAlpha[14] = {0};
static bool modState[14] = {0};

static void drawRoundedRect(float x, float y, float w, float h, float rad, float r_col, float g_col, float b_col, float a_col) {
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
    glEnable(GL_TEXTURE_2D); // FIX FOR WHITE BLOCKS
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(size, -size, 1);
    glColor4f(r, g, b, a);
    rFont::s_defaultFont.Select(true);
    rTextField c(0, 0, 1, 1);
    c.SetWidth(1000); // Prevent wrapping
    c.StringOutput(text);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void cVisualMenu::Render() {
    if (!isOpen && menuAlpha <= 0.01f) return;
    
    // Smooth open/close animation
    float targetAlpha = isOpen ? 1.0f : 0.0f;
    menuAlpha += (targetAlpha - menuAlpha) * 0.15f; 

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

    // Dynamic Scale Animation
    float scale = 0.95f + (0.05f * menuAlpha);
    glTranslatef(400, 300, 0);
    glScalef(scale, scale, 1);
    glTranslatef(-400, -300, 0);

    // Background Shadow overlay
    glDisable(GL_TEXTURE_2D);
    glColor4f(0, 0, 0, menuAlpha * 0.6f);
    glBegin(GL_QUADS);
        glVertex2f(-1000, -1000); glVertex2f(2000, -1000);
        glVertex2f(2000, 2000); glVertex2f(-1000, 2000);
    glEnd();

    // 1. Sidebar Panel
    drawRoundedRect(50, 50, 80, 500, 25, 0.05f, 0.05f, 0.06f, menuAlpha * 0.95f);
    
    // 2. Main Mod Panel
    drawRoundedRect(140, 50, 610, 500, 20, 0.08f, 0.08f, 0.09f, menuAlpha * 0.95f);

    // 3. Category Buttons (Visuals, Hud, Util)
    drawRoundedRect(350, 65, 200, 35, 15, 0.12f, 0.12f, 0.14f, menuAlpha * 0.9f);
    RenderTextCustom(375, 87, 14, "Visuals", 1, 1, 1, menuAlpha);
    RenderTextCustom(455, 87, 14, "Hud", 0.5f, 0.5f, 0.5f, menuAlpha);
    RenderTextCustom(505, 87, 14, "Util", 0.5f, 0.5f, 0.5f, menuAlpha);

    // 4. Mod Toggles Grid
    const char* mods[] = { "Ambience", "Chat Helper", "Custom HitBox", "Hit Effect", "Item Physics", "Menu", "Removals", "Aspect Ratio", "Crosshair", "FogBlur", "Hit Particles", "Jump Circle", "Object Info", "Swing Anim" };
    
    // Need mouse pos adjusted for scale
    float adjMouseX = (mouseX - 400) / scale + 400;
    float adjMouseY = (mouseY - 300) / scale + 300;

    for (int i=0; i<14; i++) {
        int col = i / 7;
        int row = i % 7;
        float x = 160 + col * 290;
        float y = 130 + row * 50;
        
        bool isHovered = (adjMouseX >= x && adjMouseX <= x + 270 && adjMouseY >= y && adjMouseY <= y + 45);
        float targetHover = isHovered ? 1.0f : 0.0f;
        hoverAlpha[i] += (targetHover - hoverAlpha[i]) * 0.2f;

        // Hover Background
        if (hoverAlpha[i] > 0.01f) {
            drawRoundedRect(x - 5, y - 5, 275, 45, 10, 0.15f, 0.15f, 0.18f, menuAlpha * 0.5f * hoverAlpha[i]);
        }
        
        RenderTextCustom(x, y + 15, 12, mods[i], 0.9f, 0.9f, 0.9f, menuAlpha);
        RenderTextCustom(x, y + 30, 8, "Adjusts gameplay settings", 0.5f, 0.5f, 0.5f, menuAlpha);
        
        // Toggle Switch
        if (modState[i]) {
            drawRoundedRect(x + 230, y + 5, 36, 18, 9, 0.6f, 0.4f, 1.0f, menuAlpha); // Active purple
            drawRoundedRect(x + 248, y + 7, 14, 14, 7, 1, 1, 1, menuAlpha); // Knob right
        } else {
            drawRoundedRect(x + 230, y + 5, 36, 18, 9, 0.2f, 0.2f, 0.25f, menuAlpha); // Inactive dark
            drawRoundedRect(x + 233, y + 7, 14, 14, 7, 0.5f, 0.5f, 0.5f, menuAlpha); // Knob left
        }
    }

    // 5. User Profile Area (Bottom Left of main panel)
    drawRoundedRect(160, 480, 200, 50, 10, 0.12f, 0.12f, 0.14f, menuAlpha * 0.9f);
    
    // Profile Box Avatar Placeholder (Using simple geometry instead of failing texture)
    drawRoundedRect(170, 490, 30, 30, 8, 0.6f, 0.4f, 1.0f, menuAlpha);

    RenderTextCustom(210, 505, 14, "Developer", 1, 1, 1, menuAlpha);
    RenderTextCustom(210, 520, 10, "UID: 666", 0.6f, 0.6f, 0.7f, menuAlpha);

    // Custom Mouse Cursor (Minecraft style small crosshair/pointer)
    glDisable(GL_TEXTURE_2D);
    glColor4f(1, 1, 1, menuAlpha);
    glBegin(GL_TRIANGLES);
    glVertex2f(adjMouseX, adjMouseY);
    glVertex2f(adjMouseX + 12, adjMouseY + 4);
    glVertex2f(adjMouseX + 4, adjMouseY + 12);
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

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
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            mouseX = event.button.x;
            mouseY = event.button.y;
            
            // Check clicks on toggles
            float scale = 0.95f + (0.05f * menuAlpha);
            float adjMouseX = (mouseX - 400) / scale + 400;
            float adjMouseY = (mouseY - 300) / scale + 300;
            
            for (int i=0; i<14; i++) {
                int col = i / 7;
                int row = i % 7;
                float x = 160 + col * 290;
                float y = 130 + row * 50;
                if (adjMouseX >= x && adjMouseX <= x + 270 && adjMouseY >= y && adjMouseY <= y + 45) {
                    modState[i] = !modState[i];
                    // Optional: could trigger a small sound effect here!
                }
            }
            return true;
        }
        if (event.type == SDL_MOUSEBUTTONUP) return true;
    }
    return false;
}"""

pattern = re.compile(r'static void drawRoundedRect.*?return false;\n}', re.DOTALL)
new_file_content = pattern.sub(new_content, content)

with open('src/tron/gMenus.cpp', 'w') as f:
    f.write(new_file_content)

