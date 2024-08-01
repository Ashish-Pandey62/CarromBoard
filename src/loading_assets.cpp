#include "loading_assets.hpp"
#include <stdexcept>
#include <algorithm>
#include <ctime>
#include <cmath>
#include<iostream>
using namespace std;
CarromGame::CarromGame() 
    : window(sf::VideoMode(1000, 1000), "Carrom Game"),
      eventHandler(*this, strikerSprite),
      frictionCoefficient(5.0f),
      restitutionCoefficient(0.2f),
      strikerPocketed(false),
      player1StrikerPosition(500, 782),
      player2StrikerPosition(500, 222),
      currentPlayer(1),
      strikerShot(false),
      coinPocketed(false),
      player1Score(0),
      player2Score(0),
      queenPocketed(false), 
      queenPocketedBy(0), 
      coinPocketedAfterQueen(false),
      gameOver(false)
      
{
    std::srand(std::time(nullptr));
    loadTextures();
    setupSprites();
    setupBackgroundSprite();
    setupPhysics();
    setupPockets();
    resetStrikerPosition();


    if (!scoreFont.loadFromFile("assets/EnterCommand.ttf")) {
        throw std::runtime_error("Failed to load font");
    }

    player1ScoreText.setFont(scoreFont);
    player1ScoreText.setCharacterSize(60);
    player1ScoreText.setFillColor(sf::Color::Red);
    player1ScoreText.setPosition(300, 5);

    player2ScoreText.setFont(scoreFont);
    player2ScoreText.setCharacterSize(60);
    player2ScoreText.setFillColor(sf::Color::Red);
    player2ScoreText.setPosition(855, 5);

    updateScoreDisplay();
}

CarromGame::~CarromGame() {
    delete world;
}

const sf::Vector2f CarromGame::PLAYER1_POSITION(200.0f, 100.0f);
const sf::Vector2f CarromGame::PLAYER2_POSITION(755.0f, 100.0f);
const sf::Vector2f CarromGame::QUIT_POSITION(475.0f, 1000.0f);

void CarromGame::loadTextures() {
    if (!boardTexture.loadFromFile("assets/board01.png") ||
        !strikerTexture.loadFromFile("assets/striker1.png") ||
        !queenTexture.loadFromFile("assets/queen.png") ||
        !blackCoinTexture.loadFromFile("assets/black.png") ||
        !whiteCoinTexture.loadFromFile("assets/white.png") ||
        !player1Texture.loadFromFile("assets/kheladi11.png") ||
        !player2Texture.loadFromFile("assets/kheladi22.png") ||
        !quitTexture.loadFromFile("assets/quit11.png") ||
        !backgroundTexture.loadFromFile("assets/bg3.png") ||
        !player1WinsTexture.loadFromFile("assets/WINNER1.png") ||
        !player2WinsTexture.loadFromFile("assets/WINNER2.png")) {
        throw std::runtime_error("Failed to load textures");
    }
}


void CarromGame::setupBackgroundSprite() {
    backgroundSprite.setTexture(backgroundTexture);
    backgroundSprite.setScale(
        window.getSize().x / static_cast<float>(backgroundTexture.getSize().x),
        window.getSize().y / static_cast<float>(backgroundTexture.getSize().y)
    );
}

bool CarromGame::isGameOver() const {
    return blackCoins.empty() && whiteCoins.empty() && !queenBody;
}


void CarromGame::handleQuitButtonClick() {
    cout << "aayo yeta tiraaaaa"<< endl;
    gameOver = true;
    if (currentPlayer == 1) {
        cout << "player 1 raixa"<<endl;
        winnerSprite.setTexture(player2WinsTexture);
    } else {
        cout << "player 2 raxa"<<endl;
        winnerSprite.setTexture(player1WinsTexture);
    }
    winnerSprite.setPosition(
        (window.getSize().x - winnerSprite.getGlobalBounds().width) / 2,
        (window.getSize().y - winnerSprite.getGlobalBounds().height) / 2
    );
}



