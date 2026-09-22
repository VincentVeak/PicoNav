#include "globals.h"
#include <EEPROM.h>

// DD gem placeholder names + tooltip descriptions (<=13 chars per line)
static const char* ddGemNames[9] = {
  "PROSPECT", "BLOODLUST", "REVIVE", "RELIC", "GLASS",
  "GAMBLE", "OVERLOAD", "MARATHON", "CRIT"
};
static const char* ddGemDesc[9][3] = {
  { "RARE DROPS",    "+12.5%",        "DEF -25%"      },  // PROSPECT
  { "ATTACK +25%",   "MAX HP -33%",   ""              },  // BLOODLUST
  { "REVIVE ONCE",   "AT 50% HP",     ""              },  // REVIVE
  { "FIRST CARD IN", "EQUIPPED DECK", "IN NEXT DRAFT" },  // RELIC
  { "ATTACK +30%",   "NO PASSIVE",    "HP REGEN"      },  // GLASS
  { "DROP ROLLS",    "DOUBLED",       "ENEMY +1 AFL"  },  // GAMBLE
  { "MAX MERIT &",   "AFFLICT CAP",   "SET TO 9"      },  // OVERLOAD
  { "2X HP REGEN",   "BUT DAMAGE",    "DEALT -20%"    },  // MARATHON
  { "CRIT CHANCE",   "TO BOTH PICOS", "SET TO 25%"    }   // CRIT
};

