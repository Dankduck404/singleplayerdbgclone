#include "mainmenu.h"
#include "constants.h"
#include <cstdlib>
#include <cmath>

// ---------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------
static Color lerpColor(Color a, Color b, float t) {
    return {
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        (unsigned char)(a.a + (b.a - a.a) * t)
    };
}

// ---------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------
MainMenu::MainMenu()
    : fadeAlpha(0.0f), fadeSpeed(0.6f), fadeInDone(false),
      bgLoaded(false), bgTileLoaded(false), hoverNew(false), hoverLoad(false)
{
    // Try loading a real background image
    if (FileExists("assets/sprites/menu_bg.png")) {
        bgTexture = LoadTexture("assets/sprites/menu_bg.png");
        bgLoaded  = true;
    }

    // Load background tile overlay
    if (FileExists("icons/backgroundtile.png")) {
        bgTile       = LoadTexture("icons/backgroundtile.png");
        bgTileLoaded = true;
    }

    // --- Menu box layout ---
    // Center both boxes horizontally, place them in the lower-middle
    float boxW  = 380.0f;
    float boxH  = 72.0f;
    float gap   = 28.0f;
    float totalH = boxH * 2 + gap;

    float startX = (SCREEN_WIDTH  - boxW) / 2.0f;
    float startY = (SCREEN_HEIGHT - totalH) / 2.0f + 60.0f; // Slight downward offset

    newGameBox  = { startX, startY,           boxW, boxH };
    loadGameBox = { startX, startY + boxH + gap, boxW, boxH };

    initEmbers();
}

MainMenu::~MainMenu() {
    if (bgLoaded)     UnloadTexture(bgTexture);
    if (bgTileLoaded) UnloadTexture(bgTile);
}

// ---------------------------------------------------------------
// Ember Particles — drifting cinders like Planet Vegeta sky
// ---------------------------------------------------------------
void MainMenu::initEmbers() {
    for (auto& e : embers) {
        e.pos   = { (float)(GetRandomValue(0, SCREEN_WIDTH)),
                    (float)(GetRandomValue(0, SCREEN_HEIGHT)) };
        e.vel   = { (float)(GetRandomValue(-20, 20)) / 10.0f,
                    (float)(GetRandomValue(-60, -15)) / 10.0f };
        e.size  = (float)(GetRandomValue(1, 4));
        e.alpha = (float)(GetRandomValue(30, 100)) / 100.0f;
        // Warm orange/red embers
        int r = GetRandomValue(0, 2);
        if (r == 0)      e.color = { 255, 80,  0,  255 };
        else if (r == 1) e.color = { 220, 40,  10, 255 };
        else             e.color = { 255, 140, 20, 255 };
    }
}

void MainMenu::updateEmbers(float delta) {
    for (auto& e : embers) {
        e.pos.x += e.vel.x * delta * 30.0f;
        e.pos.y += e.vel.y * delta * 30.0f;

        // Flicker alpha
        e.alpha += ((float)(GetRandomValue(-5, 5)) / 100.0f) * delta * 10.0f;
        if (e.alpha < 0.1f) e.alpha = 0.1f;
        if (e.alpha > 1.0f) e.alpha = 1.0f;

        // Wrap when off screen
        if (e.pos.y < -10.0f) {
            e.pos.y = (float)SCREEN_HEIGHT + 5.0f;
            e.pos.x = (float)(GetRandomValue(0, SCREEN_WIDTH));
        }
        if (e.pos.x < -10.0f) e.pos.x = (float)SCREEN_WIDTH  + 5.0f;
        if (e.pos.x > SCREEN_WIDTH  + 10.0f) e.pos.x = -5.0f;
    }
}

