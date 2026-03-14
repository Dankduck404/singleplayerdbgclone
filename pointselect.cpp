// NOTE: Player must click "New Game" to start character creation.
// Using A/D (or arrow keys) only cycles the main menu selection — it does NOT
// confirm a character pick. Skipping "New Game" leaves character state uninitialized,
// which will confuse downstream logic that checks whether a character has been chosen.
#include "pointselect.h"
#include "constants.h"
#include <cstring>

// ─────────────────────────────────────────────
//  Layout (fits within 948 × 915 window)
// ─────────────────────────────────────────────
static constexpr int PANEL_X   = 20,  PANEL_Y   = 60;
static constexpr int LABEL_W   = 130, CTRL_W    = 150;
static constexpr int ROW_H     = 68,  BTN_SIZE  = 28;

static constexpr int PORT_X    = 310, PORT_Y    = 20;
static constexpr int PORT_W    = 290, PORT_H    = 220;

static constexpr int DESC_X    = 310, DESC_Y    = 310;
static constexpr int DESC_W    = 290, DESC_H    = 350;

static constexpr int DICE_X    = 640, DICE_Y    = 20;
static constexpr int DICE_W    = 280, DICE_H    = 220;

static constexpr int CONFIRM_X = 660, CONFIRM_Y = 645;
static constexpr int CONFIRM_W = 240, CONFIRM_H = 52;

// ── Planet Vegeta palette (matches mainmenu) ──────────────────
static constexpr Color CLR_BG          = {  18,   5,   5, 255 }; // near-black bg
static constexpr Color CLR_PANEL       = {  30,   8,   5, 255 }; // dark red panel fill
static constexpr Color CLR_PANEL_SEL   = {  80,  20,   5, 255 }; // selected-row highlight
static constexpr Color CLR_PANEL_HEAD  = {  25,   6,   4, 255 }; // header strip fill
static constexpr Color CLR_PANEL_FOOT  = {  22,   5,   4, 255 }; // footer strip fill
static constexpr Color CLR_BORDER      = { 140,  45,  10, 255 }; // normal border
static constexpr Color CLR_BORDER_HOV  = { 255, 100,  20, 255 }; // hovered border
static constexpr Color CLR_TEXT        = { 255, 160,  60, 255 }; // primary text
static constexpr Color CLR_TEXT_DIM    = { 180,  70,  20, 255 }; // secondary text
static constexpr Color CLR_TEXT_MUTED  = { 100,  30,  10, 200 }; // placeholder / muted
static constexpr Color CLR_BTN_NORM    = {  35,   8,   5, 255 }; // button fill normal
static constexpr Color CLR_BTN_HOV     = { 100,  28,   8, 255 }; // button fill hovered
static constexpr Color CLR_CONFIRM_OFF = {  20,   5,   5, 255 }; // confirm fill (not ready)
static constexpr Color CLR_POINTS_OK   = { 255, 120,  30, 255 }; // points remaining
static constexpr Color CLR_POINTS_ZERO = { 220,  40,  10, 255 }; // points exhausted
static constexpr Color CLR_SHADOW      = {   0,   0,   0, 100 }; // text drop shadow
static constexpr Color CLR_HIGHLIGHT   = { 255, 140,  60,  40 }; // inner highlight line

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
PointSelect::PointSelect() {
    d.reset();
}

// ─────────────────────────────────────────────
//  Update
// ─────────────────────────────────────────────
void PointSelect::update(float delta, GameState& state) {
    (void)delta;

    // A / Left / ESC → back to class selection
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        d.reset();
        state = GameState::CHARACTER_SELECT;
        return;
    }

    // D / Right → confirm (only when all points spent)
    if ((IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) && d.pointsLeft == 0) {
        d.confirmed = true;
        state = GameState::GAMEPLAY;
        return;
    }

    handleNameInput();
    handleStatClicks();
    handleDiceClick();
    updateSize();

    // Confirm
    if (d.pointsLeft == 0) {
        Rectangle r = { (float)CONFIRM_X, (float)CONFIRM_Y,
                        (float)CONFIRM_W,  (float)CONFIRM_H };
        if (CheckCollisionPointRec(GetMousePosition(), r)
            && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            d.confirmed = true;
            state = GameState::GAMEPLAY;   // next screen — wire up later
        }
    }
}