// Deep Dungeon gem-select screen: highlighted gem (1 of 3), flat buy-in, balance.
void DrawDDGemSelect() {
  int gem = ddGemChoices[ddGemSel];

  tft.fillScreen(TFT_COLOR4);

  // Title box
  tft.fillRect(3, 3, (25 * 3), (9 * 3), TFT_COLOR1);
  tft.fillRect(0, 0, (25 * 3), (9 * 3), TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString("GEMS", 3, 3);

  // Credits balance (top-right) -- shifted 3px left so the drop shadow sits cleanly
  tft.fillRect(147, 3, (31 * 3), (9 * 3), TFT_COLOR1);
  tft.fillRect(144, 0, (31 * 3), (9 * 3), TFT_COLOR3);
  pushScaled(147, 6, 5, 5, smallIcons[11], TFT_BLACK);
  String displayCredits = String(playerCredits);
  while (displayCredits.length() < 4) displayCredits = "0" + displayCredits;
  tft.drawString(displayCredits, 165, 3);

  // Highlighted gem icon + two-line name (NAME / GEM), inventory-item style
  pushScaled(3, 45, 16, 16, gems[gem + 1], TFT_BLACK);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  tft.drawString(ddGemNames[gem], 54, 45);
  tft.drawString("GEM", 54, 69);

  // Buy-in cost box (flat 50) -- bottom-right so long gem names don't overwrite it
  tft.fillRect(165, 108, (25 * 3), (9 * 3), TFT_COLOR1);
  tft.fillRect(162, 105, (25 * 3), (9 * 3), TFT_COLOR3);
  pushScaled(165, 111, 5, 5, smallIcons[11], TFT_BLACK);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString("050", 183, 108);

  // Position indicators (3 dots, drop-shadow style; bright = highlighted)
  int startX = (240 - (3 * 21)) / 2;
  for (int i = 0; i < 3; i++) {
    if (i == ddGemSel) {
      tft.fillRect(startX + 3 + (i * 21), 117, 15, 15, TFT_COLOR3);
      tft.fillRect(startX + (i * 21), 114, 15, 15, TFT_COLOR1);
    } else {
      tft.fillRect(startX + 3 + (i * 21), 117, 15, 15, TFT_COLOR1);
      tft.fillRect(startX + (i * 21), 114, 15, 15, TFT_COLOR3);
    }
  }
}

// Hold-B tooltip for the highlighted gem -- mirrors the card/gem tooltip layout.
void DrawDDGemTooltip() {
  int gem = ddGemChoices[ddGemSel];
  tft.fillScreen(TFT_COLOR4);
  tft.fillRect(63, 6, 165, 51, TFT_COLOR1);
  tft.fillRect(60, 3, 165, 51, TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  pushScaled(6, 3, 16, 16, gems[gem + 1], TFT_BLACK);
  tft.drawString(ddGemNames[gem], 63, 6);
  tft.drawString("GEM", 63, 30);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  tft.drawString(ddGemDesc[gem][0], 3, 63);
  tft.drawString(ddGemDesc[gem][1], 3, 87);
  tft.drawString(ddGemDesc[gem][2], 3, 111);
}

// Partial redraw used when cycling gems -- only the icon/name band + dots, no full wipe.
void UpdateDDGemSelect() {
  int gem = ddGemChoices[ddGemSel];
  tft.fillRect(3, 45, 237, 48, TFT_COLOR4);
  pushScaled(3, 45, 16, 16, gems[gem + 1], TFT_BLACK);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  tft.drawString(ddGemNames[gem], 54, 45);
  tft.drawString("GEM", 54, 69);
  int startX = (240 - (3 * 21)) / 2;
  for (int i = 0; i < 3; i++) {
    if (i == ddGemSel) {
      tft.fillRect(startX + 3 + (i * 21), 117, 15, 15, TFT_COLOR3);
      tft.fillRect(startX + (i * 21), 114, 15, 15, TFT_COLOR1);
    } else {
      tft.fillRect(startX + 3 + (i * 21), 117, 15, 15, TFT_COLOR1);
      tft.fillRect(startX + (i * 21), 114, 15, 15, TFT_COLOR3);
    }
  }
}

// Enter-confirmation for the highlighted gem (YES/NO, default NO).
void DrawDDGemConfirm() {
  int gem = ddGemChoices[ddGemSel];
  tft.fillScreen(TFT_COLOR4);
  tft.fillRect(3, 3, (25 * 3), (9 * 3), TFT_COLOR1);
  tft.fillRect(0, 0, (25 * 3), (9 * 3), TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString("GEMS", 3, 3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  CenterText("ENTER WITH", 39);
  CenterText(String(ddGemNames[gem]) + "?", 63);
  // NO selected by default
  tft.fillRect(132, 99, 57, 27, TFT_COLOR1);
  tft.fillRect(129, 96, 57, 27, TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  tft.drawString("YES", 54, 99);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString("NO", 144, 99);
}

// Roll three distinct card choices, excluding guaranteed cards and prior picks.
void RollDraftChoices() {
  int start = 0;
  // RELIC gem: offer your equipped deck's first card in the first draft round.
  if (deepDungeonActive && activeGem == 3 && ddDraftRound == 0) {
    int relic = EEPROM.read(97 + equippedDeck * 10);
    if (relic >= 0 && relic < 56 && relic != 7 && relic != 12) {
      ddCardChoices[0] = relic;
      start = 1;
    }
  }
  for (int i = start; i < 3; i++) {
    int pick;
    bool dup;
    do {
      pick = random(56);
      dup = (pick == 7 || pick == 12);            // PIERCE and BLOCK are guaranteed
      for (int r = 0; r < ddDraftRound && !dup; r++)
        if (ddDraftPicks[r] == pick) dup = true;
      for (int j = 0; j < i && !dup; j++)
        if (ddCardChoices[j] == pick) dup = true;
    } while (dup);
    ddCardChoices[i] = pick;
  }
  ddCardSel = 0;
}

// Horizontal x positions of the three card frames.
static const int ddDraftX[3] = { 26, 97, 168 };

// Draw the pointer over the selected card, its name, and the 3 position squares.
static void DrawDDCardSelection() {
  int cx = ddDraftX[ddCardSel] + 21;
  tft.fillRect(cx - 9, 33, 21, 3, TFT_COLOR1);
  tft.fillRect(cx - 6, 36, 15, 3, TFT_COLOR1);
  tft.fillRect(cx - 3, 39, 9, 3, TFT_COLOR1);
  tft.fillRect(cx,     42, 3, 3, TFT_COLOR1);

  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  CenterText(cardList[ddCardChoices[ddCardSel]].name, 90);

  int startX = (240 - (3 * 21)) / 2;
  for (int i = 0; i < 3; i++) {
    if (i == ddCardSel) {
      tft.fillRect(startX + 3 + (i * 21), 117, 15, 15, TFT_COLOR3);
      tft.fillRect(startX + (i * 21), 114, 15, 15, TFT_COLOR1);
    } else {
      tft.fillRect(startX + 3 + (i * 21), 117, 15, 15, TFT_COLOR1);
      tft.fillRect(startX + (i * 21), 114, 15, 15, TFT_COLOR3);
    }
  }
}

// Card-draft screen: title + three framed cards + selected card's name.
void DrawDDCardDraft() {
  tft.fillScreen(TFT_COLOR4);

  // Title box
  tft.fillRect(3, 3, (43 * 3), (9 * 3), TFT_COLOR1);
  tft.fillRect(0, 0, (43 * 3), (9 * 3), TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString("DRAFT " + String(ddDraftRound + 1), 3, 3);

  // Three framed card sprites
  for (int i = 0; i < 3; i++) {
    pushScaled(ddDraftX[i], 42, 15, 15, abilityFrame[0], TFT_BLACK);
    pushScaled(ddDraftX[i] + 6, 48, 11, 11, cardSpriteArray[ddCardChoices[i]], TFT_BLACK);
  }

  DrawDDCardSelection();
}

// Partial redraw when cycling -- the cards stay put, only the selection moves.
void UpdateDDCardDraft() {
  for (int i = 0; i < 3; i++)
    tft.fillRect(ddDraftX[i] + 12, 33, 21, 12, TFT_COLOR4);  // clear each arrow slot
  tft.fillRect(0, 88, 240, 25, TFT_COLOR4);   // name band
  tft.fillRect(0, 114, 240, 21, TFT_COLOR4);  // square band
  DrawDDCardSelection();
}

// Hold-B tooltip for the highlighted card -- full detail with description.
void DrawDDCardDraftTooltip() {
  int c = ddCardChoices[ddCardSel];
  tft.fillScreen(TFT_COLOR4);
  pushScaled(3, 3, 15, 15, abilityFrame[0], TFT_BLACK);
  tft.fillRect(51, 3, 183, 27, TFT_COLOR3);
  tft.fillRect(234, 6, 3, 27, TFT_COLOR1);
  tft.fillRect(54, 30, 183, 3, TFT_COLOR1);
  pushScaled(9, 9, 11, 11, cardSpriteArray[c], TFT_BLACK);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString(cardList[c].name, 54, 6);
  tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
  if (cardList[c].damage < 100)
    tft.drawString(String(cardList[c].damage), 201, 36);
  else
    tft.drawString(String(cardList[c].damage), 183, 36);
  if (cardList[c].canReplace)
    pushScaled(102, 36, 5, 5, smallIcons[16], TFT_BLACK);
  if (cardList[c].priority)
    pushScaled(129, 36, 5, 5, smallIcons[15], TFT_BLACK);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  tft.drawString(cardList[c].desc1, 3, 63);
  tft.drawString(cardList[c].desc2, 3, 87);
  tft.drawString(cardList[c].desc3, 3, 111);
}

// DD merit names (shown on the bars) + tooltip descriptions (<=13 chars per line)
static const char* ddMeritNames[DD_MERIT_COUNT] = {
  "WINDRIDER", "INFLOW", "SCORCHING", "TECTONICS", "HIGH VOLTAGE",
  "FIRE AMP", "WATER AMP", "WIND AMP", "EARTH AMP", "FIRE SURGE",
  "WATER SURGE", "WIND SURGE", "EARTH SURGE", "FIRE TD", "WATER TD",
  "WIND TD", "EARTH TD", "CONDUIT", "OVERCHARGE", "EXECUTE",
  "WHIPLASH", "BLOODRUSH", "SHARPEN", "BOLSTER", "TEMPER",
  "HASTE", "KEEN EYE", "LEECH", "DESPERATION", "SCHOLAR",
  "BERSERK", "GUTS", "TECHNICIAN", "SPEED BOOST", "SECOND WIND",
  "REGEN", "VITALITY", "SPIKES", "AMBUSH", "PHANTOM",
  "DEEP FREEZE", "WARD", "OMEN", "FROSTGUARD", "INTIMIDATE",
  "REFORGE", "STACKED DECK", "SCAVENGE", "BOUNTY", "REPRIEVE",
  "MOXIE", "PURGE", "WINDFALL"
};
static const char* ddMeritDesc[DD_MERIT_COUNT][3] = {
  { "WIND MOVES",   "GAIN MOVE",    "PRIORITY"    },  // WINDRIDER
  { "WATER MOVES",  "HEAL 20% OF",  "DAMAGE DEALT" },  // INFLOW
  { "FIRE MOVES",   "+25% IF FOE",  "HAS {FI}"    },  // SCORCHING
  { "EARTH MOVES",  "+10% PER",     "{EA} COUNTER" },  // TECTONICS
  { "LIGHTNING",    "MOVES ADD +1", "{LI} COUNTER" },  // HIGH VOLTAGE
  { "FIRE MOVES",   "DEAL +10%",    "DAMAGE"      },  // FIRE AMP
  { "WATER MOVES",  "DEAL +10%",    "DAMAGE"      },  // WATER AMP
  { "WIND MOVES",   "DEAL +10%",    "DAMAGE"      },  // WIND AMP
  { "EARTH MOVES",  "DEAL +10%",    "DAMAGE"      },  // EARTH AMP
  { "FIRE MOVES",   "+15% CRIT",    "CHANCE"      },  // FIRE SURGE
  { "WATER MOVES",  "+15% CRIT",    "CHANCE"      },  // WATER SURGE
  { "WIND MOVES",   "+15% CRIT",    "CHANCE"      },  // WIND SURGE
  { "EARTH MOVES",  "+15% CRIT",    "CHANCE"      },  // EARTH SURGE
  { "FIRE MOVES",   "GAIN +2",      "TRUE DMG"    },  // FIRE TD
  { "WATER MOVES",  "GAIN +2",      "TRUE DMG"    },  // WATER TD
  { "WIND MOVES",   "GAIN +2",      "TRUE DMG"    },  // WIND TD
  { "EARTH MOVES",  "GAIN +2",      "TRUE DMG"    },  // EARTH TD
  { "STACKING",     "MOVES ADD +1", "EXTRA STACK" },  // CONDUIT
  { "FIRST STACK",  "MOVE APPLIES", "MAX STACKS"  },  // OVERCHARGE
  { "+50% DAMAGE",  "TO FOES",      "BELOW 25% HP" },  // EXECUTE
  { "AFTER A CRIT", "NEXT MOVE",    "GETS PRIORITY" },  // WHIPLASH
  { "CRITS HEAL",   "10% OF",       "MAX HP"      },  // BLOODRUSH
  { "ALL MOVES",    "GAIN +2",      "TRUE DAMAGE" },  // SHARPEN
  { "ATTACK AND",   "DEFENSE UP",   "BY 10%"      },  // BOLSTER
  { "ATTACK AND",   "DEFENSE UP",   "BY 25%"      },  // TEMPER
  { "SPEED",        "RAISED BY",    "20%"         },  // HASTE
  { "CRIT CHANCE",  "RAISED BY",    "20%"         },  // KEEN EYE
  { "ATTACKS HEAL", "20% OF",       "DAMAGE DEALT" },  // LEECH
  { "+1% ATTACK",   "PER 1% HP",    "MISSING"     },  // DESPERATION
  { "STATS +1%",    "PER CARD",     "MAX +25%"    },  // SCHOLAR
  { "UNDER 50% HP", "+25% CRIT &",  "DAMAGE"      },  // BERSERK
  { "+15% ATTACK",  "COUNTER IS",   "ON SELF"     },  // GUTS
  { "MOVES UNDER",  "50 DMG DEAL",  "+50%"        },  // TECHNICIAN
  { "SPEED RISES",  "ONE STAGE",    "EACH TURN"   },  // SPEED BOOST
  { "HEAL 25% HP",  "AFTER EACH",   "BATTLE"      },  // SECOND WIND
  { "HEAL 10% HP",  "AT THE END",   "OF EACH TURN" },  // REGEN
  { "MAX HP",       "RAISED BY",    "25%"         },  // VITALITY
  { "REFLECT 25%",  "OF DAMAGE",    "TAKEN"       },  // SPIKES
  { "ENEMY STARTS", "EACH FIGHT",   "AT 75% HP"   },  // AMBUSH
  { "START WITH",   "{MI} NULLIFY", "NEXT HIT"    },  // PHANTOM
  { "ENEMY {IC}",   "SKIPS THEIR",  "FIRST TURN"  },  // DEEP FREEZE
  { "START WITH",   "{AG} BLOCKS",  "DMG UNDER 50" },  // WARD
  { "START WITH",   "{LS} FOR +25%","CRIT CHANCE" },  // OMEN
  { "START WITH",   "{IW} UP EACH", "BATTLE"      },  // FROSTGUARD
  { "LOWER ENEMY",  "ATTACK ONE",   "STAGE"       },  // INTIMIDATE
  { "START EACH",   "BATTLE WITH",  "RANDOM CARDS" },  // REFORGE
  { "FIRST CARD",   "USED IS NOT",  "SHUFFLED"    },  // STACKED DECK
  { "DROP RATE",    "INCREASED",    "BY 5%"       },  // SCAVENGE
  { "EARN BONUS",   "CREDITS ON",   "EACH WIN"    },  // BOUNTY
  { "ONCE HEAL TO", "FULL WHEN",    "BELOW 25%"   },  // REPRIEVE
  { "SURVIVE ONE",  "LETHAL HIT",   "AT 1 HP"     },  // MOXIE
  { "REMOVE RANDOM", "AFFLICTION",   "ON ENEMY" },  // PURGE
  { "NEXT BATTLE",  "DROPS ARE",    "GUARANTEED"  },  // WINDFALL
};

// Draft rarity per merit (index matches the Merit enum / ddMeritNames).
// R_NONE = not implemented yet -> weight 0, so it cannot be drafted.
static const uint8_t ddMeritRarity[DD_MERIT_COUNT] = {
  R_UNCOMMON, // 0  WINDRIDER
  R_UNCOMMON, // 1  INFLOW
  R_UNCOMMON, // 2  SCORCHING
  R_UNCOMMON, // 3  TECTONICS
  R_COMMON,   // 4  HIGH VOLTAGE
  R_COMMON,   // 5  FIRE AMP
  R_COMMON,   // 6  WATER AMP
  R_COMMON,   // 7  WIND AMP
  R_COMMON,   // 8  EARTH AMP
  R_COMMON,   // 9  FIRE SURGE
  R_COMMON,   // 10 WATER SURGE
  R_COMMON,   // 11 WIND SURGE
  R_COMMON,   // 12 EARTH SURGE
  R_COMMON,   // 13 FIRE TD
  R_COMMON,   // 14 WATER TD
  R_COMMON,   // 15 WIND TD
  R_COMMON,   // 16 EARTH TD
  R_UNCOMMON, // 17 CONDUIT
  R_RARE,     // 18 OVERCHARGE
  R_RARE,     // 19 EXECUTE
  R_UNCOMMON, // 20 WHIPLASH
  R_UNCOMMON, // 21 BLOODRUSH
  R_COMMON,   // 22 SHARPEN
  R_COMMON,   // 23 BOLSTER
  R_RARE,     // 24 TEMPER
  R_UNCOMMON, // 25 HASTE
  R_UNCOMMON, // 26 KEEN EYE
  R_UNCOMMON, // 27 LEECH
  R_RARE,     // 28 DESPERATION
  R_RARE,     // 29 SCHOLAR
  R_RARE,     // 30 BERSERK
  R_COMMON,   // 31 GUTS
  R_UNCOMMON, // 32 TECHNICIAN
  R_UNCOMMON, // 33 SPEED BOOST
  R_RARE,     // 34 SECOND WIND
  R_UNCOMMON, // 35 REGEN
  R_RARE,     // 36 VITALITY
  R_UNCOMMON, // 37 SPIKES
  R_UNCOMMON, // 38 AMBUSH
  R_UNCOMMON, // 39 PHANTOM
  R_UNCOMMON, // 40 DEEP FREEZE
  R_UNCOMMON, // 41 WARD
  R_UNCOMMON, // 42 OMEN
  R_UNCOMMON, // 43 FROSTGUARD
  R_UNCOMMON, // 44 INTIMIDATE
  R_UNCOMMON, // 45 REFORGE
  R_UNCOMMON, // 46 STACKED DECK
  R_COMMON,   // 47 SCAVENGE
  R_COMMON,   // 48 BOUNTY
  R_RARE,     // 49 REPRIEVE
  R_RARE,     // 50 MOXIE
  R_RARE,     // 51 PURGE
  R_RARE,     // 52 WINDFALL
};

// Draft weight per rarity tier (index by MeritRarity). Tune here.
static const int meritRarityWeight[4] = { 0, 100, 35, 10 };  // NONE, COMMON, UNCOMMON, RARE

// Weighted pick of one merit id, skipping up to two already-chosen ids.
static int PickWeightedMerit(int excludeA, int excludeB) {
  int total = 0;
  for (int i = 0; i < DD_MERIT_COUNT; i++) {
    if (i == excludeA || i == excludeB) continue;
    total += meritRarityWeight[ddMeritRarity[i]];
  }
  if (total <= 0) {  // pool exhausted (defensive) -- return any valid index
    for (int i = 0; i < DD_MERIT_COUNT; i++)
      if (i != excludeA && i != excludeB) return i;
    return 0;
  }
  int roll = random(total);
  int cumulative = 0;
  for (int i = 0; i < DD_MERIT_COUNT; i++) {
    if (i == excludeA || i == excludeB) continue;
    cumulative += meritRarityWeight[ddMeritRarity[i]];
    if (roll < cumulative) return i;
  }
  return 0;
}

// Fill ddMeritChoices[3] with three distinct, rarity-weighted picks.
void RollMeritDraft() {
  ddMeritChoices[0] = PickWeightedMerit(-1, -1);
  ddMeritChoices[1] = PickWeightedMerit(ddMeritChoices[0], -1);
  ddMeritChoices[2] = PickWeightedMerit(ddMeritChoices[0], ddMeritChoices[1]);
}

void RemoveMerit(int id) {
  for (int i = 0; i < activeMeritCount; i++) {
    if (activeMerits[i] == id) {
      for (int j = i; j < activeMeritCount - 1; j++) activeMerits[j] = activeMerits[j + 1];
      activeMerits[--activeMeritCount] = -1;
      EEPROM.write(345, (byte)activeMeritCount);
      for (int k = 0; k < 9; k++) EEPROM.write(346 + k, (byte)(activeMerits[k] & 0xFF));
      EEPROM.commit();
      return;
    }
  }
}

static const char* ddAfflictionNames[DD_AFFLICTION_COUNT] = {
  "FEROCITY", "STALWART", "VIGOR", "FRENZY", "RUSH",
  "ENRAGE", "RENEW", "DRAIN", "DISPEL", "TOXIC",
  "SLOTH", "RECOIL", "BLIGHT", "DREAD", "VOLATILE"
};
static const char* ddAfflictionDesc[DD_AFFLICTION_COUNT][3] = {
  { "ENEMY ATTACK",  "RISES +10%",    "EACH LEVEL"  },  // FEROCITY
  { "ENEMY DEFENSE", "RISES +10%",    "EACH LEVEL"  },  // STALWART
  { "ENEMY MAX HP",  "RISES +3HP",    "EACH LEVEL"  },  // VIGOR
  { "ENEMY CRIT",    "RISES +10%",    "EACH LEVEL"  },  // FRENZY
  { "ENEMY SPEED",   "RISES +10%",    "EACH LEVEL"  },  // RUSH
  { "ENEMY GAINS",   "ATTACK STAGE",  "OVER TIME"   },  // ENRAGE
  { "ENEMY HEALS",   "+5% EACH LV",   "PER TURN"    },  // RENEW
  { "ENEMY HEALS",  "+10% OF DMG",   "EACH LEVEL"  },  // DRAIN
  { "REMOVES YOUR",  "COUNTER MORE",  "EACH LEVEL"  },  // DISPEL
  { "LOSE 4% HP",    "EACH TURN",     "+4% PER LV"  },  // TOXIC
  { "YOUR SPEED",    "DROPS +1",      "EACH LEVEL"  },  // SLOTH
  { "DAMAGE DEALT",  "HURTS YOU",     "+10% EACH LV" },  // RECOIL
  { "YOUR HEALING",  "CUT BY 25%",    "EACH LEVEL"  },  // BLIGHT
  { "YOUR CRIT",     "DROPS -10%",    "EACH LEVEL"  },  // DREAD
  { "ON FOE DEATH",  "TAKE +5% HP",   "EACH LEVEL"  },  // VOLATILE
};

// Boss reward: add a new affliction, or (if at the cap) level up an existing one to 3 max.
// Sets bossAfflictionMsg for the popup.
void GrantBossAffliction() {
  if (activeAfflictionCount < GetAfflictionCap()) {
    int id;
    bool dup;
    int tries = 0;
    do {
      id = random(DD_AFFLICTION_COUNT);
      dup = false;
      for (int i = 0; i < activeAfflictionCount; i++)
        if (activeAfflictions[i] == id) dup = true;
    } while (dup && ++tries < 40);
    activeAfflictions[activeAfflictionCount] = id;
    activeAfflictionLevels[activeAfflictionCount] = 1;
    activeAfflictionCount++;
    bossAfflictionId = id;
    bossAfflictionLvl = 1;
    bossAfflictionMsg = String(ddAfflictionNames[id]);
  } else {
    int candidates[9];
    int n = 0;
    for (int i = 0; i < activeAfflictionCount; i++)
      if (activeAfflictionLevels[i] < 3) candidates[n++] = i;
    if (n > 0) {
      int pick = candidates[random(n)];
      activeAfflictionLevels[pick]++;
      bossAfflictionId = activeAfflictions[pick];
      bossAfflictionLvl = activeAfflictionLevels[pick];
      bossAfflictionMsg = String(ddAfflictionNames[activeAfflictions[pick]]) + " LV" + String(activeAfflictionLevels[pick]);
    } else {
      bossAfflictionMsg = "MAXED";
    }
  }
  bossAfflictionPopup = true;
}

void DrawDDMeritBar(int i) {
  int by = 33 + i * 36;
  uint16_t bg = (i == ddMeritSel) ? TFT_COLOR2 : TFT_COLOR3;
  uint16_t fg = (i == ddMeritSel) ? TFT_COLOR3 : TFT_COLOR1;
  tft.fillRect(0, by, 234, 27, bg);
  tft.setTextColor(fg, bg);
  CenterText(ddMeritNames[ddMeritChoices[i]], by + 3);
}

// Merit-select screen: title + three full-width merit bars.
void DrawDDMeritSelect() {
  tft.fillScreen(TFT_COLOR4);
  tft.fillRect(3, 3, (37 * 3), (9 * 3), TFT_COLOR1);
  tft.fillRect(0, 0, (37 * 3), (9 * 3), TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString("MERITS", 3, 3);
  for (int i = 0; i < 3; i++) tft.fillRect(3, (33 + i * 36) + 3, 234, 27, TFT_COLOR1);
  for (int i = 0; i < 3; i++) DrawDDMeritBar(i);
}

// Redraw the three merit bars without clearing the screen.
void UpdateDDMeritSelect() {
  for (int i = 0; i < 3; i++) DrawDDMeritBar(i);
}

// Boss reward: popup naming the affliction that was added or leveled up.
void DrawBossAfflictionPopup() {
  tft.fillScreen(TFT_COLOR4);
  // Title bar sized to the text, with drop shadow (matches the affliction menu)
  int w = tft.textWidth("AFFLICTION") + 3;
  tft.fillRect(3, 3, w, (9 * 3), TFT_COLOR1);
  tft.fillRect(0, 0, w, (9 * 3), TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString("AFFLICTION", 3, 3);
  // Affliction name in a full-width bar with drop shadow (matches the merit draft)
  tft.fillRect(3, 57, 234, 27, TFT_COLOR1);
  tft.fillRect(0, 54, 234, 27, TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  CenterText(bossAfflictionMsg, 57);
}

// Hold-B tooltip for the highlighted merit (inventory-item style, no icon).
void DrawDDMeritTooltip() {
  int m = ddMeritChoices[ddMeritSel];
  tft.fillScreen(TFT_COLOR4);
  tft.fillRect(3, 12, 234, 27, TFT_COLOR1);
  tft.fillRect(0, 9, 234, 27, TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString(ddMeritNames[m], 6, 12);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  tft.drawString(ddMeritDesc[m][0], 3, 54);
  tft.drawString(ddMeritDesc[m][1], 3, 78);
  tft.drawString(ddMeritDesc[m][2], 3, 101);
}

// Status menu: merit list panel (3-row paged window; highlights focus when drilled).
void DrawMeritRows() {
  // Page 2+ (drilled past the first merit): 3-row windowed list, gem hidden.
  // Merit 0 shares page 1 with the gem, so the list window starts at merit 1.
  if (statDrill && statDrillSel >= 2) {
    int mi = statDrillSel - 1;
    int top = 1 + ((mi - 1) / 3) * 3;
    for (int r = 0; r < 3; r++) {
      int idx = top + r;
      int by = 33 + r * 33;
      if (idx >= activeMeritCount) {
        tft.fillRect(0, by, 240, 30, TFT_COLOR4);
        continue;
      }
      bool sel = idx == mi;
      uint16_t bg = sel ? TFT_COLOR2 : TFT_COLOR3;
      uint16_t fg = sel ? TFT_COLOR3 : TFT_COLOR1;
      tft.fillRect(3, by + 3, 234, 27, TFT_COLOR1);
      tft.fillRect(0, by, 234, 27, bg);
      tft.setTextColor(fg, bg);
      CenterText(ddMeritNames[activeMerits[idx]], by + 3);
    }
    return;
  }
  // Page 1: gem box (item 0) + first merit bar (item 1). Boxes redraw in place
  // with no background clear, so cycling the highlight here doesn't flicker.
  if (activeGem >= 0) {
    bool gemSel = statDrill && statDrillSel == 0;
    uint16_t gbg = gemSel ? TFT_COLOR2 : TFT_COLOR3;
    uint16_t gfg = gemSel ? TFT_COLOR3 : TFT_COLOR1;
    tft.fillRect(57, 39, 180, 51, TFT_COLOR1);
    tft.fillRect(54, 36, 180, 51, gbg);
    tft.setTextColor(gfg, gbg);
    tft.drawString(ddGemNames[activeGem], 57, 39);
    tft.drawString("GEM", 57, 63);
  }
  if (activeMeritCount == 0) {
    tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
    CenterText("NONE", 96);
    return;
  }
  bool mSel = statDrill && statDrillSel == 1;
  uint16_t mbg = mSel ? TFT_COLOR2 : TFT_COLOR3;
  uint16_t mfg = mSel ? TFT_COLOR3 : TFT_COLOR1;
  tft.fillRect(3, 96, 234, 27, TFT_COLOR1);
  tft.fillRect(0, 93, 234, 27, mbg);
  tft.setTextColor(mfg, mbg);
  CenterText(ddMeritNames[activeMerits[0]], 96);
}

// Draw the run's gem icon at (3,36). A spent REVIVE shows the no-gem icon.
void DrawRunGemIcon() {
  if (activeGem < 0) return;
  if (activeGem == 2 && ddReviveUsed)
    pushScaled(3, 36, 16, 16, gems[0], TFT_BLACK);
  else
    pushScaled(3, 36, 16, 16, gems[activeGem + 1], TFT_BLACK);
}

void DrawMeritScreen() {
  tft.fillScreen(TFT_COLOR4);
  tft.fillRect(3, 3, (37 * 3), (9 * 3), TFT_COLOR1);
  tft.fillRect(0, 0, (37 * 3), (9 * 3), TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString("MERITS", 3, 3);
  if (!(statDrill && statDrillSel >= 2)) DrawRunGemIcon();
  DrawMeritRows();
}

// Redraw only the content below the title (used on A-cycle page flips so the
// title bar never flashes). Clears the page area and restores the gem icon
// when flipping back to page 1.
void DrawMeritPage() {
  tft.fillRect(0, 28, 240, 107, TFT_COLOR4);
  if (!(statDrill && statDrillSel >= 2)) DrawRunGemIcon();
  DrawMeritRows();
}

// Tooltip for a gem by id (used by the status-menu drill-down).
// Over-cap discard: 3-row windowed list of candidates
// (current merits followed by the newly drafted one).
void DrawMeritDiscardRows() {
  int n = activeMeritCount + 1;
  int top = (ddDiscardSel / 3) * 3;
  for (int r = 0; r < 3; r++) {
    int idx = top + r;
    int by = 33 + r * 33;
    if (idx >= n) { tft.fillRect(0, by, 240, 30, TFT_COLOR4); continue; }
    bool sel = idx == ddDiscardSel;
    uint16_t bg = sel ? TFT_COLOR2 : TFT_COLOR3;
    uint16_t fg = sel ? TFT_COLOR3 : TFT_COLOR1;
    tft.fillRect(3, by + 3, 234, 27, TFT_COLOR1);
    tft.fillRect(0, by, 234, 27, bg);
    tft.setTextColor(fg, bg);
    int id = (idx < activeMeritCount) ? activeMerits[idx] : pendingMerit;
    CenterText(ddMeritNames[id], by + 3);
  }
}

void DrawMeritDiscard() {
  tft.fillScreen(TFT_COLOR4);
  int w = tft.textWidth("DISCARD ONE") + 3;
  tft.fillRect(3, 3, w, (9 * 3), TFT_COLOR1);
  tft.fillRect(0, 0, w, (9 * 3), TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString("DISCARD ONE", 3, 3);
  DrawMeritDiscardRows();
}

// YES/NO boxes, shared by the confirm draw and the A-toggle redraw.
void DrawDiscardYesNo() {
  tft.fillRect(45, 93, 66, 39, TFT_COLOR4);
  tft.fillRect(120, 93, 75, 39, TFT_COLOR4);
  if (ddDiscardConfirmYes) {
    tft.fillRect(54, 99, 57, 27, TFT_COLOR1);
    tft.fillRect(51, 96, 57, 27, TFT_COLOR3);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    tft.drawString("YES", 54, 99);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
    tft.drawString("NO", 144, 99);
  } else {
    tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
    tft.drawString("YES", 54, 99);
    tft.fillRect(132, 99, 57, 27, TFT_COLOR1);
    tft.fillRect(129, 96, 57, 27, TFT_COLOR3);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    tft.drawString("NO", 144, 99);
  }
}

void DrawMeritDiscardConfirm() {
  int id = (ddDiscardSel < activeMeritCount) ? activeMerits[ddDiscardSel] : pendingMerit;
  tft.fillScreen(TFT_COLOR4);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  CenterText("REMOVE", 24);
  tft.fillRect(3, 51, 234, 27, TFT_COLOR1);
  tft.fillRect(0, 48, 234, 27, TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  CenterText(ddMeritNames[id], 51);
  DrawDiscardYesNo();
}

void DrawGemTooltipById(int g) {
  tft.fillScreen(TFT_COLOR4);
  tft.fillRect(63, 6, 165, 51, TFT_COLOR1);
  tft.fillRect(60, 3, 165, 51, TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  pushScaled(6, 3, 16, 16, gems[g + 1], TFT_BLACK);
  tft.drawString(ddGemNames[g], 63, 6);
  tft.drawString("GEM", 63, 30);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  tft.drawString(ddGemDesc[g][0], 3, 63);
  tft.drawString(ddGemDesc[g][1], 3, 87);
  tft.drawString(ddGemDesc[g][2], 3, 111);
}

// Status menu: affliction list panel (shows level; empty for now).
void DrawAfflictionRows() {
  if (activeAfflictionCount == 0) {
    tft.fillRect(0, 33, 240, 102, TFT_COLOR4);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
    CenterText("NONE", 69);
    return;
  }
  int top = statDrill ? (statDrillSel / 3) * 3 : 0;
  for (int r = 0; r < 3; r++) {
    int idx = top + r;
    int by = 33 + r * 33;
    if (idx >= activeAfflictionCount) {
      tft.fillRect(0, by, 240, 30, TFT_COLOR4);
      continue;
    }
    bool sel = statDrill && idx == statDrillSel;
    uint16_t bg = sel ? TFT_COLOR2 : TFT_COLOR3;
    uint16_t fg = sel ? TFT_COLOR3 : TFT_COLOR1;
    tft.fillRect(3, by + 3, 234, 27, TFT_COLOR1);
    tft.fillRect(0, by, 234, 27, bg);
    tft.setTextColor(fg, bg);
    CenterText(String(ddAfflictionNames[activeAfflictions[idx]]) + " " + String(activeAfflictionLevels[idx]), by + 3);
  }
}

// Redraw only below the title (used on A-cycle page flips, no title flash).
void DrawAfflictionPage() {
  tft.fillRect(0, 28, 240, 107, TFT_COLOR4);
  DrawAfflictionRows();
}

void DrawAfflictionScreen() {
  tft.fillScreen(TFT_COLOR4);
  int w = tft.textWidth("AFFLICTIONS") + 3;
  tft.fillRect(3, 3, w, (9 * 3), TFT_COLOR1);
  tft.fillRect(0, 0, w, (9 * 3), TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString("AFFLICTIONS", 3, 3);
  DrawAfflictionRows();
}

// Tooltip for a merit by id (used by the status-menu drill-down).
void DrawMeritTooltipById(int m) {
  tft.fillScreen(TFT_COLOR4);
  tft.fillRect(3, 12, 234, 27, TFT_COLOR1);
  tft.fillRect(0, 9, 234, 27, TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString(ddMeritNames[m], 6, 12);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  tft.drawString(ddMeritDesc[m][0], 3, 54);
  tft.drawString(ddMeritDesc[m][1], 3, 78);
  tft.drawString(ddMeritDesc[m][2], 3, 101);
}

// Tooltip for an affliction by id + level.
void DrawAfflictionTooltipById(int a, int lvl) {
  tft.fillScreen(TFT_COLOR4);
  tft.fillRect(3, 12, 234, 27, TFT_COLOR1);
  tft.fillRect(0, 9, 234, 27, TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString(String(ddAfflictionNames[a]) + " " + String(lvl), 6, 12);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  tft.drawString(ddAfflictionDesc[a][0], 3, 54);
  tft.drawString(ddAfflictionDesc[a][1], 3, 78);
  tft.drawString(ddAfflictionDesc[a][2], 3, 101);
}

void VentureMenu(int dir) {
  // Determine max menu options
  int maxOptions = 1; // Farm + Dungeon = 2 options (0, 1)
  if (deepDungeonUnlocked) maxOptions = 2; // Add DD option (0, 1, 2)
  
  // Reset counter if over max
  if(selectedVentureMenu > maxOptions) 
    selectedVentureMenu = 0;
  // Enable Farm Menu when sleeping or injured
  if((ACTION == SLEEPING || ACTION == INJURED) && selectedVentureMenu > 0)
    selectedVentureMenu = 0;
  
  //Clear text
  tft.fillRect(162, 42, (25 * 3), (9 * 3), TFT_COLOR1);
  tft.fillRect(159, 39, (25 * 3), (9 * 3), TFT_COLOR3);
  tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
  //Draw Floor
  tft.fillRect(0, 30, 153, 78, TFT_COLOR4);
  tft.fillRect(0, 105, (80 * 3), (3 * 3), TFT_COLOR1);
  tft.fillRect(0, 108, (80 * 3), (9 * 3), TFT_COLOR3);
  
  // Draw indicator squares based on mode
  if (deepDungeonActive) {
    // DD Active: Farm(0), DD(1), Exit(2)
    if(selectedVentureMenu == 0) {
      tft.fillRect(168, 75, 15, 15, TFT_COLOR3); // Farm selected
      if(ACTION == SLEEPING || ACTION == INJURED) {
        tft.fillRect(192, 75, 15, 15, TFT_COLOR2);
        tft.fillRect(216, 75, 15, 15, TFT_COLOR2); // DD
      } else {
        tft.fillRect(192, 75, 15, 15, TFT_COLOR1);
        tft.fillRect(216, 75, 15, 15, TFT_COLOR1); // DD
      }
    } else if(selectedVentureMenu == 1) {
      tft.fillRect(168, 75, 15, 15, TFT_COLOR1); // Farm
      tft.fillRect(192, 75, 15, 15, TFT_COLOR3); // DD selected
      tft.fillRect(216, 75, 15, 15, TFT_COLOR1); // Exit
    } else if(selectedVentureMenu == 2) {
      tft.fillRect(168, 75, 15, 15, TFT_COLOR1); // Farm
      tft.fillRect(192, 75, 15, 15, TFT_COLOR1); // DD
      tft.fillRect(216, 75, 15, 15, TFT_COLOR3); // Exit selected
    }
  } else if (deepDungeonUnlocked) {
    // DD Unlocked: Farm(0), Venture(1), DD(2)
    if(selectedVentureMenu == 0) {
      tft.fillRect(168, 75, 15, 15, TFT_COLOR3); // Farm selected
      if(ACTION == SLEEPING || ACTION == INJURED) {
        tft.fillRect(192, 75, 15, 15, TFT_COLOR2);
        tft.fillRect(216, 75, 15, 15, TFT_COLOR2); // DD
      } else {
        tft.fillRect(192, 75, 15, 15, TFT_COLOR1);
        tft.fillRect(216, 75, 15, 15, TFT_COLOR1); // DD
      }
    } else if(selectedVentureMenu == 1) {
      tft.fillRect(168, 75, 15, 15, TFT_COLOR1); // Farm
      tft.fillRect(192, 75, 15, 15, TFT_COLOR3); // Venture selected
      tft.fillRect(216, 75, 15, 15, TFT_COLOR1); // DD
    } else if(selectedVentureMenu == 2) {
      tft.fillRect(168, 75, 15, 15, TFT_COLOR1); // Farm
      tft.fillRect(192, 75, 15, 15, TFT_COLOR1);
      tft.fillRect(216, 75, 15, 15, TFT_COLOR3); // DD selected
    }
  } else {
    // Normal: Farm(0), Venture(1)
    if(selectedVentureMenu == 0) {
      tft.fillRect(180, 75, 15, 15, TFT_COLOR3); // Farm selected
      if(ACTION == SLEEPING || ACTION == INJURED)
        tft.fillRect(201, 75, 15, 15, TFT_COLOR2);
      else
        tft.fillRect(201, 75, 15, 15, TFT_COLOR1);
    } else {
      tft.fillRect(180, 75, 15, 15, TFT_COLOR1);
      tft.fillRect(201, 75, 15, 15, TFT_COLOR3);
    }
  }
  
  //FARM
  if(selectedVentureMenu == 0){
    tft.drawString("FARM", 162, 42);
    // Draw actual plants instead of empty pots
    for(int i = 0; i < 3; i++) {
      int potX = 3 + (i * 51);
      int potY = 60;
      
      tft.fillRect(potX + 6, 48, 30, 9, TFT_COLOR3);
      
      if(potCrop[i] > 0) {
        int progressPercent = (potTimer[i] * 100) / 360;
        if(potStage[i] == 5) progressPercent = 100;
        if(progressPercent > 100) progressPercent = 100;
        int progressWidth = (progressPercent * 30) / 100;
        progressWidth = (progressWidth / 3) * 3;
        if(progressWidth > 0) {
          if(progressPercent != 100)
            tft.fillRect(potX + 9, 45, progressWidth, 9, TFT_COLOR1);
          else
            tft.fillRect(potX + 9, 45, progressWidth, 9, TFT_COLOR2);
        }
      }

      if(potStage[i] == 0) {
        pushScaled(potX, potY, 16, 16, FoodFarm[0], TFT_BLACK);
      }
      else if(potTimer[i] < 240) {
        pushScaled(potX, potY, 16, 16, FoodFarm[1], TFT_BLACK);
      }
      else if(potTimer[i] < 360) {
        pushScaled(potX, potY, 16, 16, FoodFarm[3], TFT_BLACK);
      }
      else if(potStage[i] == 5) {
        if(potCrop[i] == 1) pushScaled(potX, potY, 16, 16, FoodFarm[5], TFT_BLACK);
        else if(potCrop[i] == 2) pushScaled(potX, potY, 16, 16, FoodFarm[7], TFT_BLACK);
        else if(potCrop[i] == 3) pushScaled(potX, potY, 16, 16, FoodFarm[9], TFT_BLACK);
      }
    }
  }
  
  // DUNGEON / DD BATTLE VIEW
  if ((deepDungeonActive && selectedVentureMenu == 1) || 
      (!deepDungeonActive && selectedVentureMenu == 1)) {
    
    // Generate random enemy if needed
    if (!randomEnemyGenerated) GenerateRandomEnemy();
    
    if (deepDungeonActive) {
      if ((deepDungeonStage + 1) % 10 == 0 && getTotalBosses() > 0) {
        // Boss floor: flag it clearly instead of the floor number
        tft.drawString("BOSS", 162, 42);
      } else {
        // DD mode - show D + stage
        tft.drawString("D", 162, 42);
        if (deepDungeonStage + 1 >= 100) {
          tft.drawString(String(deepDungeonStage + 1), 180, 42);
        } else if (deepDungeonStage + 1 >= 10) {
          tft.drawString(String(deepDungeonStage + 1), 198, 42);
        } else {
          tft.drawString(String(deepDungeonStage + 1), 216, 42);
        }
      }
    } else {
      // Regular mode - show F + stage
      if (EEPROM.read(141) != 255) {
        stage = EEPROM.read(141);
      } else {
        EEPROM.write(141, 0);
        EEPROM.commit();
      }
      if (stage >= 30 && !randomEnemyGenerated) GenerateRandomEnemy();
      
      tft.drawString("F", 162, 42);
      if (stage >= 30) {
        tft.drawString("--", 198, 42);
      } else if (stage >= 9) {
        tft.drawString(String(stage + 1), 198, 42);
      } else {
        tft.drawString(String(stage + 1), 216, 42);
      }
    }
    
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    pushScaled(3, 42, 15, 15, abilityFrame[0], TFT_BLACK);
    pushScaled(54, 42, 15, 15, abilityFrame[0], TFT_BLACK);
    
    // Always show random enemy in DD mode or post-stage 5 (ventureEnemy has 5 entries: 0-4)
    if (deepDungeonActive || stage >= 30) {
      bool bossFloor = deepDungeonActive && ((deepDungeonStage + 1) % 10 == 0) && getTotalBosses() > 0;
      if (bossFloor) {
        pushScaled(9, 48, 11, 11, cardSpriteArray[bossEnemy[randomBossIndex][4]], TFT_BLACK);
        pushScaled(60, 48, 11, 11, cardSpriteArray[bossEnemy[randomBossIndex][5]], TFT_BLACK);
        if (!enemySpriteCreated) {
          CreateCharacter("enemy", headGear[bossEnemy[randomBossIndex][1]], bodyGear[bossEnemy[randomBossIndex][2]]);
          enemySpriteCreated = true;
        }
      } else {
        pushScaled(9, 48, 11, 11, cardSpriteArray[randomEnemyDeck[0]], TFT_BLACK);
        pushScaled(60, 48, 11, 11, cardSpriteArray[randomEnemyDeck[1]], TFT_BLACK);
        if (!enemySpriteCreated) {
          CreateCharacter("enemy", headGear[randomEnemyHead], bodyGear[randomEnemyBody]);
          enemySpriteCreated = true;
        }
      }
    } else {
      pushScaled(9, 48, 11, 11, cardSpriteArray[ventureEnemy[stage][4]], TFT_BLACK);
      pushScaled(60, 48, 11, 11, cardSpriteArray[ventureEnemy[stage][5]], TFT_BLACK);
      if (!enemySpriteCreated) {
        CreateCharacter("enemy", headGear[ventureEnemy[stage][1]], bodyGear[ventureEnemy[stage][2]]);
        enemySpriteCreated = true;
      }
    }
    if (enemyPet[0] != nullptr) {
      tft.pushImage(105, 60, 48, 48, enemyPet[0], TFT_BLACK);
    }

    // Display Stamina text or IN PROGRESS
    if(VENTURESTATE == VENTURING) {
      tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
      CenterText("IN PROGRESS", 111);
    } else {
      String displayStam;
      requiredStamina = 5;
      if (Stamina < 100 && Stamina >= 10)
        displayStam = "0" + String(Stamina);
      else if (Stamina < 10)
        displayStam = "00" + String(Stamina);
      else
        displayStam = Stamina;
      if (Stamina < requiredStamina)
        tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
      else
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      tft.drawString(String(String(requiredStamina) + "/" + displayStam), 147, 111);
      tft.drawString(String("STAMINA"), 3, 111);
    }
  }
  
  // DEEP DUNGEON ENTRY (when not active)
  if (!deepDungeonActive && deepDungeonUnlocked && selectedVentureMenu == 2) {
    tft.drawString("D ", 162, 42);
    tft.drawString(String(1), 216, 42);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
    tft.drawString("DEEP", 3, 42);
    tft.drawString("DUNGEON", 3, 66);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    CenterText("ENTER", 111);
  }
  
  // EXIT DD (when active)
  if (deepDungeonActive && selectedVentureMenu == 2) {
    if ((deepDungeonStage + 1) % 10 == 0 && getTotalBosses() > 0) {
      tft.drawString("BOSS", 162, 42);
    } else {
      // DD mode - show D + stage
      tft.drawString("D", 162, 42);
      if (deepDungeonStage + 1 >= 100) {
        tft.drawString(String(deepDungeonStage + 1), 180, 42);
      } else if (deepDungeonStage + 1 >= 10) {
        tft.drawString(String(deepDungeonStage + 1), 198, 42);
      } else {
        tft.drawString(String(deepDungeonStage + 1), 216, 42);
      }
    }
    tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
    tft.drawString("FORFEIT", 3, 45);
    tft.drawString("DUNGEON", 3, 69);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    CenterText("EXIT", 111);
  }
}

void InitializeEnemy(int _index, bool isRandom) {
  if (!isRandom) {
    CreateCharacter("enemy", headGear[ventureEnemy[_index][1]], bodyGear[ventureEnemy[_index][2]]);
    enemyName = names[_index];
    enemyMaxHP = 25;
    enemyHP = 25;
    enemyAttack = 10;
    enemyDefense = 10;
    enemySpeed = 10;
    for (int i = 0; i < 10; i++)
      enemyDeck[i] = cardList[ventureEnemy[_index][i + 4]];
  } else {
    bool bossFloor = deepDungeonActive && ((deepDungeonStage + 1) % 10 == 0) && getTotalBosses() > 0;
    if (bossFloor) {
      // Boss: predetermined gear + deck from the boss pool
      CreateCharacter("enemy", headGear[bossEnemy[randomBossIndex][1]], bodyGear[bossEnemy[randomBossIndex][2]]);
      enemyName = bossNames[randomBossIndex];
      for (int i = 0; i < 10; i++)
        enemyDeck[i] = cardList[bossEnemy[randomBossIndex][i + 4]];
    } else {
      CreateCharacter("enemy", headGear[randomEnemyHead], bodyGear[randomEnemyBody]);
      enemyName = names[random(0, 44)];
      for (int i = 0; i < 10; i++)
        enemyDeck[i] = cardList[randomEnemyDeck[i]];
    }
    enemyMaxHP = 25;   // HP is always 25 for everyone
    enemyHP = 25;
    if (deepDungeonActive) {
      // Non-HP stat total scales one tier per cleared boss (every 10 floors)
      int tier = deepDungeonStage / 10;
      int bst = 30 + tier * DD_STAT_TIER_STEP;
      if (bossFloor) bst = (int)(30 * DD_BOSS_STAT_MULT) + tier * DD_STAT_TIER_STEP;
      int third = bst / 3;
      enemyAttack = third + random(-2, 3);   // slight jitter so a floor isn't uniform
      enemyDefense = third + random(-2, 3);
      enemySpeed = bst - enemyAttack - enemyDefense;
      if (enemyAttack < 4) enemyAttack = 4;
      if (enemyDefense < 4) enemyDefense = 4;
      if (enemySpeed < 4) enemySpeed = 4;
    } else {
      // Main-mode post-30 enemies keep the flat baseline
      enemyAttack = 10;
      enemyDefense = 10;
      enemySpeed = 10;
    }
  }
  // enemy stat afflictions (fresh each battle)
  if (deepDungeonActive) {
    if (HasMerit(MERIT_PURGE) && activeAfflictionCount > 0) {
      int r = random(activeAfflictionCount);
      for (int i = r; i < activeAfflictionCount - 1; i++) {
        activeAfflictions[i] = activeAfflictions[i + 1];
        activeAfflictionLevels[i] = activeAfflictionLevels[i + 1];
      }
      activeAfflictionCount--;
      activeAfflictions[activeAfflictionCount] = -1;
      activeAfflictionLevels[activeAfflictionCount] = 0;
      EEPROM.write(355, (byte)activeAfflictionCount);
      for (int i = 0; i < 9; i++) EEPROM.write(356 + i, (byte)(activeAfflictions[i] & 0xFF));
      for (int i = 0; i < 9; i++) EEPROM.write(365 + i, (byte)activeAfflictionLevels[i]);
      EEPROM.commit();
    }
    int fero = GetAfflictionLevel(AFFL_FEROCITY);
    int stal = GetAfflictionLevel(AFFL_STALWART);
    int vig  = GetAfflictionLevel(AFFL_VIGOR);
    int rush = GetAfflictionLevel(AFFL_RUSH);
    if (fero) enemyAttack  = ceil(enemyAttack  * (1.0 + 0.10 * fero));
    if (stal) enemyDefense = ceil(enemyDefense * (1.0 + 0.10 * stal));
    if (rush) enemySpeed   = ceil(enemySpeed   * (1.0 + 0.10 * rush));
    if (vig) { enemyMaxHP += 3 * vig; enemyHP = enemyMaxHP; }
  }
  // Apply wounded enemy from random event
  if (woundedEnemyActive) {
    enemyHP = enemyMaxHP / 2;
    if (enemyHP < 1) enemyHP = 1;
    woundedEnemyActive = false;
    SaveEventFlags();
  }
}

void GenerateRandomEnemy() {
  randomEnemyHead = random(0, getTotalHeadGear());
  randomEnemyBody = random(0, getTotalBodyGear());
  int totalCards = getTotalCards();
  // Pick a theme; "Null" is included so non-elemental decks (and their synergies) exist
  const char* themes[6] = {"Fire", "Water", "Wind", "Earth", "Lightning", "Null"};
  String primary = themes[random(0, 6)];
  // Build the pool of on-theme cards
  int pool[100];
  int poolCount = 0;
  for (int i = 0; i < totalCards && poolCount < 100; i++)
    if (cardList[i].type == primary) pool[poolCount++] = i;
  // themeChance% of slots come from the theme pool; the rest stay fully random,
  // so decks lean into their element but still throw the occasional off-theme card.
  int themeChance = 80;
  bool hasInteraction = false;
  for (int i = 0; i < 10; i++) {
    int pick;
    if (poolCount > 0 && random(100) < themeChance)
      pick = pool[random(0, poolCount)];
    else
      pick = random(0, totalCards);
    randomEnemyDeck[i] = pick;
    if (cardList[pick].canReplace || cardList[pick].name == "DISARM") hasInteraction = true;
  }
  // Guarantee at least one counter removal/replacement card so the enemy interacts
  if (!hasInteraction) {
    for (int tries = 0; tries < 25; tries++) {
      int pick = random(0, totalCards);
      if (cardList[pick].canReplace || cardList[pick].name == "DISARM") {
        randomEnemyDeck[random(0, 10)] = pick;
        break;
      }
    }
  }
  // On a boss floor (every 10 floors in DD), pick a boss from the pool
  if (deepDungeonActive && ((deepDungeonStage + 1) % 10 == 0) && getTotalBosses() > 0)
    randomBossIndex = random(0, getTotalBosses());
  randomEnemyGenerated = true;
}

int GrantStageReward() {
  int result = -1;
  
  switch(stage) {
    case 0:
      result = GrantCard(5);
      break;
    case 1:
      result = GrantCard(4);
      break;
    case 2:
      result = GrantCard(35);
      break;
    case 3:
      result = GrantCard(8);
      break;
    case 4:
      result = GrantCard(47);
      break;
    case 5:
      result = GrantCard(51);
      break;
    case 6:
      result = GrantCard(27);
      break;
    case 7:
      result = GrantCard(20);
      break;
    case 8:
      result = GrantCard(33);
      break;
    case 9:
      result = GrantCard(44);
      break;
    case 10:
      result = GrantCard(29);
      break;
    case 11:
      result = GrantCard(11);
      break;
    case 12:
      result = GrantCard(6);
      break;
    case 13:
      result = GrantCard(24);
      break;
    case 14:
      result = GrantCard(41);
      break;
    case 15:
      result = GrantCard(21);
      break;
    case 16:
      result = GrantCard(40);
      break;
    case 17:
      result = GrantCard(30);
      break;
    case 18:
      result = GrantCard(19);
      break;
    case 19:
      result = GrantCard(42);
      break;
    case 20:
      result = GrantCard(39);
      break;
    case 21:
      result = GrantCard(13);
      break;
    case 22:
      result = GrantCard(23);
      break;
    case 23:
      result = GrantCard(17);
      break;
    case 24:
      result = GrantCard(16);
      break;
    case 25:
      result = GrantCard(49);
      break;
    case 26:
      result = GrantCard(32);
      break;
    case 27:
      result = GrantCard(10);
      break;
    case 28:
      result = GrantCard(43);
      break;
    case 29:
      result = GrantCard(36);
      break;
    default:
      // Stage 30+: grant random card
      result = GrantCard(-1);
      break;
  }
  
  return result;
}
