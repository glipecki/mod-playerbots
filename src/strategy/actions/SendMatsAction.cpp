#include "SendMatsAction.h"
#include "Mail.h"
#include "Event.h"
#include "ItemVisitors.h"
#include "ChatHelper.h"
#include "Playerbots.h"

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

bool SendMatsAction::Execute(Event event) {
    uint32 account = bot->GetSession()->GetAccountId();
    std::vector<std::string> msgParts = split(event.getParam(), ' ');

    Player* receiver = GetMaster();
//    if (msgParts.size() > 1) {
//        if (Player* p = ObjectAccessor::FindPlayer(ObjectGuid(uint64(msgParts[msgParts.size() - 1].c_str())))) {
//            receiver = p;
//        }
//    }

    bot->Whisper("Got it, i'll send you mats!", LANG_UNIVERSAL, receiver);

    GetTradeSkillMatsVisitor visitor;
    IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);

    std::vector<Item*> items = visitor.GetResult();

    const int limit = 5;
    int count = 0;

    for (Item* item : visitor.GetResult()) {
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

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

        MailDraft draft("Mats you asked for: " + item->GetTemplate()->Name1, mailBody.str());

        bot->MoveItemFromInventory(item->GetBagSlot(), item->GetSlot(), true);
        item->DeleteFromInventoryDB(trans);
        item->SetOwnerGUID(receiver->GetGUID());
        item->SaveToDB(trans);
        draft.AddItem(item);
        draft.SendMailTo(trans, MailReceiver(receiver), MailSender(bot));

        bot->Whisper("Sent mail to " + receiver->GetName() + " with " + item->GetTemplate()->Name1, LANG_UNIVERSAL, receiver);

        CharacterDatabase.CommitTransaction(trans);

        if (++count > limit) {
            bot->Whisper("I've stopped after item limit, got more items to send (" + std::to_string(items.size() - limit) + ")", LANG_UNIVERSAL, receiver);
            return true;
        }
    }

    return true;
}