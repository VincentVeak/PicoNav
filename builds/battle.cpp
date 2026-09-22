#include "globals.h"
#include <EEPROM.h>

void CreateDeck(int deckNumber) {
  if (deckNumber == 0) {
    for (int i = 97; i < 107; i++) {
      playerDeck[i - 97] = cardList[EEPROM.read(i)];
      //Serial.println(playerDeck[i].name);
    }
  } else if (deckNumber == 1) {
    for (int i = 107; i < 117; i++)
      playerDeck[i - 107] = cardList[EEPROM.read(i)];
  } else if (deckNumber == 2) {
    for (int i = 117; i < 127; i++)
      playerDeck[i - 117] = cardList[EEPROM.read(i)];
  }
}

void Shuffle(Card deck[]) {
  for (int i = 9; i > 0; i--) {
    int j = random(i + 1);   // Generate a random index from 0 to i
    swap(deck[i], deck[j]);  // Swap the elements at i and j
  }
}

void swap(Card& a, Card& b) {
  Card temp = a;
  a = b;
  b = temp;
}

void InitializeBattle() {
  static unsigned long lastTime = 0;
  if (millis() - lastTime < 1500)
    return;
  if (actionTime < 4) {
    lastTime = millis();
    if (actionTime == 0) {
      playerCounter = "";
      arenaCounter = "";
      enemyCounter = "";
      attackStage = 0;
      defenseStage = 0;
      speedStage = 0;
      enemyAttackStage = 0;
      enemyDefenseStage = 0;
      enemySpeedStage = 0;
      playerLastDamage = 0;
      enemyLastDamage = 0;
      battleTurnCount = 0;
      whiplashPrimed = false;
      stackedDeckUsed = false;
      effectMsgCount = 0;
      effectMsgIndex = 0;
      // Apply event buffs/debuffs
      if (chargedUpActive) {
        attackStage = 1;
        defenseStage = 1;
        chargedUpActive = false;
        SaveEventFlags();
      }
      if (downerActive) {
        attackStage = -1;
        defenseStage = -1;
        downerActive = false;
        SaveEventFlags();
      }
      payoutUsed = false;
      // DD affliction: SLOTH lowers the player's starting speed stage
      int sloth = GetAfflictionLevel(AFFL_SLOTH);
      if (sloth) speedStage = max(-6, speedStage - sloth);
      equippedGem = (equippedDeck == 0) ? EEPROM.read(128) : (equippedDeck == 1) ? EEPROM.read(129) : EEPROM.read(130);
      CalculateMaxHP();
      // DD merit start-of-battle grants; each announces itself before turn one
      if (deepDungeonActive) {
        if (HasMerit(MERIT_WARD)) { ManageCounters("player", "AEGIS", true); QueueEffectMsg("WARD", 0); }
        if (HasMerit(MERIT_OMEN)) { ManageCounters("player", "LUCKY STAR", true); QueueEffectMsg("OMEN", 0); }
        if (HasMerit(MERIT_PHANTOM)) { ManageCounters("player", "MIST", true); QueueEffectMsg("PHANTOM", 0); }
        if (HasMerit(MERIT_DEEP_FREEZE)) { ManageCounters("enemy", "ICE", true); QueueEffectMsg("DEEP FREEZE", 0); }
        if (HasMerit(MERIT_FROSTGUARD)) { ManageCounters("arena", "ICE WALL", true); QueueEffectMsg("FROSTGUARD", 0); }
        if (HasMerit(MERIT_AMBUSH)) { enemyHP = (int)(enemyMaxHP * 0.75); if (enemyHP < 1) enemyHP = 1; QueueEffectMsg("AMBUSH", 0); }
        if (HasMerit(MERIT_INTIMIDATE)) { enemyAttackStage = max(-6, enemyAttackStage - 1); QueueEffectMsg("INTIMIDATE", 0); }
      }
      pressureUsageCount = 0;
      playerLifesteal = false;
      enemyLifesteal = false;
      playerUsedBlock = false;
      enemyUsedBlock = false; 
      selectedBattleCard = 0;
      selectionMode = "Cards";
      concedeMenuActive = false;
      concedeSelectNo = true;
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      CenterText(name, 30);
    } else if (actionTime == 1)
      CenterText("VS", 69);
    else {
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      CenterText(enemyName, 111);
    }
    actionTime++;
  } else {
    static bool startSceneReady = false;
    // Set up the battle backdrop once (health bars + arena marker), WITHOUT the card
    // hand yet, so the grant messages play over a clean scene instead of the cards.
    if (!startSceneReady) {
      tft.fillRect(0, 0, 240, 51, TFT_COLOR4);
      if (woundedEnemyActive) {
        enemyHP = enemyMaxHP / 2;
        if (enemyHP < 1) enemyHP = 1;
        woundedEnemyActive = false;
        SaveEventFlags();
      }
      UpdateHealthBars();
      Shuffle(playerDeck);
      Shuffle(enemyDeck);
      if (HasMerit(MERIT_REFORGE)) for (int i = 0; i < 10; i++) playerDeck[i] = cardList[random(getTotalCards())];
      //Arena Counter marker
      tft.fillRect(102, 33, 36, 12, TFT_COLOR1);
      tft.fillRect(105, 12, 30, 30, TFT_COLOR4);
      startSceneReady = true;
    }
    // Announce grant merits one at a time in the bottom message box, with the arrow
    if (effectMsgIndex < effectMsgCount) {
      lastTime = millis();
      tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      CenterText(effectMsgQueue[effectMsgIndex], 111);
      DrawEffectPointer(effectMsgTargetQ[effectMsgIndex]);
      effectMsgIndex++;
      return;
    }
    // Clear the arrow + center frame, then reveal the hand and begin card select
    tft.fillRect(18, 21, 15, 15, TFT_COLOR4);
    tft.fillRect(111, -3, 15, 15, TFT_COLOR4);
    tft.fillRect(204, 21, 15, 15, TFT_COLOR4);
    tft.fillRect(96, 48, 45, 45, TFT_COLOR4);
    effectMsgCount = 0;
    effectMsgIndex = 0;
    startSceneReady = false;
    actionTime = 0;
    UpdateBattleCards(selectedBattleCard, true);
    BATTLESTATE = CARDSELECT;
  }
}

void UpdateHealthBars() {
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
  //ENEMY HEALTH BAR
  tft.fillRect(174, 6, 63, 15, TFT_COLOR4);
  tft.fillRect(150, 3, 21, 21, TFT_COLOR1);
  tft.fillRect(147, 0, 21, 21, TFT_COLOR3);
  tft.fillRect(177, 9, 60, 12, TFT_COLOR3);
  if (enemyHP > (enemyMaxHP / 4)) {
    pushScaled(150, 3, 5, 5, smallIcons[0], TFT_BLACK);
    int result = round((enemyHP * 100.0 / enemyMaxHP) / 5) * 5 / 5;
    tft.fillRect(174, 6, (result * 3), 12, TFT_COLOR1);
  } else {
    pushScaled(150, 3, 5, 5, smallIcons[1], TFT_BLACK);
    int result = round((enemyHP * 100.0 / enemyMaxHP) / 5) * 5 / 5;
    if (result == 0 && enemyHP != 0)
      result = 1;
    tft.fillRect(174, 6, (result * 3), 12, TFT_COLOR2);
  }
}

void UpdateBattleCards(int selectedCard, bool refresh) {
  if (refresh) {
    //Clear the UseCard UI stuff
    tft.fillRect(102, 54, 33, 33, TFT_COLOR4);
    tft.fillRect(144, 63, 9, 15, TFT_COLOR4);
    tft.fillRect(84, 63, 9, 15, TFT_COLOR4);
    tft.fillRect(111, 3, 15, 12, TFT_COLOR4);

    pushScaled(48, 48, 15, 15, (uint16_t*)abilityFrame[0], TFT_BLACK);
    pushScaled(54, 54, 11, 11, (uint16_t*)cardSpriteArray[playerDeck[0].cardIndex], TFT_BLACK);

    pushScaled(96, 48, 15, 15, (uint16_t*)abilityFrame[0], TFT_BLACK);
    pushScaled(102, 54, 11, 11, (uint16_t*)cardSpriteArray[playerDeck[1].cardIndex], TFT_BLACK);

    pushScaled(144, 48, 15, 15, (uint16_t*)abilityFrame[0], TFT_BLACK);
    pushScaled(150, 54, 11, 11, (uint16_t*)cardSpriteArray[playerDeck[2].cardIndex], TFT_BLACK);
  }
  if (selectionMode == "Counters")
    return;
  //Card 1
  if (selectedCard == 0) {
    pushScaled(63, 90, 5, 5, (uint16_t*)smallIcons[3], TFT_BLACK);
    tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    int textWidth = tft.textWidth(String(playerDeck[0].name));
    int xPos = (TFT_HEIGHT - (textWidth)) / 2;
    int yPos = 37 * 3;
    tft.drawString(String(playerDeck[0].name), xPos, yPos);
  } else {
    tft.fillRect(63, 90, 15, 15, TFT_COLOR4);
  }
  //Card 2
  if (selectedCard == 1) {
    pushScaled(111, 90, 5, 5, (uint16_t*)smallIcons[3], TFT_BLACK);
    tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    int textWidth = tft.textWidth(String(playerDeck[1].name));
    int xPos = (TFT_HEIGHT - (textWidth)) / 2;
    int yPos = 37 * 3;
    tft.drawString(String(playerDeck[1].name), xPos, yPos);
  } else {
    tft.fillRect(111, 90, 15, 15, TFT_COLOR4);
  }
  //Card 3
  if (selectedCard == 2) {
    pushScaled(159, 90, 5, 5, (uint16_t*)smallIcons[3], TFT_BLACK);
    tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    int textWidth = tft.textWidth(String(playerDeck[2].name));
    int xPos = (TFT_HEIGHT - (textWidth)) / 2;
    int yPos = 37 * 3;
    tft.drawString(String(playerDeck[2].name), xPos, yPos);
  } else {
    tft.fillRect(159, 90, 15, 15, TFT_COLOR4);
  }
}

void UpdateCounters(int selected) {
  if (selected == 0) {
    tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    int textWidth = tft.textWidth(playerCounter);
    int xPos = (TFT_HEIGHT - (textWidth)) / 2;
    int yPos = 37 * 3;
    tft.drawString(playerCounter, xPos, yPos);

    pushScaled(18, 21, 5, 5, (uint16_t*)smallIcons[13], TFT_BLACK);
    tft.fillRect(111, -3, 15, 15, TFT_COLOR4);
    tft.fillRect(204, 24, 15, 12, TFT_COLOR4);
  }
  if (selected == 1) {
    tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    int textWidth = tft.textWidth(arenaCounter);
    int xPos = (TFT_HEIGHT - (textWidth)) / 2;
    int yPos = 37 * 3;
    tft.drawString(arenaCounter, xPos, yPos);

    tft.fillRect(18, 24, 15, 12, TFT_COLOR4);
    pushScaled(111, -3, 5, 5, (uint16_t*)smallIcons[13], TFT_BLACK);
    tft.fillRect(204, 24, 15, 12, TFT_COLOR4);
  }
  if (selected == 2) {
    tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    int textWidth = tft.textWidth(enemyCounter);
    int xPos = (TFT_HEIGHT - (textWidth)) / 2;
    int yPos = 37 * 3;
    tft.drawString(enemyCounter, xPos, yPos);

    tft.fillRect(18, 24, 15, 12, TFT_COLOR4);
    tft.fillRect(111, -3, 15, 15, TFT_COLOR4);
    pushScaled(204, 21, 5, 5, (uint16_t*)smallIcons[13], TFT_BLACK);
  }
}

// Queue a battle effect message with its arrow target (0=player,1=arena,2=enemy).
void QueueEffectMsg(String msg, int target) {
  if (effectMsgCount >= 8) return;
  effectMsgQueue[effectMsgCount] = msg;
  effectMsgTargetQ[effectMsgCount] = target;
  effectMsgCount++;
}

// Point the target arrow at the affected side, clearing the other two.
void DrawEffectPointer(int target) {
  battleArrowTarget = target;
  if (target == 0) {
    pushScaled(18, 21, 5, 5, (uint16_t*)smallIcons[13], TFT_BLACK);
    tft.fillRect(111, -3, 15, 15, TFT_COLOR4);
    tft.fillRect(204, 24, 15, 12, TFT_COLOR4);
  } else if (target == 1) {
    tft.fillRect(18, 24, 15, 12, TFT_COLOR4);
    pushScaled(111, -3, 5, 5, (uint16_t*)smallIcons[13], TFT_BLACK);
    tft.fillRect(204, 24, 15, 12, TFT_COLOR4);
  } else if (target == 2) {
    tft.fillRect(18, 24, 15, 12, TFT_COLOR4);
    tft.fillRect(111, -3, 15, 15, TFT_COLOR4);
    pushScaled(204, 21, 5, 5, (uint16_t*)smallIcons[13], TFT_BLACK);
  }
  // These effects have no counter of their own: show an empty center frame
  // rather than leaving whatever counter icon was last drawn there.
  tft.fillRect(105, 57, 24, 24, TFT_COLOR4);
  pushScaled(96, 48, 15, 15, (uint16_t*)abilityFrame[0], TFT_BLACK);
}

