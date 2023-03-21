#include <iostream>
#include <string>

#include "command.h"

class ListPeerCommand : public Command {
public:
    ListPeerCommand() : Command("listpeer", "List peers from the local storage.") {};

protected:
    void execute() override {
        auto storage = node->getStorage();
        auto peerIds = storage->listPeerId();
        if (!peerIds.empty()) {
            for (auto& id : peerIds)
                std::cout << static_cast<std::string>(id) << std::endl;

            std::cout << "Total " << peerIds.size() << " peers." << std::endl;
        } else {
            std::cout << "No peer exists." << std::endl;
        }
    };
};
