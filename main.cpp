#include "raylib.h"
#include "constants.h"
#include "gamestate.h"
#include "mainmenu.h"
#include "charselect.h"
#include "pointselect.h"
#include "world.h"
#include "player.h"
#include <memory>
#include <cstdlib>

// ── Fade transition ───────────────────────────────────────────────────────────
// Screens request a state change normally. main.cpp intercepts it, fades to
// black, switches the active state, then fades back in — all without touching
// the screen classes.

static constexpr float FADE_SPEED = 2.2f;   // full fade in ~0.45 seconds

enum class FadePhase { NONE, OUT, IN };

int main() {
    // --- Convert BYOND icons to PNG/GIF on startup (skips already-converted files) ---
    system("py dmi_to_png.py --skip-existing >nul 2>&1");

    // --- Init Window ---
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);

    // --- State Machine ---
    GameState state        = GameState::MAIN_MENU;
    GameState pendingState = GameState::MAIN_MENU; // target after fade

    // --- Fade ---
    FadePhase fadePhase = FadePhase::NONE;
    float     fadeAlpha = 0.0f;            // 0 = clear, 1 = full black

    // --- Screens ---
    MainMenu    mainMenu;
    CharSelect  charSelect;
    PointSelect pointSelect;

    // Gameplay objects (created on demand)
    std::unique_ptr<World>  world;
    std::unique_ptr<Player> player;
    Camera2D camera = { 0 };

    Texture2D tileSheet   = { 0 };
    Texture2D playerSheet = { 0 };

    // --- Game Loop ---
    while (!WindowShouldClose()) {
        float delta = GetFrameTime();

        // ── Fade logic (runs every frame, before screen update) ───────────────
        if (fadePhase == FadePhase::OUT) {
            fadeAlpha += FADE_SPEED * delta;
            if (fadeAlpha >= 1.0f) {
                fadeAlpha = 1.0f;

                // Mid-fade: perform the actual state switch + any init
                state = pendingState;
                if (state == GameState::GAMEPLAY) {
                    player = std::make_unique<Player>();
                    world  = std::make_unique<World>();
                    world->loadAround(player->currentChunkX(), player->currentChunkY());
                    camera.zoom   = 1.0f;
                    camera.offset = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
                }

                fadePhase = FadePhase::IN;
            }
        } else if (fadePhase == FadePhase::IN) {
            fadeAlpha -= FADE_SPEED * delta;
            if (fadeAlpha <= 0.0f) {
                fadeAlpha = 0.0f;
                fadePhase = FadePhase::NONE;
            }
        }

        // ── Screen update (only when not mid-fade-out) ────────────────────────
        if (fadePhase != FadePhase::OUT) {
            GameState prevState = state;

            switch (state) {
            case GameState::MAIN_MENU:
                mainMenu.update(delta, state);
                break;
            case GameState::CHARACTER_SELECT:
                charSelect.update(delta, state);
                break;
            case GameState::POINT_SELECT:
                pointSelect.update(delta, state);
                break;
            case GameState::GAMEPLAY:
                player->update(delta);
                world->update(player->currentChunkX(), player->currentChunkY());
                camera.target = {
                    player->position.x + TILE_SIZE / 2.0f,
                    player->position.y + TILE_SIZE / 2.0f
                };
                break;
            }

            // If the screen requested a state change, intercept it and fade
            if (state != prevState) {
                pendingState = state;
                state        = prevState;   // hold current screen while fading out
                fadePhase    = FadePhase::OUT;
                fadeAlpha    = 0.0f;
            }
        }

        // ── Draw ──────────────────────────────────────────────────────────────
        BeginDrawing();

        switch (state) {

        // ==========================================
        //  MAIN MENU
        // ==========================================
        case GameState::MAIN_MENU:
            mainMenu.draw();
            break;

        // ==========================================
        //  CHARACTER SELECT
        // ==========================================
        case GameState::CHARACTER_SELECT:
            charSelect.draw();
            break;

        // ==========================================
        //  POINT SELECT
        // ==========================================
        case GameState::POINT_SELECT:
            pointSelect.draw();
            break;

        // ==========================================
        //  GAMEPLAY
        // ==========================================
        case GameState::GAMEPLAY:
            ClearBackground({ 20, 5, 5, 255 });

            BeginMode2D(camera);
                world->draw(camera, tileSheet);
                player->draw(playerSheet);
            EndMode2D();

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
            break;
        }

        // Fade overlay — drawn on top of everything every frame
        if (fadeAlpha > 0.0f) {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                { 0, 0, 0, (unsigned char)(fadeAlpha * 255) });
        }

        EndDrawing();
    }

    // --- Cleanup ---
    CloseWindow();
    return 0;
}