void BattleCardSelection() {
  if (BATTLESTATE == CARDSELECT) {
    
    //CONCEDE MENU - Must check FIRST before individual button checks
    if (concedeMenuActive) {
      // Handle NO/YES selection with A button
      if (AButton == "pressed" && millis() - lastButtonPressTime > 250) {
        lastButtonPressTime = millis();
        concedeSelectNo = !concedeSelectNo;
        
        // Clear and redraw buttons
        tft.fillRect(45, 78, 66, 39, TFT_COLOR4);
        tft.fillRect(138, 78, 66, 39, TFT_COLOR4);
        
        if (concedeSelectNo) {
          // NO selected
          tft.fillRect(48, 84, 54, 27, TFT_COLOR1);
          tft.fillRect(45, 81, 54, 27, TFT_COLOR3);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          tft.drawString("NO", 57, 84);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          tft.drawString("YES", 141, 84);
        } else {
          // YES selected
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          tft.drawString("NO", 63, 84);
          tft.fillRect(141, 84, 57, 27, TFT_COLOR1);
          tft.fillRect(138, 81, 57, 27, TFT_COLOR3);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          tft.drawString("YES", 141, 84);
        }
      }
      
      // Handle B button confirmation
      if (BButton == "pressed" && millis() - lastButtonPressTime > 250) {
        lastButtonPressTime = millis();
        concedeMenuActive = false;
        
        if (concedeSelectNo) {
          // NO selected - return to battle, redraw entire battle screen
          tft.fillScreen(TFT_COLOR4);
          DrawBackground();
          UpdateHealthBars();
          tft.fillRect(102, 33, 36, 12, TFT_COLOR1);
          tft.fillRect(105, 12, 30, 30, TFT_COLOR4);
          selectionMode = "Cards";
          selectedBattleCard = 0;
          UpdateBattleCards(selectedBattleCard, true);
          ManageCounters("player", playerCounter, true);
          ManageCounters("enemy", enemyCounter, true);
          ManageCounters("arena", arenaCounter, true);
        } else {
          // YES selected - concede (defeat without injury)
          BATTLESTATE = END;
          beginTurn = "concede";
          actionTime = 0;
          Mood = Mood - 50;
          if (Mood < 0)
            Mood = 0;
          actionTime = 0;
          DrawBackground();
          UpdateHealthBars();
        }
      }
      return;  // Don't process other inputs while menu is active
    }
    
    // Check for A+B held BEFORE individual held checks
    if (AButton == "held" && BButton == "held" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      concedeMenuActive = true;
      concedeSelectNo = true;  // Default to NO
      
      // Draw concede confirmation (same style as purchase menu)
      tft.fillScreen(TFT_COLOR4);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      CenterText("CONCEDE?", 39);
      
      // NO selected by default
      tft.fillRect(48, 84, 54, 27, TFT_COLOR1);
      tft.fillRect(45, 81, 54, 27, TFT_COLOR3);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      tft.drawString("NO", 57, 84);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      tft.drawString("YES", 141, 84);
      return;
    }
    
    //Card cycling
    if (selectionMode == "Cards") {
      if (AButton == "pressed" && millis() - lastButtonPressTime > 250) {
        lastButtonPressTime = millis();
        if (selectedBattleCard < 2) {
          selectedBattleCard++;
          UpdateBattleCards(selectedBattleCard, false);
        } else {
          selectedBattleCard = 0;
          UpdateBattleCards(selectedBattleCard, false);
        }
      }
    }  //Counter Cycling
    else if (selectionMode == "Counters") {
      if (AButton == "pressed" && millis() - lastButtonPressTime > 250) {
        lastButtonPressTime = millis();
        if (selectedCounter < 2) {
          selectedCounter++;
          if (selectedCounter == 0) {
            if (playerCounter == "")
              selectedCounter++;
          }
          if (selectedCounter == 1) {
            if (arenaCounter == "")
              selectedCounter++;
          }
          if (selectedCounter == 2) {
            if (enemyCounter == "")
              selectedCounter = 0;
          }
          UpdateCounters(selectedCounter);
        } else {
          selectedCounter = 0;
          if (selectedCounter == 0) {
            if (playerCounter == "")
              selectedCounter++;
          }
          if (selectedCounter == 1) {
            if (arenaCounter == "")
              selectedCounter++;
          }
          if (selectedCounter == 2) {
            if (enemyCounter == "")
              selectedCounter = 0;
          }
          UpdateCounters(selectedCounter);
        }
      }
    }
    //Switch between Card and Counter cylcing (only if B is not also held)
    if (AButton == "held" && BButton != "held" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (selectionMode == "Cards") {
        //exit if there are no counters on board
        if (playerCounter == "") {
          if (arenaCounter == "") {
            if (enemyCounter == "")
              return;
          }
        }
        selectionMode = "Counters";
        selectedCounter = 0;
        if (selectedCounter == 0) {
          if (playerCounter == "")
            selectedCounter++;
        }
        if (selectedCounter == 1) {
          if (arenaCounter == "")
            selectedCounter++;
        }
        if (selectedCounter == 2) {
          if (enemyCounter == "")
            selectedCounter = 0;
        }
        UpdateCounters(selectedCounter);
        tft.fillRect(63, 90, 15, 15, TFT_COLOR4);
        tft.fillRect(111, 90, 15, 15, TFT_COLOR4);
        tft.fillRect(159, 90, 15, 15, TFT_COLOR4);
      } else if (selectionMode == "Counters") {
        selectionMode = "Cards";
        selectedBattleCard = 0;
        tft.fillRect(18, 24, 15, 12, TFT_COLOR4);
        tft.fillRect(111, 3, 15, 12, TFT_COLOR4);
        tft.fillRect(204, 24, 15, 12, TFT_COLOR4);
        UpdateBattleCards(selectedBattleCard, false);
      }
    }
    //Card selection/tooltips
    if (selectionMode == "Cards") {
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        BATTLESTATE = ACTION1;
        tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
        tft.fillRect(48, 48, 141, 57, TFT_COLOR4);
        enemyBattleCard = ChooseEnemyCard();
        CalculateSpeed();
        beginTurn = "Begin";
      }
      //Card tooltip held (only if A is not also held)
      if (BButton == "held" && AButton != "held" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (TOOLTIP == RELEASED) {
          TOOLTIP = ACTIVE;
          tft.fillScreen(TFT_COLOR4);
          pushScaled(3, 3, 15, 15, abilityFrame[0], TFT_BLACK);
          tft.fillRect(51, 3, 183, 27, TFT_COLOR3);
          tft.fillRect(234, 6, 3, 27, TFT_COLOR1);
          tft.fillRect(54, 30, 183, 3, TFT_COLOR1);
          pushScaled(9, 9, 11, 11, cardSpriteArray[playerDeck[selectedBattleCard].cardIndex], TFT_BLACK);
          tft.drawString(playerDeck[selectedBattleCard].name, 54, 6);
          tft.fillRect(54, 39, 15, 6, TFT_COLOR1);
          tft.fillRect(75, 39, 15, 6, TFT_COLOR1);
          tft.fillRect(51, 36, 15, 6, TFT_COLOR3);
          bool repeat = false;
          for (int i = 0; i < 8; i++) {
            if (playerDeck[i].cardIndex == playerDeck[selectedBattleCard].cardIndex && i != selectedBattleCard)
              repeat = true;
          }
          if (repeat)
            tft.fillRect(72, 36, 15, 6, TFT_COLOR3);
          tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
          if (playerDeck[selectedBattleCard].damage < 100)
            tft.drawString(String(playerDeck[selectedBattleCard].damage), 201, 36);
          else
            tft.drawString(String(playerDeck[selectedBattleCard].damage), 183, 36);
          tft.fillRect(102, 36, 42, 15, TFT_COLOR4);
          if(playerDeck[selectedBattleCard].canReplace)
            pushScaled(102, 36, 5, 5, smallIcons[16], TFT_BLACK);
          if(playerDeck[selectedBattleCard].priority)
            pushScaled(129, 36, 5, 5, smallIcons[15], TFT_BLACK);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          tft.drawString(playerDeck[selectedBattleCard].desc1, 3, 63);
          tft.drawString(playerDeck[selectedBattleCard].desc2, 3, 87);
          tft.drawString(playerDeck[selectedBattleCard].desc3, 3, 111);
        }
      }
      //Card tooltip released
      if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
        lastButtonPressTime = millis();
        TOOLTIP = RELEASED;
        //tft.fillRect(0, 0, 240, 51, TFT_COLOR4);
        tft.fillScreen(TFT_COLOR4);
        DrawBackground();
        UpdateHealthBars();
        tft.fillRect(102, 33, 36, 12, TFT_COLOR1);
        tft.fillRect(105, 12, 30, 30, TFT_COLOR4);
        UpdateBattleCards(selectedBattleCard, true);
        ManageCounters("player", playerCounter, true);
        ManageCounters("enemy", enemyCounter, true);
        ManageCounters("arena", arenaCounter, true);
      }
    }

    if (selectionMode == "Counters") {
      //Card tooltip held (only if A is not also held)
      if (BButton == "held" && AButton != "held" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (TOOLTIP == RELEASED) {
          TOOLTIP = ACTIVE;
          tft.fillScreen(TFT_COLOR4);
          pushScaled(3, 3, 15, 15, abilityFrame[0], TFT_BLACK);
          tft.fillRect(51, 3, 183, 27, TFT_COLOR3);
          tft.fillRect(234, 6, 3, 27, TFT_COLOR1);
          tft.fillRect(54, 30, 183, 3, TFT_COLOR1);
          String counterName;
          if (selectedCounter == 0)
            counterName = playerCounter;
          else if (selectedCounter == 1)
            counterName = arenaCounter;
          else if (selectedCounter == 2)
            counterName = enemyCounter;
          for (int i = 0; i < getTotalCounters(); i++) {
            if (counterList[i].name == counterName) {
              tooltipCounter = counterList[i];
              tooltipCounter.counterIndex = i;
            }
          }
          pushScaled(12, 12, 8, 8, counterArray[tooltipCounter.counterIndex], TFT_BLACK);
          tft.drawString(tooltipCounter.name, 54, 6);
          tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
          tft.drawString(String("{" + String(tooltipCounter.abbreviation) + "}"), 168, 36);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          tft.drawString(tooltipCounter.desc1, 3, 63);
          tft.drawString(tooltipCounter.desc2, 3, 87);
          tft.drawString(tooltipCounter.desc3, 3, 111);
        }
      }
      //Card tooltip released
      if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
        lastButtonPressTime = millis();
        TOOLTIP = RELEASED;
        //tft.fillRect(0, 0, 240, 51, TFT_COLOR4);
        tft.fillScreen(TFT_COLOR4);
        DrawBackground();
        UpdateHealthBars();
        tft.fillRect(102, 33, 36, 12, TFT_COLOR1);
        tft.fillRect(105, 12, 30, 30, TFT_COLOR4);
        UpdateCounters(selectedCounter);
        UpdateBattleCards(selectedBattleCard, true);
        ManageCounters("player", playerCounter, true);
        ManageCounters("enemy", enemyCounter, true);
        ManageCounters("arena", arenaCounter, true);
      }
    }  //end counters
  }
}

void CalculateSpeed() {
  // order by priority bracket first, then real speed
  int _speed = (speedStage != 0) ? Speed * (4 + speedStage) / 4 : Speed;
  int _enemySpeed = (enemySpeedStage != 0) ? enemySpeed * (4 + enemySpeedStage) / 4 : enemySpeed;

  if (playerCounter == "WATER") _speed = (_speed * 0.75);
  if (enemyCounter == "WATER") _enemySpeed = (_enemySpeed * 0.75);

  // Priority brackets: 0 normal, 1 priority, 3 BLOCK. Higher acts first.
  int pBracket = 0;
  int eBracket = 0;

  if (playerCounter == "WIND") {
    pBracket = 1;
    if (playerDeck[selectedBattleCard].name != "AIR SLASH" && playerDeck[selectedBattleCard].name != "LUCKY STAR" && playerDeck[selectedBattleCard].name != "GALE FORCE" && playerDeck[selectedBattleCard].name != "UPDRAFT")
      ManageCounters("player", "", true);
  }
  if (enemyCounter == "WIND") {
    eBracket = 1;
    if (enemyDeck[enemyBattleCard].name != "AIR SLASH" && enemyDeck[enemyBattleCard].name != "LUCKY STAR" && enemyDeck[enemyBattleCard].name != "GALE FORCE" && enemyDeck[enemyBattleCard].name != "UPDRAFT")
      ManageCounters("enemy", "", true);
  }

  if (equippedGem == 9 && playerDeck[selectedBattleCard].damage == 0)  // Swift Gem
    if (pBracket < 1) pBracket = 1;

  if (playerDeck[selectedBattleCard].priority && equippedGem != 10) {  // Vital Gem cancels priority
    int b = (playerDeck[selectedBattleCard].name == "BLOCK") ? 3 : 1;
    if (pBracket < b) pBracket = b;
  }
  if (HasMerit(MERIT_WINDRIDER) && playerDeck[selectedBattleCard].type == "Wind" && pBracket < 1) pBracket = 1;
  if (HasMerit(MERIT_WHIPLASH) && whiplashPrimed) { if (pBracket < 1) pBracket = 1; whiplashPrimed = false; }
  if (enemyDeck[enemyBattleCard].priority) {
    int b = (enemyDeck[enemyBattleCard].name == "BLOCK") ? 3 : 1;
    if (eBracket < b) eBracket = b;
  }

  bool reversed = HasCounter("arena", "REVERSAL");
  if (pBracket != eBracket) {
    if (pBracket > eBracket) { firstPlayer = "player"; secondPlayer = "enemy"; }
    else { firstPlayer = "enemy"; secondPlayer = "player"; }
  } else if (reversed) {  // TIME FLIP: same bracket, slower goes first
    if (_speed <= _enemySpeed) { firstPlayer = "player"; secondPlayer = "enemy"; }
    else { firstPlayer = "enemy"; secondPlayer = "player"; }
  } else {  // same bracket, faster goes first
    if (_speed >= _enemySpeed) { firstPlayer = "player"; secondPlayer = "enemy"; }
    else { firstPlayer = "enemy"; secondPlayer = "player"; }
  }
}

// True if an element type lines up with a counter's element (for AI synergy weighting).
bool ElementMatchesCounter(String type, String counter) {
  if (counter == "") return false;
  if (type == "Fire") return counter == "FIRE" || counter == "BLAZE";
  if (type == "Water") return counter == "WATER" || counter == "RAIN" || counter == "ICE";
  if (type == "Wind") return counter == "WIND" || counter == "GUST" || counter == "MIST";
  if (type == "Earth") return counter == "EARTH" || counter == "EARTH2" || counter == "EARTH3" || counter == "SANDSTORM";
  if (type == "Lightning") return counter == "LIGHTNING" || counter == "LIGHTNING2" || counter == "LIGHTNING3" || counter == "STORM";
  return false;
}

