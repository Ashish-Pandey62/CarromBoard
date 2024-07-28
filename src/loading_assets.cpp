#include "loading_assets.hpp"
#include <stdexcept>
#include <algorithm>
#include <ctime>
#include <cmath>

CarromGame::CarromGame() 
    : window(sf::VideoMode(1000, 1000), "Carrom Game"),
      eventHandler(strikerSprite),
      frictionCoefficient(0.9f),
      restitutionCoefficient(0.2f),
      strikerPocketed(false),
      player1StrikerPosition(500, 784),
      player2StrikerPosition(500, 200),
      currentPlayer(1)  ,
      strikerShot(false),
      coinPocketed(false)
{
    std::srand(std::time(nullptr));
    loadTextures();
    setupSprites();
    setupPhysics();
    setupPockets();
}

CarromGame::~CarromGame() {
    delete world;
}

void CarromGame::loadTextures() {
    if (!bgTexture.loadFromFile("assets/bg.png") ||
        !boardTexture.loadFromFile("assets/board.png") ||
        !strikerTexture.loadFromFile("assets/striker.png") ||
        !queenTexture.loadFromFile("assets/queen.png") ||
        !blackCoinTexture.loadFromFile("assets/black.png") ||
        !whiteCoinTexture.loadFromFile("assets/white.png")) {
        throw std::runtime_error("Failed to load textures");
    }
}

