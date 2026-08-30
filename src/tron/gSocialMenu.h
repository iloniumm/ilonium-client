#ifndef ArmageTron_SOCIAL_MENU_H
#define ArmageTron_SOCIAL_MENU_H

#include "uMenu.h"
#include "rTexture.h"
#include <string>

// Cross-platform helper to open an external URL in the OS default browser
void OpenURL(const std::string& url);

// Custom uMenu action item with 2D icon texture rendering
class uMenuItemSocialLink : public uMenuItemAction {
protected:
    std::string url_;
    rFileTexture* iconTex_;

public:
    uMenuItemSocialLink(uMenu *M, const tOutput& name, const tOutput& help, const std::string& url, const char* iconPath = nullptr);
    virtual ~uMenuItemSocialLink();

    virtual void Render(REAL x, REAL y, REAL alpha = 1.0f, bool selected = false) override;
    virtual void Enter() override;
};

// Opens the Social Hub menu (uMenu)
void sg_SocialsMenu();

#endif
