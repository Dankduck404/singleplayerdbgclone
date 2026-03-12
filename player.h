#pragma once
#include "raylib.h"
#include "constants.h"

class Player {
public:
    Player();

    Vector2 position;
    float   hunger;
    float   thirst;
    float   stamina;

    void update(float delta);
    void draw(Texture2D sheet) const;

    int currentChunkX() const;
    int currentChunkY() const;
    int tileX()         const;
    int tileY()         const;
};
