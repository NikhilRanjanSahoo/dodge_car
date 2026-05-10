# Car Dodging Game (C++ & SFML)

## 📌 Overview

A fast-paced 2D arcade racing game built entirely in **C++** using the **Simple and Fast Multimedia Library (SFML)**. The objective is to survive as long as possible by dodging oncoming traffic. As time progresses, the road speed and enemy spawn rates dynamically increase, providing a progressively challenging experience.

This project was developed to demonstrate core **Object-Oriented Programming (OOP)** principles, modern C++ memory management, and game loop architecture.

## ✨ Features

* **Object-Oriented Architecture:** Utilizes inheritance and encapsulation with a base `Car` class and derived `PlayerCar` and `EnemyCar` classes.
* **Dynamic Difficulty & Relative Physics:** Enemy cars spawn with randomized speeds. Their movement is calculated relative to the increasing road speed, creating the realistic illusion of incoming traffic.
* **State Machine Engine:** Fully implemented Game States (`Playing`, `Paused`, `GameOver`) controlled by a context-sensitive `Enter` keybinding.
* **Modern Memory Management:** Uses `std::unique_ptr` and standard vectors to handle entity spawning and despawning, ensuring zero memory leaks during gameplay.
* **Audio & Visual Integration:** Features continuous background music, localized sound effects for collisions, and textured sprites.
* **Dynamic HUD:** Real-time score tracking and a speed multiplier display formatted using `std::stringstream`.

## 🎮 Controls

* **Left Arrow / A :** Steer Left
* **Right Arrow / D :** Steer Right
* **Enter :** Start Game / Pause / Resume / Restart (Context Sensitive)
* **Escape :** Quit Game

## 🛠️ Requirements & Dependencies

To compile and run this game, you must have the following installed on your system:

* A C++ Compiler (GCC / MinGW) supporting C++14 or higher.
* [SFML (Simple and Fast Multimedia Library)](https://www.sfml-dev.org/) (Version 2.5.1 or later).

**Required Folder Structure:**
Ensure your repository is structured exactly like this before running:

```text
/car_dodge
│── main.cpp
│── DS-DIGI.TTF
├── /graphics
│   ├── WhiteCar.png
│   ├── RedCar1.png
│   ├── YellowCar1.png
│   └── road.png
└── /audio
    ├── music.ogg
    ├── crash.ogg
    └── game_over2.ogg

```

## 🚀 How to Compile and Run (Linux / Ubuntu / WSL)

Open your terminal, navigate to the project directory, and compile the code using `g++`, making sure to link all the necessary SFML modules:

```bash
g++ main.cpp -o play -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

```

Once compiled, launch the game:

```bash
./play

```

