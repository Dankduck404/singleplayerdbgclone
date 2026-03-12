#include "world.h"
#include "constants.h"

// Deterministic tile ID from tile coordinates
static int tileAt(int tx, int ty) {
    unsigned int h = (unsigned int)(tx * 1619 + ty * 31337);
    h ^= (h >> 16);
    h *= 0x45d9f3b;
    h ^= (h >> 16);
    return (int)(h % 16);
}

static Color tileColor(int id) {
    if (id < 8)  return {  40, 20, 10, 255 };  // ground / dust
    if (id < 12) return {  65, 35, 18, 255 };  // rock
    if (id < 14) return { 180, 30,  5, 255 };  // lava
    return { 55, 28, 12, 255 };                 // rubble
}

World::World() : centreChunkX(0), centreChunkY(0) {}

void World::loadAround(int chunkX, int chunkY) {
    centreChunkX = chunkX;
    centreChunkY = chunkY;
}

void World::update(int chunkX, int chunkY) {
    centreChunkX = chunkX;
    centreChunkY = chunkY;
}

void World::draw(Camera2D camera, Texture2D tileSheet) const {
    // Compute visible tile range from camera frustum
    float left   = camera.target.x - (camera.offset.x / camera.zoom);
    float top    = camera.target.y - (camera.offset.y / camera.zoom);
    float right  = camera.target.x + (camera.offset.x / camera.zoom);
    float bottom = camera.target.y + (camera.offset.y / camera.zoom);

    int startX = (int)(left   / TILE_SIZE) - 1;
    int startY = (int)(top    / TILE_SIZE) - 1;
    int endX   = (int)(right  / TILE_SIZE) + 1;
    int endY   = (int)(bottom / TILE_SIZE) + 1;

    for (int ty = startY; ty <= endY; ty++) {
        for (int tx = startX; tx <= endX; tx++) {
            Color c = tileColor(tileAt(tx, ty));
            DrawRectangle(
                tx * TILE_SIZE,
                ty * TILE_SIZE,
                TILE_SIZE - 1,
                TILE_SIZE - 1,
                c
            );
        }
    }
}
