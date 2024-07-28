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


private:
    CarromGame& game;
    sf::Sprite& strikerSprite;
    bool strikerDragging;
    bool strikerLocked;
    sf::Vector2f dragStart;
    const float MIN_X = 238.0f;
    const float MAX_X = 768.0f;
    const float STRIKER_Y_PLAYER1 = 784.0f;
    const float STRIKER_Y_PLAYER2 = 200.0f;

    void MousePressed(sf::Event::MouseButtonEvent& event);
    void MouseReleased(sf::Event::MouseButtonEvent& event);
    void MouseMoved(sf::Event::MouseMoveEvent& event);

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