// Enemy AI: pick from the top three cards, weighting by board synergy so the
// opponent feels intentional rather than a flat 33% roll.
int ChooseEnemyCard() {
  int weights[3];
  int total = 0;
  for (int i = 0; i < 3; i++) {
    Card c = enemyDeck[i];
    int w = 10;  // base chance for every card
    // Removal/replacement when the player is holding a counter worth stripping
    if (playerCounter != "" && (c.canReplace || c.name == "DISARM"))
      w += 20;
    // Elemental synergy with counters already building on the board
    if (c.type != "Null" && c.type != "") {
      if (ElementMatchesCounter(c.type, enemyCounter)) w += 10;
      if (ElementMatchesCounter(c.type, arenaCounter)) w += 8;
    }
    // Playing off an existing arena counter
    if (c.arenaCounter != "" && arenaCounter != "") w += 6;
    weights[i] = w;
    total += w;
  }
  int roll = random(total);
  int acc = 0;
  for (int i = 0; i < 3; i++) {
    acc += weights[i];
    if (roll < acc) return i;
  }
  return 0;
}

bool HasCounter(String target, String counterType) {
  String* counter;
  if (target == "player") counter = &playerCounter;
  else if (target == "enemy") counter = &enemyCounter;  
  else if (target == "arena") counter = &arenaCounter;
  else return false;
  
  // For stacking counters (Earth, Lightning), check for any level
  if (counterType == "EARTH" || counterType == "LIGHTNING" || counterType == "ICE WALL") {
    return GetStackLevel(target, counterType) > 0;
  }
  
  // For non-stacking counters, exact match
  return (*counter == counterType);
}

void TryHeal(String target, int amount, String successMessage) {
  bool reversed = HasCounter("arena", "BLOOD MOON");
  // DD affliction: BLIGHT cuts the player's healing 25% per level
  if (target == "player" && !reversed) {
    int blight = GetAfflictionLevel(AFFL_BLIGHT);
    if (blight) amount = (int)(amount * max(0.0, 1.0 - 0.25 * blight));
  }
  
  if (target == "player") {
    if (reversed) {
      HP -= amount;
      if (HP < 0) HP = 0;
    } else {
      HP += amount;
      if (HP > maxHP) HP = maxHP;
    }
  } else {
    if (reversed) {
      enemyHP -= amount;
      if (enemyHP < 0) enemyHP = 0;
    } else {
      enemyHP += amount;
      if (enemyHP > enemyMaxHP) enemyHP = enemyMaxHP;
    }
  }
  
  UpdateHealthBars();
  battleMessage = reversed ? "BLOOD MOON!" : successMessage;
  
  // Trigger death if HP reaches zero from Blood Moon reversal
  if (reversed) {
    if (target == "player" && HP <= 0) {
      animFrames = 0;
      ACTION = INJURED;
    } else if (target == "enemy" && enemyHP <= 0) {
      enemyAnimFrames = 0;
      enemyAction = "injured";
    }
  }
}

void AddStacks(String target, String counterType, int stacks) {
  // Don't replace if target has a different counter type
  if (target == "player" && playerCounter != "" && !HasCounter(target, counterType)) return;
  if (target == "enemy" && enemyCounter != "" && !HasCounter(target, counterType)) return;
  if (target == "arena" && arenaCounter != "" && !HasCounter(target, counterType)) return;
  
  int currentLevel = GetStackLevel(target, counterType);
  int newLevel = min(3, currentLevel + stacks);
  
  String newCounter = counterType;
  if (newLevel == 2) newCounter += "2";
  else if (newLevel == 3) newCounter += "3";
  
  ManageCounters(target, newCounter, true);
}

void SetStacks(String target, String counterType, int level) {
  if (level <= 0) {
    ManageCounters(target, "", true);
    return;
  }
  
  // Don't replace if target has a different counter type
  if (target == "player" && playerCounter != "" && !HasCounter(target, counterType)) return;
  if (target == "enemy" && enemyCounter != "" && !HasCounter(target, counterType)) return;
  if (target == "arena" && arenaCounter != "" && !HasCounter(target, counterType)) return;
  
  String newCounter = counterType;
  if (level == 2) newCounter += "2";
  else if (level == 3) newCounter += "3";
  
  ManageCounters(target, newCounter, true);
}

void RemoveStacks(String target, String counterType, int stacks) {
  int currentLevel = GetStackLevel(target, counterType);
  int newLevel = max(0, currentLevel - stacks);
  
  if (newLevel == 0) {
    ManageCounters(target, "", true);
  } else {
    String newCounter = counterType;
    if (newLevel == 2) newCounter += "2";
    else if (newLevel == 3) newCounter += "3";
    ManageCounters(target, newCounter, true);
  }
}

// Full-screen detail view for a card (reused for card select and mid-battle inspect).
void DrawCardTooltip(Card c) {
  tft.fillScreen(TFT_COLOR4);
  pushScaled(3, 3, 15, 15, (uint16_t*)abilityFrame[0], TFT_BLACK);
  tft.fillRect(51, 3, 183, 27, TFT_COLOR3);
  tft.fillRect(234, 6, 3, 27, TFT_COLOR1);
  tft.fillRect(54, 30, 183, 3, TFT_COLOR1);
  pushScaled(9, 9, 11, 11, (uint16_t*)cardSpriteArray[c.cardIndex], TFT_BLACK);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString(c.name, 54, 6);
  tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
  if (c.damage < 100)
    tft.drawString(String(c.damage), 201, 36);
  else
    tft.drawString(String(c.damage), 183, 36);
  if (c.canReplace)
    pushScaled(102, 36, 5, 5, (uint16_t*)smallIcons[16], TFT_BLACK);
  if (c.priority)
    pushScaled(129, 36, 5, 5, (uint16_t*)smallIcons[15], TFT_BLACK);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  tft.drawString(c.desc1, 3, 63);
  tft.drawString(c.desc2, 3, 87);
  tft.drawString(c.desc3, 3, 111);
}

// Full-screen detail view for the counter currently in play (uses global tooltipCounter).
void DrawCounterTooltip() {
  tft.fillScreen(TFT_COLOR4);
  pushScaled(3, 3, 15, 15, (uint16_t*)abilityFrame[0], TFT_BLACK);
  tft.fillRect(51, 3, 183, 27, TFT_COLOR3);
  tft.fillRect(234, 6, 3, 27, TFT_COLOR1);
  tft.fillRect(54, 30, 183, 3, TFT_COLOR1);
  pushScaled(12, 12, 8, 8, (uint16_t*)counterArray[tooltipCounter.counterIndex], TFT_BLACK);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString(tooltipCounter.name, 54, 6);
  tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
  tft.drawString(String("{" + String(tooltipCounter.abbreviation) + "}"), 168, 36);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  tft.drawString(tooltipCounter.desc1, 3, 63);
  tft.drawString(tooltipCounter.desc2, 3, 87);
  tft.drawString(tooltipCounter.desc3, 3, 111);
}

void UseCard(String user) {
  if (user == "player") {
    tft.fillRect(81, 63, 9, 15, TFT_COLOR4);
    tft.fillRect(147, 63, 9, 15, TFT_COLOR4);
    pushScaled(96, 48, 15, 15, (uint16_t*)abilityFrame[0], TFT_BLACK);
    pushScaled(102, 54, 11, 11, (uint16_t*)cardSpriteArray[playerDeck[selectedBattleCard].cardIndex], TFT_BLACK);
    tft.fillRect(147, 63, 3, 15, TFT_COLOR1);
    tft.fillRect(150, 66, 3, 9, TFT_COLOR1);
    tft.fillRect(153, 69, 3, 3, TFT_COLOR1);
    tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    int textWidth = tft.textWidth(String(playerDeck[selectedBattleCard].name));
    int xPos = (TFT_HEIGHT - (textWidth)) / 2;
    int yPos = 37 * 3;
    tft.drawString(String(playerDeck[selectedBattleCard].name), xPos, yPos);
    ACTION = ATTACKING;
    tooltipCard = playerDeck[selectedBattleCard];
    battleArrowUser = "player";
  }  //end Player section
  if (user == "enemy") {
    //enemyBattleCard = ChooseEnemyCard();
    tft.fillRect(81, 63, 9, 15, TFT_COLOR4);
    tft.fillRect(147, 63, 9, 15, TFT_COLOR4);
    pushScaled(96, 48, 15, 15, (uint16_t*)abilityFrame[0], TFT_BLACK);
    pushScaled(102, 54, 11, 11, (uint16_t*)cardSpriteArray[enemyDeck[enemyBattleCard].cardIndex], TFT_BLACK);
    tft.fillRect(87, 63, 3, 15, TFT_COLOR1);
    tft.fillRect(84, 66, 3, 9, TFT_COLOR1);
    tft.fillRect(81, 69, 3, 3, TFT_COLOR1);
    tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    int textWidth = tft.textWidth(String(enemyDeck[enemyBattleCard].name));
    int xPos = (TFT_HEIGHT - (textWidth)) / 2;
    int yPos = 37 * 3;
    tft.drawString(String(enemyDeck[enemyBattleCard].name), xPos, yPos);
    enemyAction = "attacking";
    tooltipCard = enemyDeck[enemyBattleCard];
    battleArrowUser = "enemy";
  }
}

//Manages the actual (re)placement and removal of counters and sprites
void ManageCounters(String target, String counter, bool _canReplace) {
  // Does nothing if it cannot replace an existing counter
  if (_canReplace == false) {
    if (target == "player" && playerCounter != "")
      return;
    if (target == "enemy" && enemyCounter != "")
      return;
    if (target == "arena" && arenaCounter != "" && !HasCounter("arena", "ICE WALL"))

      return;
  }
  
  int xPos = 0;
  int yPos = 0;

  if (target == "player") {
    if (playerCounter == "" || _canReplace)
      playerCounter = counter;
    else
      return;
    xPos = 15;
    yPos = 36;
  }
  if (target == "enemy") {
    if (enemyCounter == "" || _canReplace)
      enemyCounter = counter;
    else
      return;
    xPos = 201;
    yPos = 36;
  }
  if (target == "arena") {
    if (arenaCounter == "" || _canReplace)
      arenaCounter = counter;
    else
      return;
    xPos = 108;
    yPos = 15;
  }
  
  // ----- DISPLAY ALL COUNTER TYPES -----
  if (counter == "FIRE")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[9], TFT_BLACK);
  if (counter == "BLAZE")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[10], TFT_BLACK);
  if (counter == "WATER")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[6], TFT_BLACK);
  if (counter == "RAIN")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[7], TFT_BLACK);
  if (counter == "ICE")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[8], TFT_BLACK);
  if (counter == "WIND")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[11], TFT_BLACK);
  if (counter == "GUST")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[12], TFT_BLACK);  
  if (counter == "CURSE")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[2], TFT_BLACK);  
  if (counter == "MIST")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[3], TFT_BLACK); 
  if (counter == "LIFE STEAL")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[4], TFT_BLACK);   
  if (counter == "REVERSAL")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[5], TFT_BLACK);  
  if (counter == "AEGIS")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[19], TFT_BLACK);  
  if (counter == "NATURE")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[20], TFT_BLACK);  
  if (counter == "BLOOD MOON")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[22], TFT_BLACK);  
  if (counter == "LUCKY STAR")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[23], TFT_BLACK);  


  // Earth Stacks
  if (counter == "EARTH")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[13], TFT_BLACK);
  if (counter == "EARTH2")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[14], TFT_BLACK);
  if (counter == "EARTH3")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[15], TFT_BLACK);
  if (counter == "SANDSTORM")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[0], TFT_BLACK);  

  // Lightning Stacks
  if (counter == "LIGHTNING")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[16], TFT_BLACK);
  if (counter == "LIGHTNING2")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[17], TFT_BLACK);
  if (counter == "LIGHTNING3")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[18], TFT_BLACK);
  if (counter == "STORM")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[1], TFT_BLACK);  

  //Ice Wall stacks  
  if (counter == "ICE WALL")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[26], TFT_BLACK);
  if (counter == "ICE WALL2")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[25], TFT_BLACK);
  if (counter == "ICE WALL3")
    pushScaled(xPos, yPos, 8, 8, (uint16_t*)counterArray[24], TFT_BLACK);

  // Clear counter
  if (counter == "")
    tft.fillRect(xPos, yPos, 24, 24, TFT_COLOR4);
}