// ─────────────────────────────────────────────
//  Input helpers
// ─────────────────────────────────────────────
void PointSelect::handleNameInput() {
    Rectangle nameBox = { (float)PORT_X, (float)PORT_Y, (float)PORT_W, 32 };
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        d.nameActive = CheckCollisionPointRec(GetMousePosition(), nameBox);

    if (!d.nameActive) return;

    int key = GetCharPressed();
    int len  = (int)strlen(d.playerName);
    while (key > 0) {
        if (key >= 32 && key <= 125 && len < 62) {
            d.playerName[len++] = (char)key;
            d.playerName[len]   = '\0';
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && len > 0)
        d.playerName[len - 1] = '\0';
}

void PointSelect::handleStatClicks() {
    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < (int)d.stats.size(); ++i) {
        int y  = PANEL_Y + i * ROW_H;
        int cx = PANEL_X + LABEL_W;

        // Label cell → select/deselect
        Rectangle labelCell = { (float)PANEL_X, (float)y, (float)LABEL_W, (float)ROW_H };
        if (CheckCollisionPointRec(mouse, labelCell)
            && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            d.selectedStat = (d.selectedStat == i) ? -1 : i;

        // + button
        Rectangle plusBtn = {
            (float)(cx + 12),
            (float)(y + (ROW_H - BTN_SIZE) / 2),
            (float)BTN_SIZE, (float)BTN_SIZE
        };
        if (CheckCollisionPointRec(mouse, plusBtn)
            && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)
            && d.pointsLeft > 0) {
            d.stats[i].value++;
            d.pointsLeft--;
        }

        // – button
        Rectangle minusBtn = {
            (float)(cx + CTRL_W - 12 - BTN_SIZE),
            (float)(y + (ROW_H - BTN_SIZE) / 2),
            (float)BTN_SIZE, (float)BTN_SIZE
        };
        if (CheckCollisionPointRec(mouse, minusBtn)
            && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)
            && d.stats[i].value > -1) {
            d.stats[i].value--;
            d.pointsLeft++;
        }
    }
}

void PointSelect::handleDiceClick() {
    Rectangle diceHit = { (float)(DICE_X + 30), (float)(DICE_Y + 30), 120, 100 };
    if (CheckCollisionPointRec(GetMousePosition(), diceHit)
        && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        d.isMale    = (GetRandomValue(0, 1) == 1);
        d.hairStyle = GetRandomValue(0, 7);
        d.skinTone  = GetRandomValue(0, 3);
    }
}

void PointSelect::updateSize() {
    int heavy = d.stats[0].value + d.stats[1].value + d.stats[4].value; // str+end+res
    int light = d.stats[2].value + d.stats[5].value + d.stats[6].value; // spd+off+def
    d.isLarge = (heavy > light);
    d.isSmall = (light > heavy);
}

// ─────────────────────────────────────────────
//  Draw helpers
// ─────────────────────────────────────────────
void PointSelect::box(int x, int y, int w, int h, Color fill, Color border, float thick) const {
    DrawRectangle(x, y, w, h, fill);
    DrawRectangleLinesEx({ (float)x, (float)y, (float)w, (float)h }, thick, border);
}

