#include "eventsHandling.hpp"
#include "loading_assets.hpp" 
#include <algorithm>
#include <math.h>

EventHandler::EventHandler(CarromGame& game, sf::Sprite& striker)
    : game(game), strikerSprite(striker), strikerDragging(false), strikerLocked(false),
      maxDragDistance(145.0f), currentDragDistance(0.0f), dragStarted(false), strikerReleased(false) {
    powerIndicator.setSize(sf::Vector2f(20, 0));
    powerIndicator.setFillColor(sf::Color::Green);
    aimingLine.setPrimitiveType(sf::LineStrip);
}
void EventHandler::updateAimingLine() {
    aimingLine.clear();
    if (strikerLocked && currentDragDistance > 0) {
        sf::Vector2f strikerPos = strikerSprite.getPosition();
        float lineLength = 400.0f;
        int numPoints = 20;
        float spacing = lineLength / numPoints;

        for (int i = 0; i <= numPoints; ++i) {
            float distance = i * spacing;
            float alpha = 1.0f - (distance / lineLength);
            sf::Vertex point(
                sf::Vector2f(
                    strikerPos.x - std::sin(dragAngle) * distance,
                    strikerPos.y +std::cos(dragAngle) * distance
                ),
                sf::Color(255, 0, 0, static_cast<sf::Uint8>(255 * alpha))
            );
            aimingLine.append(point);
        }
    }
}



void EventHandler::handleEvents(sf::RenderWindow& window) {
    sf::Event event;
    while (window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::MouseButtonPressed:
                MousePressed(event.mouseButton);
                break;
            case sf::Event::MouseButtonReleased:
                MouseReleased(event.mouseButton);
                break;
            case sf::Event::MouseMoved:
                MouseMoved(event.mouseMove);
                break;
            default:
                break;
        }
    }
}


void EventHandler::MousePressed(sf::Event::MouseButtonEvent& event) {
    sf::Vector2f clickPosition(event.x, event.y);
    sf::Vector2f strikerPosition = game.getCurrentStrikerPosition();
    
    if (event.button == sf::Mouse::Left) {
        if (strikerLocked && strikerSprite.getGlobalBounds().contains(clickPosition)) {
            dragStarted = true;
            dragStart = clickPosition;
            currentDragDistance = 0.0f;
            updatePowerIndicator();
        } else if (!strikerLocked && strikerSprite.getGlobalBounds().contains(clickPosition)) {
            strikerDragging = true;
            dragStart = clickPosition;
        }
    }
    else if (event.button == sf::Mouse::Right) {
        if (strikerSprite.getGlobalBounds().contains(clickPosition)) {
            strikerLocked = !strikerLocked;
            if (strikerLocked) {
                strikerDragging = false;
            }
            dragStarted = false;
            currentDragDistance = 0.0f;
            updatePowerIndicator();
        }
    }
}




void EventHandler::MouseReleased(sf::Event::MouseButtonEvent& event) {
    if (event.button == sf::Mouse::Left) {
        if (strikerLocked && dragStarted) {
            strikerReleased = true;
            strikerAngle = dragAngle;
            strikerPower = currentDragDistance / maxDragDistance;
        }
        strikerDragging = false;
        dragStarted = false;
        currentDragDistance = 0.0f;
        updatePowerIndicator();
        updateAimingLine();
    }
}

void EventHandler::MouseMoved(sf::Event::MouseMoveEvent& event) {
    if (strikerLocked && dragStarted) {
        sf::Vector2f currentPos(event.x, event.y);
        sf::Vector2f dragVector = currentPos - dragStart;
        float dragDistance = std::sqrt(dragVector.x * dragVector.x + dragVector.y * dragVector.y);
        if (dragDistance > 0) {
            currentDragDistance = std::min(dragDistance, maxDragDistance);
            dragAngle = std::atan2(dragVector.x,-dragVector.y);
            updatePowerIndicator();
            updateAimingLine();  
        } else {
            currentDragDistance = 0;
            dragAngle = 0;
            updatePowerIndicator();
            updateAimingLine();
        }
    }
    
    else if (strikerDragging && !strikerLocked) {
        sf::Vector2f currentPos(event.x, event.y);
        sf::Vector2f diff = currentPos - dragStart;
        
        float newX = std::clamp(strikerSprite.getPosition().x + diff.x, 
                                MIN_X, MAX_X);
        float newY = (game.getCurrentPlayer() == 1) ? STRIKER_Y_PLAYER1 : STRIKER_Y_PLAYER2;
        strikerSprite.setPosition(newX, newY);
        
        dragStart = sf::Vector2f(event.x, event.y);
    }
}


void EventHandler::updatePowerIndicator() {
    float height = currentDragDistance;
    sf::Vector2f strikerPos = strikerSprite.getPosition();

    powerIndicator.setSize(sf::Vector2f(20, height));
    powerIndicator.setOrigin(10, height);
    powerIndicator.setPosition(strikerPos);
    powerIndicator.setRotation(dragAngle * 180 / M_PI);

    float ratio = currentDragDistance / maxDragDistance;
    sf::Color color = sf::Color(
        static_cast<sf::Uint8>(255 * ratio),
        static_cast<sf::Uint8>(255 * (1 - ratio)),
        0
    );
    powerIndicator.setFillColor(color);
}