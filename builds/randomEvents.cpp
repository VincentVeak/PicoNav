#include "globals.h"
#include <EEPROM.h>

static const int CLEANSE_COST = 15;

// Event definitions
struct EventDef {
  const char* title1;
  const char* title2;
  const char* desc1;
  const char* desc2;
  const char* desc3;
  int weight;
  bool isNegative;
  EventKind kind;
  EventContext context;
};

const EventDef eventDefs[] = {
  {"SPARE", "CHANGE", "FOUND SOME", "CREDITS ON", "THE GROUND!", 15, false, EK_STATIC, CTX_ANY},
  {"REFRESHING", "", "FEELING", "GREAT TODAY!", "", 10, false, EK_STATIC, CTX_ANY},
  {"CHARGED UP", "", "+1 ATK & DEF", "FOR YOUR", "NEXT BATTLE!", 10, false, EK_STATIC, CTX_ANY},
  {"DOWNER", "", "-1 ATK & DEF", "FOR YOUR", "NEXT BATTLE!", 10, true, EK_STATIC, CTX_ANY},
  {"MERCHANT", "", "A MERCHANT", "OFFERS 50%", "DISCOUNT!", 8, false, EK_STATIC, CTX_ANY},
  {"WOUNDED", "ENEMY", "NEXT ENEMY", "STARTS AT", "HALF HEALTH!", 8, false, EK_STATIC, CTX_ANY},
  {"HARVEST", "BOUNTY", "ALL CROPS", "ARE NOW", "READY!", 5, false, EK_STATIC, CTX_ANY},
  {"AMBUSH", "", "ATTACKED!", "LOST HALF", "YOUR HP!", 8, true, EK_STATIC, CTX_ANY},
  {"PICKPOCKET", "", "A THIEF", "STOLE AN", "ITEM!", 8, true, EK_STATIC, CTX_ANY},
  {"FREEBIE", "", "SOMEONE LEFT", "AN ITEM", "FOR YOU!", 10, false, EK_STATIC, CTX_ANY},
  {"RELIC", "CACHE", "FOUND A", "CARD IN", "THE RUINS!", 10, false, EK_STATIC, CTX_DD},
  {"CURSED", "SHRINE", "A CURSE", "TAKES", "HOLD!", 5, true, EK_STATIC, CTX_DD},
  {"LUCKY", "FIND", "FORTUNE", "SMILES ON", "YOU!", 1, false, EK_STATIC, CTX_DD},
  {"RESPITE", "", "A CURSE", "LOOSENS", "ITS GRIP", 5, false, EK_STATIC, CTX_DD},
  {"DUST", "STORM", "THE ENEMY", "GROWS", "STRONGER!", 10, true, EK_STATIC, CTX_DD},
  {"STRANGE", "AURA", "AN OMEN", "HANGS IN", "THE AIR", 10, false, EK_STATIC, CTX_ANY},
  {"RISKY", "CHEST", "A CHEST", "SITS HERE", "", 10, false, EK_CHOICE, CTX_DD},
  {"GEM", "TRADE", "A TRADER", "WANTS YOUR", "GEM", 10, false, EK_CHOICE, CTX_DD},
  {"MERIT", "SWAP", "SWAP OUT", "A MERIT?", "", 5, false, EK_CHOICE, CTX_DD},
  {"CLEANSE", "FONT", "EASE A", "CURSE FOR", "CREDITS?", 10, false, EK_CHOICE, CTX_DD},
  {"BLOOD", "PACT", "A CURSE", "FOR A", "CARD", 10, true, EK_CHOICE, CTX_DD},
  {"CARD", "TRADER", "TRADE A", "CARD FOR", "ANOTHER?", 10, false, EK_CHOICE, CTX_DD},
  {"PETTY", "THIEF", "A THIEF", "WANTS A", "FIGHT!", 10, false, EK_BATTLE, CTX_DD},
  {"MIMIC", "", "IS THAT", "A CHEST?", "", 10, false, EK_BATTLE, CTX_DD},
  {"MIRROR", "IMAGE", "FACE YOUR", "OWN", "REFLECTION!", 10, false, EK_BATTLE, CTX_DD},
  {"PRESSURE", "POINT", "DOUBLE CRIT", "NEXT", "BATTLE!", 10, false, EK_STATIC, CTX_ANY}
};

void SaveEventFlags() {
  EEPROM.write(340, chargedUpActive ? 1 : 0);
  EEPROM.write(341, downerActive ? 1 : 0);
  EEPROM.write(342, merchantDiscountActive ? 1 : 0);
  EEPROM.write(343, woundedEnemyActive ? 1 : 0);
  EEPROM.write(375, (byte)(dustStormAffl & 0xFF));
  EEPROM.write(376, (byte)(strangeAuraCounter & 0xFF));
  EEPROM.write(377, (byte)(strangeAuraTarget & 0xFF));
  EEPROM.write(378, (byte)(respiteAffl & 0xFF));
  EEPROM.write(379, pressurePointActive ? 1 : 0);
  EEPROM.commit();
}

void LoadEventFlags() {
  chargedUpActive = (EEPROM.read(340) == 1);
  downerActive = (EEPROM.read(341) == 1);
  merchantDiscountActive = (EEPROM.read(342) == 1);
  woundedEnemyActive = (EEPROM.read(343) == 1);
  { int v = EEPROM.read(375); dustStormAffl = (v == 255) ? -1 : v; }
  { int v = EEPROM.read(376); strangeAuraCounter = (v == 255) ? -1 : v; }
  { int v = EEPROM.read(377); strangeAuraTarget = (v == 255) ? -1 : v; }
  { int v = EEPROM.read(378); respiteAffl = (v == 255) ? -1 : v; }
  pressurePointActive = (EEPROM.read(379) == 1);
}

// --- Card-draft-reward helpers (Blood Pact, Card Trader) ---
static int UnownedDDCount() {
  int n = 0, total = getTotalCards();
  for (int i = 0; i < total; i++)
    if (!bitRead(ddCardOwnership[i / 8], i % 8)) n++;
  return n;
}

// Fill ddCardChoices[3] with distinct unowned DD cards (optionally excluding one).
static void RollEventDraftCards(int exclude = -1) {
  int pool[64], n = 0, total = getTotalCards();
  for (int i = 0; i < total; i++)
    if (!bitRead(ddCardOwnership[i / 8], i % 8) && i != exclude) pool[n++] = i;
  for (int k = 0; k < 3; k++) {
    if (n <= 0) { ddCardChoices[k] = (k > 0) ? ddCardChoices[k - 1] : 0; continue; }
    int idx = random(n);
    ddCardChoices[k] = pool[idx];
    pool[idx] = pool[--n];
  }
  ddCardSel = 0;
}

