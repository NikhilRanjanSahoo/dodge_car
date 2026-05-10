#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

using namespace sf;

// ==========================================
// BASE CLASS: Car
// ==========================================
class Car {
protected:
    Sprite sprite;
    float speed;
    
public:
    // Pass the texture by reference to avoid copying heavy image data
    Car(float startX, float startY, float startSpeed, const Texture& texture) : speed(startSpeed) {
        sprite.setTexture(texture);
        sprite.setPosition(startX, startY);
        sprite.setScale(0.5f, 0.5f); 
    }
    
    virtual ~Car() = default; 
    virtual void update(float deltaTime) = 0; 

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

public:
    PlayerCar(float startX, float startY, float speed, float minX, float maxX, const Texture& tex) 
        : Car(startX, startY, speed, tex), roadMinX(minX), roadMaxX(maxX) {}

    void update(float deltaTime) override {
        if (Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::A)) {
            sprite.move(-speed * deltaTime, 0.f);
        }
        if (Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D)) {
            sprite.move(speed * deltaTime, 0.f);
        }

        Vector2f pos = sprite.getPosition();
        // Adjust clamp so the whole car stays on screen (subtracting width of sprite)
        if (pos.x < roadMinX) {
            sprite.setPosition(roadMinX, pos.y);
        } else if (pos.x + sprite.getGlobalBounds().width > roadMaxX) { 
            sprite.setPosition(roadMaxX - sprite.getGlobalBounds().width, pos.y);
        }
    }
};

// ==========================================
// DERIVED CLASS: EnemyCar
// ==========================================
class EnemyCar : public Car {
public:
    EnemyCar(float startX, float startY, float speed, const Texture& tex) 
        : Car(startX, startY, speed, tex) {}

    void update(float deltaTime) override {
        sprite.move(0.f, speed * deltaTime);
    }
};