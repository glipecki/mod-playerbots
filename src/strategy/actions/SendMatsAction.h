/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SENDMATSACTION_H
#define _PLAYERBOT_SENDMATSACTION_H

#include "InventoryAction.h"
#include "Mail.h"

class PlayerbotAI;

class SendMatsAction : public InventoryAction
{
    public:
        SendMatsAction(PlayerbotAI* botAI) : InventoryAction(botAI, "sendmats") { }

        bool Execute(Event event) override;
    private:
        GameObject* findMailbox();
        std::vector<Item*> findSkillItems();
        std::string mailBody(Player* receiver, Item* item);
        void moveItem(Player* receiver, Item* item, MailDraft mail);
};

#endif
