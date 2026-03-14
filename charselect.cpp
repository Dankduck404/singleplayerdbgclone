#include "charselect.h"
#include "constants.h"

// ── Layout constants ──────────────────────────────────────────────────────────
// Screen: 948 x 915
// 3 boxes × 240px + 2 gaps × 60px + 2 margins × 54px = 948px exactly
static constexpr float BOX_W      = 240.0f;
static constexpr float BOX_H      = 300.0f;
static constexpr float GAP        = 60.0f;
static constexpr float MARGIN_X   = 54.0f;
static constexpr float BOX_TOP_Y  = 260.0f;   // vertical start of boxes

// Label style (simulates H3 bold by drawing text twice, offset 1px)
static constexpr int   LABEL_SIZE = 26;

// ── Constructor ───────────────────────────────────────────────────────────────
CharSelect::CharSelect() {
    for (int i = 0; i < 3; i++) {
        boxes[i]   = { MARGIN_X + i * (BOX_W + GAP), BOX_TOP_Y, BOX_W, BOX_H };
        hovered[i] = false;
    }
}

// ── Update ────────────────────────────────────────────────────────────────────
void CharSelect::update(float delta, GameState& state) {
    (void)delta;
    Vector2 mouse = GetMousePosition();

    for (int i = 0; i < 3; i++) {
        hovered[i] = CheckCollisionPointRec(mouse, boxes[i]);
        if (hovered[i] && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            state = GameState::POINT_SELECT;
        }
    }

    // Keyboard navigation
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
        state = GameState::MAIN_MENU;
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
        state = GameState::POINT_SELECT;
}

// ── Draw helpers ──────────────────────────────────────────────────────────────
static void drawBoldText(const char* text, int x, int y, int size, Color color) {
    // Simulate bold by drawing the text four times with 1px offsets, then on top
    DrawText(text, x + 1, y,     size, { 0, 0, 0, 60 });
    DrawText(text, x,     y + 1, size, { 0, 0, 0, 60 });
    DrawText(text, x + 1, y + 1, size, { 0, 0, 0, 60 });
    DrawText(text, x,     y,     size, color);
    DrawText(text, x + 1, y,     size, color); // extra pass = visually thicker
}

void CharSelect::drawTitle() const {
    const char* title = "Choose Your Class";
    int size  = 48;
    int textW = MeasureText(title, size);
    int x     = (SCREEN_WIDTH - textW) / 2;
    int y     = 80;

    // Glow shadow
    DrawText(title, x - 2, y - 2, size, { 180, 30, 0, 50 });
    DrawText(title, x + 2, y + 2, size, { 180, 30, 0, 50 });
    // Main
    DrawText(title, x, y, size, { 255, 100, 30, 255 });

    // Divider
    int divW = 500;
    DrawLine((SCREEN_WIDTH - divW) / 2, y + size + 8,
             (SCREEN_WIDTH + divW) / 2, y + size + 8,
             { 180, 50, 10, 160 });
}

void CharSelect::drawBackground() const {
    // Match the main menu procedural sky so the transition feels seamless
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        float t = (float)y / (float)SCREEN_HEIGHT;
        unsigned char r = (unsigned char)(60  + (18  - 60)  * t);
        unsigned char g = (unsigned char)(8   + (5   - 8)   * t);
        unsigned char b = (unsigned char)(8   + (5   - 8)   * t);
        DrawLine(0, y, SCREEN_WIDTH, y, { r, g, b, 255 });
    }
    // Lava horizon glow
    for (int y = (int)(SCREEN_HEIGHT * 0.65f); y < SCREEN_HEIGHT; y++) {
        float t = (float)(y - SCREEN_HEIGHT * 0.65f) / (SCREEN_HEIGHT * 0.35f);
        DrawLine(0, y, SCREEN_WIDTH, y,
            { (unsigned char)(30 + 80 * t), (unsigned char)(3 + 10 * t), 3,
              (unsigned char)(80 * t) });
    }
}

void CharSelect::drawBoxes() const {
    for (int i = 0; i < 3; i++) {
        const Rectangle& r = boxes[i];
        bool hov = hovered[i];

        // Box fill
        Color fill = hov
            ? Color{ 120, 30,  5, 210 }
            : Color{  30,  8,  5, 190 };
        DrawRectangleRec(r, fill);

        // Border
        Color border = hov
            ? Color{ 255, 100, 20, 255 }
            : Color{ 140,  45, 10, 200 };
        DrawRectangleLinesEx(r, hov ? 2.0f : 1.0f, border);

        // Inner top highlight
        DrawLine((int)r.x + 2, (int)r.y + 2,
                 (int)(r.x + r.width) - 2, (int)r.y + 2,
                 { 255, 140, 60, 35 });

        // Placeholder icon area (upper 60% of box)
        float iconPad  = 16.0f;
        Rectangle icon = { r.x + iconPad, r.y + iconPad,
                           r.width - iconPad * 2, r.height * 0.6f - iconPad };
        DrawRectangleLinesEx(icon, 1.0f, { 180, 60, 15, 80 });
        const char* ph = "[Sprite]";
        int phW = MeasureText(ph, 18);
        DrawText(ph,
            (int)(icon.x + (icon.width  - phW) / 2),
            (int)(icon.y + (icon.height - 18)  / 2),
            18, { 140, 50, 15, 120 });

        // ── H3 Bold label beneath box ────────────────────────────────────────
        const char* label = LABELS[i];
        int labelW = MeasureText(label, LABEL_SIZE);
        int labelX = (int)(r.x + (r.width - labelW) / 2);
        int labelY = (int)(r.y + r.height + 18);

        drawBoldText(label, labelX, labelY, LABEL_SIZE, { 230, 110, 30, 255 });
    }
}

void CharSelect::draw() const {
    drawBackground();
    drawTitle();
    drawBoxes();

    // Navigation hints
    DrawText("ESC / A / \x11 — Back to Menu     D / \x10 — Next", 14, SCREEN_HEIGHT - 26, 16, { 80, 25, 10, 140 });
}