void CarromGame::setupSprites() {

    // Set up player1 sprite
    player1Sprite.setTexture(player1Texture);
    player1Sprite.setOrigin(PLAYER_SPRITE_WIDTH / 2, PLAYER_SPRITE_HEIGHT / 2);
    player1Sprite.setPosition(PLAYER1_POSITION);
    player1Sprite.setScale(
        PLAYER_SPRITE_WIDTH / player1Texture.getSize().x,
        PLAYER_SPRITE_HEIGHT / player1Texture.getSize().y
    );

    // Set up player2 sprite
    player2Sprite.setTexture(player2Texture);
    player2Sprite.setOrigin(PLAYER_SPRITE_WIDTH / 2, PLAYER_SPRITE_HEIGHT / 2);
    player2Sprite.setPosition(PLAYER2_POSITION);
    player2Sprite.setScale(
        PLAYER_SPRITE_WIDTH / player2Texture.getSize().x,
        PLAYER_SPRITE_HEIGHT / player2Texture.getSize().y
    );

    // Set up quit sprite
    quitSprite.setTexture(quitTexture);
    quitSprite.setOrigin(PLAYER_SPRITE_WIDTH / 2, PLAYER_SPRITE_HEIGHT / 2);
    quitSprite.setPosition(QUIT_POSITION);
    quitSprite.setScale(
        PLAYER_SPRITE_WIDTH / quitTexture.getSize().x,
        PLAYER_SPRITE_HEIGHT / quitTexture.getSize().y
    );

    // board and other things setting upp
    boardSprite.setTexture(boardTexture);
    strikerSprite.setTexture(strikerTexture);
    queenSprite.setTexture(queenTexture);

    boardSprite.setScale(800.0f / boardTexture.getSize().x, 800.0f / boardTexture.getSize().y);
    boardSprite.setPosition(100, 100); // this board image is acting as a background haii

    //  we are centering the origin of each sprite to their center (initially it is set to top left corner)
    strikerSprite.setOrigin(strikerTexture.getSize().x / 2.0f, strikerTexture.getSize().y / 2.0f);
    queenSprite.setOrigin(queenTexture.getSize().x / 2.0f, queenTexture.getSize().y / 2.0f);

    // Set scale for striker and queen
    strikerSprite.setScale(
        STRIKER_DIAMETER / strikerTexture.getSize().x,
        STRIKER_DIAMETER / strikerTexture.getSize().y
    );
    queenSprite.setScale(
        QUEEN_DIAMETER / queenTexture.getSize().x,
        QUEEN_DIAMETER / queenTexture.getSize().y
    );

    // initial position fo striker and queen 
    strikerSprite.setPosition(500, 784);
    


    float centerX = 500; 
    float centerY = 500;
    queenSprite.setPosition(centerX, centerY); // queen remains in the center of board

    float coinSpacing = COIN_DIAMETER * 0.9; 

    std::vector<std::pair<int, int>> positions = {
        {0, -2}, {1, -1}, {-1, -1}, {2, 0}, {-2, 0}, {1, 1}, {-1, 1}, {0, 2},
        {0, -3}, {2, -2}, {-2, -2}, {3, -1}, {-3, -1}, {3, 1}, {-3, 1}, {2, 2}, {-2, 2}, {0, 3}  
    };

    // randomly generate positions for coin spacing
    std::random_shuffle(positions.begin(), positions.end());

    std::vector<sf::Sprite> allCoins;
    for (int i = 0; i < positions.size(); ++i) {
        sf::Sprite coin;
        
        // Alternating black and white coins
        if (i % 2 == 0) {
            coin.setTexture(blackCoinTexture);
        } else {
            coin.setTexture(whiteCoinTexture);
        }

        float x = centerX + positions[i].first * coinSpacing;
        float y = centerY + positions[i].second * coinSpacing;

        coin.setOrigin(coin.getTexture()->getSize().x / 2.0f, coin.getTexture()->getSize().y / 2.0f);
        coin.setScale(
            COIN_DIAMETER / coin.getTexture()->getSize().x,
            COIN_DIAMETER / coin.getTexture()->getSize().y
        );
        coin.setPosition(x, y); 

        allCoins.push_back(coin);
    }

    // Separate coins into black and white vectors
    for (const auto& coin : allCoins) {
        if (coin.getTexture() == &blackCoinTexture) {
            blackCoins.push_back(coin);
        } else {
            whiteCoins.push_back(coin);
        }
    }
}
void CarromGame::updateScoreDisplay() {
    player1ScoreText.setString(std::to_string(player1Score));
    player2ScoreText.setString(std::to_string(player2Score));
}

void CarromGame::updateStrikerPosition(const b2Vec2& newPosition) {
    if (strikerBody) {
        strikerBody->SetTransform(newPosition, strikerBody->GetAngle());
        strikerBody->SetLinearVelocity(b2Vec2(0, 0));
    }
}

void CarromGame::updateStrikerPositionFromEventHandler() {
    sf::Vector2f newPosition = eventHandler.getNewStrikerPosition();
    if (newPosition.x != -1 && newPosition.y != -1) {
        updateStrikerPosition(b2Vec2(newPosition.x / 30.0f, newPosition.y / 30.0f));
        eventHandler.resetNewStrikerPosition();
    }
}

