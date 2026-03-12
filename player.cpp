#include "player.h"

Player::Player()
    : position{ 0.0f, 0.0f },
      hunger(100.0f), thirst(100.0f), stamina(100.0f)
{}

void Player::update(float delta) {
    Vector2 dir = { 0.0f, 0.0f };
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    dir.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  dir.y += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  dir.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dir.x += 1.0f;

    position.x += dir.x * PLAYER_SPEED * delta;
    position.y += dir.y * PLAYER_SPEED * delta;

    bool moving = (dir.x != 0.0f || dir.y != 0.0f);
    hunger  -= 0.5f  * delta;
    thirst  -= 0.8f  * delta;
    stamina += (moving ? -8.0f : 5.0f) * delta;

    if (hunger  < 0.0f)   hunger  = 0.0f;
    if (thirst  < 0.0f)   thirst  = 0.0f;
    if (stamina < 0.0f)   stamina = 0.0f;
    if (stamina > 100.0f) stamina = 100.0f;
}

void Player::draw(Texture2D sheet) const {
    // Placeholder until sprite sheet is wired up
    DrawRectangle(
        (int)position.x,
        (int)position.y,
        TILE_SIZE, TILE_SIZE,
        { 200, 160, 60, 255 }
    );
}

int Player::currentChunkX() const {
    return (int)(position.x / (CHUNK_WIDTH  * TILE_SIZE));
}

int Player::currentChunkY() const {
    return (int)(position.y / (CHUNK_HEIGHT * TILE_SIZE));
}

int Player::tileX() const { return (int)(position.x / TILE_SIZE); }
int Player::tileY() const { return (int)(position.y / TILE_SIZE); }