//Manages the post-action effects of counters
void CounterEffects(String effect, bool refresh) {
  tft.fillRect(81, 63, 9, 15, TFT_COLOR4);
  tft.fillRect(147, 63, 9, 15, TFT_COLOR4);
  tft.fillRect(18, 24, 15, 12, TFT_COLOR4);
  tft.fillRect(111, 3, 15, 12, TFT_COLOR4);
  tft.fillRect(207, 24, 15, 12, TFT_COLOR4);
  
  String counterName;
  if (effect == "player") counterName = playerCounter;
  else if (effect == "enemy") counterName = enemyCounter;
  else if (effect == "arena") counterName = arenaCounter;
  
  if (counterName == "") return;
  battleArrowTarget = (effect == "player") ? 0 : (effect == "arena") ? 1 : 2;

  for (int i = 0; i < getTotalCounters(); i++) {
    if (counterList[i].name == counterName) {
      tooltipCounter = counterList[i];
      tooltipCounter.counterIndex = i;
    }
  }
  if (effect == "player") {
    pushScaled(18, 21, 5, 5, (uint16_t*)smallIcons[13], TFT_BLACK);
    pushScaled(96, 48, 15, 15, (uint16_t*)abilityFrame[0], TFT_BLACK);
    pushScaled(105, 57, 8, 8, (uint16_t*)counterArray[tooltipCounter.counterIndex], TFT_BLACK);
  }
  if (effect == "enemy") {
    pushScaled(207, 21, 5, 5, (uint16_t*)smallIcons[13], TFT_BLACK);
    pushScaled(96, 48, 15, 15, (uint16_t*)abilityFrame[0], TFT_BLACK);
    pushScaled(105, 57, 8, 8, (uint16_t*)counterArray[tooltipCounter.counterIndex], TFT_BLACK);
  }
  if (effect == "arena") {
    pushScaled(111, -3, 5, 5, (uint16_t*)smallIcons[13], TFT_BLACK);
    pushScaled(96, 48, 15, 15, (uint16_t*)abilityFrame[0], TFT_BLACK);
    pushScaled(105, 57, 8, 8, (uint16_t*)counterArray[tooltipCounter.counterIndex], TFT_BLACK);
  }

  // ----- COUNTER EFFECTS -----
  if (!refresh) {
    // FIRE: Burn damage (only on player/enemy, not arena)
    if (effect != "arena" && HasCounter(effect, "FIRE")) {
      int maxHealthPoints = (effect == "player") ? maxHP : enemyMaxHP;
      int burnDamage = maxHealthPoints / 8;
      
      if (effect == "player") {
        HP -= burnDamage;
        if (HP < 0) HP = 0;
        UpdateHealthBars();
        battleMessage = "BURNED!";
        animFrames = 0;
        ACTION = HURT;
      } else if (effect == "enemy") {
        enemyHP -= burnDamage;
        if (enemyHP < 0) enemyHP = 0;
        UpdateHealthBars();
        battleMessage = "BURNED!";
        enemyAnimFrames = 0;
        
        if (enemyHP <= 0) {
          enemyAction = "injured";
        } else {
          enemyAction = "hurt";
        }
      }
    }
    // STORM: Add Lightning to both players (only when checking arena)
    if (effect == "arena" && HasCounter("arena", "STORM")) {
      if (playerCounter == "" || HasCounter("player", "LIGHTNING")) {
        AddStacks("player", "LIGHTNING", 1);
      }
      if (enemyCounter == "" || HasCounter("enemy", "LIGHTNING")) {
        AddStacks("enemy", "LIGHTNING", 1);
      }
      battleMessage = "ELECTRIFIED!";
    }
    // RAIN: Replace counters with Water if player doesn't have Water, Ice, or Mist
    if (effect == "arena" && HasCounter("arena", "RAIN")) {
      if (!HasCounter("player", "WATER") && !HasCounter("player", "ICE") && !HasCounter("player", "MIST") && !HasCounter("player", "LIFE STEAL")) {
        ManageCounters("player", "WATER", true);
      }
      if (!HasCounter("enemy", "WATER") && !HasCounter("enemy", "ICE") && !HasCounter("enemy", "MIST") && !HasCounter("enemy", "LIFE STEAL")) {
        ManageCounters("enemy", "WATER", true);
      }
      battleMessage = "IT'S RAINING!";
    }
    // BLAZE: Replace all counters with Fire on both players (only when checking arena)
    if (effect == "arena" && HasCounter("arena", "BLAZE")) {
      ManageCounters("player", "FIRE", true);
      ManageCounters("enemy", "FIRE", true);
      battleMessage = "IT'S BURNING!";
    }
    // CURSE: Deals 50% of last attack damage back to attacker
    if (effect != "arena" && HasCounter(effect, "CURSE")) {
      int curseDamage = 0;
      if (effect == "player") {
        curseDamage = (int)ceil(playerLastDamage * 0.5);
        HP -= curseDamage;
        if (HP < 0) HP = 0;
        UpdateHealthBars();
        battleMessage = "CURSED!";
        animFrames = 0;
        ACTION = HURT;
      } else if (effect == "enemy") {
        curseDamage = (int)ceil(enemyLastDamage * 0.5);
        enemyHP -= curseDamage;
        if (enemyHP < 0) enemyHP = 0;
        UpdateHealthBars();
        battleMessage = "CURSED!";
        enemyAnimFrames = 0;
        
        if (enemyHP <= 0) {
          enemyAction = "injured";
        } else {
          enemyAction = "hurt";
        }
      }
      playerLastDamage = 0;
      enemyLastDamage = 0;
    }
    // SANDSTORM: Deal 12.5% damage to both players without Earth or Nature counter
    if (effect == "arena" && HasCounter("arena", "SANDSTORM")) {
      bool playerDamaged = false;
      bool enemyDamaged = false;
      
      if (!HasCounter("player", "EARTH") && !HasCounter("player", "NATURE")) {
        int sandDamage = (int)ceil(maxHP * 0.125);
        HP -= sandDamage;
        if (HP < 0) HP = 0;
        playerDamaged = true;
      }
      if (!HasCounter("enemy", "EARTH") && !HasCounter("enemy", "NATURE")) {
        int sandDamage = (int)ceil(enemyMaxHP * 0.125);
        enemyHP -= sandDamage;
        if (enemyHP < 0) enemyHP = 0;
        enemyDamaged = true;
      }
      
      // Trigger animations for damaged players
      if (playerDamaged) {
        animFrames = 0;
        ACTION = HURT;
      }
      if (enemyDamaged) {
        enemyAnimFrames = 0;
        if (enemyHP <= 0) {
          enemyAction = "injured";
        } else {
          enemyAction = "hurt";
        }
      }
      
      if (playerDamaged || enemyDamaged) {
        UpdateHealthBars();
        battleMessage = "SANDSTORM!";
      }
    }
    // GUST: Remove all counters from both players
    if (effect == "arena" && HasCounter("arena", "GUST")) {
      if (!HasCounter("player", "WIND")) {
        ManageCounters("player", "", true);
      }
      if (!HasCounter("enemy", "WIND")) {
        ManageCounters("enemy", "", true);
      }
      battleMessage = "GUST OF WIND!";
    }
    // NATURE: Heal 12.5% max HP at end of turn
    if (effect != "arena" && HasCounter(effect, "NATURE")) {
      int maxHealthPoints = (effect == "player") ? maxHP : enemyMaxHP;
      int healAmount = (int)ceil(maxHealthPoints * 0.125);
      TryHeal(effect, healAmount);
    }

  }//END COUNTER EFFECTS
}//END function

void CalculateDamage(String target, Card _card) {
  float attackModifier = 1.0;
  float defenseModifier = 1.0;
  int damage = 0;
  bool critHit = false;
  String user;
  String userCounter;
  String targetCounter;
  int userATK;
  int targetDEF;
  
  // Setup user/target variables (keep existing logic)
  if (target == "enemy") {
    user = "player";
    userCounter = playerCounter;
    targetCounter = enemyCounter;
    //Attack Stage Modifier
    if (attackStage == 0)
      userATK = Attack;
    else if (attackStage != 0)
      userATK = Attack * (4 + attackStage)/4;
    //Defense Stage Modifier
    if (enemyDefenseStage == 0)
      targetDEF = enemyDefense;
    else if (enemyDefenseStage != 0)
      targetDEF = enemyDefense * (4 + enemyDefenseStage)/4;
  } else {
    user = "enemy";
    userCounter = enemyCounter;
    targetCounter = playerCounter;
    //Attack Stage Modifier
    if (enemyAttackStage == 0)
      userATK = enemyAttack;
    else if (enemyAttackStage != 0)
      userATK = enemyAttack * (4 + enemyAttackStage)/4;
    //Defense Stage Modifier
    if(defenseStage == 0)
      targetDEF = Defense;
    else if (defenseStage != 0)
      targetDEF = Defense * (4 + defenseStage)/4;
  }

  // Ice counter - frozen units cannot use ANY card, skip turn entirely
  if (HasCounter(user, "ICE")) {
    tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
    battleMessage = "FROZEN!";
    
    // Clear Ice after being frozen (consumed on use)
    ManageCounters(user, "", true);
    
    // Move the card to bottom of deck as if used
    if (user == "player") {
      Card temp = playerDeck[selectedBattleCard];
      for (int i = selectedBattleCard; i < 9; i++) 
        playerDeck[i] = playerDeck[i + 1];
      playerDeck[9] = temp;
    } else {
      Card temp = enemyDeck[enemyBattleCard];
      for (int i = enemyBattleCard; i < 9; i++) 
        enemyDeck[i] = enemyDeck[i + 1];
      enemyDeck[9] = temp;
    }
    return; // Exit completely, no card effects at all
  }

  // Check if target used Block - nullifies opponent's entire card
  if ((target == "enemy" && enemyUsedBlock) || (target == "player" && playerUsedBlock)) {
    tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
    battleMessage = "BLOCKED!";
    // Move used card to bottom of deck
    if (user == "player") {
      Card temp = playerDeck[selectedBattleCard];
      for (int i = selectedBattleCard; i < 10 - 1; i++)
        playerDeck[i] = playerDeck[i + 1];
      playerDeck[9] = temp;
    } else if (user == "enemy") {
      Card temp = enemyDeck[enemyBattleCard];
      for (int i = enemyBattleCard; i < 10 - 1; i++)
        enemyDeck[i] = enemyDeck[i + 1];
      enemyDeck[9] = temp;
    }
    return;
  }
  
  // Ice Wall arena counter blocks damage and reduces stacks (replaceable by other arenas)
  if (HasCounter("arena", "ICE WALL") && _card.damage > 0) {
    if (_card.arenaCounter != "") {
      // Arena counter card destroys Ice Wall completely
      SetStacks("arena", "ICE WALL", 0);
      ManageCounters("arena", "", true);
    } else {
      // Normal damaging card gets blocked
      tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
      battleMessage = "ICE WALL!";
      RemoveStacks("arena", "ICE WALL", 1);
      if (user == "player") {
        Card temp = playerDeck[selectedBattleCard];
        for (int i = selectedBattleCard; i < 10 - 1; i++)
          playerDeck[i] = playerDeck[i + 1];
        playerDeck[9] = temp;
      } else if (user == "enemy") {
        Card temp = enemyDeck[enemyBattleCard];
        for (int i = enemyBattleCard; i < 10 - 1; i++)
          enemyDeck[i] = enemyDeck[i + 1];
        enemyDeck[9] = temp;
      }
      return;
    }
  }
  
  // Apply counters and calculate damage normally
  if (_card.selfCounter != "") {
    if (_card.selfCounter == "EARTH" || _card.selfCounter == "LIGHTNING") {
      int n = 1;
      if (deepDungeonActive && user == "player") {
        if (HasMerit(MERIT_OVERCHARGE)) n = 3;
        else {
          if (HasMerit(MERIT_CONDUIT)) n++;
          if (HasMerit(MERIT_HIGH_VOLTAGE) && _card.selfCounter == "LIGHTNING") n++;
        }
      }
      AddStacks(user, _card.selfCounter, n);
    } else {
      ManageCounters(user, _card.selfCounter, _card.canReplace);
    }
  }
  if (_card.oppCounter != "") {
    if (_card.oppCounter == "EARTH" || _card.oppCounter == "LIGHTNING") {
      int n = 1;
      if (deepDungeonActive && user == "player") {
        if (HasMerit(MERIT_OVERCHARGE)) n = 3;
        else {
          if (HasMerit(MERIT_CONDUIT)) n++;
          if (HasMerit(MERIT_HIGH_VOLTAGE) && _card.oppCounter == "LIGHTNING") n++;
        }
      }
      AddStacks(target, _card.oppCounter, n);
    } else {
      ManageCounters(target, _card.oppCounter, _card.canReplace);
    }
  }
  if (_card.arenaCounter != "") {
    ManageCounters("arena", _card.arenaCounter, _card.canReplace);
  }
  
  int earthStacks = GetStackLevel(target, "EARTH");
  if (earthStacks == 1) defenseModifier = 1.2;
  else if (earthStacks == 2) defenseModifier = 1.4;  
  else if (earthStacks == 3) defenseModifier = 1.6;
  
  HandleCardEffects(target, _card, attackModifier, defenseModifier);
  
  //Damage and Crit check
  if (_card.damage != 0) {
    int critCheck = random(100);
    int critThreshold = 5;
    if (deepDungeonActive && activeGem == 8)  // CRIT gem: 25% for both picos
      critThreshold = 25;
    if(HasCounter(user, "LUCKY STAR"))
      critThreshold = 25;
    if(_card.name == "SONICBOOM")
      critThreshold = critThreshold * 2;
    if (deepDungeonActive) {
      if (user == "enemy") critThreshold += 10 * GetAfflictionLevel(AFFL_FRENZY);  // FRENZY
      if (user == "player") {
        critThreshold -= 10 * GetAfflictionLevel(AFFL_DREAD);  // DREAD
        if (HasMerit(MERIT_KEEN_EYE)) critThreshold += 20;                            // KEEN EYE
        if (HasMerit(MERIT_FIRE_SURGE)  && _card.type == "Fire")  critThreshold += 15; // *SURGE
        if (HasMerit(MERIT_WATER_SURGE) && _card.type == "Water") critThreshold += 15;
        if (HasMerit(MERIT_WIND_SURGE)  && _card.type == "Wind")  critThreshold += 15;
        if (HasMerit(MERIT_EARTH_SURGE) && _card.type == "Earth") critThreshold += 15;
        if (HasMerit(MERIT_BERSERK) && HP < maxHP / 2) critThreshold += 25;             // BERSERK
      }
      if (critThreshold < 0) critThreshold = 0;
    }
    if (critCheck < critThreshold) {
      critHit = true;
      // CRIT: Use base stats (ignore debuffs), keep positive modifiers only
      int critATK, critDEF;
      float critAtkMod = max(1.0f, attackModifier);  // Keep boosts, ignore penalties
      float critDefMod = min(1.0f, defenseModifier); // Keep reductions, ignore boosts
      if (target == "enemy") {
        critATK = (attackStage > 0) ? Attack * (4 + attackStage) / 4 : Attack;
        critDEF = (enemyDefenseStage < 0) ? enemyDefense * (4 + enemyDefenseStage) / 4 : enemyDefense;
      } else {
        critATK = (enemyAttackStage > 0) ? enemyAttack * (4 + enemyAttackStage) / 4 : enemyAttack;
        critDEF = (defenseStage < 0) ? Defense * (4 + defenseStage) / 4 : Defense;
      }
      damage = (((2.0 + 10) / 250) * (critATK / (float)critDEF) * _card.damage + 2) * critAtkMod / critDefMod * 2;
      battleMessage = "CRITICAL HIT!";
    } else {
      damage = (((2.0 + 10) / 250) * (userATK / (float)targetDEF) * _card.damage + 2) * attackModifier / defenseModifier;
    }
    if (deepDungeonActive && activeGem == 7 && user == "player")
      damage = (int)(damage * 0.8);  // MARATHON: -20% damage dealt
    // DD merits: player elemental amp (mult) + true damage (flat, ignores DEF)
    if (deepDungeonActive && user == "player") {
      float amp = 1.0;
      if (HasMerit(MERIT_FIRE_AMP)  && _card.type == "Fire")  amp *= 1.10;
      if (HasMerit(MERIT_WATER_AMP) && _card.type == "Water") amp *= 1.10;
      if (HasMerit(MERIT_WIND_AMP)  && _card.type == "Wind")  amp *= 1.10;
      if (HasMerit(MERIT_EARTH_AMP) && _card.type == "Earth") amp *= 1.10;
      if (HasMerit(MERIT_BERSERK) && HP < maxHP / 2) amp *= 1.25;                  // BERSERK
      if (HasMerit(MERIT_EXECUTE) && enemyHP < enemyMaxHP / 4) amp *= 1.50;        // EXECUTE
      if (HasMerit(MERIT_TECHNICIAN) && _card.damage < 50) amp *= 1.50;            // TECHNICIAN
      if (HasMerit(MERIT_DESPERATION)) amp *= (1.0 + (float)(maxHP - HP) / maxHP); // DESPERATION
      if (HasMerit(MERIT_SCORCHING) && _card.type == "Fire" && HasCounter("enemy", "FIRE")) amp *= 1.25;  // SCORCHING
      if (HasMerit(MERIT_TECTONICS) && _card.type == "Earth")
        amp *= (1.0 + 0.10 * GetStackLevel("player", "EARTH"));                    // TECTONICS
      if (HasMerit(MERIT_GUTS) && playerCounter != "") amp *= 1.15;                // GUTS
      if (amp != 1.0) damage = (int)(damage * amp);
      int td = 0;
      if (HasMerit(MERIT_SHARPEN)) td += 2;
      if (HasMerit(MERIT_FIRE_TD)  && _card.type == "Fire")  td += 2;
      if (HasMerit(MERIT_WATER_TD) && _card.type == "Water") td += 2;
      if (HasMerit(MERIT_WIND_TD)  && _card.type == "Wind")  td += 2;
      if (HasMerit(MERIT_EARTH_TD) && _card.type == "Earth") td += 2;
      damage += td;
    }
  }
  //Check if the targt has AEGIS active
  if (HasCounter(target, "AEGIS") && (_card.damage * attackModifier) <= 50 && _card.damage != 0) {
    damage = 0;
    tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
    battleMessage = "BLOCKED!";
  }
  //Check if the target has Mist active
  if (HasCounter(target, "MIST") && _card.damage > 0) {
    damage = 0;
    ManageCounters(target, "", true);
    battleMessage = "MIST FADED!";
  }
  // Track damage dealt for CURSE counter
  if (user == "player")
    playerLastDamage = damage;
  else
    enemyLastDamage = damage;

  if (user == "player") {
    enemyHP = enemyHP - damage;
    if (enemyHP < 0) enemyHP = 0;
    UpdateHealthBars();
    if (enemyHP <= 0 && damage > 0) {
      enemyAnimFrames = 0;
      enemyAction = "injured";
      // DD affliction: VOLATILE — enemy death chips the player (floored at 1)
      int vol = GetAfflictionLevel(AFFL_VOLATILE);
      if (vol) {
        HP -= (int)ceil(maxHP * 0.05 * vol);
        if (HP < 1) HP = 1;
        UpdateHealthBars();
        animFrames = 0; ACTION = HURT;
        battleMessage = "VOLATILE";
        DrawEffectPointer(0);
      }
    } else {
      if(damage > 0){
        enemyAnimFrames = 0;
        enemyAction = "hurt";
      }
    }
    
    if (HasMerit(MERIT_STACKED_DECK) && !stackedDeckUsed) {
      stackedDeckUsed = true;
    } else {
      Card temp = playerDeck[selectedBattleCard];
      for (int i = selectedBattleCard; i < 10 - 1; i++)
        playerDeck[i] = playerDeck[i + 1];
      playerDeck[9] = temp;
    }
    
  } else if (user == "enemy") {
    if(damage > 0){
      HP = HP - damage;
      if (HP < 0) HP = 0;
      if (HP <= 0 && HasMerit(MERIT_MOXIE)) { HP = 1; RemoveMerit(MERIT_MOXIE); QueueEffectMsg("MOXIE", 0); }
      if (HP > 0 && HP <= maxHP / 4 && HasMerit(MERIT_REPRIEVE)) { HP = maxHP; RemoveMerit(MERIT_REPRIEVE); QueueEffectMsg("REPRIEVE", 0); }
      UpdateHealthBars();
      animFrames = 0;
      ACTION = HURT;
    }
    // Move used card to bottom of deck
    Card temp = enemyDeck[enemyBattleCard];
    for (int i = enemyBattleCard; i < 10 - 1; i++) {
      enemyDeck[i] = enemyDeck[i + 1];
    }
    enemyDeck[9] = temp;
  }

  // Handle one-time lifesteal effect (from cards like LIFE LEECH, RECHARGE, etc)
  if (user == "player" && playerLifesteal) {
    TryHeal("player", damage, "HP DRAINED!");
    playerLifesteal = false;  // Always reset one-time effect
  } else if (user == "enemy" && enemyLifesteal) {
    TryHeal("enemy", damage, "HP DRAINED!");
    enemyLifesteal = false;  // Always reset one-time effect
  }
  // Handle LIFE STEAL counter (grants lifesteal on every attack)
  else if (user == "player" && HasCounter("player", "LIFE STEAL")) {
    TryHeal("player", damage, "HP DRAINED!");
  } else if (user == "enemy" && HasCounter("enemy", "LIFE STEAL")) {
    TryHeal("enemy", damage, "HP DRAINED!");
  }
  // player sustain
  if (deepDungeonActive && user == "player") {
    if (damage > 0 && HasMerit(MERIT_LEECH)) { TryHeal("player", (int)ceil(damage * 0.20)); QueueEffectMsg("LEECH", 0); }
    if (damage > 0 && HasMerit(MERIT_INFLOW) && _card.type == "Water") { TryHeal("player", (int)ceil(damage * 0.20)); QueueEffectMsg("INFLOW", 0); }
    if (critHit && damage > 0 && HasMerit(MERIT_BLOODRUSH)) { TryHeal("player", (int)ceil(maxHP * 0.10)); QueueEffectMsg("BLOODRUSH", 0); }
    if (critHit && HasMerit(MERIT_WHIPLASH)) whiplashPrimed = true;
  }
  // DD affliction: RECOIL — player takes a share of damage they deal (floored at 1)
  if (user == "player" && damage > 0) {
    int recoil = GetAfflictionLevel(AFFL_RECOIL);
    if (recoil) {
      HP -= (int)ceil(damage * 0.10 * recoil);
      if (HP < 1) HP = 1;
      UpdateHealthBars();
      animFrames = 0; ACTION = HURT;
      QueueEffectMsg("RECOIL", 0);
    }
  }
  // DD affliction: DRAIN — enemy heals a share of damage they deal
  if (user == "enemy" && damage > 0) {
    int drain = GetAfflictionLevel(AFFL_DRAIN);
    if (drain) TryHeal("enemy", (int)ceil(damage * 0.10 * drain));
    // DD merit: SPIKES — reflect 25% of damage received back to the enemy
    if (HasMerit(MERIT_SPIKES)) {
      enemyHP -= (int)ceil(damage * 0.25);
      if (enemyHP < 0) enemyHP = 0;
      UpdateHealthBars();
      QueueEffectMsg("SPIKES", 2);
    }
  }
}