// Add a random affliction, or level one up if at the cap (mirrors Cursed Shrine).
static int AddRandomAffliction() {
  int affId = -1;
  // Can we add a brand-new affliction? (a slot is free and an unused id exists)
  bool canAdd = (activeAfflictionCount < GetAfflictionCap());
  if (canAdd) {
    bool anyUnused = false;
    for (int id = 0; id < DD_AFFLICTION_COUNT && !anyUnused; id++) {
      bool active = false;
      for (int i = 0; i < activeAfflictionCount; i++)
        if (activeAfflictions[i] == id) active = true;
      if (!active) anyUnused = true;
    }
    canAdd = anyUnused;
  }
  // Existing afflictions that can still level up
  int cand[9], nc = 0;
  for (int i = 0; i < activeAfflictionCount; i++)
    if (activeAfflictionLevels[i] < 3) cand[nc++] = i;
  bool canLevel = (nc > 0);

  bool doLevel;
  if (canAdd && canLevel) doLevel = (random(2) == 0);   // 50/50 when both are possible
  else if (canLevel) doLevel = true;
  else if (canAdd) doLevel = false;
  else return -1;   // fully maxed: nothing left to worsen

  if (doLevel) {
    int p = cand[random(nc)];
    activeAfflictionLevels[p]++;
    affId = activeAfflictions[p];
    bloodPactLeveled = true;
  } else {
    int id, tries = 0;
    bool dup;
    do {
      id = random(DD_AFFLICTION_COUNT);
      dup = false;
      for (int i = 0; i < activeAfflictionCount; i++)
        if (activeAfflictions[i] == id) dup = true;
    } while (dup && ++tries < 60);
    activeAfflictions[activeAfflictionCount] = id;
    activeAfflictionLevels[activeAfflictionCount] = 1;
    activeAfflictionCount++;
    affId = id;
    bloodPactLeveled = false;
  }
  SaveDeepDungeon();
  return affId;
}

// Owned DD cards not in the equipped deck can be traded away (avoids orphaning a slot).
static bool CardInEquippedDeck(int cardId) {
  int base = getDeckBaseAddr() + getEquippedDeck() * 10;
  for (int i = 0; i < 10; i++)
    if (EEPROM.read(base + i) == cardId) return true;
  return false;
}
int TradeableCardCount() {
  int n = 0, total = getTotalCards();
  for (int i = 0; i < total; i++)
    if (bitRead(ddCardOwnership[i / 8], i % 8) && !CardInEquippedDeck(i)) n++;
  return n;
}
int NthTradeableCard(int idx) {
  int n = 0, total = getTotalCards();
  for (int i = 0; i < total; i++)
    if (bitRead(ddCardOwnership[i / 8], i % 8) && !CardInEquippedDeck(i)) {
      if (n == idx) return i;
      n++;
    }
  return -1;
}

// True if an event may be picked right now: not already granted, and its
// context matches the current mode.
static bool EventEligible(int i) {
  if (i == EVENT_CHARGED_UP && chargedUpActive) return false;
  if (i == EVENT_DOWNER && downerActive) return false;
  if (i == EVENT_MERCHANT && merchantDiscountActive) return false;
  if (i == EVENT_WOUNDED_ENEMY && woundedEnemyActive) return false;
  if (i == EVENT_PRESSURE_POINT && pressurePointActive) return false;
  if (eventDefs[i].context == CTX_DD && !deepDungeonActive) return false;
  if (eventDefs[i].context == CTX_OVERWORLD && deepDungeonActive) return false;
  return true;
}

bool IsBadEvent(int type) {
  return type == EVENT_DOWNER || type == EVENT_AMBUSH || type == EVENT_PICKPOCKET
      || type == EVENT_CURSED_SHRINE || type == EVENT_DUST_STORM || type == EVENT_PETTY_THIEF;
}

void TriggerRandomEvent() {
  if (randomEventPending) return;

#if EVENT_DEBUG
  if (forceEventIndex >= 0) {
    if (forceEventIndex < EVENT_COUNT) {
      pendingEventType = (EventType)forceEventIndex;
      forceEventIndex = -1;
      randomEventPending = true;
      eventWalkStart = millis();
      eventState = EVENT_WALKING;
      return;
    }
    forceEventIndex = -1;
  }
#endif

  // Total weight of eligible events
  int totalWeight = 0;
  for (int i = 0; i < EVENT_COUNT; i++)
    if (EventEligible(i)) totalWeight += eventDefs[i].weight;

  if (totalWeight == 0) return;

  // Weighted random selection over eligible events (TIMID gets a 50% reroll on bad ones)
  for (int attempt = 0; attempt < 2; attempt++) {
    int roll = random(totalWeight);
    int cumulative = 0;
    pendingEventType = EVENT_SPARE_CHANGE;
    for (int i = 0; i < EVENT_COUNT; i++) {
      if (!EventEligible(i)) continue;
      cumulative += eventDefs[i].weight;
      if (roll < cumulative) {
        pendingEventType = (EventType)i;
        break;
      }
    }
    if (currentNature != NAT_TIMID || !IsBadEvent(pendingEventType) || random(2) == 0)
      break;
  }

  randomEventPending = true;
  eventWalkStart = millis();
  eventState = EVENT_WALKING;
}

void UpdateEventWalk() {
  if (eventState != EVENT_WALKING) return;

  static unsigned long lastWalkTime = 0;
  static int walkFrame = 0;

  // A/B mash skips the walk-up
  if (AButton == "pressed" || BButton == "pressed") {
    xPosition = eventTargetX;
    AButton = "";
    BButton = "";
    lastButtonPressTime = millis();
  }

  // Arrived (or skipped): switch to the alert prompt
  if (xPosition == eventTargetX) {
    moveLeft = false;
    ACTION = IDLE;
    randomIdle = 99;
    eventState = EVENT_ALERT;
    display.fillRect(0, 0, 240, 54, TFT_COLOR4);
    display.fillRect(0, 54, 240, 3, TFT_COLOR1);
    DrawCharacter(xPosition, 60, true);
    if (AMENUS == NA && MENUSTATE == MAINMENU) {
      tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      tft.drawString(String("???"), 30 * 3, 37 * 3);
    }
    return;
  }

  // Smooth step, rendered directly each tick so it doesn't lag behind DrawCharacter's throttle
  if (millis() - lastWalkTime < 1000) return;
  lastWalkTime = millis();

  int step = 9;
  if (xPosition < eventTargetX) {
    moveLeft = false;
    xPosition += step;
    if (xPosition > eventTargetX) xPosition = eventTargetX;
  } else {
    moveLeft = true;
    xPosition -= step;
    if (xPosition < eventTargetX) xPosition = eventTargetX;
  }

  walkFrame ^= 1;
  display.fillRect(0, 0, 240, 54, TFT_COLOR4);
  display.fillRect(0, 54, 240, 3, TFT_COLOR1);
  if (!moveLeft)
    display.pushImage(xPosition, 9, 48, 48, GetMirrorPet(walkFrame), TFT_BLACK);
  else
    display.pushImage(xPosition, 9, 48, 48, (uint16_t*)pet[walkFrame], TFT_BLACK);
  display.pushSprite(0, 51, TFT_BLACK);

  // Reached the spot on this step
  if (xPosition == eventTargetX) {
    moveLeft = false;
    ACTION = IDLE;
    randomIdle = 99;
    eventState = EVENT_ALERT;
    display.fillRect(0, 0, 240, 54, TFT_COLOR4);
    display.fillRect(0, 54, 240, 3, TFT_COLOR1);
    DrawCharacter(xPosition, 60, true);
    if (AMENUS == NA && MENUSTATE == MAINMENU) {
      tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      tft.drawString(String("???"), 30 * 3, 37 * 3);
    }
  }
}

