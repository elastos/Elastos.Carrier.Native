#include <iostream>
#include <string>

#include "command.h"

class StorageListValueCommand : public StorageCommand {
public:
    StorageListValueCommand() : StorageCommand("listvalue", "List values from the local storage.") {};

protected:
    void setupOptions() override {
        auto app = getApp();
    };

    void execute() override {
        auto storage = node->getStorage();
        auto value = storage->getValue(valueid);
        if (value)
            std::cout << *value << std::endl;
        else
            std::cout << "Value " << valueid << " not exists.";
    };

private:
    std::string id {};
};
