#include "raylib.h"
#include "constants.h"
#include "gamestate.h"
#include "mainmenu.h"
#include "world.h"
#include "player.h"
#include <memory>

int main() {
    // --- Init Window ---
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);

    // --- State Machine ---
    GameState state = GameState::MAIN_MENU;

    // --- Screens ---
    MainMenu mainMenu;

    // Gameplay objects (created on demand later)
    std::unique_ptr<World>  world;
    std::unique_ptr<Player> player;
    Camera2D camera = { 0 };

    Texture2D tileSheet   = { 0 };
    Texture2D playerSheet = { 0 };

    // --- Game Loop ---
    while (!WindowShouldClose()) {
        float delta = GetFrameTime();

        switch (state) {

        // ==========================================
        //  MAIN MENU
        // ==========================================
        case GameState::MAIN_MENU:
            mainMenu.update(delta, state);

            BeginDrawing();
                mainMenu.draw();
            EndDrawing();

            // When state transitions to GAMEPLAY, init world + player
            if (state == GameState::GAMEPLAY) {
                player = std::make_unique<Player>();
                world  = std::make_unique<World>();
                world->loadAround(player->currentChunkX(), player->currentChunkY());

                camera.zoom   = 1.0f;
                camera.offset = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
            }
            break;

        // ==========================================
        //  GAMEPLAY
        // ==========================================
        case GameState::GAMEPLAY:
            player->update(delta);
            world->update(player->currentChunkX(), player->currentChunkY());

            camera.target = {
                player->position.x + TILE_SIZE / 2.0f,
                player->position.y + TILE_SIZE / 2.0f
            };

            BeginDrawing();
                ClearBackground({ 20, 5, 5, 255 });

                BeginMode2D(camera);
                    world->draw(camera, tileSheet);
                    player->draw(playerSheet);
                EndMode2D();

                // HUD
                DrawText(TextFormat("Chunk: %d, %d",
                    player->currentChunkX(), player->currentChunkY()),
                    10, 10, 20, WHITE);
                DrawText(TextFormat("Tile:  %d, %d",
                    player->tileX(), player->tileY()),
                    10, 35, 20, WHITE);
                DrawText(TextFormat("Hunger:  %.0f%%", player->hunger),  10, 65,  18, YELLOW);
                DrawText(TextFormat("Thirst:  %.0f%%", player->thirst),  10, 85,  18, SKYBLUE);
                DrawText(TextFormat("Stamina: %.0f%%", player->stamina), 10, 105, 18, GREEN);
                DrawFPS(SCREEN_WIDTH - 80, 10);

            EndDrawing();
            break;
        }
    }

    // --- Cleanup ---
    CloseWindow();
    return 0;
}