void DrawEventPrompt() {
  int idx = (int)pendingEventType;
  if (idx < 0 || idx >= EVENT_COUNT) return;
  
  const EventDef* evt = &eventDefs[idx];
  
  tft.fillScreen(TFT_COLOR4);
  tft.fillRect(6, 6, 231, 51, TFT_COLOR1);
  tft.fillRect(3, 3, 231, 51, TFT_COLOR3);
  
  // Title text
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  if (strlen(evt->title2) > 0) {
    CenterText(evt->title1, 6);
    CenterText(evt->title2, 30);
  } else {
    CenterText(evt->title1, 18);
  }
  
  // Description area
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  CenterText(evt->desc1, 63);
  CenterText(evt->desc2, 87);
  if (strlen(evt->desc3) > 0) {
    CenterText(evt->desc3, 111);
  }
}

// Redraw the Card Trader grid at the CURRENT page/selection (no CompileListMenu,
// which would advance the page). Used to restore the screen after a tooltip.
void DrawTradeGridRedraw() {
  tft.fillScreen(TFT_COLOR4);
  int w = tft.textWidth("DISCARD") + 3;
  tft.fillRect(3, 3, w, 27, TFT_COLOR1);
  tft.fillRect(0, 0, w, 27, TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString("DISCARD", 3, 3);
  pushScaled(0, 42, 15, 15, abilityFrame[0], TFT_BLACK);
  pushScaled(48, 42, 15, 15, abilityFrame[0], TFT_BLACK);
  pushScaled(96, 42, 15, 15, abilityFrame[0], TFT_BLACK);
  pushScaled(144, 42, 15, 15, abilityFrame[0], TFT_BLACK);
  pushScaled(192, 42, 15, 15, abilityFrame[0], TFT_BLACK);
  pushScaled(15, 69, 5, 5, smallIcons[13], TFT_BLACK);
  tft.fillRect(12, 72, 21, 3, TFT_COLOR1);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  if (pageNum < 10) tft.drawString("0" + String(pageNum), 6, 48);
  else tft.drawString(String(pageNum), 6, 48);
  for (int i = 0; i < 4; i++)
    if (menuValues[i] >= 0) pushScaled(54 + i * 48, 48, 11, 11, cardSpriteArray[menuValues[i]], TFT_BLACK);
  pushScaled(15 + menuItem * 48, 27, 5, 5, smallIcons[13], TFT_BLACK);
  if (menuItem == 0) CenterText("NEXT PAGE", 90);
  else CenterText(cardList[menuValues[menuItem - 1]].name, 90);
}

// Interactive choice screen. List events use a 3-row scrolling window with a
// trailing CANCEL row; the rest are yes/no (eventChoiceSel: 1 = YES, 0 = NO).
void DrawEventChoice() {
  // Card draft (Blood Pact, or Card Trader replacement step)
  if (pendingEventType == EVENT_BLOOD_PACT && eventChoiceStep == 1) {
    ddCardSel = eventChoiceSel;
    ddDraftRound = 0;
    DrawDDCardDraft();
    return;
  }
  // Card Trader step 2: confirm the trade (shows the picked card)
  if (pendingEventType == EVENT_CARD_TRADER && eventChoiceStep == 2) {
    tft.fillScreen(TFT_COLOR4);
    pushScaled(96, 3, 15, 15, abilityFrame[0], TFT_BLACK);
    pushScaled(102, 9, 11, 11, cardSpriteArray[eventDiscardCard], TFT_BLACK);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
    CenterText(cardList[eventDiscardCard].name, 51);
    CenterText("TRADE THIS?", 75);
    bool yes = (eventChoiceSel == 1);
    if (yes) {
      tft.fillRect(57, 105, 57, 27, TFT_COLOR1);
      tft.fillRect(54, 102, 57, 27, TFT_COLOR3);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    } else tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
    tft.drawString("YES", 57, 105);
    if (!yes) {
      tft.fillRect(138, 105, 57, 27, TFT_COLOR1);
      tft.fillRect(135, 102, 57, 27, TFT_COLOR3);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    } else tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
    tft.drawString("NO", 150, 105);
    return;
  }
  // Card Trader step 1: deck-editor card picker (NEXT PAGE item + 4 framed card slots)
  if (pendingEventType == EVENT_CARD_TRADER && eventChoiceStep == 1) {
    tft.fillScreen(TFT_COLOR4);
    int w = tft.textWidth("DISCARD") + 3;
    tft.fillRect(3, 3, w, 27, TFT_COLOR1);
    tft.fillRect(0, 0, w, 27, TFT_COLOR3);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    tft.drawString("DISCARD", 3, 3);
    pushScaled(0, 42, 15, 15, abilityFrame[0], TFT_BLACK);
    pushScaled(48, 42, 15, 15, abilityFrame[0], TFT_BLACK);
    pushScaled(96, 42, 15, 15, abilityFrame[0], TFT_BLACK);
    pushScaled(144, 42, 15, 15, abilityFrame[0], TFT_BLACK);
    pushScaled(192, 42, 15, 15, abilityFrame[0], TFT_BLACK);
    pushScaled(15, 69, 5, 5, smallIcons[13], TFT_BLACK);
    tft.fillRect(12, 72, 21, 3, TFT_COLOR1);
    pushScaled(15, 27, 5, 5, smallIcons[13], TFT_BLACK);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
    CenterText("NEXT PAGE", 90);
    menuItem = 0;
    menuIndex = 0;
    pageNum = 0;
    CompileListMenu("trade");
    return;
  }
  if (pendingEventType == EVENT_MERIT_SWAP || pendingEventType == EVENT_CLEANSE_FONT) {
    tft.fillScreen(TFT_COLOR4);
    bool merit = (pendingEventType == EVENT_MERIT_SWAP);
    int listN = merit ? activeMeritCount : activeAfflictionCount;
    const char* title = merit ? "SWAP MERIT" : "CLEANSE";
    int w = tft.textWidth(title) + 3;
    tft.fillRect(3, 3, w, 27, TFT_COLOR1);
    tft.fillRect(0, 0, w, 27, TFT_COLOR3);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    tft.drawString(title, 3, 3);
    if (!merit) {   // Cleanse: show the credit cost up front (icon + amount)
      tft.fillRect(180, 3, 57, 27, TFT_COLOR1);
      tft.fillRect(177, 0, 57, 27, TFT_COLOR3);
      pushScaled(180, 6, 5, 5, smallIcons[11], TFT_BLACK);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      tft.drawString(String(CLEANSE_COST), 198, 3);
    }
    int total = listN + 1;
    int top = (eventChoiceSel / 3) * 3;
    for (int r = 0; r < 3; r++) {
      int idx = top + r;
      int by = 45 + r * 30;
      if (idx >= total) { tft.fillRect(0, by, 240, 27, TFT_COLOR4); continue; }
      bool sel = (idx == eventChoiceSel);
      uint16_t bg = sel ? TFT_COLOR2 : TFT_COLOR3;
      uint16_t fg = sel ? TFT_COLOR3 : TFT_COLOR1;
      tft.fillRect(3, by + 3, 234, 27, TFT_COLOR1);
      tft.fillRect(0, by, 234, 27, bg);
      tft.setTextColor(fg, bg);
      String label;
      if (idx == listN) label = "CANCEL";
      else if (merit) label = MeritName(activeMerits[idx]);
      else label = String(AfflictionName(activeAfflictions[idx])) + " " + String(activeAfflictionLevels[idx]);
      CenterText(label, by + 3);
    }
    return;
  }
  tft.fillScreen(TFT_COLOR4);
  const char* q1 = "";
  const char* q2 = "";
  const char* q3 = "";
  if (pendingEventType == EVENT_GEM_TRADE) { q1 = "PAY 25 TO"; q2 = "REROLL GEM?"; }
  else if (pendingEventType == EVENT_RISKY_CHEST) { q1 = "OPEN CHEST?"; }
  else if (pendingEventType == EVENT_BLOOD_PACT) { q1 = "MAKE"; q2 = "PACT?"; }
  else if (pendingEventType == EVENT_CARD_TRADER) { q1 = "TRADE A"; q2 = "CARD?"; }
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  if (strlen(q1) > 0) CenterText(q1, 18);
  if (strlen(q2) > 0) CenterText(q2, 42);
  if (strlen(q3) > 0) CenterText(q3, 66);
  bool yes = (eventChoiceSel == 1);
  if (yes) {
    tft.fillRect(57, 99, 57, 27, TFT_COLOR1);
    tft.fillRect(54, 96, 57, 27, TFT_COLOR3);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  } else tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  tft.drawString("YES", 57, 99);
  if (!yes) {
    tft.fillRect(138, 99, 57, 27, TFT_COLOR1);
    tft.fillRect(135, 96, 57, 27, TFT_COLOR3);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  } else tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  tft.drawString("NO", 150, 99);
}

void DrawEventResult() {
  tft.fillScreen(TFT_COLOR4);
  display.fillSprite(TFT_BLACK);
  
  // Draw floor and static character if this event shows character
  if (eventShowCharacter) {
    tft.fillRect(0, 105, 240, 3, TFT_COLOR1);
    tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
    
    int frame, emote;
    if (eventCharacterAction == HURT) {
      frame = 9;
      emote = 14;  // blank
    } else if (eventCharacterAction == ATTACKING) {
      frame = 6;   
      emote = 14;  // blank
    } else {  // WAKE
      frame = 7;
      emote = 2;   // happy
    }
  }
  
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  if (eventResultText1.length() > 0) 
    CenterText(eventResultText1, 6);
  if (eventResultText2.length() > 0) 
    CenterText(eventResultText2, 30);
  if (eventResultText3.length() > 0) 
    CenterText(eventResultText3, 54);
  if(eventResultText2.startsWith("+"))
    pushScaled(129, 33, 5, 5, smallIcons[11], TFT_BLACK);
  if(pendingEventType == EVENT_FREEBIE && eventFreebieItem >= 0)
    pushScaled(xPosition + 75, 57, 16, 16, items[eventFreebieItem * 3], TFT_BLACK);
  if(eventRelicCard >= 0){
    pushScaled(147, 57, 15, 15, abilityFrame[0], TFT_BLACK);
    pushScaled(153, 63, 11, 11, cardSpriteArray[eventRelicCard], TFT_BLACK);
  }
  if(eventResultGem >= 0)
    pushScaled(147, 57, 16, 16, gems[eventResultGem + 1], TFT_BLACK);
  if(pendingEventType == EVENT_AMBUSH){
    //PLAYER HEALTH BAR
    tft.fillRect(27, 6, 63, 15, TFT_COLOR4);
    tft.fillRect(3, 3, 21, 21, TFT_COLOR1);
    tft.fillRect(0, 0, 21, 21, TFT_COLOR3);

    tft.fillRect(30, 9, 60, 12, TFT_COLOR3);
    if (HP > (maxHP / 4)) {
      pushScaled(3, 3, 5, 5, smallIcons[0], TFT_BLACK);
      int result = round((HP * 100.0 / maxHP) / 5) * 5 / 5;
      tft.fillRect(27, 6, (result * 3), 12, TFT_COLOR1);
    } else {
      pushScaled(3, 3, 5, 5, smallIcons[1], TFT_BLACK);
      int result = round((HP * 100.0 / maxHP) / 5) * 5 / 5;
      if (result == 0 && HP != 0)
        result = 1;
      tft.fillRect(27, 6, (result * 3), 12, TFT_COLOR2);
    }
  }
}

void ExecuteEvent() {
  if (IsBadEvent(pendingEventType)) {
    SetDayFlag(DF_BAD_EVENT);
    AddNaturePoints(NAT_TIMID, 2);
  }
  switch (pendingEventType) {
    case EVENT_SPARE_CHANGE: case EVENT_REFRESHING: case EVENT_CHARGED_UP: case EVENT_HARVEST_BOUNTY:
    case EVENT_FREEBIE: case EVENT_RELIC_CACHE: case EVENT_LUCKY_FIND: case EVENT_RESPITE:
      SetDayFlag(DF_GOOD_EVENT);
      break;
    default:
      break;
  }
  switch (pendingEventType) {
    case EVENT_SPARE_CHANGE: {
      int amount = random(10, 31);
      playerCredits += amount;
      if (playerCredits > 9999) playerCredits = 9999;
      SaveCredits();
      eventResultText1 = "FOUND";
      eventResultText2 = "+" + String(amount) + "  ";
      eventResultText3 = "";
      eventShowCharacter = true;
      ACTION = WAKE;
      eventCharacterAction = WAKE;
      xPosition = 99;
      break;
    }
    case EVENT_REFRESHING:
      Hunger = 100;
      Stamina = 100;
      Mood = 100;
      HP = maxHP;
      SavePetStats();
      eventResultText1 = "ALL METERS";
      eventResultText2 = "RESTORED!";
      eventResultText3 = "";
      eventShowCharacter = true;
      ACTION = WAKE;
      eventCharacterAction = WAKE;
      xPosition = 99;
      break;
    case EVENT_CHARGED_UP:
      chargedUpActive = true;
      SaveEventFlags();
      eventResultText1 = "+1 ATK/DEF";
      eventResultText2 = "NEXT BATTLE";
      eventResultText3 = "";
      eventShowCharacter = true;
      ACTION = ATTACKING;
      eventCharacterAction = ATTACKING;
      xPosition = 99;
      break;
    case EVENT_DOWNER:
      downerActive = true;
      SaveEventFlags();
      eventResultText1 = "-1 ATK/DEF";
      eventResultText2 = "NEXT BATTLE";
      eventResultText3 = "";
      eventShowCharacter = true;
      ACTION = HURT;
      eventCharacterAction = HURT;
      xPosition = 99;
      break;
    case EVENT_MERCHANT:
      merchantDiscountActive = true;
      SaveEventFlags();
      eventResultText1 = "50% OFF";
      eventResultText2 = "NEXT PURCHASE";
      eventResultText3 = "";
      eventShowCharacter = true;
      ACTION = WAKE;
      eventCharacterAction = WAKE;
      xPosition = 99;
      break;
    case EVENT_WOUNDED_ENEMY:
      woundedEnemyActive = true;
      SaveEventFlags();
      eventResultText1 = "NEXT ENEMY";
      eventResultText2 = "HALF HP!";
      eventResultText3 = "";
      eventShowCharacter = true;
      ACTION = ATTACKING;
      eventCharacterAction = ATTACKING;
      xPosition = 99;
      break;
    case EVENT_HARVEST_BOUNTY: {
      bool hadCrops = false;
      for (int i = 0; i < 3; i++) {
        if (potCrop[i] > 0 && potStage[i] < 5) {
          potStage[i] = 5;
          potTimer[i] = 360;
          hadCrops = true;
        }
      }
      if (hadCrops) {
        cropReady = true;
        eventResultText1 = "ALL CROPS";
        eventResultText2 = "NOW READY!";
        eventResultText3 = "";
        eventShowCharacter = true;
        ACTION = WAKE;
        eventCharacterAction = WAKE;
        xPosition = 99;
      } else {
        playerCredits += 10;
        if (playerCredits > 9999) playerCredits = 9999;
        SaveCredits();
        eventResultText1 = "NO CROPS.";
        eventResultText2 = "+10  ";
        eventResultText3 = "";
        eventShowCharacter = true;
        ACTION = WAKE;
        eventCharacterAction = WAKE;
        xPosition = 99;
      }
      break;
    }
    case EVENT_AMBUSH: {
      int damage = HP / 2;
      if (damage < 1) damage = 1;
      HP -= damage;
      if (HP < 1) HP = 1;
      SavePetStats();
      eventResultText1 = "";
      eventResultText2 = "-" + String(damage) + " HP";
      eventResultText3 = "";
      eventShowCharacter = true;
      ACTION = HURT;
      eventCharacterAction = HURT;
      xPosition = 99;
      break;
    }
    case EVENT_PICKPOCKET: {
      int addrs[] = {143, 144, 145, 146, 147, 148, 149};
      int validItems[7];
      int itemCount = 0;
      
      // Check EEPROM directly for consumable quantities
      for (int i = 0; i < 7; i++) {
        int qty = EEPROM.read(addrs[i]);
        if (qty > 0 && qty != 255) validItems[itemCount++] = i;
      }
      
      if (itemCount > 0) {
        int idx = validItems[random(itemCount)];
        int addr = addrs[idx];
        int qty = EEPROM.read(addr);
        qty--;
        EEPROM.write(addr, qty);
        EEPROM.commit();
        
        const char* names[] = {"SWEET FRUIT", "ENERGY DRINK", "HEALTH POTION", "MED KIT", "TASTY CAKE", "SCRY GLASS", "FLOOR MAP"};
        eventResultText1 = names[idx];
        eventResultText2 = "WAS STOLEN!";
        eventResultText3 = "";
        eventShowCharacter = true;
        ACTION = HURT;
        eventCharacterAction = HURT;
        xPosition = 99;
      } else {
        eventResultText1 = "NOTHING";
        eventResultText2 = "STOLEN!";
        eventResultText3 = "";
        eventShowCharacter = true;
        ACTION = WAKE;
        eventCharacterAction = WAKE;
        xPosition = 99;
      }
      break;
    }
    case EVENT_FREEBIE: {
      int itemType = random(7);
      int addrs[] = {143, 144, 145, 146, 147, 148, 149};
      int addr = addrs[itemType];
      int qty = EEPROM.read(addr);
      if (qty == 255) qty = 0;
      qty++;
      if (qty > 9) qty = 9;
      EEPROM.write(addr, qty);
      EEPROM.commit();
      const char* names[] = {"SWEET FRUIT", "ENERGY DRINK", "HEALTH POTION", "MED KIT", "TASTY CAKE", "SCRY GLASS", "FLOOR MAP"};
      eventResultText1 = "RECEIVED";
      eventResultText2 = names[itemType];
      eventResultText3 = "";
      eventShowCharacter = true;
      ACTION = WAKE;
      eventCharacterAction = WAKE;
      eventFreebieItem = itemType;  // Add this line
      xPosition = 51;
      break;
    }
    case EVENT_RELIC_CACHE: {
      int c = GrantDDCard(-1);
      if (c >= 0) {
        SaveDeepDungeon();
        eventRelicCard = c;
        eventResultText1 = cardList[c].name;
        eventResultText2 = "OBTAINED!";
        eventResultText3 = "";
        xPosition = 48;
      } else {
        playerCredits += 10;
        if (playerCredits > 9999) playerCredits = 9999;
        SaveCredits();
        eventRelicCard = -1;
        eventResultText1 = "NOTHING NEW";
        eventResultText2 = "+10  ";
        eventResultText3 = "";
        xPosition = 99;
      }
      eventShowCharacter = true;
      ACTION = WAKE;
      eventCharacterAction = WAKE;
      break;
    }
    case EVENT_CURSED_SHRINE: {
      int affId = -1;
      bool leveled = false;
      if (activeAfflictionCount < GetAfflictionCap()) {
        int id, tries = 0;
        bool dup;
        do {
          id = random(DD_AFFLICTION_COUNT);
          dup = false;
          for (int i = 0; i < activeAfflictionCount; i++)
            if (activeAfflictions[i] == id) dup = true;
        } while (dup && ++tries < 60);
        activeAfflictions[activeAfflictionCount] = id;
        activeAfflictionLevels[activeAfflictionCount] = 1;
        activeAfflictionCount++;
        affId = id;
      } else {
        int cand[9], n = 0;
        for (int i = 0; i < activeAfflictionCount; i++)
          if (activeAfflictionLevels[i] < 3) cand[n++] = i;
        if (n > 0) {
          int p = cand[random(n)];
          activeAfflictionLevels[p]++;
          affId = activeAfflictions[p];
          leveled = true;
        }
      }
      SaveDeepDungeon();
      if (affId >= 0) {
        eventResultText1 = AfflictionName(affId);
        eventResultText2 = leveled ? "LEVEL RAISED!" : "GAINED";
      } else {
        eventResultText1 = "A CURSE";
        eventResultText2 = "LINGERS";
      }
      eventResultText3 = "";
      eventShowCharacter = true;
      ACTION = HURT;
      eventCharacterAction = HURT;
      xPosition = 99;
      break;
    }
    case EVENT_LUCKY_FIND: {
      if (activeMeritCount < GetMeritCap()) {
        int id, tries = 0;
        bool dup;
        do {
          id = random(DD_MERIT_COUNT);
          dup = false;
          for (int i = 0; i < activeMeritCount; i++)
            if (activeMerits[i] == id) dup = true;
        } while (dup && ++tries < 60);
        activeMerits[activeMeritCount++] = id;
        SaveDeepDungeon();
        if (id == MERIT_PURGE) {
          ResolveMeritAcquire(id);
          eventResultText1 = "CURSE";
          eventResultText2 = "PURGED!";
        } else {
          eventResultText1 = MeritName(id);
          eventResultText2 = "GRANTED!";
        }
        eventResultText3 = "";
      } else {
        eventResultText1 = "MERITS FULL";
        eventResultText2 = "NOTHING";
        eventResultText3 = "GAINED.";
      }
      eventShowCharacter = true;
      ACTION = WAKE;
      eventCharacterAction = WAKE;
      xPosition = 99;
      break;
    }
    case EVENT_RESPITE: {
      if (activeAfflictionCount > 0) {
        int pick = random(activeAfflictionCount);
        respiteAffl = activeAfflictions[pick];   // temporary: -1 level for the next battle only
        SaveEventFlags();
        eventResultText1 = AfflictionName(respiteAffl);
        eventResultText2 = "TEMP DECREASE";
        eventResultText3 = "";
      } else {
        eventResultText1 = "NO CURSE";
        eventResultText2 = "TO EASE.";
        eventResultText3 = "";
      }
      eventShowCharacter = true;
      ACTION = WAKE;
      eventCharacterAction = WAKE;
      xPosition = 99;
      break;
    }
    case EVENT_DUST_STORM: {
      int opts[] = {AFFL_FEROCITY, AFFL_STALWART, AFFL_VIGOR, AFFL_RUSH};
      int id = opts[random(4)];
      dustStormAffl = id;
      SaveEventFlags();
      eventResultText1 = "NEXT ENEMY:";
      eventResultText2 = String(AfflictionName(id)) + "!";
      eventResultText3 = "";
      eventShowCharacter = true;
      ACTION = HURT;
      eventCharacterAction = HURT;
      xPosition = 99;
      break;
    }
    case EVENT_STRANGE_AURA: {
      static const int auraCtr[] = {19, 3, 23, 20, 8, 9, 2, 23, 9, 8, 1, 22, 26};
      static const int auraTgt[] = { 0, 0,  0,  0, 1, 1, 1,  1, 0, 0, 2,  2,  2};
      int k = random(sizeof(auraCtr) / sizeof(auraCtr[0]));
      strangeAuraCounter = auraCtr[k];
      strangeAuraTarget = auraTgt[k];
      SaveEventFlags();
      const char* tw = (auraTgt[k] == 1) ? "ON ENEMY" : (auraTgt[k] == 2) ? "ON ARENA" : "ON YOU";
      eventResultText1 = counterList[strangeAuraCounter].name;
      eventResultText2 = tw;
      eventResultText3 = "";
      eventShowCharacter = true;
      ACTION = IDLE;
      eventCharacterAction = IDLE;
      xPosition = 99;
      break;
    }
    case EVENT_RISKY_CHEST: {
      if (eventChoiceSel != 1) {
        eventResultText1 = "LEFT IT";
        eventResultText2 = "ALONE";
        eventResultText3 = "";
        eventShowCharacter = true;
        ACTION = IDLE;
        eventCharacterAction = IDLE;
        xPosition = 99;
        break;
      }
      if (random(2) == 0) {
        int c = -1;
        if (random(2) == 0) c = GrantDDCard(-1);
        if (c >= 0) {
          SaveDeepDungeon();
          eventRelicCard = c;
          eventResultText1 = cardList[c].name;
          eventResultText2 = "OBTAINED!";
          eventResultText3 = "";
          xPosition = 48;
        } else {
          playerCredits += 10;
          if (playerCredits > 9999) playerCredits = 9999;
          SaveCredits();
          eventRelicCard = -1;
          eventResultText1 = "FOUND";
          eventResultText2 = "+10  ";
          eventResultText3 = "";
          xPosition = 99;
        }
        eventShowCharacter = true;
        ACTION = WAKE;
        eventCharacterAction = WAKE;
      } else {
        int dmg = (maxHP + 1) / 2;   // 50% of MAX HP (~13), cannot kill
        if (dmg < 1) dmg = 1;
        HP -= dmg;
        if (HP < 1) HP = 1;
        SavePetStats();
        eventRelicCard = -1;
        eventResultText1 = "A TRAP!";
        eventResultText2 = "-" + String(dmg) + " HP";
        eventResultText3 = "";
        eventShowCharacter = true;
        ACTION = HURT;
        eventCharacterAction = HURT;
        xPosition = 99;
      }
      break;
    }
    case EVENT_GEM_TRADE: {
      if (eventChoiceSel != 1) {
        eventResultText1 = "KEPT YOUR";
        eventResultText2 = "GEM";
        eventResultText3 = "";
        eventShowCharacter = true;
        ACTION = IDLE;
        eventCharacterAction = IDLE;
        xPosition = 99;
        break;
      }
      if (playerCredits < 25 || activeGem < 0) {
        eventResultText1 = "NOT ENOUGH";
        eventResultText2 = "CREDITS";
        eventResultText3 = "";
        eventShowCharacter = true;
        ACTION = HURT;
        eventCharacterAction = HURT;
        xPosition = 99;
        break;
      }
      playerCredits -= 25;
      SaveCredits();
      int g;
      do { g = random(9); } while (g == activeGem);
      activeGem = g;
      SaveDeepDungeon();
      eventResultGem = g;
      eventResultText1 = GemName(g);
      eventResultText2 = "GEM!";
      eventResultText3 = "";
      eventShowCharacter = true;
      ACTION = WAKE;
      eventCharacterAction = WAKE;
      xPosition = 48;
      break;
    }
    case EVENT_MERIT_SWAP: {
      if (eventChoiceSel >= activeMeritCount) {
        eventResultText1 = "KEPT YOUR";
        eventResultText2 = "MERITS";
        eventResultText3 = "";
        eventShowCharacter = true;
        ACTION = IDLE;
        eventCharacterAction = IDLE;
        xPosition = 99;
        break;
      }
      int id, tries = 0;
      bool dup;
      do {
        id = random(DD_MERIT_COUNT);
        dup = false;
        for (int i = 0; i < activeMeritCount; i++)
          if (activeMerits[i] == id) dup = true;
      } while (dup && ++tries < 60);
      activeMerits[eventChoiceSel] = id;
      SaveDeepDungeon();
      if (id == MERIT_PURGE) {
        ResolveMeritAcquire(id);
        eventResultText1 = "CURSE";
        eventResultText2 = "PURGED!";
      } else {
        eventResultText1 = MeritName(id);
        eventResultText2 = "SWAPPED IN!";
      }
      eventResultText3 = "";
      eventShowCharacter = true;
      ACTION = WAKE;
      eventCharacterAction = WAKE;
      xPosition = 99;
      break;
    }
    case EVENT_CLEANSE_FONT: {
      if (eventChoiceSel >= activeAfflictionCount) {
        eventResultText1 = "LEFT THE";
        eventResultText2 = "FONT";
        eventResultText3 = "";
        eventShowCharacter = true;
        ACTION = IDLE;
        eventCharacterAction = IDLE;
        xPosition = 99;
        break;
      }
      if (playerCredits < CLEANSE_COST) {
        eventResultText1 = "NOT ENOUGH";
        eventResultText2 = "CREDITS";
        eventResultText3 = "";
        eventShowCharacter = true;
        ACTION = HURT;
        eventCharacterAction = HURT;
        xPosition = 99;
        break;
      }
      playerCredits -= CLEANSE_COST;
      SaveCredits();
      int affId = activeAfflictions[eventChoiceSel];
      if (activeAfflictionLevels[eventChoiceSel] > 1) {
        activeAfflictionLevels[eventChoiceSel]--;
      } else {
        for (int j = eventChoiceSel; j < activeAfflictionCount - 1; j++) {
          activeAfflictions[j] = activeAfflictions[j + 1];
          activeAfflictionLevels[j] = activeAfflictionLevels[j + 1];
        }
        activeAfflictionCount--;
        activeAfflictions[activeAfflictionCount] = -1;
        activeAfflictionLevels[activeAfflictionCount] = 0;
      }
      SaveDeepDungeon();
      eventResultText1 = AfflictionName(affId);
      eventResultText2 = "CLEANSED!";
      eventResultText3 = "";
      eventShowCharacter = true;
      ACTION = WAKE;
      eventCharacterAction = WAKE;
      xPosition = 99;
      break;
    }
    case EVENT_PRESSURE_POINT:
      pressurePointActive = true;
      SaveEventFlags();
      eventResultText1 = "DOUBLE CRIT";
      eventResultText2 = "NEXT BATTLE";
      eventResultText3 = "";
      eventShowCharacter = true;
      ACTION = WAKE;
      eventCharacterAction = WAKE;
      xPosition = 99;
      break;
    default:
      eventResultText1 = "NO EVENT FOUND";
      eventResultText2 = "";
      eventResultText3 = "";
      break;
  }
  tft.fillScreen(TFT_COLOR4);
  eventState = EVENT_RESULT;
  DrawEventResult();
}

// Short two-line result screen (used by choice cancels and empty-list cases).
static void EventResultSimple(const char* l1, const char* l2, bool good) {
  eventResultText1 = l1;
  eventResultText2 = l2;
  eventResultText3 = "";
  eventShowCharacter = true;
  ACTION = IDLE;              // neutral: idle, no emote
  eventCharacterAction = IDLE;
  xPosition = 99;
  eventState = EVENT_RESULT;
  DrawEventResult();
}

// Show a received card on the result screen (card icon + hold-B tooltip via eventRelicCard).
static void ShowReceivedCard(int c) {
  eventRelicCard = c;
  eventResultText1 = cardList[c].name;
  eventResultText2 = "OBTAINED!";
  eventResultText3 = "";
  eventShowCharacter = true;
  ACTION = WAKE;
  eventCharacterAction = WAKE;
  xPosition = 48;
  eventState = EVENT_RESULT;
  DrawEventResult();
}

// Grant a random unowned DD card (excluding one); returns its id, or -1 if none.
static int GrantRandomUnowned(int exclude) {
  int pool[64], n = 0, total = getTotalCards();
  for (int i = 0; i < total; i++)
    if (!bitRead(ddCardOwnership[i / 8], i % 8) && i != exclude) pool[n++] = i;
  if (n == 0) return -1;
  int c = pool[random(n)];
  GrantDDCard(c);
  return c;
}

// Hand a battle event off to the real card battle (DD-only). The victory/defeat screen
// is the outcome; eventBattle keeps the win from advancing the floor or starting a cooldown.
void StartEventBattle() {
  EventType ev = pendingEventType;
  // Tear down the event overlay
  randomEventPending = false;
  eventShowCharacter = false;
  eventRelicCard = -1;
  eventResultGem = -1;
  pendingEventType = EVENT_NONE;
  eventState = EVENT_IDLE;

  eventBattle = true;   // set before InitializeEnemy so it skips the boss-floor path
  eventBattleType = ev;
  randomEnemyGenerated = false;
  enemySpriteCreated = false;
  GenerateRandomEnemy();

  // Per-event disguise: gear must be set before InitializeEnemy builds the sprite
  if (ev == EVENT_MIMIC) {
    randomEnemyHead = 11;   // mimic-chest headgear
  } else if (ev == EVENT_MIRROR_IMAGE) {
    int wardrobe = EEPROM.read(134);   // equipped gear loadout (separate from the deck)
    randomEnemyHead = EEPROM.read(135 + wardrobe * 2);
    randomEnemyBody = EEPROM.read(136 + wardrobe * 2);
  }
  InitializeEnemy(0, true);
  CreateDDDeck(EEPROM.read(321));   // load the player's DD deck

  if (ev == EVENT_PETTY_THIEF) {
    enemyName = "THIEF";
    enemyHP = enemyMaxHP / 2;   // a weakling: starts at half HP
    if (enemyHP < 1) enemyHP = 1;
  } else if (ev == EVENT_MIMIC) {
    enemyName = "MIMIC";
  } else if (ev == EVENT_MIRROR_IMAGE) {
    enemyName = "MIRROR";
    for (int i = 0; i < 10; i++) enemyDeck[i] = playerDeck[i];   // fight your own deck
  }

  // Draw the arena so the intro renders on the battle scene, not a blank screen
  xPosition = 0;
  ACTION = IDLE;   // neutral pose through the name/VS intro, like a normal battle
  EMOTION = NORMAL;
  emoteFrames = 14;   // clear the alert "!" emote carried over from the event
  enemyAction = "attacking";
  tft.fillScreen(TFT_COLOR4);
  tft.fillRect(0, 105, 240, 3, TFT_COLOR1);
  tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
  display.fillRect(0, 0, 240, 54, TFT_COLOR4);
  display.fillRect(0, 54, 240, 3, TFT_COLOR1);
  DrawCharacter(0, 60, true);
  DrawEnemy(192);
  ACTION = ATTACKING;   // combat pose for the fight itself

  VENTURESTATE = INACTIVE;
  MENUSTATE = BATTLE;
  BATTLESTATE = COMMENCED;
  beginTurn = "";
  actionTime = 0;
}

void HandleEventButton() {
  if (!randomEventPending) return;
  
  if (eventState == EVENT_ALERT) {
    eventState = EVENT_PROMPT;
    EMOTION = NORMAL;
    ACTION = IDLE;
    DrawEventPrompt();
  }
  else if (eventState == EVENT_PROMPT) {
    if (eventDefs[pendingEventType].kind == EK_CHOICE) {
      eventState = EVENT_CHOICE;
      eventChoiceSel = 0;
      eventChoiceStep = 0;
      if (pendingEventType == EVENT_MERIT_SWAP) eventChoiceCount = activeMeritCount + 1;
      else if (pendingEventType == EVENT_CLEANSE_FONT) eventChoiceCount = activeAfflictionCount + 1;
      else if (pendingEventType == EVENT_BLOOD_PACT) {
        bloodPactAffl = -1;
        eventChoiceCount = 2;   // confirm the pact first
      }
      else if (pendingEventType == EVENT_CARD_TRADER) {
        if (TradeableCardCount() == 0) { EventResultSimple("NO CARDS", "TO TRADE", true); return; }
        eventChoiceCount = 2;   // start with a yes/no confirm
      }
      else eventChoiceCount = 2;   // yes/no events
      DrawEventChoice();
    } else if (eventDefs[pendingEventType].kind == EK_BATTLE) {
      StartEventBattle();
    } else {
      ExecuteEvent();
    }
  }
  else if (eventState == EVENT_CHOICE) {
    if (pendingEventType == EVENT_BLOOD_PACT) {
      if (eventChoiceStep == 0) {
        if (eventChoiceSel != 1) { EventResultSimple("PACT", "DECLINED", true); return; }
        if (UnownedDDCount() == 0) { EventResultSimple("NO NEW", "CARDS", true); return; }
        bloodPactAffl = AddRandomAffliction();   // the pact's cost
        RollEventDraftCards();
        eventChoiceStep = 1;
        eventChoiceSel = 0;
        eventChoiceCount = 3;
        DrawEventChoice();
      } else {
        int c = ddCardChoices[eventChoiceSel];
        GrantDDCard(c);
        SaveDeepDungeon();
        eventRelicCard = c;
        eventResultText1 = cardList[c].name;
        eventResultText2 = "OBTAINED!";
        eventResultText3 = "";
        xPosition = 48;
        eventShowCharacter = true;
        ACTION = WAKE;
        eventCharacterAction = WAKE;
        eventChoiceStep = 0;   // result sub-step: card first, then affliction
        eventState = EVENT_RESULT;
        DrawEventResult();
      }
    }
    else if (pendingEventType == EVENT_CARD_TRADER) {
      if (eventChoiceStep == 0) {
        if (eventChoiceSel != 1) { EventResultSimple("KEPT YOUR", "CARDS", true); return; }
        eventChoiceStep = 1;
        DrawEventChoice();   // open the card picker
      } else if (eventChoiceStep == 1) {
        if (menuItem == 0) {
          CompileListMenu("trade");   // next page
          menuItem = 0;
          tft.fillRect(15, 33, 210, 9, TFT_COLOR4);
          pushScaled(15, 27, 5, 5, smallIcons[13], TFT_BLACK);
          tft.fillRect(0, 87, 240, 27, TFT_COLOR4);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          CenterText("NEXT PAGE", 90);
        } else {
          eventDiscardCard = menuValues[menuItem - 1];
          eventChoiceStep = 2;
          eventChoiceSel = 0;      // NO by default
          eventChoiceCount = 2;
          DrawEventChoice();
        }
      } else {
        if (eventChoiceSel != 1) { EventResultSimple("KEPT YOUR", "CARDS", true); return; }
        bitClear(ddCardOwnership[eventDiscardCard / 8], eventDiscardCard % 8);
        int c = GrantRandomUnowned(eventDiscardCard);
        if (c >= 0) { SaveDeepDungeon(); ShowReceivedCard(c); }
        else { GrantDDCard(eventDiscardCard); SaveDeepDungeon(); EventResultSimple("NO OTHER", "CARDS", true); }
      }
    }
    else {
      ExecuteEvent();
    }
  }
  else if (eventState == EVENT_RESULT) {
    if (pendingEventType == EVENT_BLOOD_PACT && eventChoiceStep == 0 && bloodPactAffl >= 0) {
      eventChoiceStep = 1;
      eventRelicCard = -1;
      eventResultText1 = AfflictionName(bloodPactAffl);
      eventResultText2 = bloodPactLeveled ? "LEVEL RAISED!" : "GAINED";
      eventResultText3 = "";
      eventShowCharacter = true;
      ACTION = HURT;
      eventCharacterAction = HURT;
      xPosition = 99;
      DrawEventResult();
      return;
    }
    // Clear and return to normal
    randomEventPending = false;
    eventShowCharacter = false;
    eventRelicCard = -1;
    eventResultGem = -1;
    tft.fillScreen(TFT_COLOR4);
    display.fillRect(0, 0, 240, 54, TFT_COLOR4);
    display.fillRect(0, 54, 240, 3, TFT_COLOR1);
    pendingEventType = EVENT_NONE;
    eventState = EVENT_IDLE;
    randomIdle = 1;
    EMOTION = NORMAL;
    ACTION = WALKING;
    xPosition = 51;
    DrawBackground();
    DrawMainMenu();
    DrawCharacter(xPosition, 60, true);
    if (VENTURESTATE == VENTURING && countdownSeconds == 0 && AMENUS == NA) {
      tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      tft.drawString(String("BATTLE"), 24 * 3, 37 * 3);
    }
  }
}

void CheckRandomEvent() {
  if (randomEventPending || eventState != EVENT_IDLE) return;
  if (MENUSTATE != MAINMENU || ACTION == SLEEPING || ACTION == INJURED || gamePause) return;

#if EVENT_DEBUG
  if (forceEventIndex >= 0) { TriggerRandomEvent(); return; }
#endif

  unsigned long now = millis();
  if (now - lastEventCheck < 60000) return;
  lastEventCheck = now;
  
  if (random(100) < 1) {
    TriggerRandomEvent();
  }
}

#if EVENT_DEBUG
// Queue an event to fire on the next main-menu check, bypassing the roll/cooldown.
void DebugFireEvent(int i) { forceEventIndex = i; }
// Print every event and its index over serial.
void DebugListEvents() {
  for (int i = 0; i < EVENT_COUNT; i++)
    Serial.printf("%d %s %s\n", i, eventDefs[i].title1, eventDefs[i].title2);
}
#endif