void CarromGame::checkAllBodiesAtRest() {
    if (areAllBodiesAtRest()) {
        resetStrikerPosition();
    }
}


bool CarromGame::areAllBodiesAtRest() const {
    const float restThreshold = 2.0f; 

    if (strikerBody->GetLinearVelocity().LengthSquared() > restThreshold) {
        return false;
    }

    for (const auto& coinBody : coinBodies) {
        if (coinBody->GetLinearVelocity().LengthSquared() > restThreshold) {
            return false;
        }
    }

    return true;
}


void CarromGame::switchTurn() {
    currentPlayer = (currentPlayer == 1) ? 2 : 1;
    resetStrikerPosition();
    strikerShot = false;
    coinPocketed = false;
    
    // Reset queen-related flags if the queen wasn't successfully pocketed
    if (queenPocketed && !coinPocketedAfterQueen) {
        queenPocketed = false;
        queenPocketedBy = 0;
    }
}

void CarromGame::resetStrikerPosition() {
    sf::Vector2f newPosition = getCurrentStrikerPosition();
    if (strikerBody) {
        strikerBody->SetTransform(b2Vec2(newPosition.x / 30.0f, newPosition.y / 30.0f), 0);
        strikerBody->SetLinearVelocity(b2Vec2(0, 0));
    }
    strikerSprite.setPosition(newPosition);
}

sf::Vector2f CarromGame::getCurrentStrikerPosition() const {
    return (currentPlayer == 1) ? player1StrikerPosition : player2StrikerPosition;
}



void CarromGame::handleTurn() {
    if (strikerShot && areAllBodiesAtRest()) {
        if (queenPocketed) {
            if (coinPocketedAfterQueen) {
                // Player successfully pocketed a coin after the queen
                if (queenPocketedBy == 1) {
                    player1Score += 25; // Queen's score
                    player1Score += 5;  // Assume 5 points for the additional coin
                } else {
                    player2Score += 25; // Queen's score
                    player2Score += 5;  // Assume 5 points for the additional coin
                }
                queenPocketed = false;
                queenPocketedBy = 0;
                coinPocketedAfterQueen = false;
                // Don't switch turn, player gets another chance
                resetStrikerPosition();
            } else {
                // Player failed to pocket a coin after the queen
                returnQueen();
                switchTurn();
            }
        } else if (!coinPocketed || strikerPocketed) {
            switchTurn();
        } else {
            // Player pocketed a coin, they get another turn
            resetStrikerPosition();
        }
        
        strikerShot = false;
        coinPocketed = false;
        strikerPocketed = false;
    }
}


void CarromGame::returnQueen() {
    if (!queenBody) {
        b2BodyDef queenDef;
        queenDef.type = b2_dynamicBody;
        queenDef.position.Set(500 / 30.0f, 500 / 30.0f);
        queenBody = world->CreateBody(&queenDef);

        b2CircleShape queenShape;
        queenShape.m_radius = QUEEN_DIAMETER / 2.0f / 30.0f;

        b2FixtureDef queenFixtureDef;
        queenFixtureDef.shape = &queenShape;
        queenFixtureDef.density = 1.0f;
        queenFixtureDef.friction = frictionCoefficient;
        queenFixtureDef.restitution = restitutionCoefficient;

        queenBody->CreateFixture(&queenFixtureDef);
        queenBody->SetLinearDamping(0.3f);
    } else {
        queenBody->SetTransform(b2Vec2(500 / 30.0f, 500 / 30.0f), 0);
        queenBody->SetLinearVelocity(b2Vec2(0, 0));
    }
    
    queenSprite.setPosition(500, 500);
    queenPocketed = false;
    queenPocketedBy = 0;
}

void CarromGame::setupPockets() {
    std::vector<sf::Vector2f> pocketPositions = {
        {154, 160}, {841, 158}, {153, 846}, {843, 845}
    };

    for (const auto& pos : pocketPositions) {
        sf::CircleShape pocket(POCKET_DIAMETER / 2);
        pocket.setFillColor(sf::Color::Black);
        pocket.setPosition(pos.x - POCKET_DIAMETER / 2, pos.y - POCKET_DIAMETER / 2);
        pockets.push_back(pocket);
    }
}

