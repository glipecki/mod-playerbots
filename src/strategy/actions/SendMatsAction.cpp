#include "SendMatsAction.h"
#include "Mail.h"
#include "Event.h"
#include "ItemVisitors.h"
#include "ChatHelper.h"
#include "Playerbots.h"

class FindItemByFlagVisitor : public FindItemVisitor
{
public:
    FindItemByFlagVisitor(uint32 flag) : flag(flag) { }

    bool Accept(ItemTemplate const* itemTemplate) override {
//          return itemTemplate->Flags2 & flag;
        return true;
    }

private:
    uint32 flag;
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

    FindItemByFlagVisitor visitor(ITEM_FLAG2_USED_IN_A_TRADESKILL);
    IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
    for (Item* item : visitor.GetResult()) {
        std::stringstream falgsss;
        falgsss << std::hex << item->GetTemplate()->Flags;
        std::string flags = falgsss.str();

        std::stringstream falgsss2;
        falgsss2 << std::hex << item->GetTemplate()->Flags;
        std::string flags2 = falgsss2.str();

        bot->Whisper(
          "Found: "
              + item->GetTemplate()->Name1
              + " ("
              + std::to_string(item->GetTemplate()->ItemId)
              + ", Flags: "
                + flags
//              + std::to_string(item->GetTemplate()->Flags)
              + ", Flags2: "
                + flags2
//              + std::to_string(item->GetTemplate()->Flags2)
              + ")",
          LANG_UNIVERSAL,
          receiver
          );
    }

//    std::ostringstream body;
//    body << "Hello, " << receiver->GetName() << "," << std::endl;
//    body << std::endl;
//    body << "Here are the mats you asked for" << std::endl;
//    body << "Thanks," << std::endl;
//    body << bot->GetName() << std::endl;
//
//    MailDraft draft("Mats you asked for", body.str());

    return true;
}