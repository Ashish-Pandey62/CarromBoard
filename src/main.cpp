#include "loading_assets.hpp"
#include <iostream>

int main() {
    try {
        CarromGame game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}