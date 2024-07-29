#pragma once
#include <SFML/Graphics.hpp>
#include "eventsHandling.hpp"
#include <vector>
#include <box2d/box2d.h>
#include <queue>

class EventHandler;


class CarromGame {
private:
    int currentPlayer;
    sf::Vector2f player1StrikerPosition;
    sf::Vector2f player2StrikerPosition; 
    bool strikerShot;
    bool coinPocketed;

    sf::RenderWindow window;
    sf::Sprite bgSprite;
    sf::Sprite boardSprite;
    sf::Sprite strikerSprite;
    sf::Sprite queenSprite;
    std::vector<sf::Sprite> blackCoins;
    std::vector<sf::Sprite> whiteCoins;
    std::vector<sf::CircleShape> pockets;

    sf::Sprite player1Sprite;
    sf::Sprite player2Sprite;
    sf::Sprite quitSprite;
    sf::Texture player1Texture;
    sf::Texture player2Texture;
    sf::Texture quitTexture;

    static const constexpr float PLAYER_SPRITE_WIDTH = 200.0f;
    static const constexpr float PLAYER_SPRITE_HEIGHT = 200.0f;
    static const sf::Vector2f PLAYER1_POSITION;
    static const sf::Vector2f PLAYER2_POSITION;
    static const sf::Vector2f QUIT_POSITION;

    const float COIN_DIAMETER = 35.0f;
    const float STRIKER_DIAMETER = 46.0f;
    const float QUEEN_DIAMETER = 35.0f;
    const float POCKET_DIAMETER = 50.0f;
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
    // void resetStrikerPosition();
    bool areAllBodiesAtRest() const;
    void handlePocketedStriker();
    void placeLastPocketedCoinInCenter();
    bool isSpaceAvailableInCenter() const;
    void moveCentralCoinToSide();

        // New members for pocket system
    std::queue<b2Body*> pocketedCoins;
    bool strikerPocketed;
    sf::Vector2f initialStrikerPosition;      //YO PAXI UNCOMMENT GARNA PARNA SAKNE NI HUNA SAKXA HAII....

public:
    CarromGame();
    ~CarromGame();
    void switchTurn();
    void resetStrikerPosition();
    void handleTurn();
    sf::Vector2f getCurrentStrikerPosition() const;
    int getCurrentPlayer() const { return currentPlayer; } 
    void run();
    void setFrictionCoefficient(float friction) { frictionCoefficient = friction; }
    void setRestitutionCoefficient(float restitution) { restitutionCoefficient = restitution; }
};