void CarromGame::setupPhysics() {
    b2Vec2 gravity(0.0f, 0.0f);
    world = new b2World(gravity);

    // Creating board boundaries
    b2BodyDef boardDef;
    boardDef.position.Set(500.0f / 30.0f, 500.0f / 30.0f);
    b2Body* boardBody = world->CreateBody(&boardDef);

    b2EdgeShape boardEdge;
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &boardEdge;
    fixtureDef.restitution = restitutionCoefficient;

    // Top edge defn
    boardEdge.SetTwoSided(b2Vec2(-375.0f / 30.0f, -375.0f / 30.0f), b2Vec2(375.0f / 30.0f, -375.0f / 30.0f));
    boardBody->CreateFixture(&fixtureDef);

    // Bottom edge defn
    boardEdge.SetTwoSided(b2Vec2(-380.0f / 30.0f, 380.0f / 30.0f), b2Vec2(380.0f / 30.0f, 380.0f / 30.0f));
    boardBody->CreateFixture(&fixtureDef);

    // Left edge defn
    boardEdge.SetTwoSided(b2Vec2(-380.0f / 30.0f, -380.0f / 30.0f), b2Vec2(-380.0f / 30.0f, 380.0f / 30.0f));
    boardBody->CreateFixture(&fixtureDef);

    // Right edge defn
    boardEdge.SetTwoSided(b2Vec2(380.0f / 30.0f, -380.0f / 30.0f), b2Vec2(380.0f / 30.0f, 380.0f / 30.0f));
    boardBody->CreateFixture(&fixtureDef);

    // Creating striker
    b2BodyDef strikerDef;
    strikerDef.type = b2_dynamicBody;
    strikerDef.position.Set(strikerSprite.getPosition().x / 30.0f, strikerSprite.getPosition().y / 30.0f);
    strikerBody = world->CreateBody(&strikerDef);

    b2CircleShape strikerShape;
    strikerShape.m_radius = STRIKER_DIAMETER / 2.0f / 30.0f;

    b2FixtureDef strikerFixtureDef;
    strikerFixtureDef.shape = &strikerShape;
    strikerFixtureDef.density = 1.0f;
    strikerFixtureDef.friction = frictionCoefficient;
    strikerFixtureDef.restitution = restitutionCoefficient;

    strikerBody->CreateFixture(&strikerFixtureDef);

    // Creating coins
    b2CircleShape coinShape;
    coinShape.m_radius = COIN_DIAMETER / 2.0f / 30.0f;

    b2FixtureDef coinFixtureDef;
    coinFixtureDef.shape = &coinShape;
    coinFixtureDef.density = 1.0f;
    coinFixtureDef.friction = frictionCoefficient;
    coinFixtureDef.restitution = restitutionCoefficient;

    for (const auto& coin : blackCoins) {
        b2BodyDef coinDef;
        coinDef.type = b2_dynamicBody;
        coinDef.position.Set(coin.getPosition().x / 30.0f, coin.getPosition().y / 30.0f);
        b2Body* coinBody = world->CreateBody(&coinDef);
        coinBody->CreateFixture(&coinFixtureDef);
        coinBodies.push_back(coinBody);
    }

    for (const auto& coin : whiteCoins) {
        b2BodyDef coinDef;
        coinDef.type = b2_dynamicBody;
        coinDef.position.Set(coin.getPosition().x / 30.0f, coin.getPosition().y / 30.0f);
        b2Body* coinBody = world->CreateBody(&coinDef);
        coinBody->CreateFixture(&coinFixtureDef);
        coinBodies.push_back(coinBody);
    }

    // Creating queen
    b2BodyDef queenDef;
    queenDef.type = b2_dynamicBody;
    queenDef.position.Set(queenSprite.getPosition().x / 30.0f, queenSprite.getPosition().y / 30.0f);
    queenBody = world->CreateBody(&queenDef);

    b2CircleShape queenShape;
    queenShape.m_radius = QUEEN_DIAMETER / 2.0f / 30.0f;

    b2FixtureDef queenFixtureDef;
    queenFixtureDef.shape = &queenShape;
    queenFixtureDef.density = 1.0f;
    queenFixtureDef.friction = frictionCoefficient;
    queenFixtureDef.restitution = restitutionCoefficient;

    queenBody->CreateFixture(&queenFixtureDef);
    queenBody->SetLinearDamping(0.3f);


    for (auto* coinBody : coinBodies) {
        coinBody->SetLinearDamping(0.3f);
    }

    previousPositions.resize(coinBodies.size() + 2); // +2 for striker and queen
    for (size_t i = 0; i < previousPositions.size(); ++i) {
        previousPositions[i] = b2Vec2(0, 0);
    }
    // Adding pocket 


    b2CircleShape pocketShape;
    pocketShape.m_radius = POCKET_DIAMETER / 2.0f / 30.0f;

    b2FixtureDef pocketFixtureDef;
    pocketFixtureDef.shape = &pocketShape;
    pocketFixtureDef.isSensor = true;

    for (const auto& pocket : pockets) {
        b2BodyDef pocketDef;
        pocketDef.position.Set(pocket.getPosition().x / 30.0f, pocket.getPosition().y / 30.0f);
        b2Body* pocketBody = world->CreateBody(&pocketDef);
        pocketBody->CreateFixture(&pocketFixtureDef);
    }
}

