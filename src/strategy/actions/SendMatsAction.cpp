#include "SendMatsAction.h"
#include "Mail.h"
#include "Event.h"
#include "ItemVisitors.h"
#include "ChatHelper.h"
#include "Playerbots.h"

bool SendMatsAction::Execute(Event event) {
    Player* receiver = GetMaster();

    const GuidVector gameObjects = *context->GetValue<GuidVector >("nearest game objects");
    if (!findMailbox()) {
        bot->Whisper("There is no mailbox nearby", LANG_UNIVERSAL, receiver);
        return false;
    }

    bot->Whisper("Got it, i'll send you mats!", LANG_UNIVERSAL, receiver);
    std::vector<Item*> items = findSkillItems();

    const int limit = 5;
    int count = 0;
    for (Item* item : items) {
        if (count < limit) {
            // bot->Whisper("Sent mail to " + receiver->GetName() + " with " + item->GetTemplate()->Name1, LANG_UNIVERSAL, receiver);
            moveItem(
                receiver,
                item,
                MailDraft(
                    "Mats you asked for: " + item->GetTemplate()->Name1,
                    mailBody(receiver, item)
                )
            );
        } else {
            bot->Whisper("I've stopped after item limit, got more items to send (" + std::to_string(items.size() - limit) + ")", LANG_UNIVERSAL, receiver);
            break;
        }
        count++;
    }
    return true;
}

class GetTradeSkillMatsVisitor : public FindItemVisitor
{
public:
    GetTradeSkillMatsVisitor() { }

    bool Accept(ItemTemplate const* itemTemplate) override {
        switch (itemTemplate->Class)
            // todo: nie wysyłaj soulbound? quest? itp?
        {
            case ITEM_CLASS_TRADE_GOODS:
            case ITEM_CLASS_MISC:
            case ITEM_CLASS_REAGENT:
            case ITEM_CLASS_GEM:
            case ITEM_CLASS_RECIPE:
                return true;
            default:
                return false;
        }
    }
};

GameObject* SendMatsAction::findMailbox() {
    GuidVector gos = *context->GetValue<GuidVector >("nearest game objects");
    for (ObjectGuid const guid : gos) {
        if (GameObject* go = botAI->GetGameObject(guid)) {
            if (go->GetGoType() == GAMEOBJECT_TYPE_MAILBOX) {
                return go;
            }
        }
    }
}

std::vector<Item*> SendMatsAction::findSkillItems() {
    GetTradeSkillMatsVisitor visitor;
    IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
    return visitor.GetResult();
}

std::string SendMatsAction::mailBody(Player* receiver, Item* item) {
    std::ostringstream mailBody;
    mailBody << "Hello, " << receiver->GetName() << ",\n";
    mailBody << "\n";
    mailBody << "Here are the mats you asked for.";
    mailBody << "Name: " << item->GetTemplate()->Name1 << "\n";
    mailBody << "ID: " << item->GetTemplate()->ItemId << "\n";
    mailBody << "Class: " << item->GetTemplate()->Class << "\n";
    mailBody << "SubClass: " << item->GetTemplate()->SubClass << "\n";
    mailBody << "Bonding: " << item->GetTemplate()->Bonding << "\n";
    mailBody << "IsSoulBound: " << item->IsSoulBound() << "\n";
    mailBody << "IsBoundAccountWide: " << item->IsBoundAccountWide() << "\n";
    mailBody << "IsBoundByEnchant: " << item->IsBoundByEnchant() << "\n";
    mailBody << "IsLocked: " << item->IsLocked() << "\n";
    mailBody << "IsCurrencyToken: " << item->IsCurrencyToken() << "\n";
    mailBody << "CanBeTraded(mail): " << item->CanBeTraded(true) << "\n";
    mailBody << "CanBeTraded(trade): " << item->CanBeTraded(false, true) << "\n";
    mailBody << "IsConjuredConsumable(trade): " << item->IsConjuredConsumable() << "\n";
    mailBody << "\n\n";
    mailBody << bot->GetName() << "\n";
    return mailBody.str();
}

void SendMatsAction::moveItem(Player* receiver, Item* item, MailDraft mail) {
    CharacterDatabaseTransaction trn = CharacterDatabase.BeginTransaction();
    bot->MoveItemFromInventory(item->GetBagSlot(), item->GetSlot(), true);
    item->DeleteFromInventoryDB(trn);
    item->SetOwnerGUID(receiver->GetGUID());
    item->SaveToDB(trn);
    mail.AddItem(item);
    mail.SendMailTo(trn, MailReceiver(receiver), MailSender(bot));
    CharacterDatabase.CommitTransaction(trn);
}