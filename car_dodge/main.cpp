#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>  
#include <iomanip> 
#include <memory>

using namespace sf;

// ==========================================
// GAME STATE MACHINE
// ==========================================
enum class GameState {
    Playing,
    Paused,
    GameOver
};

// ==========================================
// BASE CLASS: Car
// ==========================================
class Car {
protected:
    Sprite sprite;
    float m_speed; 
    
public:
    Car(float startX, float startY, float startSpeed, const Texture& texture) : m_speed(startSpeed) {
        sprite.setTexture(texture);
        sprite.setScale(0.5f, 0.5f); 
        sprite.setPosition(startX, startY);
    }
    
    virtual ~Car() = default; 
    
    virtual void update(float dt, float currentRoadSpeed = 0.f) = 0; 

    void draw(RenderWindow& window) {
        window.draw(sprite);
    }

    FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }
    
    Vector2f getPosition() const {
        return sprite.getPosition();
    }
};

// ==========================================
// DERIVED CLASS: PlayerCar
// ==========================================
class PlayerCar : public Car {
private:
    float roadMinX; 
    float roadMaxX; 
    int m_direction; // -1 for Left, 1 for Right, 0 for Idle

public:
    PlayerCar(float startX, float startY, float speed, float minX, float maxX, const Texture& tex) 
        : Car(startX, startY, speed, tex), roadMinX(minX), roadMaxX(maxX), m_direction(0) {}

    // --- Encapsulated Movement Methods ---
    void moveLeft() { m_direction = -1; }
    void moveRight() { m_direction = 1; }

    void update(float dt, float currentRoadSpeed = 0.f) override {
        // Prevent compiler warning for unused parameter
        (void)currentRoadSpeed; 

        // Calculate intended movement based on direction
        float movement = m_direction * m_speed * dt;
        
        Vector2f pos = sprite.getPosition();
        float newX = pos.x + movement;

        // Clamp boundaries explicitly before applying position
        if (newX < roadMinX) {
            newX = roadMinX;
        } else if (newX + sprite.getGlobalBounds().width > roadMaxX) { 
            newX = roadMaxX - sprite.getGlobalBounds().width;
        }

        sprite.setPosition(newX, pos.y);

        // Reset direction so the car doesn't slide indefinitely
        m_direction = 0;
    }
};

// ==========================================
// DERIVED CLASS: EnemyCar
// ==========================================
class EnemyCar : public Car {
public:
    EnemyCar(float startX, float startY, float speed, const Texture& tex) 
        : Car(startX, startY, speed, tex) {}

    void update(float dt, float currentRoadSpeed = 0.f) override {
        // Enemies fall down the screen based on the sum of the road speed and their own speed
        float relativeSpeed = currentRoadSpeed + m_speed;
        sprite.move(0.f, relativeSpeed * dt);
    }
};