bool PointSelect::button(Rectangle r, const char* label, int fs) const {
    Vector2 mouse   = GetMousePosition();
    bool    hovered = CheckCollisionPointRec(mouse, r);
    DrawRectangleRec(r, hovered ? CLR_BTN_HOV : CLR_BTN_NORM);
    DrawRectangleLinesEx(r, 1.5f, hovered ? CLR_BORDER_HOV : CLR_BORDER);
    int tw = MeasureText(label, fs);
    DrawText(label, (int)(r.x + (r.width - tw) / 2),
                    (int)(r.y + (r.height - fs) / 2), fs, CLR_TEXT);
    return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void PointSelect::drawMultiline(const char* text, int x, int y, int fs, int ls, Color c) const {
    const char* p   = text;
    char line[128]  = {};
    int  lineY      = y, i = 0;
    while (*p) {
        if (*p == '\n' || *(p + 1) == '\0') {
            if (*(p + 1) == '\0' && *p != '\n') line[i++] = *p;
            line[i] = '\0';
            DrawText(line, x, lineY, fs, c);
            lineY += fs + ls;
            i = 0;
        } else {
            line[i++] = *p;
        }
        ++p;
    }
}

// ─────────────────────────────────────────────
//  Draw sections
// ─────────────────────────────────────────────
void PointSelect::drawBackground() const {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        float t = (float)y / (float)SCREEN_HEIGHT;
        unsigned char r = (unsigned char)(60  + (18  - 60)  * t);
        unsigned char g = (unsigned char)(8   + (5   - 8)   * t);
        unsigned char b = (unsigned char)(8   + (5   - 8)   * t);
        DrawLine(0, y, SCREEN_WIDTH, y, { r, g, b, 255 });
    }
}

void PointSelect::drawTipsAndPoints() const {
    box(PANEL_X, 10, 110, 38, CLR_PANEL, CLR_BORDER);
    DrawText("Tips Menu", PANEL_X + 8, 20, 16, CLR_TEXT_DIM);

    box(PANEL_X + 118, 10, 162, 38, CLR_PANEL, CLR_BORDER);
    const char* pts = TextFormat("%d Points Left", d.pointsLeft);
    DrawText(pts, PANEL_X + 128, 20, 16,
             d.pointsLeft > 0 ? CLR_POINTS_OK : CLR_POINTS_ZERO);
}

void PointSelect::drawStatPanel() const {
    for (int i = 0; i < (int)d.stats.size(); ++i) {
        int y  = PANEL_Y + i * ROW_H;
        int cx = PANEL_X + LABEL_W;
        Color rowFill = (d.selectedStat == i) ? CLR_PANEL_SEL : CLR_PANEL;

        // Label cell
        box(PANEL_X, y, LABEL_W, ROW_H, rowFill, CLR_BORDER);
        DrawText(d.stats[i].name, PANEL_X + 8, y + (ROW_H - 18) / 2, 17, CLR_TEXT);

        // Control cell
        box(cx, y, CTRL_W, ROW_H, rowFill, CLR_BORDER);

        // + / – buttons (draw-only — clicks handled in update)
        Rectangle plusBtn = {
            (float)(cx + 12),
            (float)(y + (ROW_H - BTN_SIZE) / 2),
            (float)BTN_SIZE, (float)BTN_SIZE
        };
        Rectangle minusBtn = {
            (float)(cx + CTRL_W - 12 - BTN_SIZE),
            (float)(y + (ROW_H - BTN_SIZE) / 2),
            (float)BTN_SIZE, (float)BTN_SIZE
        };
        button(plusBtn,  "+", 18);
        button(minusBtn, "-", 18);

        // Value centered between buttons
        const char* valStr = TextFormat("%d", d.stats[i].value);
        int tw = MeasureText(valStr, 18);
        DrawText(valStr,
            (int)(plusBtn.x + BTN_SIZE + (minusBtn.x - plusBtn.x - BTN_SIZE - tw) / 2),
            y + (ROW_H - 18) / 2, 18, CLR_TEXT);
    }
}

