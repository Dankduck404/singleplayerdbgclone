#pragma once
#include "raylib.h"
#include "gamestate.h"

class MainMenu {
public:
    MainMenu();
    ~MainMenu();

    void update(float delta, GameState& state);
    void draw() const;

private:
    // Fade-in
    float   fadeAlpha;          // 0.0 (black) -> 1.0 (fully visible)
    float   fadeSpeed;
    bool    fadeInDone;

    // Background
    Texture2D bgTexture;        // Loaded from assets/sprites/menu_bg.png if present
    bool      bgLoaded;

    // Menu boxes
    Rectangle newGameBox;
    Rectangle loadGameBox;

    // Hover state
    bool hoverNew;
    bool hoverLoad;

    // Animated background particles (Planet Vegeta embers)
    struct Ember {
        Vector2 pos;
        Vector2 vel;
        float   size;
        float   alpha;
        Color   color;
    };
    static constexpr int EMBER_COUNT = 60;
    Ember embers[EMBER_COUNT];

    void initEmbers();
    void updateEmbers(float delta);
    void drawBackground() const;
    void drawEmbers() const;
    void drawMenuBoxes() const;
    void drawTitle() const;
};