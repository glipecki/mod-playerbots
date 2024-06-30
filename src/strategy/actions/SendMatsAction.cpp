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
    std::string const msg = event.getParam();
    std::vector<std::string> msgParts = split(msg, ' ');

    Player* receiver = GetMaster();
    if (msgParts.size() > 1) {
        if (Player* p = ObjectAccessor::FindPlayer(ObjectGuid(uint64(msgParts[msgParts.size() - 1].c_str())))) {
            receiver = p;
        }
    }

    bot->Whisper("Got it, i'll send you mats!", LANG_UNIVERSAL, receiver);

    GetTradeSkillMatsVisitor visitor;
    IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);

    std::vector<Item*> items = visitor.GetResult();

    std::ostringstream mailBody;
    mailBody << "Hello, " << receiver->GetName() << ",\n";
    mailBody << "\n";
    mailBody << "Here are the mats you asked for";
    mailBody << "\n\n";
    mailBody << bot->GetName() << "\n";

    const int limit = 5;
    int count = 0;

    for (Item* item : visitor.GetResult()) {
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

        MailDraft draft("Mats you asked for: " + item->GetTemplate()->Name1, mailBody.str());
        bot->MoveItemFromInventory(item->GetBagSlot(), item->GetSlot(), true);
        item->DeleteFromInventoryDB(trans);
        item->SetOwnerGUID(receiver->GetGUID());
        item->SaveToDB(trans);
        draft.AddItem(item);
        draft.SendMailTo(trans, MailReceiver(receiver), MailSender(bot));
        bot->Whisper("Sent mail to " + receiver->GetName(), LANG_UNIVERSAL, receiver);

        CharacterDatabase.CommitTransaction(trans);

        if (++count > limit) {
            bot->Whisper("I've stopped after item limit, got more items to send (" + std::to_string(items.size() - limit) + ")", LANG_UNIVERSAL, receiver);
            return true;
        }
    }

    return true;
}