void PointSelect::drawPortrait() const {
    box(PORT_X, PORT_Y, PORT_W, PORT_H, CLR_PANEL, CLR_BORDER);

    // Name bar
    Rectangle nameBox = { (float)PORT_X, (float)PORT_Y, (float)PORT_W, 32 };
    DrawRectangleRec(nameBox, d.nameActive ? Color{ 50, 12, 5, 255 } : CLR_PANEL);
    DrawRectangleLinesEx(nameBox, 1.5f, CLR_BORDER);
    const char* display = (strlen(d.playerName) == 0) ? "Name (click to enter)" : d.playerName;
    Color nc = (strlen(d.playerName) == 0) ? CLR_TEXT_MUTED : CLR_TEXT;
    DrawText(display, PORT_X + 8, PORT_Y + 7, 15, nc);

    // Portrait placeholder
    box(PORT_X + 10, PORT_Y + 40, 130, 130, CLR_PANEL_HEAD, CLR_BORDER);
    DrawText("Portrait", PORT_X + 38, PORT_Y + 95, 14, CLR_TEXT_MUTED);

    // Hair
    int hairX = PORT_X + PORT_W - 90, hairY = PORT_Y + 40;
    box(hairX, hairY,       80, 60, CLR_PANEL, CLR_BORDER);
    DrawText("Hair",  hairX + 22, hairY + 22, 13, CLR_TEXT_DIM);

    // Class
    box(hairX, hairY + 68, 80, 40, CLR_PANEL, CLR_BORDER);
    DrawText("Class", hairX + 20, hairY + 80, 13, CLR_TEXT_DIM);

    // Skin swatches (keep actual skin colors)
    box(hairX,      hairY + 116, 40, 40, { 185, 115,  60, 255 }, CLR_BORDER);
    box(hairX + 40, hairY + 116, 40, 40, { 220, 170, 100, 255 }, CLR_BORDER);
    DrawText("Skin Color ^", PORT_X + 10, PORT_Y + PORT_H + 8, 13, CLR_TEXT_DIM);
}

void PointSelect::drawDescPanel() const {
    box(DESC_X, DESC_Y, DESC_W, DESC_H, CLR_PANEL, CLR_BORDER);
    box(DESC_X, DESC_Y, DESC_W, 38, CLR_PANEL_HEAD, CLR_BORDER);

    if (d.selectedStat < 0) {
        DrawText("Click a stat name", DESC_X + 10, DESC_Y + 10, 15, CLR_TEXT_DIM);
        DrawText("for it to appear",  DESC_X + 10, DESC_Y + 24, 13, CLR_TEXT_DIM);
    } else {
        DrawText(d.stats[d.selectedStat].name, DESC_X + 10, DESC_Y + 11, 16, CLR_TEXT);
        drawMultiline(d.stats[d.selectedStat].description,
                      DESC_X + 12, DESC_Y + 50, 15, 6, CLR_TEXT);
    }
    if (d.selectedStat < 0) {
        DrawText("Stat description that can", DESC_X + 12, DESC_Y + 60, 14, CLR_TEXT_MUTED);
        DrawText("affect Starting Size",      DESC_X + 12, DESC_Y + 80, 14, CLR_TEXT_MUTED);
    }

    // Size footer
    int footerY = DESC_Y + DESC_H - 60;
    box(DESC_X, footerY, DESC_W, 60, CLR_PANEL_FOOT, CLR_BORDER);
    DrawText("Size", DESC_X + 10, footerY + 8, 14, CLR_TEXT_DIM);
    const char* sizeText = (d.selectedStat >= 0)
        ? d.stats[d.selectedStat].sizeNote
        : "Large: +str, +end, -spd\nSmall: -str, -end, +off, +def";
    drawMultiline(sizeText, DESC_X + 60, footerY + 6, 12, 4, CLR_TEXT_DIM);
}