void CarromGame::handleQueenPocketed() {
    queenPocketed = true;
    queenPocketedBy = currentPlayer;
    coinPocketedAfterQueen = false;
    
    if (queenBody) {
        world->DestroyBody(queenBody);
        queenBody = nullptr;
    }
    queenSprite.setPosition(-100, -100);  // Move off-screen

    resetStrikerPosition();
    strikerShot = false;
}



void CarromGame::checkPocketCollisions() {
    // Check for queen pocketing
    if (queenBody) {
        b2Vec2 queenPosition = queenBody->GetPosition();
        for (const auto& pocket : pockets) {
            sf::Vector2f pocketCenter = pocket.getPosition() + sf::Vector2f(POCKET_DIAMETER / 2, POCKET_DIAMETER / 2);
            b2Vec2 pocketPosition(pocketCenter.x / 30.0f, pocketCenter.y / 30.0f);
            
            float distance = b2Distance(queenPosition, pocketPosition);
            if (distance < (POCKET_DIAMETER / 2 + QUEEN_DIAMETER / 2) / 30.0f) {
                handleQueenPocketed();
                break;
            }
        }
    }

    // Check for striker pocketing
    b2Vec2 strikerPosition = strikerBody->GetPosition();
    for (const auto& pocket : pockets) {
        sf::Vector2f pocketCenter = pocket.getPosition() + sf::Vector2f(POCKET_DIAMETER / 2, POCKET_DIAMETER / 2);
        b2Vec2 pocketPosition(pocketCenter.x / 30.0f, pocketCenter.y / 30.0f);
        
        float distance = b2Distance(strikerPosition, pocketPosition);
        if (distance < (POCKET_DIAMETER / 2 + STRIKER_DIAMETER / 2) / 30.0f) {
            handlePocketedStriker();
            return; // Exit the function as turn will switch
        }
    }

    // Check for coin pocketing
    for (auto it = coinBodies.begin(); it != coinBodies.end();) {
        b2Body* coinBody = *it;
        b2Vec2 coinPosition = coinBody->GetPosition();
        
        bool pocketed = false;
        for (const auto& pocket : pockets) {
            sf::Vector2f pocketCenter = pocket.getPosition() + sf::Vector2f(POCKET_DIAMETER / 2, POCKET_DIAMETER / 2);
            b2Vec2 pocketPosition(pocketCenter.x / 30.0f, pocketCenter.y / 30.0f);
            
            float distance = b2Distance(coinPosition, pocketPosition);
            if (distance < (POCKET_DIAMETER / 2 + COIN_DIAMETER / 2) / 30.0f) {
                pocketed = true;
                break;
            }
        }
        
        if (pocketed) {
            int points = getCoinValue(coinBody);
            if (currentPlayer == 1) {
                player1Score += points;
                player1PocketedCoins.push_back(coinBody);
            } else {
                player2Score += points;
                player2PocketedCoins.push_back(coinBody);
            }
            coinPocketed = true;
            
            if (queenPocketed && queenPocketedBy == currentPlayer) {
                coinPocketedAfterQueen = true;
            }
            
            // Remove the coin from the game
            for (auto& coin : blackCoins) {
                if (coin.getPosition() == sf::Vector2f(coinPosition.x * 30.0f, coinPosition.y * 30.0f)) {
                    coin.setPosition(-100, -100); // Move off-screen
                    break;
                }
            }
            for (auto& coin : whiteCoins) {
                if (coin.getPosition() == sf::Vector2f(coinPosition.x * 30.0f, coinPosition.y * 30.0f)) {
                    coin.setPosition(-100, -100); // Move off-screen
                    break;
                }
            }
            
            world->DestroyBody(coinBody);
            it = coinBodies.erase(it);
        } else {
            ++it;
        }
    }

    updateScoreDisplay();

    // Check if the game is over
    if (coinBodies.empty()) {
        gameOver = true;
        if (player1Score > player2Score) {
            winnerSprite.setTexture(player1WinsTexture);
        } else if (player2Score > player1Score) {
            winnerSprite.setTexture(player2WinsTexture);
        }
        winnerSprite.setPosition(
            (window.getSize().x - winnerSprite.getGlobalBounds().width) / 2,
            (window.getSize().y - winnerSprite.getGlobalBounds().height) / 2
        );
    }
}


