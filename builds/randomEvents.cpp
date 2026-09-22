#include "globals.h"
#include <EEPROM.h>

// Event definitions
struct EventDef {
  const char* title1;
  const char* title2;
  const char* desc1;
  const char* desc2;
  const char* desc3;
  int weight;
  bool isNegative;
};

const EventDef eventDefs[] = {
  {"SPARE", "CHANGE", "FOUND SOME", "CREDITS ON", "THE GROUND!", 15, false},
  {"REFRESHING", "", "FEELING", "GREAT TODAY!", "", 10, false},
  {"CHARGED UP", "", "+1 ATK & DEF", "FOR YOUR", "NEXT BATTLE!", 10, false},
  {"DOWNER", "", "-1 ATK & DEF", "FOR YOUR", "NEXT BATTLE!", 10, true},
  {"MERCHANT", "", "A MERCHANT", "OFFERS 50%", "DISCOUNT!", 8, false},
  {"WOUNDED", "ENEMY", "NEXT ENEMY", "STARTS AT", "HALF HEALTH!", 8, false},
  {"HARVEST", "BOUNTY", "ALL CROPS", "ARE NOW", "READY!", 5, false},
  {"AMBUSH", "", "ATTACKED!", "LOST HALF", "YOUR HP!", 8, true},
  {"PICKPOCKET", "", "A THIEF", "STOLE AN", "ITEM!", 8, true},
  {"FREEBIE", "", "SOMEONE LEFT", "AN ITEM", "FOR YOU!", 10, false}
};

void SaveEventFlags() {
  EEPROM.write(340, chargedUpActive ? 1 : 0);
  EEPROM.write(341, downerActive ? 1 : 0);
  EEPROM.write(342, merchantDiscountActive ? 1 : 0);
  EEPROM.write(343, woundedEnemyActive ? 1 : 0);
  EEPROM.commit();
}

void LoadEventFlags() {
  chargedUpActive = (EEPROM.read(340) == 1);
  downerActive = (EEPROM.read(341) == 1);
  merchantDiscountActive = (EEPROM.read(342) == 1);
  woundedEnemyActive = (EEPROM.read(343) == 1);
}

void TriggerRandomEvent() {
  if (randomEventPending) return;
  
  // Calculate total weight, excluding events with active flags
  int totalWeight = 0;
  for (int i = 0; i < EVENT_COUNT; i++) {
    // Skip events with active flags
    if (i == EVENT_CHARGED_UP && chargedUpActive) continue;
    if (i == EVENT_DOWNER && downerActive) continue;
    if (i == EVENT_MERCHANT && merchantDiscountActive) continue;
    if (i == EVENT_WOUNDED_ENEMY && woundedEnemyActive) continue;
    
    totalWeight += eventDefs[i].weight;
  }
  
  // No valid events available
  if (totalWeight == 0) return;
  
  // Weighted random selection
  int roll = random(totalWeight);
  int cumulative = 0;
  pendingEventType = EVENT_SPARE_CHANGE;
  
  for (int i = 0; i < EVENT_COUNT; i++) {
    // Skip events with active flags
    if (i == EVENT_CHARGED_UP && chargedUpActive) continue;
    if (i == EVENT_DOWNER && downerActive) continue;
    if (i == EVENT_MERCHANT && merchantDiscountActive) continue;
    if (i == EVENT_WOUNDED_ENEMY && woundedEnemyActive) continue;
    
    cumulative += eventDefs[i].weight;
    if (roll < cumulative) {
      pendingEventType = (EventType)i;
      break;
    }
  }
  
  randomEventPending = true;
  eventWalkStart = millis();
  
  // Walk to position
  eventState = EVENT_WALKING;
}

void UpdateEventWalk() {
  if (eventState != EVENT_WALKING) return;
  
  static unsigned long lastWalkTime = 0;

  // A/B mash skips the walk-up: jump to the target so the arrival block below
  // fires this frame. Only this approach animation is skippable; ALERT/PROMPT/
  // RESULT already advance one press at a time, and future battle/minigame
  // events run in their own states, so they stay unaffected.
  if (AButton == "pressed" || BButton == "pressed") {
    xPosition = eventTargetX;
    AButton = "";
    BButton = "";
    lastButtonPressTime = millis();
  }

  // Already at target?
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
  
  // Walk timing
  if (millis() - lastWalkTime < 400) return;
  lastWalkTime = millis();
  
  // Clear character area before moving
  display.fillRect(0, 0, 240, 54, TFT_COLOR4);
  display.fillRect(0, 54, 240, 3, TFT_COLOR1);
  
  // Move toward target
  if (xPosition < eventTargetX) {
    moveLeft = false;
    xPosition += 9;
    if (xPosition > eventTargetX) xPosition = eventTargetX;
  } else {
    moveLeft = true;
    xPosition -= 9;
    if (xPosition < eventTargetX) xPosition = eventTargetX;
  }
  
  ACTION = WALKING;
  DrawCharacter(xPosition, 60, !moveLeft);
  
  // Just arrived?
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
  
  // Timeout
  if (millis() - eventWalkStart > 8000) {
    xPosition = eventTargetX;
    moveLeft = false;
    ACTION = IDLE;
    randomIdle = 99;
    eventState = EVENT_ALERT;
    EMOTION = ALERT;
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
  if(pendingEventType == EVENT_SPARE_CHANGE)
    pushScaled(129, 33, 5, 5, smallIcons[11], TFT_BLACK);
  if(pendingEventType == EVENT_HARVEST_BOUNTY){
    if(eventResultText1 == "NO CROPS")
      pushScaled(129, 33, 5, 5, smallIcons[11], TFT_BLACK);
  }
  if(pendingEventType == EVENT_SPARE_CHANGE)
  pushScaled(129, 33, 5, 5, smallIcons[11], TFT_BLACK);
  if(pendingEventType == EVENT_FREEBIE && eventFreebieItem >= 0)
    pushScaled(xPosition + 75, 57, 16, 16, items[eventFreebieItem * 3], TFT_BLACK);
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

void HandleEventButton() {
  if (!randomEventPending) return;
  
  if (eventState == EVENT_ALERT) {
    eventState = EVENT_PROMPT;
    EMOTION = NORMAL;
    ACTION = IDLE;
    DrawEventPrompt();
  }
  else if (eventState == EVENT_PROMPT) {
    ExecuteEvent();
  }
  else if (eventState == EVENT_RESULT) {
    // Clear and return to normal
    randomEventPending = false;
    eventShowCharacter = false;
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
  
  unsigned long now = millis();
  if (now - lastEventCheck < 60000) return;
  lastEventCheck = now;
  
  if (random(100) < 1) {
    TriggerRandomEvent();
  }
}