int GetStackLevel(String target, String counterType) {
  String* counter;
  if (target == "player") counter = &playerCounter;
  else if (target == "enemy") counter = &enemyCounter;  
  else counter = &arenaCounter;
  
  if (counterType != "EARTH" && counterType != "LIGHTNING" && counterType != "ICE WALL") {
    return (*counter == counterType) ? 1 : 0;
  }
  
  if (*counter == counterType) return 1;
  if (*counter == counterType + "2") return 2;
  if (*counter == counterType + "3") return 3;
  return 0;
}

// Handle special card effects
void HandleCardEffects(String target, Card _card, float &attackModifier, float &defenseModifier) {
  String user = (target == "enemy") ? "player" : "enemy";
  
  // AFTERSHOCK: If user has Earth counter, opponent speed -1 stage
  if (_card.name == "AFTERSHOCK") {
    if (HasCounter(user, "EARTH")) {
      if (target == "enemy") {
        enemySpeedStage = max(-6, enemySpeedStage - 1);
        battleMessage = "OPP SPD DOWN";
      } else {
        speedStage = max(-6, speedStage - 1);
        battleMessage = "YOUR SPD DOWN";
      }
    }
  }
  // AIR SLASH: Remove Wind from self for 3x damage (Wind doesn't stack)
  if (_card.name == "AIR SLASH") {
    if (HasCounter(user, "WIND")) {
      attackModifier = 3.0;
      ManageCounters(user, "", true); // Remove Wind counter
    }
  }
  // BLOCK: Set flag to prevent damage and opponent effects this turn
  if (_card.name == "BLOCK") {
    if (user == "player")
      playerUsedBlock = true;
    else
      enemyUsedBlock = true;
  }
  // BLOOD MOON: Add Blood Moon arena counter
  if (_card.name == "BLOOD MOON") {
    ManageCounters("arena", "BLOOD MOON", true);
  }
  // BOIL: turn only the target's water counter into a fire counter
  if (_card.name == "BOIL") {
    if (HasCounter(target, "WATER")) {
      ManageCounters(target, "FIRE", true);
    }
  }
  // CHANNEL: user attack +1 stage, shuffle deck
  if (_card.name == "CHANNEL") {
    if (user == "player") {
      attackStage = min(6, attackStage + 1);
      Shuffle(playerDeck);
      battleMessage = "YOUR ATK UP!";
    } else {
      enemyAttackStage = min(6, enemyAttackStage + 1);
      Shuffle(enemyDeck);
      battleMessage = "OPP ATK UP!";
    }
  }
  // COMBUSTION: 4x damage if Blaze counter is active on arena
  if (_card.name == "COMBUSTION") {
    if (HasCounter("arena", "BLAZE")) {
      attackModifier = 4.0;
    }
  }
    // DESOLATION: Remove arena counter and raise attack +1
  if (_card.name == "DESOLATION") {
    ManageCounters("arena", "", true);
    if (user == "player") {
      attackStage = min(6, attackStage + 1);
      battleMessage = "YOUR ATK UP!";
    } else {
      enemyAttackStage = min(6, enemyAttackStage + 1);
      battleMessage = "OPP ATK UP!";
    }
  }
  // DISARM: Remove counter from opponent
  if (_card.name == "DISARM") {
    ManageCounters(target, "", true);
  }
  // DISCHARGE: Remove Lightning from opponent for 3x damage (Lightning stacks)
  if (_card.name == "DISCHARGE") {
    if (HasCounter(target, "LIGHTNING")) {
      attackModifier = 3.0;
      ManageCounters(target, "", true); // Remove Lightning counter
    }
  }
  // EARTHQUAKE: Damage multiplied by Earth stacks + 1
  if (_card.name == "EARTHQUAKE") {
    int stacks = GetStackLevel(user, "EARTH"); 
    if (stacks > 0) {
      attackModifier = stacks + 1; // 1x, 2x, 3x damage based on stacks
    }
  }
  // EROSION: Remove one Earth stack, damage multiplied by turns if Earth removed
  if (_card.name == "EROSION") {
    if (HasCounter(user, "EARTH")) {
      int stacks = GetStackLevel(user, "EARTH");
      if (stacks > 1) {
        SetStacks(user, "EARTH", stacks - 1);
      }
      // Only multiply damage if Earth was actually removed
      int damageMultiplier = battleTurnCount;
      if (damageMultiplier < 1) damageMultiplier = 1;
      attackModifier = damageMultiplier;
    }
    // If no Earth counter, just deal base 15 damage (no modifier)
  }
  // EXPLOSION: Remove Fire from self for 5x damage (Fire doesn't stack)
  if (_card.name == "EXPLOSION") {
    if (HasCounter(user, "FIRE")) {
      attackModifier = 5.0;
      ManageCounters(user, "", true); // Remove Fire counter
    }
  }
  // FLARE: Remove Fire counter from opponent for 3x damage (Fire doesn't stack)
  if (_card.name == "FLARE") {
    if (HasCounter(target, "FIRE")) {
      attackModifier = 3.0;
      ManageCounters(target, "", true); // Remove Fire counter (single counter only)
    }
  }
  // FORTIFY: Upgrade Earth stacks to max (3 stacks) if any exist
  if (_card.name == "FORTIFY") {
    if (HasCounter(user, "EARTH")) {
      SetStacks(user, "EARTH", 3); // Set to 3 stacks regardless of current level
    }
  }
  // FORCE RUSH: Add Wind counter and divide damage by turn count
  if (_card.name == "FORCE RUSH") {
    ManageCounters(user, "WIND", false);
    int turnDivisor = max(1, battleTurnCount);
    attackModifier = 1.0 / turnDivisor;
  }
  // FREEZE: Turn opponent water counter into ice counter
  if (_card.name == "FREEZE") {
    if (HasCounter(target, "WATER")) {
      ManageCounters(target, "ICE", true);
      battleMessage = "FROZEN!";
    }
  }
  // GALE FORCE: Remove all counters and multiply damage by count removed
  if (_card.name == "GALE FORCE") {
    int countersRemoved = 0;
    if (playerCounter != "") countersRemoved++;
    if (enemyCounter != "") countersRemoved++;
    if (arenaCounter != "") countersRemoved++;
    
    ManageCounters("player", "", true);
    ManageCounters("enemy", "", true);
    ManageCounters("arena", "", true);
    
    if (countersRemoved > 0) {
      attackModifier = countersRemoved;
    }
  }
  // HYDRATION: Add/Replace self counter for water and heals 25% max HP
  if (_card.name == "HYDRATION") {
    ManageCounters(user, "WATER", true);
    int healAmount = (user == "player") ? (int)ceil(maxHP * 0.25) : (int)ceil(enemyMaxHP * 0.25);
    TryHeal(user, healAmount);
  }
  // ICICLE : Remove self water counter for 2x damage
  if (_card.name == "ICICLE") {
    if (HasCounter(user, "WATER")) {
      attackModifier = 2.0;
      ManageCounters(user, "", true);
    }
  }
  // INCINERATE: Replace target counter with fire counter
  if (_card.name == "INCINERATE") {
    if ((target == "enemy" && enemyCounter != "") || (target == "player" && playerCounter != "")) {
      ManageCounters(target, "FIRE", true);
    }
  }
  // INFERNO: Add Blaze to arena
  if (_card.name == "INFERNO") {
    ManageCounters("arena", "BLAZE", true);
  }
  // INSULT: 0 damage, opponent defense -1 stage
  if (_card.name == "INSULT") {
    if (target == "enemy") {
      enemyDefenseStage = max(-6, enemyDefenseStage - 1);
      battleMessage = "OPP DEF DOWN";
    } else {
      defenseStage = max(-6, defenseStage - 1);
      battleMessage = "YOUR DEF DOWN";
    }
  }
  // JUXTAPOSE: Switch counters with opponent
  if (_card.name == "JUXTAPOSE") {
    String tempCounter = playerCounter;
    playerCounter = enemyCounter;
    enemyCounter = tempCounter;
    ManageCounters("player", playerCounter, true);
    ManageCounters("enemy", enemyCounter, true);
  }
  // LIFE LEECH: Lifesteal damage, 2x damage if user has Nature counter
  if (_card.name == "LIFE LEECH") {
    if (HasCounter(user, "NATURE")) {
      attackModifier = 2.0;
    }
    if (user == "player") {
      playerLifesteal = true;
    } else {
      enemyLifesteal = true;
    }
  }
  // LIFE STEAL: change user's water counter to a life steal counter
  if (_card.name == "LIFE STEAL") {
    if(HasCounter(user, "WATER"))
      ManageCounters(user, "LIFE STEAL", true);
  }
  // LIGHTNING: Damage multiplied by Lightning stacks (Lightning DOES stack)
  if (_card.name == "LIGHTNING") {
    int stacks = GetStackLevel(user, "LIGHTNING"); 
    if (stacks > 0) {
      attackModifier = stacks; // 1x, 2x, 3x damage based on stacks
    }
  }
  // LUCKY STAR: Turn Wind counter into a Lucky Star counter
  if (_card.name == "LUCKY STAR") {
    if (HasCounter(user, "WIND")) {
      ManageCounters(user, "LUCKY STAR", true);
    }
  }
  // MISTWALK: Turn water or fire counter into mist
  if (_card.name == "MISTWALK") {
    if (HasCounter(user, "WATER") || HasCounter(user, "FIRE")) {
      ManageCounters(user, "MIST", true);
    }
  }
  // NATURALIZE: Remove opponent counter for 2x damage and heal (only if counter exists)
  if (_card.name == "NATURALIZE") {
    if ((target == "enemy" && enemyCounter != "") || (target == "player" && playerCounter != "")) {
      attackModifier = 2.0;
      ManageCounters(target, "", true);
      int healAmount = (user == "player") ? (int)ceil(maxHP * 0.125) : (int)ceil(enemyMaxHP * 0.125);
      TryHeal(user, healAmount);
    }
    // If no counter exists, normal damage with no modifiers or heals
  }
  // OVERDRIVE: user gets +1 Atk +1 Speed, -1 Def
  if (_card.name == "OVERDRIVE") {
    if (user == "player") {
      attackStage = min(6, attackStage + 1);
      speedStage = min(6, speedStage + 1);
      defenseStage = min(6, defenseStage - 1);
      battleMessage = "+ATK/SPD -DEF";
    } else {
      enemyAttackStage = min(6, enemyAttackStage + 1);
      enemySpeedStage = min(6, enemySpeedStage + 1);
      enemyDefenseStage = min(6, enemyDefenseStage - 1);
      battleMessage = "+ATK/SPD -DEF";
    }
  }
  // OVERHEAT: Lowers user's defense 2 stages
  if (_card.name == "OVERHEAT") {
    if (user == "player") {
      defenseStage = max(-6, defenseStage - 2);
      battleMessage = "YOUR DEF DOWN";
    } else {
      enemyDefenseStage = max(-6, enemyDefenseStage - 2);
      battleMessage = "OPP DEF DOWN";
    }
  }
  // PAYOUT: Mark that payout was used (doesn't stack)
  if (_card.name == "PAYOUT") {
    payoutUsed = true;
  }
  // PIERCE: Ignore all defensive modifiers
  if (_card.name == "PIERCE") {
    defenseModifier = 1.0; // Override any defense bonuses
  }
  // PRESSURE: Increment usage counter and multiply damage by count
  if (_card.name == "PRESSURE") {
    pressureUsageCount++;
    attackModifier = pressureUsageCount;
  }
  // RECHARGE: Removes all Lightning counters on self for 3x damage and heal
  if (_card.name == "RECHARGE") {
    if (HasCounter(user, "LIGHTNING")) {
      ManageCounters(user, "", true); // Remove Lightning counter
      attackModifier = 3.0;
      if (user == "player") {
        playerLifesteal = true;
      } else {
        enemyLifesteal = true;
      }
    }
  }
  // RECOVER: Heals 35% of maximum health
  if (_card.name == "RECOVER") {
    int healAmount = (user == "player") ? (int)ceil(0.35 * maxHP) : (int)ceil(0.35 * enemyMaxHP);
    TryHeal(user, healAmount);
  }
  // REFRESH: Remove self counter and heal 12.5% max HP
  if (_card.name == "REFRESH") {
    ManageCounters(user, "", true);
    int healAmount = (user == "player") ? (int)ceil(maxHP * 0.125) : (int)ceil(enemyMaxHP * 0.125);
    TryHeal(user, healAmount);
  }
  // STONE RAIN: Add Sandstorm to arena
  if (_card.name == "STONE RAIN") {
    ManageCounters("arena", "SANDSTORM", true);
  }
  // STONEWALL: Add or increment earth counters to the user 
  if (_card.name == "STONEWALL") {
    AddStacks(user, "EARTH", 1);
  }
  // STORM: Add Storm to arena
  if (_card.name == "STORM") {
    ManageCounters("arena", "STORM", true);
  }
  // TERRAFORM: Upgrade Earth to Nature counter
  if (_card.name == "TERRAFORM") {
    if (HasCounter(user, "EARTH")) {
      ManageCounters(user, "NATURE", true);
    }
  }
  // THUNDER: Deals 4x damage if Storm counter is on the arena
  if (_card.name == "THUNDER") {
    if (HasCounter("arena", "STORM")) {
      attackModifier = 4.0;
    }
  }
  // TIME FLIP: Add Reversal to arena
  if (_card.name == "TIME FLIP") {
    ManageCounters("arena", "REVERSAL", true);
  }
  // TORNADO: Shuffle deck (Wind counter applied by basic system)
  if (_card.name == "TORNADO") {
    if(user == "player")
      Shuffle(playerDeck);
    else
      Shuffle(enemyDeck);
  }
  // TORRENT: Add Rain to arena
  if (_card.name == "TORRENT") {
    ManageCounters("arena", "RAIN", true);    // Always replace arena counter
  }
  // UNDERTOW: Replace existing counter with WATER and deal 2x damage
  if (_card.name == "UNDERTOW") {
    String target = (user == "player") ? "enemy" : "player";
    
    if ((target == "enemy" && enemyCounter != "") || (target == "player" && playerCounter != "")) {
      ManageCounters(target, "WATER", true);
      attackModifier = 2.0;
    }
  }
  // UPDRAFT: Replace self counter with wind counter
  if (_card.name == "UPDRAFT") {
    if ((user == "enemy" && enemyCounter != "") || (user == "player" && playerCounter != "")) {
      ManageCounters(user, "WIND", true);
    } else if (HasCounter(user, "WIND")) {
      ManageCounters(user, "WIND", true); //Keep Wind Counter
    }
  }
  // WINDSTORM: Add Gust to arena
  if (_card.name == "WINDSTORM") {
    ManageCounters("arena", "GUST", true);
  }



  //END HandleCardEffects()
} 

