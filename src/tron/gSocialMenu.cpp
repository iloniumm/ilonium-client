#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#endif
#include "gSocialMenu.h"
#include "rGL.h"
#include "rScreen.h"
#include "rFont.h"
#include <cstdlib>
#include <iostream>

static bool IsLaunchableURL(const std::string& url) {
    if (url.compare(0, 8, "https://") != 0 && url.compare(0, 7, "http://") != 0)
        return false;

    return url.find_first_of("\"'`$;&|<>\\\n\r") == std::string::npos;
}

void OpenURL(const std::string& url) {
    if (!IsLaunchableURL(url))
        return;

#ifdef _WIN32
    // ShellExecute rather than "start": that needs a shell, and the console it
    // opens flashes over the game and takes focus with it
    ShellExecuteA( NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL );
    int rc = 0;
#elif __APPLE__
    int rc = std::system(("open \"" + url + "\"").c_str());
#else
    int rc = std::system(("xdg-open \"" + url + "\" &").c_str());
#endif
    (void)rc;
}

uMenuItemSocialLink::uMenuItemSocialLink(uMenu *M, const tOutput& name, const tOutput& help, const std::string& url, const char* iconPath)
    : uMenuItemAction(M, name, help), url_(url), iconTex_(nullptr)
{
    if (iconPath && *iconPath) {
        iconTex_ = tNEW(rFileTexture)(rTextureGroups::TEX_FONT, iconPath, false, false, true);
    }
}

uMenuItemSocialLink::~uMenuItemSocialLink() {
    if (iconTex_) {
        delete iconTex_;
        iconTex_ = nullptr;
    }
}

void uMenuItemSocialLink::Render(REAL x, REAL y, REAL alpha, bool selected) {
#ifndef DEDICATED
    if (sr_glOut) {
        if (iconTex_) {
            iconTex_->Select();
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(1.0f, 1.0f, 1.0f, alpha);

            REAL iconSize = 0.08f;
            REAL aspect = (REAL(sr_screenHeight) / sr_screenWidth) * (4.0 / 3.0);
            REAL iconW = iconSize * aspect;
            REAL iconH = iconSize;

            REAL iconX = x - 0.45f;
            REAL iconY = y;

            glBegin(GL_QUADS);
                glTexCoord2f(0.0f, 0.0f); glVertex2f(iconX - iconW * 0.5f, iconY + iconH * 0.5f);
                glTexCoord2f(1.0f, 0.0f); glVertex2f(iconX + iconW * 0.5f, iconY + iconH * 0.5f);
                glTexCoord2f(1.0f, 1.0f); glVertex2f(iconX + iconW * 0.5f, iconY - iconH * 0.5f);
                glTexCoord2f(0.0f, 1.0f); glVertex2f(iconX - iconW * 0.5f, iconY - iconH * 0.5f);
            glEnd();
            glDisable(GL_TEXTURE_2D);
        }
    }
#endif
    DisplayTextSpecial(x, y, name_, selected, alpha, 0);
}

void uMenuItemSocialLink::Enter() {
    OpenURL(url_);
}

#include "ModMenu.h"

void sg_SocialsMenu() {
#ifndef DEDICATED
    ModMenu::OpenTab(8);
#endif
}