void CarromGame::handlePocketedStriker() {
    strikerPocketed = true;

    // Deduct points and handle coin return
    if (currentPlayer == 1) {
        player1Score = std::max(0, player1Score - 5);
        if (!player1PocketedCoins.empty()) {
            placeLastPocketedCoinInCenter(player1PocketedCoins);
            player1Score = std::max(0, player1Score - getCoinValue(player1PocketedCoins.back()));
            player1PocketedCoins.pop_back();
        }
    } else {
        player2Score = std::max(0, player2Score - 5);
        if (!player2PocketedCoins.empty()) {
            placeLastPocketedCoinInCenter(player2PocketedCoins);
            player2Score = std::max(0, player2Score - getCoinValue(player2PocketedCoins.back()));
            player2PocketedCoins.pop_back();
        }
    }

    // Destroy the current striker body
    if (strikerBody) {
        world->DestroyBody(strikerBody);
        strikerBody = nullptr;
    }

    // Get the new position for the striker
    sf::Vector2f newPosition = getCurrentStrikerPosition();

    // Recreate the striker body
    b2BodyDef strikerDef;
    strikerDef.type = b2_dynamicBody;
    strikerDef.position.Set(newPosition.x / 30.0f, newPosition.y / 30.0f);
    strikerBody = world->CreateBody(&strikerDef);

    b2CircleShape strikerShape;
    strikerShape.m_radius = STRIKER_DIAMETER / 2.0f / 30.0f;

    b2FixtureDef strikerFixtureDef;
    strikerFixtureDef.shape = &strikerShape;
    strikerFixtureDef.density = 1.0f;
    strikerFixtureDef.friction = frictionCoefficient;
    strikerFixtureDef.restitution = restitutionCoefficient;

    strikerBody->CreateFixture(&strikerFixtureDef);

    // Update the striker sprite position
    strikerSprite.setPosition(newPosition);

    // Reset striker-related flags
    strikerShot = false;
    eventHandler.resetStrikerRelease();

    // Update the score display
    updateScoreDisplay();

    // Switch turn after handling the pocketed striker
    switchTurn();
}




int CarromGame::getCoinValue(b2Body* coinBody) {
    if (coinBody == queenBody) return 25;
    for (const auto& coin : blackCoins) {
        if (coin.getPosition() == sf::Vector2f(coinBody->GetPosition().x * 30.0f, coinBody->GetPosition().y * 30.0f)) {
            return 5;
        }
    }
    return 10; 
}

void CarromGame::placeLastPocketedCoinInCenter(std::vector<b2Body*>& pocketedCoins) {
    if (pocketedCoins.empty()) return;

    b2Body* lastPocketedCoin = pocketedCoins.back();
    pocketedCoins.pop_back();

    lastPocketedCoin->SetTransform(b2Vec2(500 / 30.0f, 500 / 30.0f), 0);
    lastPocketedCoin->SetLinearVelocity(b2Vec2(0, 0));
    coinBodies.push_back(lastPocketedCoin);

    // Update sprite position
    sf::Sprite* coinSprite = nullptr;
    for (auto& coin : blackCoins) {
        if (coin.getPosition() == sf::Vector2f(-100, -100)) {
            coinSprite = &coin;
            break;
        }
    }
    if (!coinSprite) {
        for (auto& coin : whiteCoins) {
            if (coin.getPosition() == sf::Vector2f(-100, -100)) {
                coinSprite = &coin;
                break;
            }
        }
    }
    if (coinSprite) {
        coinSprite->setPosition(500, 500);
    }
}

bool CarromGame::isSpaceAvailableInCenter() const {
    const float CENTER_X = 500.0f;
    const float CENTER_Y = 500.0f;
    const float CHECK_RADIUS = COIN_DIAMETER * 1.5f;

    for (const auto& coinBody : coinBodies) {
        b2Vec2 coinPos = coinBody->GetPosition();
        float distance = std::sqrt(std::pow(coinPos.x * 30.0f - CENTER_X, 2) + std::pow(coinPos.y * 30.0f - CENTER_Y, 2));
        if (distance < CHECK_RADIUS) {
            return false;
        }
    }
    return true;
}