// ==========================================
// MAIN FUNCTION
// ==========================================
int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    RenderWindow window(VideoMode(800, 600), "Car Dodging Game");
    window.setFramerateLimit(60);

    // --- 1. LOAD GRAPHICS ---
    Texture playerTex, redCarTex, yellowCarTex, roadTex;
    if (!playerTex.loadFromFile("graphics/WhiteCar.png") ||
        !redCarTex.loadFromFile("graphics/RedCar1.png") ||
        !yellowCarTex.loadFromFile("graphics/YellowCar1.png") ||
        !roadTex.loadFromFile("graphics/road.png")) {
        std::cerr << "Error: Could not load graphics! Ensure 'graphics/' folder exists.\n";
        return -1; 
    }

    Font font;
    if (!font.loadFromFile("DS-DIGI.TTF")) {
        std::cerr << "Error: Could not load DS-DIGI.TTF!\n";
        return -1;
    }

    // --- 2. LOAD AUDIO ---
    Music bgMusic;
    if (!bgMusic.openFromFile("audio/music.ogg")) {
        std::cerr << "Error: Could not load audio/music.ogg!\n";
    } else {
        bgMusic.setLoop(true); 
        bgMusic.setVolume(50.f); 
        bgMusic.play();
    }

    SoundBuffer crashBuffer, game_overBuffer;
    if (!crashBuffer.loadFromFile("audio/crash.ogg") || !game_overBuffer.loadFromFile("audio/game_over2.ogg")) {
        std::cerr << "Error: Could not load crash or game_over audio!\n";
    }
    Sound crashSound(crashBuffer); 
    Sound game_overSound(game_overBuffer);

    // --- 3. SETUP BACKGROUND & HUD ---
    Sprite roadBackground1(roadTex);
    Sprite roadBackground2(roadTex);
    roadBackground1.setPosition(0.f, 0.f);
    roadBackground2.setPosition(0.f, -600.f);
    float baseRoadSpeed = 300.f;

    Text scoreText("SCORE:0", font, 25);
    scoreText.setPosition(20.f, 20.f);
    scoreText.setFillColor(Color::White);

    Text speedText("SPEED:1.0X", font, 25);
    speedText.setPosition(670.f, 20.f);
    speedText.setFillColor(Color::White);

    // Fixed UI text logic (removed \t to fix centering)
    Text gameOverText("   GAME OVER\n>>Enter<< to Restart", font, 45);
    FloatRect textRect = gameOverText.getLocalBounds();
    gameOverText.setOrigin(textRect.left + textRect.width/2.0f, textRect.top  + textRect.height/2.0f);
    gameOverText.setPosition(400.f, 300.f); 
    gameOverText.setFillColor(Color::White);
    gameOverText.setOutlineColor(sf::Color::Black);
    gameOverText.setOutlineThickness(2.f);

    Text pausedText(">>ENTER<<", font, 70);
    FloatRect pTextRect = pausedText.getLocalBounds();
    pausedText.setOrigin(pTextRect.left + pTextRect.width/2.0f, pTextRect.top + pTextRect.height/2.0f);
    pausedText.setPosition(400.f, 300.f);
    pausedText.setFillColor(Color::White);
    pausedText.setOutlineColor(sf::Color::Black);
    pausedText.setOutlineThickness(2.f);

    // --- 4. INITIALIZE GAME STATE ---
    GameState currentState = GameState::Paused; // Start paused so player can prep
    float spawnTimer = 0.0f;
    float baseSpawnInterval = 1.5f;
    int score = 0;
    float globalSpeedMultiplier = 1.0f; 

    PlayerCar player(400.f, 450.f, 350.f, 150.f, 650.f, playerTex); 
    std::vector<std::unique_ptr<EnemyCar>> enemies; 
    Clock clock; 

    // ==========================================
    // THE GAME LOOP
    // ==========================================
    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        if (deltaTime > 0.1f) deltaTime = 0.1f; 

        // --- A. EVENT HANDLING ---
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) window.close();
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) window.close();
            
            // Context-Sensitive Enter Key
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Enter) {
                
                if (currentState == GameState::GameOver) {
                    
                    // Stop previous sounds and perform heavy music rewind here, while screen is static
                    game_overSound.stop(); 
                    crashSound.stop();
                    bgMusic.stop();

                    currentState = GameState::Playing;
                    score = 0;
                    globalSpeedMultiplier = 1.0f;
                    spawnTimer = 0.0f;
                    enemies.clear(); 

                    bgMusic.setVolume(50.f); 
                    bgMusic.play();
                } 
                else if (currentState == GameState::Playing) {
                    currentState = GameState::Paused;
                    bgMusic.pause();
                } 
                else if (currentState == GameState::Paused) {
                    currentState = GameState::Playing;
                    bgMusic.play();
                }
            }
        }

        // --- B. UPDATE PHASE ---
        if (currentState == GameState::Playing) {
            
            globalSpeedMultiplier += deltaTime * 0.01f; 
            if (globalSpeedMultiplier > 2.5f) globalSpeedMultiplier = 2.5f; // Cap difficulty
            
            float currentRoadSpeed = baseRoadSpeed * globalSpeedMultiplier;

            // 1. Scroll Background
            roadBackground1.move(0.f, currentRoadSpeed * deltaTime);
            roadBackground2.move(0.f, currentRoadSpeed * deltaTime);

            if (roadBackground1.getPosition().y >= 600.f) roadBackground1.setPosition(0.f, roadBackground2.getPosition().y - 600.f);
            if (roadBackground2.getPosition().y >= 600.f) roadBackground2.setPosition(0.f, roadBackground1.getPosition().y - 600.f);

            // 2. Handle Player Input
            if (Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::A)) {
                player.moveLeft();
            }
            else if (Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D)) {
                player.moveRight();
            }
            
            // 3. Apply Player Physics
            player.update(deltaTime, currentRoadSpeed);

            // 4. Enemy Spawning Logic
            spawnTimer += deltaTime;
            float currentSpawnInterval = baseSpawnInterval / globalSpeedMultiplier;

            if (spawnTimer >= currentSpawnInterval) {
                float randomX = 150.f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (650.f - 150.f - 52.f))); 
                const Texture* chosenTex = (std::rand() % 2 == 0) ? &redCarTex : &yellowCarTex;

                // Enemy's unique driving speed added to the road speed
                float randomBonusSpeed = 20.f + static_cast<float>(std::rand() % 81); 
                
                enemies.push_back(std::make_unique<EnemyCar>(randomX, -150.f, randomBonusSpeed, *chosenTex));
                spawnTimer = 0.0f; 
            }

            // 5. Update Enemies & Check Collisions
            for (auto it = enemies.begin(); it != enemies.end(); ) {
                EnemyCar* enemy = it->get(); 
                enemy->update(deltaTime, currentRoadSpeed); 

                FloatRect playerBounds = player.getBounds();
                FloatRect enemyBounds = enemy->getBounds();
                
                // Forgiving Hitboxes
                playerBounds.left += 10.f; playerBounds.width -= 20.f;
                playerBounds.top += 5.f; playerBounds.height -= 10.f;
                
                enemyBounds.left += 5.f; enemyBounds.width -= 10.f;
                enemyBounds.top += 5.f; enemyBounds.height -= 10.f;

                if (playerBounds.intersects(enemyBounds)) {
                    // Play sound effects FIRST so there is no delay
                    crashSound.play();
                    game_overSound.play();

                    // Lightweight pause so the thread doesn't block
                    bgMusic.pause(); 
                    
                    currentState = GameState::GameOver;
                    break;
                }

                // Memory Cleanup & Scoring
                float enemyY = enemy->getPosition().y;
                if (enemyY > 650.f) { 
                    score += 10;            
                    it = enemies.erase(it); 
                } else {
                    ++it; 
                }
            }

            /// 6. Update HUD Strings using stringstream
            
            // Score String
            std::stringstream scoreStream;
            scoreStream << "SCORE:" << score;
            scoreText.setString(scoreStream.str());

            // Speed String (formatted to 1 decimal place)
            std::stringstream speedStream;
            speedStream << "SPEED:" << std::fixed << std::setprecision(1) << globalSpeedMultiplier << "X";
            speedText.setString(speedStream.str());
        }

        // --- C. RENDER PHASE ---
        window.clear(); 

        window.draw(roadBackground1);
        window.draw(roadBackground2);

        player.draw(window);
        for (const auto& enemy : enemies) {
            enemy->draw(window);
        }

        window.draw(scoreText);
        window.draw(speedText);

        if (currentState == GameState::GameOver) {
            window.draw(gameOverText);
        } else if (currentState == GameState::Paused) {
            window.draw(pausedText); 
        }

        window.display();
    }

    return 0;
}