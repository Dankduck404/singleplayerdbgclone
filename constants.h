#pragma once

// Window
constexpr int SCREEN_WIDTH  = 948;
constexpr int SCREEN_HEIGHT = 915;
constexpr int TARGET_FPS    = 60;
constexpr const char* GAME_TITLE = "Planet Vegeta";

// Tiles
constexpr int TILE_SIZE     = 32;   // pixels per tile
constexpr int CHUNK_WIDTH   = 400;  // tiles per chunk
constexpr int CHUNK_HEIGHT  = 400;

// Viewport (how many tiles visible on screen)
constexpr int VIEW_TILES_X  = SCREEN_WIDTH  / TILE_SIZE; // ~40
constexpr int VIEW_TILES_Y  = SCREEN_HEIGHT / TILE_SIZE; // ~22

// World
constexpr int WORLD_CHUNKS_X = 5;  // 5x5 chunk world to start
constexpr int WORLD_CHUNKS_Y = 5;

// Player
constexpr float PLAYER_SPEED = 160.0f; // pixels per second

// Tile IDs
constexpr int TILE_GROUND   = 0;
constexpr int TILE_ROCK     = 1;
constexpr int TILE_LAVA     = 2;
constexpr int TILE_DUST     = 3;