void CarromGame::moveCentralCoinToSide() {
    const float CENTER_X = 500.0f;
    const float CENTER_Y = 500.0f;
    const float MOVE_DISTANCE = COIN_DIAMETER * 2;

    for (auto& coinBody : coinBodies) {
        b2Vec2 coinPos = coinBody->GetPosition();
        float distance = std::sqrt(std::pow(coinPos.x * 30.0f - CENTER_X, 2) + std::pow(coinPos.y * 30.0f - CENTER_Y, 2));
        if (distance < COIN_DIAMETER) {
            // Move the coin to the right
            coinBody->SetTransform(b2Vec2((CENTER_X + MOVE_DISTANCE) / 30.0f, CENTER_Y / 30.0f), 0);
            break;
        }
    }
}



void CarromGame::updatePhysics() {



    const float timeStep = 1.0f / 300.0f;  // Smaller time step
    const int velocityIterations = 8;
    const int positionIterations = 3;

    // Store current positions
    previousPositions[0] = strikerBody->GetPosition();
    if (queenBody) {
        previousPositions[1] = queenBody->GetPosition();
    }
    for (size_t i = 0; i < coinBodies.size(); ++i) {
        previousPositions[i + 2] = coinBodies[i]->GetPosition();
    }

    for (int i = 0; i < 5; ++i) {  // Perform multiple steps per frame
        world->Step(timeStep, velocityIterations, positionIterations);
    }

    // Update striker position
    if (strikerBody) {
        b2Vec2 position = strikerBody->GetPosition();
        strikerSprite.setPosition(position.x * 30.0f, position.y * 30.0f);
    }
    // Update queen position
    if (queenBody) {
        b2Vec2 queenPos = queenBody->GetPosition();
        queenSprite.setPosition(queenPos.x * 30.0f, queenPos.y * 30.0f);
    }

    // Update coin positions
    auto coinBody = coinBodies.begin();
    for (auto& coin : blackCoins) {
        if (coinBody != coinBodies.end()) {
            b2Vec2 pos = (*coinBody)->GetPosition();
            coin.setPosition(pos.x * 30.0f, pos.y * 30.0f);
            ++coinBody;
        }
    }
    for (auto& coin : whiteCoins) {
        if (coinBody != coinBodies.end()) {
            b2Vec2 pos = (*coinBody)->GetPosition();
            coin.setPosition(pos.x * 30.0f, pos.y * 30.0f);
            ++coinBody;
        }
    }

    if (frameCounter > 0) {
        strikerBody->ApplyLinearImpulseToCenter(remainingForce, true);
        frameCounter--;
    }

    checkAllBodiesAtRest();
    handleCollisions();
    checkPocketCollisions();
}

void CarromGame::applyStrikerForce(float angle, float power) {
    float forceX = std::cos(angle) * power * 20.0f;
    float forceY = std::sin(angle) * power * 20.0f;
    strikerBody->ApplyLinearImpulseToCenter(b2Vec2(-forceY,forceX), true);

    // Storing the remaining force to apply over next few frames
    remainingForce = b2Vec2(-forceY*2.5f, forceX*2.5f) ; 
    frameCounter = 2;

}

void CarromGame::handleCollisions() {
    // Check for collisions between coins and board boundaries
    for (auto* coinBody : coinBodies) {
        b2Vec2 position = coinBody->GetPosition();
        b2Vec2 velocity = coinBody->GetLinearVelocity();

        
        float boardLeft = 50.0f / 30.0f;
        float boardRight = 950.0f / 30.0f;
        float boardTop = 50.0f / 30.0f;
        float boardBottom = 950.0f / 30.0f;

        // Check left and right boundaries
        if (position.x < boardLeft || position.x > boardRight) {
            velocity.x = -velocity.x * restitutionCoefficient;
            coinBody->SetLinearVelocity(velocity);
        }

        // Check top and bottom boundaries
        if (position.y < boardTop || position.y > boardBottom) {
            velocity.y = -velocity.y * restitutionCoefficient;
            coinBody->SetLinearVelocity(velocity);
        }
    }

    // Apply friction to all moving bodies
    for (b2Body* body = world->GetBodyList(); body; body = body->GetNext()) {
                if (body->GetType() == b2_dynamicBody) {
            b2Vec2 velocity = body->GetLinearVelocity();
            float speed = velocity.Length();
            if (speed > 0.1f) {  // Only applying friction if speed is above a threshold
                float frictionForce = frictionCoefficient * body->GetMass() * 9.81f;
                b2Vec2 frictionVector = -velocity;
                frictionVector.Normalize();
                frictionVector *= frictionForce;
                body->ApplyForceToCenter(frictionVector, true);
            } else if (speed <= 0.1f) {
                body->SetLinearVelocity(b2Vec2(0, 0));
            }
        }

    }
}