void PointSelect::drawDiceArea() const {
    box(DICE_X, DICE_Y, DICE_W, DICE_H, CLR_PANEL, CLR_BORDER);

    int dieX = DICE_X + 30, dieY = DICE_Y + 30;

    // Left die (4-face)
    box(dieX, dieY, 70, 70, CLR_BTN_NORM, CLR_BORDER);
    DrawCircle(dieX + 18, dieY + 18, 6, CLR_TEXT);
    DrawCircle(dieX + 52, dieY + 18, 6, CLR_TEXT);
    DrawCircle(dieX + 18, dieY + 52, 6, CLR_TEXT);
    DrawCircle(dieX + 52, dieY + 52, 6, CLR_TEXT);

    // Right die (3-face)
    box(dieX + 50, dieY + 30, 70, 70, CLR_BTN_NORM, CLR_BORDER);
    DrawCircle(dieX + 85, dieY + 45, 6, CLR_TEXT);
    DrawCircle(dieX + 95, dieY + 65, 6, CLR_TEXT);
    DrawCircle(dieX + 75, dieY + 85, 6, CLR_TEXT);

    DrawText("Dice Roll",          DICE_X + 160, DICE_Y + 50, 15, CLR_TEXT);
    DrawText("(Changes Sex,",      DICE_X + 160, DICE_Y + 72, 13, CLR_TEXT_DIM);
    DrawText("hair & skin tone)",  DICE_X + 160, DICE_Y + 90, 13, CLR_TEXT_DIM);

    // Hover highlight on dice hit area
    Rectangle diceHit = { (float)(dieX), (float)(dieY), 120, 100 };
    if (CheckCollisionPointRec(GetMousePosition(), diceHit))
        DrawRectangleLinesEx(diceHit, 2.0f, CLR_BORDER_HOV);
}

bool PointSelect::drawConfirmButton() const {
    bool ready = (d.pointsLeft == 0);
    Rectangle r = { (float)CONFIRM_X, (float)CONFIRM_Y,
                    (float)CONFIRM_W,  (float)CONFIRM_H };

    Color fill   = ready ? CLR_BTN_NORM  : CLR_CONFIRM_OFF;
    Color border = ready ? CLR_BORDER_HOV : Color{ 60, 20, 10, 255 };
    Color text   = ready ? CLR_TEXT       : Color{ 80, 30, 10, 255 };

    DrawRectangleRec(r, fill);
    DrawRectangleLinesEx(r, 2.0f, border);

    // Inner highlight on ready
    if (ready)
        DrawLine((int)r.x + 2, (int)r.y + 2,
                 (int)(r.x + r.width) - 2, (int)r.y + 2,
                 CLR_HIGHLIGHT);

    int tw = MeasureText("Confirm", 20);
    DrawText("Confirm",
             (int)(r.x + (r.width  - tw) / 2),
             (int)(r.y + (r.height - 20) / 2),
             20, text);
    return ready
        && CheckCollisionPointRec(GetMousePosition(), r)
        && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

// ─────────────────────────────────────────────
//  Draw (main)
// ─────────────────────────────────────────────
void PointSelect::draw() const {
    drawBackground();
    drawTipsAndPoints();
    drawStatPanel();
    drawPortrait();
    drawDescPanel();
    drawDiceArea();
    drawConfirmButton();

    // Build classification badge
    if (d.isLarge)
        DrawText("Build: Large",  CONFIRM_X, CONFIRM_Y - 28, 14, CLR_TEXT);
    else if (d.isSmall)
        DrawText("Build: Small",  CONFIRM_X, CONFIRM_Y - 28, 14, CLR_TEXT_DIM);
    else
        DrawText("Build: Medium", CONFIRM_X, CONFIRM_Y - 28, 14, CLR_TEXT_DIM);

    if (d.confirmed)
        DrawText("CONFIRMED!", SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2, 30, CLR_TEXT);

    DrawText("ESC / A / \x11 — Back to Class Select     D / \x10 — Confirm (when 0 pts left)", 14, SCREEN_HEIGHT - 26, 16, CLR_TEXT_DIM);
}