// ---------------------------------------------------------------
// Update
// ---------------------------------------------------------------
void MainMenu::update(float delta, GameState& state) {
    // Fade in
    if (!fadeInDone) {
        fadeAlpha += fadeSpeed * delta;
        if (fadeAlpha >= 1.0f) {
            fadeAlpha  = 1.0f;
            fadeInDone = true;
        }
        return; // Don't process input during fade-in
    }

    updateEmbers(delta);

    // Mouse hover detection
    Vector2 mouse = GetMousePosition();
    hoverNew  = CheckCollisionPointRec(mouse, newGameBox);
    hoverLoad = CheckCollisionPointRec(mouse, loadGameBox);

    // Click handling (no functionality yet — hooked up later)
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (hoverNew) {
            state = GameState::CHARACTER_SELECT;
        }
        if (hoverLoad) {
            // TODO: Load save file
        }
    }
}

// ---------------------------------------------------------------
// Draw
// ---------------------------------------------------------------
void MainMenu::drawBackground() const {
    if (bgLoaded) {
        // Scale texture to fit window
        Rectangle src  = { 0, 0, (float)bgTexture.width, (float)bgTexture.height };
        Rectangle dest = { 0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
        DrawTexturePro(bgTexture, src, dest, { 0, 0 }, 0.0f, WHITE);
    } else {
        // Procedural Planet Vegeta sky gradient
        // Top = deep blood red, bottom = dark charcoal
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            float t = (float)y / (float)SCREEN_HEIGHT;
            Color row = lerpColor(
                { 60,  8,  8,  255 },   // top: deep blood red
                { 18,  5,  5,  255 },   // bottom: near black
                t
            );
            DrawLine(0, y, SCREEN_WIDTH, y, row);
        }

        // Lava glow on the horizon (lower third)
        for (int y = (int)(SCREEN_HEIGHT * 0.65f); y < SCREEN_HEIGHT; y++) {
            float t = (float)(y - SCREEN_HEIGHT * 0.65f) / (SCREEN_HEIGHT * 0.35f);
            Color glow = { (unsigned char)(30 + 80 * t),
                           (unsigned char)(3  + 10 * t),
                           3, (unsigned char)(80 * t) };
            DrawLine(0, y, SCREEN_WIDTH, y, glow);
        }

        // Rocky silhouette ridge line
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            float nx   = (float)x / SCREEN_WIDTH;
            // Simple layered pseudo-mountain ridge
            int h1 = (int)(80  + sinf(nx * 6.2f)  * 40 + sinf(nx * 13.7f) * 20);
            int h2 = (int)(120 + sinf(nx * 4.1f + 1.2f) * 55 + sinf(nx * 9.3f) * 25);
            int baseY = SCREEN_HEIGHT - h2;

            // Back ridge (darker)
            DrawLine(x, baseY - h1 / 2, x, SCREEN_HEIGHT, { 12, 4, 4, 255 });
            // Front ridge (slightly lighter)
            DrawLine(x, SCREEN_HEIGHT - h2 / 2, x, SCREEN_HEIGHT, { 22, 6, 4, 255 });
        }
    }

    // --- Tiled background tile at 40% opacity ---
    if (bgTileLoaded && bgTile.width > 0 && bgTile.height > 0) {
        Color tint = { 255, 255, 255, 102 }; // 102 / 255 ≈ 40%
        for (int ty = 0; ty < SCREEN_HEIGHT; ty += bgTile.height) {
            for (int tx = 0; tx < SCREEN_WIDTH; tx += bgTile.width) {
                DrawTexture(bgTile, tx, ty, tint);
            }
        }
    }
}

void MainMenu::drawEmbers() const {
    for (const auto& e : embers) {
        Color c = e.color;
        c.a     = (unsigned char)(e.alpha * 255 * fadeAlpha);
        DrawCircleV(e.pos, e.size, c);
    }
}