void CarromGame::interpolatePositions(float alpha) {
    b2Vec2 position;
    b2Vec2 previousPosition;
    sf::Vector2f interpolatedPosition;

    if (strikerBody) {
        position = strikerBody->GetPosition();
        previousPosition = previousPositions[0];
        interpolatedPosition = sf::Vector2f(
            (position.x * alpha + previousPosition.x * (1 - alpha)) * 30.0f,
            (position.y * alpha + previousPosition.y * (1 - alpha)) * 30.0f
        );
        strikerSprite.setPosition(interpolatedPosition);
    }

    if (queenBody) {
        position = queenBody->GetPosition();
        previousPosition = previousPositions[1];
        interpolatedPosition = sf::Vector2f(
            (position.x * alpha + previousPosition.x * (1 - alpha)) * 30.0f,
            (position.y * alpha + previousPosition.y * (1 - alpha)) * 30.0f
        );
        queenSprite.setPosition(interpolatedPosition);
    }

    auto coinBody = coinBodies.begin();
    size_t index = 2;
    for (auto& coin : blackCoins) {
        if (coinBody != coinBodies.end()) {
            position = (*coinBody)->GetPosition();
            previousPosition = previousPositions[index];
            interpolatedPosition = sf::Vector2f(
                (position.x * alpha + previousPosition.x * (1 - alpha)) * 30.0f,
                (position.y * alpha + previousPosition.y * (1 - alpha)) * 30.0f
            );
            coin.setPosition(interpolatedPosition);
            ++coinBody;
            ++index;
        }
    }
    for (auto& coin : whiteCoins) {
        if (coinBody != coinBodies.end()) {
            position = (*coinBody)->GetPosition();
            previousPosition = previousPositions[index];
            interpolatedPosition = sf::Vector2f(
                (position.x * alpha + previousPosition.x * (1 - alpha)) * 30.0f,
                (position.y * alpha + previousPosition.y * (1 - alpha)) * 30.0f
            );
            coin.setPosition(interpolatedPosition);
            ++coinBody;
            ++index;
        }
    }
}


void CarromGame::run() {
    sf::Clock clock;
    float accumulator = 0.0f;
    const float fixedTimeStep = 1.0f / 60.0f;

    while (window.isOpen()) {
        float frameTime = clock.restart().asSeconds();
        accumulator += frameTime;

        eventHandler.handleEvents(window);
        updateStrikerPositionFromEventHandler(); 

        if(!gameOver){

        if (eventHandler.isStrikerReleased()) {
            float angle = eventHandler.getStrikerAngle();
            float power = eventHandler.getStrikerPower();
            applyStrikerForce(angle, power);
            eventHandler.resetStrikerRelease();
            strikerShot = true;
        }

        while (accumulator >= fixedTimeStep) {
            updatePhysics();
            checkPocketCollisions();
            handleTurn();
            accumulator -= fixedTimeStep;
        }

        float alpha = accumulator / fixedTimeStep;
        interpolatePositions(alpha);

        


            if (gameOver) {
                gameOver = true;
                if (player1Score > player2Score) {
                    winnerSprite.setTexture(player1WinsTexture);
                } else if (player2Score > player1Score) {
                    winnerSprite.setTexture(player2WinsTexture);
                }
                winnerSprite.setPosition(
                    (window.getSize().x - winnerSprite.getGlobalBounds().width) / 2,
                    (window.getSize().y - winnerSprite.getGlobalBounds().height) / 2
                );
                
            }

            }

        window.clear(sf::Color::White);
        
        window.draw(backgroundSprite);
        

        
        window.draw(boardSprite);
        for (const auto& pocket : pockets) {
            window.draw(pocket);
        }
        window.draw(strikerSprite);
        window.draw(queenSprite);
        window.draw(player1Sprite);
        window.draw(player2Sprite);
        window.draw(quitSprite);
        
        for (const auto& coin : blackCoins) {
            window.draw(coin);
        }
        for (const auto& coin : whiteCoins) {
            window.draw(coin);
        }
        
        if (eventHandler.isStrikerLocked() && eventHandler.getCurrentDragDistance() > 0) {
            window.draw(eventHandler.getPowerIndicator());
            window.draw(eventHandler.getAimingLine());
        }
        window.draw(player1ScoreText);
        window.draw(player2ScoreText);
        window.draw(winnerSprite);
        
        window.display();
        



    }
}