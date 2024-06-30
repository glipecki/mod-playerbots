#include "SendMatsAction.h"
#include "Mail.h"
#include "Event.h"
#include "ItemVisitors.h"
#include "ChatHelper.h"
#include "Playerbots.h"

class GetAllItemsVisitor : public FindItemVisitor
{
public:
    GetAllItemsVisitor() { }

    bool Accept(ItemTemplate const* itemTemplate) override {
        return true;
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

    GetAllItemsVisitor visitor;
    IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
    for (Item* item : visitor.GetResult()) {
          if (this->IsItemUsefulForSkill(item->GetTemplate())) {
              std::stringstream message;
              message << "Found: " << item->GetTemplate()->Name1 << " x" << std::to_string(item->GetCount()) << " (" << std::to_string(item->GetTemplate()->ItemId) << ")";
//              message << "Class: " << std::to_string(item->GetTemplate()->Class) << ", ";
//              message << "SubClass: " << std::to_string(item->GetTemplate()->SubClass) << ")";
//              message << "ITEM_CLASS_REAGENT? " << ((item->GetTemplate()->Class & ITEM_CLASS_REAGENT) > 0) << ", ";
              //        message << "ITEM_CLASS_TRADE_GOODS: " << (item->GetTemplate()->Class & ITEM_CLASS_TRADE_GOODS) << ", ";
              //        message << "ITEM_CLASS_RECIPE: " << (item->GetTemplate()->Class & ITEM_CLASS_RECIPE) << "";
              bot->Whisper(
                message.str(),
                LANG_UNIVERSAL,
                receiver
              );
          }
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

bool SendMatsAction::IsItemUsefulForSkill(ItemTemplate const* proto)
{
    switch (proto->Class)
    {
        case ITEM_CLASS_TRADE_GOODS:
        case ITEM_CLASS_MISC:
        case ITEM_CLASS_REAGENT:
        case ITEM_CLASS_GEM:
        {
            return true;
        }
        case ITEM_CLASS_RECIPE:
        {
            switch (proto->SubClass)
            {
                case ITEM_SUBCLASS_LEATHERWORKING_PATTERN:
                case ITEM_SUBCLASS_TAILORING_PATTERN:
                case ITEM_SUBCLASS_ENGINEERING_SCHEMATIC:
                case ITEM_SUBCLASS_BLACKSMITHING:
                case ITEM_SUBCLASS_COOKING_RECIPE:
                case ITEM_SUBCLASS_ALCHEMY_RECIPE:
                case ITEM_SUBCLASS_FIRST_AID_MANUAL:
                case ITEM_SUBCLASS_ENCHANTING_FORMULA:
                case ITEM_SUBCLASS_FISHING_MANUAL:
                    return true;
            }
        }
    }
    return false;
}