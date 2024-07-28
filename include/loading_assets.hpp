#pragma once
#include <SFML/Graphics.hpp>
#include "eventsHandling.hpp"
#include <vector>
#include <box2d/box2d.h>
#include <queue>

class CarromGame {
private:
    sf::RenderWindow window;
    sf::Sprite bgSprite;
    sf::Sprite boardSprite;
    sf::Sprite strikerSprite;
    sf::Sprite queenSprite;
    std::vector<sf::Sprite> blackCoins;
    std::vector<sf::Sprite> whiteCoins;
    std::vector<sf::CircleShape> pockets;

    const float COIN_DIAMETER = 35.0f;
    const float STRIKER_DIAMETER = 46.0f;
    const float QUEEN_DIAMETER = 35.0f;
    const float POCKET_DIAMETER = 50.0f;

    sf::Texture bgTexture;
    sf::Texture boardTexture;
    sf::Texture strikerTexture;
    sf::Texture queenTexture;
    sf::Texture blackCoinTexture;
    sf::Texture whiteCoinTexture;

    void loadTextures();
    void setupSprites();
    void setupPockets();

    EventHandler eventHandler;

    b2World* world;
    b2Body* strikerBody;
    b2Body* queenBody;
    std::vector<b2Body*> coinBodies;

    void setupPhysics();
    void updatePhysics();
    void applyStrikerForce(float angle, float power);

    float frictionCoefficient;
    float restitutionCoefficient;

    void handleCollisions();
    b2Vec2 remainingForce;
    int frameCounter;
    std::vector<b2Vec2> previousPositions;

    void interpolatePositions(float alpha);
    void checkPocketCollisions();
    void checkAllBodiesAtRest();
    void resetStrikerPosition();
    bool areAllBodiesAtRest() const;
    void handlePocketedStriker();
    void placeLastPocketedCoinInCenter();
    bool isSpaceAvailableInCenter() const;
    void moveCentralCoinToSide();

        // New members for pocket system
    std::queue<b2Body*> pocketedCoins;
    bool strikerPocketed;
    sf::Vector2f initialStrikerPosition;

public:
    CarromGame();
    ~CarromGame();
    void run();
    void setFrictionCoefficient(float friction) { frictionCoefficient = friction; }
    void setRestitutionCoefficient(float restitution) { restitutionCoefficient = restitution; }
};