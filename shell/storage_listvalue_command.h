#include <iostream>
#include <string>

#include "command.h"

class ListValueCommand : public Command {
public:
    ListValueCommand() : Command("listvalue", "List values from the local storage.") {};

protected:
    void execute() override {
        auto storage = node->getStorage();
        auto valueIds = storage->listValueId();
        if (!valueIds.empty()) {
            for (auto& id : valueIds)
                std::cout << static_cast<std::string>(id) << std::endl;

            std::cout << "Total " << valueIds.size() << " values." << std::endl;
        } else {
            std::cout << "No Value exists." << std::endl;
        }
    };
};
