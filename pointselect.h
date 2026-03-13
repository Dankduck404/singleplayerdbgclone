#pragma once
#include "raylib.h"
#include "gamestate.h"
#include <array>

// ─────────────────────────────────────────────
//  Data types (kept in header so main.cpp can
//  read confirmed character data later)
// ─────────────────────────────────────────────
struct Stat {
    const char* name;
    int         value;
    const char* description;
    const char* sizeNote;
};

struct CharData {
    static constexpr int TOTAL_POINTS = 5;

    std::array<Stat, 9> stats = {{
        { "Strength",     0, "Raw physical power.\nAffects melee damage\nand carry capacity.",     "Large: +str" },
        { "Endurance",    0, "Stamina and toughness.\nAffects max health\nand fatigue resist.",     "Large: +end\nSmall: -end" },
        { "Speed",        0, "Movement and reaction.\nAffects dodge chance\nand turn order.",       "Small: +spd\nLarge: -spd" },
        { "Force",        0, "Ki output pressure.\nScales blast size\nand knockback.",              "—" },
        { "Resistance",   0, "Damage mitigation.\nReduces incoming\nphysical hits.",                "Large: +res" },
        { "Offense",      0, "Combat technique.\nBoosts hit accuracy\nand crit rate.",              "Small: +off" },
        { "Defense",      0, "Reactive guarding.\nLowers damage taken\nfrom skilled attacks.",      "Small: +def" },
        { "Regeneration", 0, "Natural healing rate.\nHealth recovered\nbetween encounters.",        "—" },
        { "Recovery",     0, "Bounce-back speed.\nReduces stun duration\nand ki recharge delay.",   "—" },
    }};

    int  pointsLeft   = TOTAL_POINTS;
    int  selectedStat = -1;
    bool confirmed    = false;

    char playerName[64] = "";
    bool nameActive     = false;

    int  skinTone   = 0;
    int  hairStyle  = 0;
    bool isMale     = true;

    bool isLarge    = false;
    bool isSmall    = false;

    void reset() {
        for (auto& s : stats) s.value = 0;
        pointsLeft   = TOTAL_POINTS;
        selectedStat = -1;
        confirmed    = false;
        playerName[0] = '\0';
        nameActive   = false;
        skinTone = hairStyle = 0;
        isMale   = true;
        isLarge  = isSmall = false;
    }
};

// ─────────────────────────────────────────────
//  Screen class
// ─────────────────────────────────────────────
class PointSelect {
public:
    PointSelect();

    void update(float delta, GameState& state);
    void draw()  const;

    const CharData& data() const { return d; }

private:
    CharData d;

    // ── helpers ──────────────────────────────
    void handleNameInput();
    void handleStatClicks();
    void handleDiceClick();
    void updateSize();

    void drawBackground()  const;
    void drawTipsAndPoints() const;
    void drawStatPanel()   const;
    void drawPortrait()    const;
    void drawDescPanel()   const;
    void drawDiceArea()    const;
    bool drawConfirmButton() const;
    void drawMultiline(const char* text, int x, int y, int fs, int ls, Color c) const;
    bool button(Rectangle r, const char* label, int fs = 18) const;
    void box(int x, int y, int w, int h, Color fill, Color border, float thick = 1.5f) const;
};