void BattleManager() {
  static unsigned long lastTime = 0;
  if (BATTLESTATE == COMMENCED)
    InitializeBattle();
  else if (BATTLESTATE == CARDSELECT)
    BattleCardSelection();
  else if (BATTLESTATE == ACTION1) {
    if (micros() - lastTime >= 500000) {  // check if one second has elapsed
      lastTime = micros();
      actionTime++;
    }
    // Hold B (without A) to inspect the card being used; the auto-advance timer is paused
    // while inspecting and reset the instant you let go, so it never sits frozen with no tooltip.
    if (BButton == "held" && AButton != "held" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (TOOLTIP == RELEASED) {
        TOOLTIP = ACTIVE;
        if (beginTurn.startsWith("Counter"))
          DrawCounterTooltip();          // counter-effect step: show the counter, not the card
        else
          DrawCardTooltip(tooltipCard);
      }
    }
    if (TOOLTIP == ACTIVE)
      actionTime = 0;   // hold nothing advances while the tooltip is up
    if (BButton == "released" && TOOLTIP == ACTIVE && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      TOOLTIP = RELEASED;
      actionTime = 0;   // fresh timer once they return to the battle
      tft.fillScreen(TFT_COLOR4);
      DrawBackground();
      UpdateHealthBars();
      tft.fillRect(102, 33, 36, 12, TFT_COLOR1);
      tft.fillRect(105, 12, 30, 30, TFT_COLOR4);
      ManageCounters("player", playerCounter, true);
      ManageCounters("enemy", enemyCounter, true);
      ManageCounters("arena", arenaCounter, true);
      // redraw the center piece (counter icon on counter steps, else the card in play)
      pushScaled(96, 48, 15, 15, (uint16_t*)abilityFrame[0], TFT_BLACK);
      if (beginTurn.startsWith("Counter")) {
        pushScaled(105, 57, 8, 8, (uint16_t*)counterArray[tooltipCounter.counterIndex], TFT_BLACK);
        if (battleArrowTarget >= 0) DrawEffectPointer(battleArrowTarget);  // restore the counter arrow
      } else {
        pushScaled(102, 54, 11, 11, (uint16_t*)cardSpriteArray[tooltipCard.cardIndex], TFT_BLACK);
        // restore the attack arrow pointing from the attacker
        if (battleArrowUser == "player") {
          tft.fillRect(147, 63, 3, 15, TFT_COLOR1);
          tft.fillRect(150, 66, 3, 9, TFT_COLOR1);
          tft.fillRect(153, 69, 3, 3, TFT_COLOR1);
        } else if (battleArrowUser == "enemy") {
          tft.fillRect(87, 63, 3, 15, TFT_COLOR1);
          tft.fillRect(84, 66, 3, 9, TFT_COLOR1);
          tft.fillRect(81, 69, 3, 3, TFT_COLOR1);
        }
      }
      tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      CenterText(battleMessage, 111);
    }
    //Shows the first card being used in battle
    if (beginTurn == "Begin" && actionTime == 2) {
      battleMessage = "";
      tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
      UseCard(firstPlayer);
      beginTurn = "Button1";
      actionTime = 0;
    }
    //Button is pressed and damage is assigned
    if (beginTurn == "Button1") {
      if ((BButton == "pressed" && millis() - lastButtonPressTime > 500) || actionTime >= battleAutoAdvance) {
        if (BButton == "pressed") lastButtonPressTime = millis();
        if (firstPlayer == "player")
          CalculateDamage("enemy", playerDeck[selectedBattleCard]);
        else
          CalculateDamage("player", enemyDeck[enemyBattleCard]);
        // Check if someone died
        if (HP <= 0) {
          beginTurn = "Waiting";
        }
        else if (enemyHP <= 0) {
          beginTurn = "Waiting";
        }
        else if (battleMessage == "")
          beginTurn = "Begin2";
        else {
          beginTurn = "Message1";
          CenterText(battleMessage, 111);
        }
        actionTime = 0;
        //TEST. NEED TO MAKE A CHECK TO SEE IF HEALTH DROPS TO 0 TO END BATTLE
        //BATTLESTATE = ACTION2;
      }
    }
    //Additional message is displayed if available
    if (beginTurn == "Message1" && actionTime == 4) {
      tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
      beginTurn = "Begin2";
      battleMessage = "";
    }
    //Second card is revealed
    if (beginTurn == "Begin2" && actionTime == 4) {
      tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
      UseCard(secondPlayer);
      beginTurn = "Button2";
      actionTime = 0;
    }
    //Button is pressed for second card effect/damage
    if (beginTurn == "Button2") {
      if ((BButton == "pressed" && millis() - lastButtonPressTime > 500) || actionTime >= battleAutoAdvance) {
        if (BButton == "pressed") lastButtonPressTime = millis();
        if (secondPlayer == "player")
          CalculateDamage("enemy", playerDeck[selectedBattleCard]);
        else
          CalculateDamage("player", enemyDeck[enemyBattleCard]);
        // Check if someone died
        if (HP <= 0) {
          beginTurn = "Waiting";
        }
        else if (enemyHP <= 0) {
          beginTurn = "Waiting";
        }
        else if (battleMessage == "")
          beginTurn = "Counter1";
        else {
          beginTurn = "Message2";
          CenterText(battleMessage, 111);
        }
        actionTime = 0;
        //TEST. NEED TO MAKE A CHECK TO SEE IF HEALTH DROPS TO 0 TO END BATTLE
      }
    }
    //Additional message for second card effect
    if (beginTurn == "Message2" && actionTime == 4) {
      tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
      beginTurn = "Counter1";
      battleMessage = "";
      actionTime = 0;
    }
    //End of turn checks. Begins with counters on first player
    //If counter, then show the message or skip to next step
    if (beginTurn == "Counter1" && actionTime == 4) {
      tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
      CounterEffects(firstPlayer, false);
      if (battleMessage == "")
        beginTurn = "Counter2";
      else {
        beginTurn = "Counter1Effect";
        CenterText(battleMessage, 111);
      }
      actionTime = 0;
    }
    //Player must press B to move to the next check if a counter is present
    if (beginTurn == "Counter1Effect" && actionTime >= 2) {
      if ((BButton == "pressed" && millis() - lastButtonPressTime > 500) || actionTime >= battleAutoAdvance) {
        if (BButton == "pressed") lastButtonPressTime = millis();
        tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
        battleMessage = "";
        beginTurn = "Counter2";
        actionTime = 0;
      }
    }
    //Second player counter check
    if (beginTurn == "Counter2") {
      tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
      CounterEffects(secondPlayer, false);
      if (battleMessage == "")
        beginTurn = "Counter3";
      else {
        beginTurn = "Counter2Effect";
        CenterText(battleMessage, 111);
      }
      actionTime = 0;
    }
    //Player must press B to move to the next check if a counter is present
    if (beginTurn == "Counter2Effect" && actionTime >= 2) {
      if ((BButton == "pressed" && millis() - lastButtonPressTime > 500) || actionTime >= battleAutoAdvance) {
        if (BButton == "pressed") lastButtonPressTime = millis();
        tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
        battleMessage = "";
        beginTurn = "Counter3";
        actionTime = 0;
      }
    }
    //Arena counter check
    if (beginTurn == "Counter3") {
      tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
      CounterEffects("arena", false);
      if (battleMessage == "") {
        beginTurn = "Waiting";
        actionTime = 4;
      } else {
        beginTurn = "Counter3Effect";
        CenterText(battleMessage, 111);
        actionTime = 0;
      }
    }
    //Player must press B to move to the next check if a counter is present
    if (beginTurn == "Counter3Effect" && actionTime >= 2) {
      if ((BButton == "pressed" && millis() - lastButtonPressTime > 500) || actionTime >= battleAutoAdvance) {
        if (BButton == "pressed") lastButtonPressTime = millis();
        tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
        battleMessage = "";
        beginTurn = "Waiting";
        actionTime = 4;
      }
    }
    //End of turn
    if (beginTurn == "Waiting" && actionTime == 4) {
      // Check if someone died before continuing the battle
      if (HP <= 0 || enemyHP <= 0) {
        return;  // Don't reset to card selection if someone is dead
      }
      playerUsedBlock = false;
      enemyUsedBlock = false;
      battleTurnCount++;
      // DD per-turn affliction ticks (both fighters confirmed alive above)
      if (deepDungeonActive) {
        int toxic = GetAfflictionLevel(AFFL_TOXIC);
        if (toxic) {
          HP -= (int)ceil(maxHP * 0.04 * toxic);
          if (HP < 1) HP = 1;
          animFrames = 0; ACTION = HURT;  // flash the player
          QueueEffectMsg("TOXIC", 0);     // affects the player
        }
        int renew = GetAfflictionLevel(AFFL_RENEW);
        if (renew) {
          enemyHP = min(enemyMaxHP, enemyHP + (int)ceil(enemyMaxHP * 0.05 * renew));
          QueueEffectMsg("RENEW", 2);     // affects the enemy
        }
        if (GetAfflictionLevel(AFFL_ENRAGE)) {
          enemyAttackStage = min(6, enemyAttackStage + 1);
          QueueEffectMsg("ENRAGE", 2);    // affects the enemy
        }
        int dispel = GetAfflictionLevel(AFFL_DISPEL);
        if (dispel && playerCounter != "") {
          RemoveStacks("player", playerCounter, dispel);
          QueueEffectMsg("DISPEL", 0);    // strips the player's counter
        }
        // DD merits: per-turn player effects
        if (HasMerit(MERIT_REGEN)) { TryHeal("player", (int)ceil(maxHP * 0.10)); QueueEffectMsg("REGEN", 0); }
        if (HasMerit(MERIT_SPEED_BOOST)) speedStage = min(6, speedStage + 1);
        UpdateHealthBars();
      }
      // Clear Ice at turn end
      if (HasCounter("player", "ICE")) 
        ManageCounters("player", "", true);
      if (HasCounter("enemy", "ICE")) 
        ManageCounters("enemy", "", true);
      if (!HasCounter("player", "LIFE STEAL")) 
        playerLifesteal = false;
      if (!HasCounter("enemy", "LIFE STEAL")) 
        enemyLifesteal = false;
      selectedBattleCard = 0;
      concedeMenuActive = false;
      concedeSelectNo = true;
      if (effectMsgCount > 0) {
        // Show queued effect messages one at a time, each pointing at its target
        effectMsgIndex = 0;
        tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
        CenterText(effectMsgQueue[0], 111);
        DrawEffectPointer(effectMsgTargetQ[0]);
        beginTurn = "EffectMsg";
        actionTime = 0;
      } else {
        UpdateBattleCards(selectedBattleCard, true);
        BATTLESTATE = CARDSELECT;
        actionTime = 0;
        battleMessage = "";
      }
    }
    // Drain the effect-message queue (one per delay), then start the next turn
    if (beginTurn == "EffectMsg" && actionTime == 4) {
      effectMsgIndex++;
      if (effectMsgIndex < effectMsgCount) {
        tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
        CenterText(effectMsgQueue[effectMsgIndex], 111);
        DrawEffectPointer(effectMsgTargetQ[effectMsgIndex]);
        actionTime = 0;
      } else {
        effectMsgCount = 0;
        effectMsgIndex = 0;
        tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
        // clear the effect arrow + blank center frame so they don't linger
        tft.fillRect(18, 21, 15, 15, TFT_COLOR4);
        tft.fillRect(111, -3, 15, 15, TFT_COLOR4);
        tft.fillRect(204, 21, 15, 15, TFT_COLOR4);
        tft.fillRect(96, 48, 45, 45, TFT_COLOR4);
        battleMessage = "";
        selectedBattleCard = 0;
        UpdateBattleCards(selectedBattleCard, true);
        BATTLESTATE = CARDSELECT;
        beginTurn = "Waiting";
        actionTime = 0;
      }
    }
    //TOOLTIP FOR CARDS IF HELD
    if (beginTurn == "Button1" || beginTurn == "Button2") {
      if (BButton == "held" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (TOOLTIP == RELEASED) {
          TOOLTIP = ACTIVE;
          tft.fillScreen(TFT_COLOR4);
          pushScaled(3, 3, 15, 15, abilityFrame[0], TFT_BLACK);
          tft.fillRect(51, 3, 183, 27, TFT_COLOR3);
          tft.fillRect(234, 6, 3, 27, TFT_COLOR1);
          tft.fillRect(54, 30, 183, 3, TFT_COLOR1);
          pushScaled(9, 9, 11, 11, cardSpriteArray[tooltipCard.cardIndex], TFT_BLACK);
          tft.drawString(tooltipCard.name, 54, 6);
          tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
          if (tooltipCard.damage < 100)
            tft.drawString(String(tooltipCard.damage), 201, 36);
          else
            tft.drawString(String(tooltipCard.damage), 183, 36);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          tft.drawString(tooltipCard.desc1, 3, 63);
          tft.drawString(tooltipCard.desc2, 3, 87);
          tft.drawString(tooltipCard.desc3, 3, 111);
        }
      }
      //Card tooltip released
      if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
        lastButtonPressTime = millis();
        TOOLTIP = RELEASED;
        //tft.fillRect(0, 0, 240, 51, TFT_COLOR4);
        tft.fillScreen(TFT_COLOR4);
        DrawBackground();
        UpdateHealthBars();
        if (beginTurn == "Button1")
          UseCard(firstPlayer);
        if (beginTurn == "Button2")
          UseCard(secondPlayer);
        ManageCounters("player", playerCounter, true);
        ManageCounters("enemy", enemyCounter, true);
        ManageCounters("arena", arenaCounter, true);
        CenterText(battleMessage, 111);
      }
    }
    //TOOLTIP FOR COUNTERS IF HELD
    if (beginTurn == "Counter1Effect" || beginTurn == "Counter2Effect" || beginTurn == "Counter3Effect") {
      if (BButton == "held" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (TOOLTIP == RELEASED) {
          TOOLTIP = ACTIVE;
          tft.fillScreen(TFT_COLOR4);
          pushScaled(3, 3, 15, 15, abilityFrame[0], TFT_BLACK);
          tft.fillRect(51, 3, 183, 27, TFT_COLOR3);
          tft.fillRect(234, 6, 3, 27, TFT_COLOR1);
          tft.fillRect(54, 30, 183, 3, TFT_COLOR1);
          pushScaled(12, 12, 8, 8, counterArray[tooltipCounter.counterIndex], TFT_BLACK);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          tft.drawString(tooltipCounter.name, 54, 6);
          tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
          tft.drawString(String("{" + String(tooltipCounter.abbreviation) + "}"), 168, 36);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          tft.drawString(tooltipCounter.desc1, 3, 63);
          tft.drawString(tooltipCounter.desc2, 3, 87);
          tft.drawString(tooltipCounter.desc3, 3, 111);
        }
      }
      //Card tooltip released
      if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
        lastButtonPressTime = millis();
        TOOLTIP = RELEASED;
        //tft.fillRect(0, 0, 240, 51, TFT_COLOR4);
        tft.fillScreen(TFT_COLOR4);
        DrawBackground();
        UpdateHealthBars();
        tft.fillRect(102, 33, 36, 12, TFT_COLOR1);
        tft.fillRect(105, 12, 30, 30, TFT_COLOR4);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        if (beginTurn == "Counter1Effect")
          CounterEffects(firstPlayer, true);
        else if (beginTurn == "Counter2Effect")
          CounterEffects(secondPlayer, true);
        else if (beginTurn == "Counter3Effect")
          CounterEffects("arena", true);
        ManageCounters("player", playerCounter, true);
        ManageCounters("enemy", enemyCounter, true);
        ManageCounters("arena", arenaCounter, true);
        CenterText(battleMessage, 111);
      }
    }
  } else if (BATTLESTATE == END) {
    static int result = -1;
    static int creditsEarned = 0;
    static int gearResult = -1;
    static bool isHeadGear = false;
    if (millis() - lastTime >= 750) {  // check if one second has elapsed
      lastTime = millis();
      actionTime++;
    }
    else if (beginTurn == "victory" && actionTime < 2) {
      tft.fillRect(108, 0, 24, 39, TFT_COLOR4);
      tft.fillRect(48, 24, 192, 81, TFT_COLOR4);
      tft.fillRect(48, 105, 192, 3, TFT_COLOR1);
    }
    else if (beginTurn == "victory" && actionTime == 2) {
      beginTurn = "exp";
      result = -1;  // Reset reward tracking
      gearResult = -1;
      tft.fillRect(0, 24, 240, 36, TFT_COLOR4);
      tft.fillRect(48, 24, 192, 51, TFT_COLOR4);
      tft.fillRect(112, 0, 138, 24, TFT_COLOR4);
      tft.fillRect(48, 105, 192, 3, TFT_COLOR1);
      tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
      tft.fillRect(60, 30, 120, 75, TFT_COLOR4);
      tft.fillRect(102, 18, 129, 27, TFT_COLOR1);
      tft.fillRect(99, 15, 129, 27, TFT_COLOR3);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      tft.drawString("VICTORY", 102, 18);
      // Calculate random credits (1-10, equal odds)
      creditsEarned = random(1, 11);

      // Apply multipliers
      float multiplier = 1.0;
      if (equippedGem == 5) multiplier *= 1.5;
      if (payoutUsed) multiplier *= 1.5;
      if (deepDungeonUnlocked && !deepDungeonActive) multiplier *= 2.0;

      creditsEarned = (int)(creditsEarned * multiplier);
      if (HasMerit(MERIT_BOUNTY)) creditsEarned += random(1, 4);                 // BOUNTY: +1-3 credits
      if (HasMerit(MERIT_SECOND_WIND)) { HP = min(maxHP, HP + (int)ceil(maxHP * 0.25)); SavePetStats(); }  // SECOND WIND
      AddCredits(creditsEarned);

      // Card and gear rewards depend on mode
      int gearDropChance = scryGlassActive ? 40 : 20;  // DD base 20%, doubled to 40%
      int regularGearChance = scryGlassActive ? 20 : 10;  // Regular base 10%, doubled to 20%
      if (deepDungeonActive) {
        // Drop chances as parts-per-1000 so PROSPECT's +12.5% is exact.
        int cardChance = 200;                 // 20% base DD card
        int gearChance = gearDropChance * 10;  // 200 (or 400 with Scry Glass)
        if (activeGem == 0) {                 // PROSPECT: +12.5 percentage points
          cardChance += 125;
          gearChance += 125;
        } else if (activeGem == 5) {          // GAMBLE: double the drop chance
          cardChance *= 2;
          gearChance *= 2;
        }
        if (HasMerit(MERIT_SCAVENGE)) {
          cardChance += 50;
          gearChance += 50;
        }
        if (HasMerit(MERIT_WINDFALL)) { cardChance = 1000; gearChance = 1000; RemoveMerit(MERIT_WINDFALL); }
        if (random(1000) < cardChance) {
          result = GrantDDCard(-1);
        }
        if (random(1000) < gearChance) {
          if (random(2) == 0) {
            gearResult = GrantHeadGear();
            isHeadGear = true;
          } else {
            gearResult = GrantBodyGear();
            isHeadGear = false;
          }
        }
        // Boss defeated: owe a merit draft + affliction after the victory screen
        if (((deepDungeonStage + 1) % 10 == 0) && getTotalBosses() > 0)
          bossRewardPending = true;
        // Increment DD stage
        deepDungeonStage++;
        if (deepDungeonStage > deepDungeonHighScore) {
          deepDungeonHighScore = deepDungeonStage;
        }
        SaveDeepDungeon();
      } else {
        // Regular mode: grant stage reward card
        result = GrantStageReward();
        // Regular mode: 10% gear drop (20% with Scry Glass)
        if (random(100) < regularGearChance) {
          if (random(2) == 0) {
            gearResult = GrantHeadGear();
            isHeadGear = true;
          } else {
            gearResult = GrantBodyGear();
            isHeadGear = false;
          }
        }
        // Increment and save data (cap at 30 for infinite random mode)
        if (stage < 30) {
          // Check if this unlocks DD (completing stage 29 -> stage 30)
          if (stage == 29 && !deepDungeonUnlocked) {
            deepDungeonUnlocked = true;
            ddUnlockShown = false;
            EEPROM.write(255, 1);
          }
          stage++;
          EEPROM.write(141, stage);
        }
      }
      scryGlassActive = false;  // Consume effect after battle
      
      randomEnemyGenerated = false;
      enemySpriteCreated = false;
      EEPROM.put(33, headOwnership);  // Save head gear ownership
      EEPROM.put(65, bodyOwnership);  // Save body gear ownership
      EEPROM.commit();
      // Display rewards
      if (result != -1 && gearResult != -1) {
        // Card and gear
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        
        if(creditsEarned > 9){
          tft.drawString("+ ", 63, 63);
          pushScaled(84, 66, 5, 5, smallIcons[11], TFT_BLACK);
          tft.drawString(String(creditsEarned), 102, 63);
        } else {
          tft.drawString("+ ", 81, 63);
          pushScaled(102, 66, 5, 5, smallIcons[11], TFT_BLACK);
          tft.drawString(String(creditsEarned), 120, 63);
        }
        pushScaled(141, 51, 15, 15, abilityFrame[0], TFT_BLACK);
        pushScaled(147, 57, 11, 11, cardSpriteArray[result], TFT_BLACK);
        pushScaled(192, 51, 15, 15, equipmentFrame[0], TFT_BLACK);
        if (isHeadGear) {
          pushScaled(198, 57, 11, 11, headGearIcons[gearResult], TFT_BLACK);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          CenterText(headNames[gearResult], 111);
        } else {
          pushScaled(198, 57, 11, 11, bodyGearIcons[gearResult], TFT_BLACK);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          CenterText(bodyNames[gearResult], 111);
        }
      } else if (result != -1) {
        // Card only
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        if (creditsEarned > 9) {
          tft.drawString("+ ", 93, 63);
          pushScaled(114, 66, 5, 5, smallIcons[11], TFT_BLACK);
          tft.drawString(String(creditsEarned), 132, 63);
        } else {
          tft.drawString("+ ", 111, 63);
          pushScaled(132, 66, 5, 5, smallIcons[11], TFT_BLACK);
          tft.drawString(String(creditsEarned), 150, 63);
        }
        pushScaled(174, 51, 15, 15, abilityFrame[0], TFT_BLACK);
        pushScaled(180, 57, 11, 11, cardSpriteArray[result], TFT_BLACK);
      } else if (gearResult != -1) {
        // Gear only
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        if (creditsEarned > 9) {
          tft.drawString("+ ", 93, 63);
          pushScaled(114, 66, 5, 5, smallIcons[11], TFT_BLACK);
          tft.drawString(String(creditsEarned), 132, 63);
        } else {
          tft.drawString("+ ", 111, 63);
          pushScaled(132, 66, 5, 5, smallIcons[11], TFT_BLACK);
          tft.drawString(String(creditsEarned), 150, 63);
        }
        pushScaled(174, 51, 15, 15, equipmentFrame[0], TFT_BLACK);
        if (isHeadGear) {
          pushScaled(180, 57, 11, 11, headGearIcons[gearResult], TFT_BLACK);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          CenterText(headNames[gearResult], 111);
        } else {
          pushScaled(180, 57, 11, 11, bodyGearIcons[gearResult], TFT_BLACK);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          CenterText(bodyNames[gearResult], 111);
        }
      } else {
        // Credits only
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        tft.drawString("+ ", 135, 63);
        pushScaled(156, 66, 5, 5, smallIcons[11], TFT_BLACK);
        tft.drawString(String(creditsEarned), 174, 63);
      }
    }
    else if (beginTurn == "exp") {
        //Tooltip of the card granted
        if (BButton == "held" && millis() - lastButtonPressTime > 500 && result != -1) {
          lastButtonPressTime = millis();
          if (TOOLTIP == RELEASED) {
            TOOLTIP = ACTIVE;
            tft.fillScreen(TFT_COLOR4);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            pushScaled(3, 3, 15, 15, abilityFrame[0], TFT_BLACK);
            tft.fillRect(51, 3, 183, 27, TFT_COLOR3);
            tft.fillRect(234, 6, 3, 27, TFT_COLOR1);
            tft.fillRect(54, 30, 183, 3, TFT_COLOR1);
            pushScaled(9, 9, 11, 11, cardSpriteArray[result], TFT_BLACK);
            tft.drawString(cardList[result].name, 54, 6);
            tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
            if (cardList[result].damage < 100)
              tft.drawString(String(cardList[result].damage), 201, 36);
            else
              tft.drawString(String(cardList[result].damage), 183, 36);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(cardList[result].desc1, 3, 63);
            tft.drawString(cardList[result].desc2, 3, 87);
            tft.drawString(cardList[result].desc3, 3, 111);
          }
        }
        //Card tooltip released
        if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
          lastButtonPressTime = millis();
          TOOLTIP = RELEASED;
          //tft.fillRect(0, 0, 240, 51, TFT_COLOR4);
          tft.fillScreen(TFT_COLOR4);
          DrawBackground();
          UpdateHealthBars();
          tft.fillRect(0, 24, 240, 36, TFT_COLOR4);
          tft.fillRect(48, 24, 192, 51, TFT_COLOR4);
          tft.fillRect(112, 0, 138, 24, TFT_COLOR4);
          tft.fillRect(48, 105, 192, 3, TFT_COLOR1);
          tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
          tft.fillRect(60, 30, 120, 75, TFT_COLOR4);
          tft.fillRect(102, 18, 129, 27, TFT_COLOR1);
          tft.fillRect(99, 15, 129, 27, TFT_COLOR3);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          tft.drawString("VICTORY", 102, 18);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          // Display rewards
          if (result != -1 && gearResult != -1) {
            // Card and gear
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString("+ ", 81, 63);
            pushScaled(102, 66, 5, 5, smallIcons[11], TFT_BLACK);
            tft.drawString(String(creditsEarned), 120, 63);
            pushScaled(141, 51, 15, 15, abilityFrame[0], TFT_BLACK);
            pushScaled(147, 57, 11, 11, cardSpriteArray[result], TFT_BLACK);
            pushScaled(192, 51, 15, 15, equipmentFrame[0], TFT_BLACK);
            if (isHeadGear) {
              pushScaled(198, 57, 11, 11, headGearIcons[gearResult], TFT_BLACK);
              tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
              CenterText(headNames[gearResult], 111);
            } else {
              pushScaled(198, 57, 11, 11, bodyGearIcons[gearResult], TFT_BLACK);
              tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
              CenterText(bodyNames[gearResult], 111);
            }
          } else if (result != -1) {
            // Card only
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString("+ ", 111, 63);
            pushScaled(132, 66, 5, 5, smallIcons[11], TFT_BLACK);
            tft.drawString(String(creditsEarned), 150, 63);
            pushScaled(174, 51, 15, 15, abilityFrame[0], TFT_BLACK);
            pushScaled(180, 57, 11, 11, cardSpriteArray[result], TFT_BLACK);
          } else if (gearResult != -1) {
            // Gear only
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString("+ ", 111, 63);
            pushScaled(132, 66, 5, 5, smallIcons[11], TFT_BLACK);
            tft.drawString(String(creditsEarned), 150, 63);
            pushScaled(174, 51, 15, 15, equipmentFrame[0], TFT_BLACK);
            if (isHeadGear) {
              pushScaled(180, 57, 11, 11, headGearIcons[gearResult], TFT_BLACK);
              tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
              CenterText(headNames[gearResult], 111);
            } else {
              pushScaled(180, 57, 11, 11, bodyGearIcons[gearResult], TFT_BLACK);
              tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
              CenterText(bodyNames[gearResult], 111);
            }
          } else {
            // Credits only
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString("+ ", 135, 63);
            pushScaled(156, 66, 5, 5, smallIcons[11], TFT_BLACK);
            tft.drawString(String(creditsEarned), 174, 63);
          }
        }
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
         // Free enemy sprite memory
        for (int i = 0; i < 12; i++) {
          if (enemyPet[i] != nullptr) {
            delete[] enemyPet[i];
            enemyPet[i] = nullptr;
          }
          if (mirrorEnemyPet[i] != nullptr) {
            delete[] mirrorEnemyPet[i];
            mirrorEnemyPet[i] = nullptr;
          }
        }
        enemySpriteCreated = false;  // Reset flag so sprite gets recreated
        
        // Check if DD was just unlocked
        if (deepDungeonUnlocked && !ddUnlockShown && !deepDungeonActive) {
          ddUnlockShown = true;
          // Show DD unlock notification
          tft.fillRect(51, 30, 192, 75, TFT_COLOR4);
          tft.fillRect(99, 15, 132, 30, TFT_COLOR4);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          CenterText("DEEP DUNGEON", 33);
          tft.drawString("UNLOCKED!", 66, 60);
          return; // Wait for another B press
        }
        
        AMENUS = NA;
        MENUSTATE = MAINMENU;
        testActive = false;
        shopRefresh = true;
        beginTurn = "Waiting";  // Reset battle state for next battle
        // Boss reward: route to the merit draft (then affliction) instead of the cooldown.
        if (ACTION != INJURED && bossRewardPending) {
          bossRewardPending = false;
          bossMeritDraft = true;
          RollMeritDraft();
          ddMeritSel = 0;
          ddMeritSelect = true;
          TOOLTIP = RELEASED;
          MENUSTATE = VENTUREMENU;
          BATTLESTATE = COMMENCED;
          actionTime = 0;
          injuryTime = 0;
          ACTION = IDLE;
          tft.fillScreen(TFT_COLOR4);
          DrawDDMeritSelect();
          return;
        }
        // Ensure timer states don't block menu navigation after victory
        if (ACTION != INJURED) {
          injuryTime = 0;
          ventureTime = debug ? 11 : 1801;
          countdownSeconds = ventureTime;   // post-battle cooldown before the next fight
          battleReady = false;               // must re-select a battle from the menu
          VENTURESTATE = VENTURING;          // cooldown counts as venturing (blocks sleep/idle)
          ACTION = WALKING;                  // pet walks in place during the cooldown
          SavePetStats();  // Persist to EEPROM immediately
        }
        tft.fillScreen(TFT_COLOR4);
        DrawBackground();
        DrawMainMenu();
        randomIdle = 3;
        xPosition = 99;
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        display.fillRect(0, 0, 240, 54, TFT_COLOR4);
        display.fillRect(0, 54, 240, 3, TFT_COLOR1);
        DrawCharacter(xPosition, 60, true);
        selectedWardrobe = 0;
        mainMenuReturn = 0;
        BATTLESTATE = COMMENCED;
        actionTime = 0;
        if (ACTION == INJURED) {
          injuryTime = 3600;
          if(debug)
            injuryTime = 10;
          countdownSeconds = injuryTime;
          VENTURESTATE = INACTIVE;   // venture ends on defeat
        }
      }
    }
    else if (beginTurn == "defeat" && actionTime < 2) {
      tft.fillRect(108, 0, 24, 39, TFT_COLOR4);
      tft.fillRect(48, 24, 192, 81, TFT_COLOR4);
      tft.fillRect(48, 105, 192, 3, TFT_COLOR1);
    }
    else if (beginTurn == "defeat" && actionTime == 2) {
      beginTurn = "exp";
      tft.fillRect(48, 24, 192, 81, TFT_COLOR4);
      tft.fillRect(192, 60, 48, 48, TFT_COLOR4);
      tft.fillRect(0, 24, 240, 36, TFT_COLOR4);
      tft.fillRect(48, 24, 192, 51, TFT_COLOR4);
      tft.fillRect(112, 0, 138, 24, TFT_COLOR4);
      tft.fillRect(48, 105, 192, 3, TFT_COLOR1);
      tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
      tft.fillRect(60, 30, 120, 75, TFT_COLOR4);
      tft.fillRect(102, 18, 111, 27, TFT_COLOR1);
      tft.fillRect(99, 15, 111, 27, TFT_COLOR3);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      bool ddRevive = deepDungeonActive && activeGem == 2 && !ddReviveUsed;
      tft.drawString(ddRevive ? "REVIVE" : "DEFEAT", 102, 18);

      // Calculate credit loss (10% of total, rounded down)
      int creditsLost = playerCredits / 10;
      if (creditsLost > 0) {
        playerCredits -= creditsLost;
        SaveCredits();
      }
      randomEnemyGenerated = false;
      enemySpriteCreated = false;
      // Display credit loss and DD floor if applicable
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      if (deepDungeonActive) {
        // Credit loss (centered; room for 3 digits now that the floor moved down)
        tft.drawString("-", 102, 57);
        pushScaled(123, 60, 5, 5, smallIcons[11], TFT_BLACK);
        tft.drawString(String(creditsLost), 141, 57);
        // DD floor reached, shown cleanly at the bottom center
        tft.fillRect(0, 108, 240, 27, TFT_COLOR4);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        CenterText("DD FLOOR " + String(deepDungeonStage + 1), 111);
        if (ddRevive) {
          // REVIVE gem: consume it and keep the run on this floor (no reset,
          // no injury); restore to 50% HP so the floor can be re-attempted.
          ddReviveUsed = true;
          EEPROM.write(371, 1);
          EEPROM.commit();
          HP = (int)ceil(maxHP * 0.5);
          if (HP < 1) HP = 1;
          ACTION = IDLE;
          SavePetStats();
        } else {
          // Reset DD mode on defeat
          ResetDeepDungeon();
          ACTION = INJURED;
        }
      } else {
        tft.drawString("-", 102, 57);
        pushScaled(123, 60, 5, 5, smallIcons[11], TFT_BLACK);
        tft.drawString(String(creditsLost), 141, 57);
      }
    }
    else if (beginTurn == "concede" && actionTime < 2) {
      tft.fillRect(108, 0, 24, 39, TFT_COLOR4);
      tft.fillRect(48, 24, 192, 81, TFT_COLOR4);
      tft.fillRect(48, 105, 192, 3, TFT_COLOR1);
    }
    else if (beginTurn == "concede" && actionTime == 2) {
      beginTurn = "exp";
      tft.fillRect(0, 24, 240, 36, TFT_COLOR4);
      tft.fillRect(48, 24, 192, 51, TFT_COLOR4);
      tft.fillRect(112, 0, 138, 24, TFT_COLOR4);
      tft.fillRect(48, 105, 192, 3, TFT_COLOR1);
      tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
      tft.fillRect(60, 30, 120, 75, TFT_COLOR4);
      tft.fillRect(102, 18, 129, 27, TFT_COLOR1);
      tft.fillRect(99, 15, 129, 27, TFT_COLOR3);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      tft.drawString("CONCEDE", 102, 18);
      
      // Calculate credit loss (10% of total, rounded down)
      int creditsLost = playerCredits / 10;
      if (creditsLost > 0) {
        playerCredits -= creditsLost;
        SaveCredits();
      }
      randomEnemyGenerated = false;
      enemySpriteCreated = false;
      
      // Display credit loss and DD floor if applicable
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      if (deepDungeonActive) {
        // Show DD floor reached
        String ddFloor = "D" + String(deepDungeonStage + 1);
        tft.drawString("-", 75, 57);
        pushScaled(96, 60, 5, 5, smallIcons[11], TFT_BLACK);
        tft.drawString(String(creditsLost), 114, 57);
        tft.drawString(ddFloor, 156, 57);
        // Reset DD mode on defeat
        ResetDeepDungeon();
        // NO injury for concede
      } else {
        tft.drawString("-", 102, 57);
        pushScaled(123, 60, 5, 5, smallIcons[11], TFT_BLACK);
        tft.drawString(String(creditsLost), 141, 57);
        // NO injury for concede
      }
    }
  }
  if (TOOLTIP != ACTIVE && concedeMenuActive == false) {
    DrawCharacter(0, 60, true);
    if (BATTLESTATE != END && beginTurn != "exp" && MENUSTATE == BATTLE)
      DrawEnemy(192);
  }
}