void CarromGame::setupSprites() {

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


void CarromGame::checkAllBodiesAtRest() {
    if (areAllBodiesAtRest()) {
        resetStrikerPosition();
    }
}


bool CarromGame::areAllBodiesAtRest() const {
    const float restThreshold = 0.1f;  // Adjust this value as needed

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



// void CarromGame::resetStrikerPosition() {
//     strikerBody->SetTransform(b2Vec2(initialStrikerPosition.x / 30.0f, initialStrikerPosition.y / 30.0f), 0);
//     strikerBody->SetLinearVelocity(b2Vec2(0, 0));
//     strikerSprite.setPosition(initialStrikerPosition);
// }

void CarromGame::switchTurn() {
    currentPlayer = (currentPlayer == 1) ? 2 : 1;
    resetStrikerPosition();
    strikerShot = false;
    coinPocketed = false;
}

void CarromGame::resetStrikerPosition() {
    if (currentPlayer == 1) {
        strikerBody->SetTransform(b2Vec2(player1StrikerPosition.x / 30.0f, player1StrikerPosition.y / 30.0f), 0);
    } else {
        strikerBody->SetTransform(b2Vec2(player2StrikerPosition.x / 30.0f, player2StrikerPosition.y / 30.0f), 0);
    }
    strikerBody->SetLinearVelocity(b2Vec2(0, 0));
    strikerSprite.setPosition(currentPlayer == 1 ? player1StrikerPosition : player2StrikerPosition);
}


void CarromGame::handleTurn() {
    if (strikerShot && areAllBodiesAtRest()) {
        if (!coinPocketed || strikerPocketed) {
            switchTurn();
        } else {
            // Reset striker position for the same player
            resetStrikerPosition();
        }
        strikerShot = false;
        coinPocketed = false;
        strikerPocketed = false;
    }
}



void CarromGame::setupPockets() {
    std::vector<sf::Vector2f> pocketPositions = {
        {153, 160}, {845, 158}, {153, 850}, {850, 850}
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

    // Create board boundaries
    b2BodyDef boardDef;
    boardDef.position.Set(500.0f / 30.0f, 500.0f / 30.0f);
    b2Body* boardBody = world->CreateBody(&boardDef);

    b2EdgeShape boardEdge;
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &boardEdge;
    fixtureDef.restitution = restitutionCoefficient;

    // Top edge
    boardEdge.SetTwoSided(b2Vec2(-450.0f / 30.0f, -450.0f / 30.0f), b2Vec2(450.0f / 30.0f, -450.0f / 30.0f));
    boardBody->CreateFixture(&fixtureDef);

    // Bottom edge
    boardEdge.SetTwoSided(b2Vec2(-450.0f / 30.0f, 450.0f / 30.0f), b2Vec2(450.0f / 30.0f, 450.0f / 30.0f));
    boardBody->CreateFixture(&fixtureDef);

    // Left edge
    boardEdge.SetTwoSided(b2Vec2(-450.0f / 30.0f, -450.0f / 30.0f), b2Vec2(-450.0f / 30.0f, 450.0f / 30.0f));
    boardBody->CreateFixture(&fixtureDef);

    // Right edge
    boardEdge.SetTwoSided(b2Vec2(450.0f / 30.0f, -450.0f / 30.0f), b2Vec2(450.0f / 30.0f, 450.0f / 30.0f));
    boardBody->CreateFixture(&fixtureDef);

    // Create striker
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

    // Create coins
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

    // Create queen
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

    // For striker, coins, and queen
    strikerBody->SetLinearDamping(0.3f);
    queenBody->SetLinearDamping(0.3f);
    for (auto* coinBody : coinBodies) {
        coinBody->SetLinearDamping(0.3f);
    }

    previousPositions.resize(coinBodies.size() + 2); // +2 for striker and queen
    for (size_t i = 0; i < previousPositions.size(); ++i) {
        previousPositions[i] = b2Vec2(0, 0);
    }
    // Adding pocket sensors


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



void CarromGame::checkPocketCollisions() {
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
            pocketedCoins.push(coinBody);
            world->DestroyBody(coinBody);
            it = coinBodies.erase(it);
            coinPocketed = true; 
        } else {
            ++it;
        }
    }

    // Check if striker is pocketed
    b2Vec2 strikerPosition = strikerBody->GetPosition();
    for (const auto& pocket : pockets) {
        sf::Vector2f pocketCenter = pocket.getPosition() + sf::Vector2f(POCKET_DIAMETER / 2, POCKET_DIAMETER / 2);
        b2Vec2 pocketPosition(pocketCenter.x / 30.0f, pocketCenter.y / 30.0f);
        
        float distance = b2Distance(strikerPosition, pocketPosition);
        if (distance < (POCKET_DIAMETER / 2 + STRIKER_DIAMETER / 2) / 30.0f) {
            strikerPocketed = true;
            break;
        }
    }

    // Remove sprites for removed coins
    auto removeSprite = [this](std::vector<sf::Sprite>& coins) {
        coins.erase(std::remove_if(coins.begin(), coins.end(),
            [this](const sf::Sprite& coin) {
                return std::find_if(coinBodies.begin(), coinBodies.end(),
                    [&coin](const b2Body* body) {
                        return body->GetPosition().x * 30.0f == coin.getPosition().x &&
                               body->GetPosition().y * 30.0f == coin.getPosition().y;
                    }) == coinBodies.end();
            }), coins.end());
    };

    removeSprite(blackCoins);
    removeSprite(whiteCoins);

    if (strikerPocketed) {
        handlePocketedStriker();
    }
}



void CarromGame::handlePocketedStriker() {
    strikerPocketed = false;
    strikerBody->SetTransform(b2Vec2(initialStrikerPosition.x / 30.0f, initialStrikerPosition.y / 30.0f), 0);
    strikerBody->SetLinearVelocity(b2Vec2(0, 0));
    strikerSprite.setPosition(initialStrikerPosition);

    if (!pocketedCoins.empty()) {
        placeLastPocketedCoinInCenter();
    }
}


void CarromGame::placeLastPocketedCoinInCenter() {
    b2Body* lastPocketedCoin = pocketedCoins.back();
    pocketedCoins.pop();

    if (isSpaceAvailableInCenter()) {
        lastPocketedCoin->SetTransform(b2Vec2(500 / 30.0f, 500 / 30.0f), 0);
    } else {
        moveCentralCoinToSide();
        lastPocketedCoin->SetTransform(b2Vec2(500 / 30.0f, 500 / 30.0f), 0);
    }

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
    previousPositions[1] = queenBody->GetPosition();
    for (size_t i = 0; i < coinBodies.size(); ++i) {
    previousPositions[i + 2] = coinBodies[i]->GetPosition();
    }

    for (int i = 0; i < 5; ++i) {  // Perform multiple steps per frame
        world->Step(timeStep, velocityIterations, positionIterations);
    }

    // Update striker position
    b2Vec2 position = strikerBody->GetPosition();
    strikerSprite.setPosition(position.x * 30.0f, position.y * 30.0f);

    // Update queen position
    b2Vec2 queenPos = queenBody->GetPosition();
    queenSprite.setPosition(queenPos.x * 30.0f, queenPos.y * 30.0f);

    // Update coin positions
    auto coinBody = coinBodies.begin();
    for (auto& coin : blackCoins) {
        b2Vec2 pos = (*coinBody)->GetPosition();

     coin.setPosition(pos.x * 30.0f, pos.y * 30.0f);
        ++coinBody;
    }
    for (auto& coin : whiteCoins) {
        b2Vec2 pos = (*coinBody)->GetPosition();
        coin.setPosition(pos.x * 30.0f, pos.y * 30.0f);
        ++coinBody;
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

    // Store the remaining force to apply over next few frames
    remainingForce = b2Vec2(-forceY*2.5f, forceX*2.5f) ;  // Apply 3 more times
    frameCounter = 2;

}

void CarromGame::handleCollisions() {
    // Check for collisions between coins and board boundaries
    for (auto* coinBody : coinBodies) {
        b2Vec2 position = coinBody->GetPosition();
        b2Vec2 velocity = coinBody->GetLinearVelocity();

        // Board dimensions (adjust these values according to your board size)
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
            if (speed > 0.1f) {  // Only apply friction if speed is above a threshold
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
    b2Vec2 position = strikerBody->GetPosition();
    b2Vec2 previousPosition = previousPositions[0];
    sf::Vector2f interpolatedPosition(
        (position.x * alpha + previousPosition.x * (1 - alpha)) * 30.0f,
        (position.y * alpha + previousPosition.y * (1 - alpha)) * 30.0f
    );
    strikerSprite.setPosition(interpolatedPosition);

    position = queenBody->GetPosition();
    previousPosition = previousPositions[1];
    interpolatedPosition = sf::Vector2f(
        (position.x * alpha + previousPosition.x * (1 - alpha)) * 30.0f,
        (position.y * alpha + previousPosition.y * (1 - alpha)) * 30.0f
    );
    queenSprite.setPosition(interpolatedPosition);

    auto coinBody = coinBodies.begin();
    size_t index = 2;
    for (auto& coin : blackCoins) {
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
    for (auto& coin : whiteCoins) {
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


void CarromGame::run() {
    sf::Clock clock;
    float accumulator = 0.0f;
    const float fixedTimeStep = 1.0f / 60.0f;

    while (window.isOpen()) {
        float frameTime = clock.restart().asSeconds();
        accumulator += frameTime;

        eventHandler.handleEvents(window);

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

        if (areAllBodiesAtRest()  ) {
            switchTurn();
        }

        window.clear(sf::Color::White);
        
        window.draw(boardSprite);
        for (const auto& pocket : pockets) {
            window.draw(pocket);
        }
        window.draw(strikerSprite);
        window.draw(queenSprite);
        
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
        
        window.display();
    }
}