void MainMenu::drawTitle() const {
    // Title text — "PLANET VEGETA" with glow effect
    const char* title    = "PLANET VEGETA";
    const char* subtitle = "BLOODLINES";
    int titleSize    = 64;
    int subtitleSize = 26;

    int titleW    = MeasureText(title,    titleSize);
    int subtitleW = MeasureText(subtitle, subtitleSize);

    int titleX    = (SCREEN_WIDTH - titleW)    / 2;
    int subtitleX = (SCREEN_WIDTH - subtitleW) / 2;
    int titleY    = (int)(SCREEN_HEIGHT * 0.18f);

    unsigned char a = (unsigned char)(255 * fadeAlpha);

    // Glow layers (drawn behind main text)
    DrawText(title, titleX - 2, titleY - 2, titleSize, { 180, 30, 0, (unsigned char)(60 * fadeAlpha) });
    DrawText(title, titleX + 2, titleY + 2, titleSize, { 180, 30, 0, (unsigned char)(60 * fadeAlpha) });
    DrawText(title, titleX - 1, titleY + 1, titleSize, { 220, 60, 0, (unsigned char)(80 * fadeAlpha) });

    // Main title
    DrawText(title, titleX, titleY, titleSize, { 255, 100, 30, a });

    // Divider line
    int divW = 300;
    DrawLine((SCREEN_WIDTH - divW) / 2, titleY + titleSize + 8,
             (SCREEN_WIDTH + divW) / 2, titleY + titleSize + 8,
             { 180, 50, 10, (unsigned char)(160 * fadeAlpha) });

    // Subtitle
    DrawText(subtitle, subtitleX, titleY + titleSize + 18, subtitleSize,
             { 200, 80, 20, (unsigned char)(200 * fadeAlpha) });
}

void MainMenu::drawMenuBoxes() const {
    unsigned char a = (unsigned char)(255 * fadeAlpha);

    struct BoxDef { Rectangle r; bool hover; const char* label; };
    BoxDef boxes[2] = {
        { newGameBox,  hoverNew,  "NEW GAME"  },
        { loadGameBox, hoverLoad, "LOAD GAME" },
    };

    for (const auto& b : boxes) {
        // Box background
        Color bg = b.hover
            ? Color{ 120, 30, 5, (unsigned char)(200 * fadeAlpha) }
            : Color{  35,  8, 5, (unsigned char)(180 * fadeAlpha) };

        DrawRectangleRec(b.r, bg);

        // Border — thicker + brighter on hover
        Color border = b.hover
            ? Color{ 255, 100, 20, a }
            : Color{ 140,  45, 10, (unsigned char)(180 * fadeAlpha) };
        int thick = b.hover ? 2 : 1;
        DrawRectangleLinesEx(b.r, (float)thick, border);

        // Inner highlight line at top of box
        DrawLine((int)b.r.x + 2, (int)b.r.y + 2,
                 (int)(b.r.x + b.r.width) - 2, (int)b.r.y + 2,
                 { 255, 140, 60, (unsigned char)(40 * fadeAlpha) });

        // Label text
        int fontSize = 28;
        int textW    = MeasureText(b.label, fontSize);
        int textX    = (int)(b.r.x + (b.r.width  - textW) / 2);
        int textY    = (int)(b.r.y + (b.r.height - fontSize) / 2);

        Color textColor = b.hover
            ? Color{ 255, 160, 60, a }
            : Color{ 210,  90, 20, a };

        // Slight text shadow
        DrawText(b.label, textX + 1, textY + 1, fontSize,
                 { 0, 0, 0, (unsigned char)(100 * fadeAlpha) });
        DrawText(b.label, textX, textY, fontSize, textColor);
    }
}

void MainMenu::draw() const {
    drawBackground();
    drawEmbers();
    drawTitle();
    drawMenuBoxes();

    // Fade-in overlay: starts black, fades to transparent
    if (!fadeInDone) {
        unsigned char overlayA = (unsigned char)(255 * (1.0f - fadeAlpha));
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, { 0, 0, 0, overlayA });
    }

    // Version tag
    DrawText("v0.1.0 - Pre-Alpha", 10, SCREEN_HEIGHT - 24, 16,
             { 80, 25, 10, (unsigned char)(140 * fadeAlpha) });
}