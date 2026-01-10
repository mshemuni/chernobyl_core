#include <iostream>
#include <fstream>

#include "update_manager.h"

int main() {
    std::ifstream f("updates.json");
    nlohmann::json j;
    f >> j;

    UpdateManager mgr = UpdateManager::from_json(j);

    for (auto* u : mgr.available_updates()) {
        std::cout << "Available: " << u->name << "\n";
    }

    return 0;
}
