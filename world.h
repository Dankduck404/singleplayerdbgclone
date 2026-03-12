#pragma once
#include "raylib.h"

class World {
public:
    World();

    void loadAround(int chunkX, int chunkY);
    void update(int chunkX, int chunkY);
    void draw(Camera2D camera, Texture2D tileSheet) const;

private:
    int centreChunkX;
    int centreChunkY;
};
