#pragma once
#include "raylib.h"
#include "gamestate.h"

class CharSelect {
public:
    CharSelect();

    void update(float delta, GameState& state);
    void draw() const;

private:
    // Three class boxes
    Rectangle boxes[3];
    bool      hovered[3];

    static constexpr const char* LABELS[3] = { "Low Class", "Middle Class", "Elites" };

    void drawBackground() const;
    void drawBoxes()      const;
    void drawTitle()      const;
};
