#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>


class CarromGame;

class EventHandler {
public:
    EventHandler(CarromGame& game, sf::Sprite& striker);
    void handleEvents(sf::RenderWindow& window);
    bool isStrikerDragging() const { return strikerDragging; }
    bool isStrikerLocked() const { return strikerLocked; }
    float getCurrentDragDistance() const { return currentDragDistance; }
    const sf::RectangleShape& getPowerIndicator() const { return powerIndicator; }
    bool isStrikerReleased() const { return strikerReleased; }
    float getStrikerAngle() const { return strikerAngle; }
    float getStrikerPower() const { return strikerPower; }
    void resetStrikerRelease() { strikerReleased = false; }
    const sf::VertexArray& getAimingLine() const { return aimingLine; }
    // bool MouseMoved(sf::Event::MouseMoveEvent& event);
    const sf::Vector2f& getNewStrikerPosition() const { return newStrikerPosition; }
    void resetNewStrikerPosition() { newStrikerPosition = sf::Vector2f(-1, -1); }

private:
    CarromGame& game;
    sf::Sprite& strikerSprite;
    bool strikerDragging;
    bool strikerLocked;
    sf::Vector2f dragStart;
    const float MIN_X = 250.0f;
    const float MAX_X = 850.0f;
    const float STRIKER_Y_PLAYER1 = 784.0f;
    const float STRIKER_Y_PLAYER2 = 222.0f;

    void MousePressed(sf::Event::MouseButtonEvent& event);
    void MouseReleased(sf::Event::MouseButtonEvent& event);
    void MouseMoved(sf::Event::MouseMoveEvent& event);

    sf::Vector2f newStrikerPosition;


    float maxDragDistance;
    float currentDragDistance;
    sf::RectangleShape powerIndicator;
    bool dragStarted;

    float dragAngle;

    bool strikerReleased;
    float strikerAngle;
    float strikerPower;

    void updatePowerIndicator();

    void updateAimingLine();

    sf::VertexArray aimingLine;
};