#ifndef ArmageTron_TICKET_MENU_H
#define ArmageTron_TICKET_MENU_H

#include "uMenu.h"
#include "tString.h"
#include <ctime>
#include <string>

// Custom uMenuItemString subclass supporting placeholder text when input is empty
class uMenuItemPlaceholderString : public uMenuItemString {
protected:
    tString placeholder_;

public:
    uMenuItemPlaceholderString(uMenu *M, const tOutput& desc, const tOutput& help, tString &c, const tOutput& placeholder, int maxLength = 1024);
    virtual ~uMenuItemPlaceholderString() {}

    virtual void Render(REAL x, REAL y, REAL alpha = 1.0f, bool selected = false) override;
};

// Main Ticket System Menu class inheriting from uMenu
class gMenuTicketSystem : public uMenu {
protected:
    tString loginStr_;
    tString titleStr_;
    tString descStr_;

    uMenuItemPlaceholderString* itemLogin_;
    uMenuItemPlaceholderString* itemTitle_;
    uMenuItemPlaceholderString* itemDesc_;

    uMenuItem* itemSend_;
    uMenuItem* itemCooldownNotice_;
    uMenuItemExit* itemExit_;

    std::time_t lastSendTime_;
    bool isCooldownActive_;
    int cooldownMinutesLeft_;

public:
    gMenuTicketSystem();
    virtual ~gMenuTicketSystem();

    void CheckCooldown();
    void SaveCooldownTimestamp();
    void SendTicket();

    bool IsCooldownActive() const { return isCooldownActive_; }
    int GetCooldownMinutesLeft() const { return cooldownMinutesLeft_; }

protected:
    virtual void OnEnter() override;
    virtual void OnRender() override;
};

// Global trigger function to launch the ticket menu
void sg_TicketMenu();

#endif // ArmageTron_TICKET_MENU_H
