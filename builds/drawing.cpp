#include "globals.h"
#include <TimeLib.h>
#include <EEPROM.h>

// nearest-neighbor 3x upscale of a 1x sprite at draw time
void pushScaled(int x, int y, int w, int h, const uint16_t* data, uint16_t transparent) {
  static uint16_t row[240];
  for (int sy = 0; sy < h; sy++) {
    for (int sx = 0; sx < w; sx++) {
      uint16_t c = RemapColor(pgm_read_word(&data[sy * w + sx]));
      row[sx * 3] = row[sx * 3 + 1] = row[sx * 3 + 2] = c;
    }
    for (int dy = 0; dy < 3; dy++)
      tft.pushImage(x, y + sy * 3 + dy, w * 3, 1, row, transparent);
  }
}

// nearest-neighbor 3x upscale, opaque (no transparent color)
void pushScaled(int x, int y, int w, int h, const uint16_t* data) {
  static uint16_t row[240];
  for (int sy = 0; sy < h; sy++) {
    for (int sx = 0; sx < w; sx++) {
      uint16_t c = RemapColor(pgm_read_word(&data[sy * w + sx]));
      row[sx * 3] = row[sx * 3 + 1] = row[sx * 3 + 2] = c;
    }
    for (int dy = 0; dy < 3; dy++)
      tft.pushImage(x, y + sy * 3 + dy, w * 3, 1, row);
  }
}

void pushScaled(TFT_eSprite &dst, int x, int y, int w, int h, const uint16_t* data, uint16_t transparent) {
  static uint16_t row[240];
  for (int sy = 0; sy < h; sy++) {
    for (int sx = 0; sx < w; sx++) {
      uint16_t c = RemapColor(pgm_read_word(&data[sy * w + sx]));
      row[sx * 3] = row[sx * 3 + 1] = row[sx * 3 + 2] = c;
    }
    for (int dy = 0; dy < 3; dy++)
      dst.pushImage(x, y + sy * 3 + dy, w * 3, 1, row, transparent);
  }
}

// Full-image blit with palette remap (baked screens such as the title)
void PushImageRemap(int x, int y, int w, int h, const uint16_t* data) {
  static uint16_t row[240];
  for (int sy = 0; sy < h; sy++) {
    for (int sx = 0; sx < w; sx++) row[sx] = RemapColor(pgm_read_word(&data[sy * w + sx]));
    tft.pushImage(x, y + sy, w, 1, row);
  }
}


void DisplayBatteryPercentage() {
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);

  if (BL.getBatteryVolts() < 4.5) {
    if (percentage == 100)
      tft.drawString(String(percentage) + "%", 123, 3);
    if (percentage < 100 && percentage >= 10)
      tft.drawString(" " + String(percentage) + "%", 123, 3);
    if (percentage < 10)
      tft.drawString("  " + String(percentage) + "%", 123, 3);
  } else
    tft.drawString(String("CHRG"), 123, 3);
}

void DisplayTime() {
  static unsigned long lastTime = 0;
  static boolean colonVisible = true;
  int currentHour = hour();
  int currentMinute = minute();
  int currentSecond = second();

  if (twelveHr) {  //sets the time to 12hr instead of 24h format. No AM/PM due to screen size constraints
    if (currentHour == 0) {
      currentHour = 12;
    } else if (currentHour > 12) {
      currentHour -= 12;
    }
  }

  if (micros() - lastTime >= 1000000) {  // check if one second has elapsed
    lastTime = micros();
    colonVisible = !colonVisible;  // toggle visibility every second
    if (colonVisible) {
      sprintf(timeStr, "%02d:%02d", currentHour, currentMinute);  // format time with colon
    } else {
      sprintf(timeStr, "%02d %02d", currentHour, currentMinute);  // format time without colon
    }
  }
  tft.drawString(String(timeStr), 102, 3);
}

void DisplayVentureTime() {
  if (MENUSTATE == TITLE || gamePause || ACTION == SLEEPING || VENTURESTATE == ENCOUNTER)
    return;
  // Floor Map item skips the cooldown (drop to 1s so the tick below clears it cleanly)
  if (floorMapActive && countdownSeconds > 1) {
    countdownSeconds = 1;
    floorMapActive = false;
  }
  // Idle: nothing to show when no cooldown is running and no battle is queued
  if (countdownSeconds == 0 && !battleReady)
    return;

  static unsigned long lastTime = 0;
  if (micros() - lastTime >= 1000000) {  // check if one second has elapsed
    lastTime = micros();
    if (countdownSeconds > 0) {
      countdownSeconds--;
      if (countdownSeconds == 0 && !battleReady) {
        // Cooldown finished: drop to idle
        VENTURESTATE = INACTIVE;
        if (ACTION != EATING)
          ACTION = IDLE;
        xPosition = 99;
        tft.fillRect(48, 24, 192, 27, TFT_COLOR4);  // always wipe the timer + icon + bar
        if (screenOff) {
          // Player is away: fire the rainbow ping (bounded by the LED timeout, or until they wake)
          cooldownDoneNotify = true;
        } else if (!randomEventPending) {
          // Not mid-event: full idle redraw. During an event the wipe above is enough
          // so we don't clobber the event display.
          tft.fillScreen(TFT_COLOR4);
          DrawBackground();
          DrawMainMenu();
          DrawCharacter(xPosition, 60, true);
        }
        return;
      }
    }
    int remainingMinutes = countdownSeconds / 60;
    int remainingSeconds = countdownSeconds % 60;
    if (MENUSTATE == MAINMENU && eventState != EVENT_PROMPT && eventState != EVENT_RESULT) {
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      if (countdownSeconds > 0) {
        sprintf(timeStr, "%02d:%02d", remainingMinutes, remainingSeconds);
        tft.drawString(String(timeStr), 57, 27);
      } else {
        // countdown reached 0 with a battle queued: READY
        tft.drawString(String("READY"), 57, 27);
        if (ACTION != EATING)
          ACTION = IDLE;
        if (AMENUS == NA && !randomEventPending) {
          tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          tft.drawString(String("BATTLE"), 24 * 3, 37 * 3);
        }
      }
      tft.fillRect(150, 30, 21, 21, TFT_COLOR1);
      tft.fillRect(147, 27, 21, 21, TFT_COLOR3);
      pushScaled(150, 30, 5, 5, smallIcons[2], TFT_BLACK);
      tft.fillRect(177, 36, 60, 12, TFT_COLOR3);   // empty bar container
      int result = round(((ventureTime - countdownSeconds) * 100.0 / ventureTime) / 5) * 5 / 5;
      tft.fillRect(174, 33, (result * 3), 12, TFT_COLOR1);
    }
  }
}

void DisplayInjuryTime() {
  if (MENUSTATE == TITLE || gamePause || ACTION != INJURED)
    return;
  
  // Safety check: if injuryTime is 0 but we're still INJURED, fix the state
  if (injuryTime == 0) {
    injuryTime = 3600;  // Reset to 1 hour default
    countdownSeconds = 0;  // But keep countdown at 0 so they can revive
  }
  
  static unsigned long lastTime = 0;

  if (micros() - lastTime >= 1000000) {  // check if one second has elapsed
    lastTime = micros();
    if (countdownSeconds > 0)
      countdownSeconds--;
    // Calculate remaining minutes and seconds
    int remainingMinutes = countdownSeconds / 60;
    int remainingSeconds = countdownSeconds % 60;
    if (MENUSTATE == MAINMENU) {
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      if (countdownSeconds > 0) {
        sprintf(timeStr, "%02d:%02d", remainingMinutes, remainingSeconds);
        tft.drawString(String(timeStr), 57, 27);
      } else {
        tft.drawString(String("REVIVE"), 39, 27);
        if(AMENUS == NA){
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          tft.drawString(String("REVIVE"), 21 * 3, 37 * 3);
        }
      }
      int result = round(((injuryTime - countdownSeconds) * 100.0 / injuryTime) / 5) * 5 / 5;
      tft.fillRect(174, 33, (result * 3), 12, TFT_COLOR1);
    }
  }
}

void DrawScene() {
  if (MENUSTATE == MAINMENU) {
    //DisplayTime();
    // Skip normal drawing during event prompt/result only
    if (randomEventPending && (eventState == EVENT_PROMPT)) {
      ToggleMenus();
      return;
    }
    if (randomEventPending && (eventState == EVENT_RESULT)) {
      display.fillRect(0, 0, 240, 54, TFT_COLOR4);
      display.fillRect(0, 54, 240, 3, TFT_COLOR1);
      DrawCharacter(xPosition, 60, true);
      ToggleMenus();
      return;
    }
    // Begin Random Event
    if (randomEventPending && eventState == EVENT_ALERT) {
      emoteFrames = 3;
      display.fillRect(0, 0, 240, 54, TFT_COLOR4);
      display.fillRect(0, 54, 240, 3, TFT_COLOR1);
      DrawCharacter(xPosition, 60, true);
      ToggleMenus();
      return;
    }
    // Random event walk-up: UpdateEventWalk owns the pet, skip the venture/idle render
    if (randomEventPending && eventState == EVENT_WALKING) {
      ToggleMenus();
      return;
    }
    if (ACTION == EATING) {
      DrawCharacter(xPosition, 60, true);   // per-frame so the item B-skip stays responsive
    } else if ((ACTION == WALKING || ACTION == IDLE) && VENTURESTATE == INACTIVE && !randomEventPending) {
      if (countdownSeconds > 0)
        VentureWalk();   // cooldown running: park the pet so it doesn't cover the timer bar
      else
        WalkPet();
    } else if (VENTURESTATE == VENTURING) {
      VentureWalk();
    } else if (VENTURESTATE == ENCOUNTER) {
      VentureEncounter();
    } else if (!randomEventPending) {
      DrawCharacter(99, 60, false);
    }
    if(cropReady && !randomEventPending)
      pushScaled(93, 6, 7, 5, cropReadyIcon[0], TFT_BLACK);
    if (!randomEventPending)
      ToggleMenus();
  } else if (MENUSTATE == FARM) {
    DrawFarm();
    ToggleMenus();
  } else if (MENUSTATE != MAINMENU && MENUSTATE != BATTLE && MENUSTATE != TUTORIAL && VENTURESTATE != ENCOUNTER) {
    //drawingBatteryIcon(batteryAnim);
    if (VENTURESTATE != VENTURING)
      xPosition = 99;
    else
      xPosition = 51;
    randomIdle = 3;
    moveLeft = false;
    if (ACTION == WALKING && VENTURESTATE == INACTIVE)
      ACTION = IDLE;
    ToggleMenus();
  } else if (MENUSTATE == BATTLE) {
    BattleManager(); 
  }else if (MENUSTATE == TUTORIAL) {
    xPosition = 189;
    TutorialManager();
  }
}

void DrawMainMenu() {
  if(eventState != EVENT_PROMPT && eventState != EVENT_RESULT){
    //BATTERY
    drawingBatteryIcon(batteryAnim);
    //DisplayBatteryPercentage();

    //HEALTH BAR
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
    //Venture Mode Bar
    if (VENTURESTATE == VENTURING) {
      tft.fillRect(150, 30, 21, 21, TFT_COLOR1);
      tft.fillRect(147, 27, 21, 21, TFT_COLOR3);
      pushScaled(150, 30, 5, 5, smallIcons[2], TFT_BLACK);
      tft.fillRect(177, 36, 60, 12, TFT_COLOR3);
      int result = round(((ventureTime - countdownSeconds) * 100.0 / ventureTime) / 5) * 5 / 5;
      tft.fillRect(174, 33, (result * 3), 12, TFT_COLOR1);
    } else {
      tft.fillRect(0, 27, 240, 24, TFT_COLOR4);
    }
    //Injury Bar
    if (ACTION == INJURED) {
      tft.fillRect(150, 30, 21, 21, TFT_COLOR1);
      tft.fillRect(147, 27, 21, 21, TFT_COLOR3);
      tft.fillRect(150, 36, 15, 3, TFT_COLOR1);
      tft.fillRect(156, 30, 3, 15, TFT_COLOR1);
      tft.fillRect(177, 36, 60, 12, TFT_COLOR3);
      //int result = round(((injuryTime - countdownSeconds) * 100.0 / injuryTime) / 5) * 5 / 5;
      //tft.fillRect(174, 33, (result * 3), 12, TFT_COLOR1);
    }

    if(cropReady){
      pushScaled(93, 6, 7, 5, cropReadyIcon[0], TFT_BLACK);
    }
    //Do not disturb icon
    if (doNotDisturb) {
      tft.fillRect(177, 6, 15, 15, TFT_COLOR3);
      tft.fillRect(180, 12, 9, 3, TFT_COLOR1);
      tft.fillRect(177, 6, 3, 3, TFT_COLOR4);
      tft.fillRect(189, 6, 3, 3, TFT_COLOR4);
      tft.fillRect(177, 18, 3, 3, TFT_COLOR4);
      tft.fillRect(189, 18, 3, 3, TFT_COLOR4);
    }
    //Pause notification
    if (gamePause) {
      tft.fillRect(48, 57, 147, 27, TFT_COLOR1);
      tft.fillRect(45, 54, 147, 27, TFT_COLOR3);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      tft.drawString(String("PAUSED"), 48, 57);

      tft.fillRect(162, 57, 6, 21, TFT_COLOR1);
      tft.fillRect(174, 57, 6, 21, TFT_COLOR1);
    }
  }
}

void DrawBackground() {
  tft.fillRect(0, 105, 240, 3, TFT_COLOR1);
  tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
  if(debug){
    tft.setTextColor(TFT_GREEN, TFT_COLOR3);
    CenterText("DEBUG MODE", 111);
  }
}

void DrawCharacter(int xPos, int yPos, bool mirror) {
  static unsigned long lastTime = 0;
  static unsigned long skipCheerNext = 0;

  // A/B-to-skip the eating animation. Checked every frame, BEFORE the throttle below, so a press
  // is caught immediately instead of only on the ~750ms animation ticks. On skip we play a quick
  // one-shot "half cheer" (sunshine emote opening then closing, ~400ms) so it's clear the item
  // was used, then drop to idle. eatSkipReady is reset in UseItem so the confirm press can't skip;
  // it arms once the buttons are released, then a fresh A or B press triggers the skip.
  if (ACTION == EATING) {
    if (skipCheerStage != 0) {                 // mid skip-cheer flash
      if (millis() >= skipCheerNext) {
        display.fillRect(0, 0, 240, 54, TFT_COLOR4);
        display.fillRect(0, 54, 240, 3, TFT_COLOR1);
        if (skipCheerStage == 1) {             // sunshine -> close
          display.pushImage(xPos, yPos - 51, 48, 48, GetMirrorPet(7), TFT_BLACK);
          pushScaled(display, xPos + 48, yPos - 51, 8, 8, (uint16_t*)emotes[14], TFT_BLACK);
          display.pushSprite(0, 51, TFT_BLACK);
          skipCheerStage = 2;
          skipCheerNext = millis() + 200;
        } else {                               // flash done -> clean idle
          skipCheerStage = 0;
          animFrames = 0;
          playerFrames = 0;
          if (VENTURESTATE == INACTIVE)
            ACTION = IDLE;
          else
            ACTION = WALKING;
          lastTime = 0;
          display.pushImage(xPos, yPos - 51, 48, 48, GetMirrorPet(playerFrames), TFT_BLACK);
          display.pushSprite(0, 51, TFT_BLACK);
        }
      }
      return;
    }
    if (eatSkipReady && (AButton == "pressed" || BButton == "pressed")) {  // fresh press -> skip
      display.fillRect(0, 0, 240, 54, TFT_COLOR4);
      display.fillRect(0, 54, 240, 3, TFT_COLOR1);
      display.pushImage(xPos, yPos - 51, 48, 48, GetMirrorPet(7), TFT_BLACK);
      pushScaled(display, xPos + 48, yPos - 51, 8, 8, (uint16_t*)emotes[2], TFT_BLACK);  // sunshine
      display.pushSprite(0, 51, TFT_BLACK);
      skipCheerStage = 1;
      skipCheerNext = millis() + 200;
      eatSkipReady = false;
      AButton = "";
      BButton = "";                            // consume so ToggleMenus doesn't also act on this press
      lastButtonPressTime = millis();
      return;
    }
    if (AButton != "pressed" && BButton != "pressed")  // confirm press released; skipping is armed
      eatSkipReady = true;
  }

  // A/B-to-skip the wake-up cheer, checked every frame (before the throttle) like eating so
  // quick presses aren't dropped. Positive random-event results play this same cheer with the
  // button handler gated, so a press here also advances the event instead of getting stuck.
  if (ACTION == WAKE) {
    static bool wakeSkipReady = false;
    if (AButton != "pressed" && BButton != "pressed")
      wakeSkipReady = true;   // arm once the wake/confirm press is released
    if (wakeSkipReady && (AButton == "pressed" || BButton == "pressed")) {
      animFrames = 0;
      emoteFrames = 14;
      randomIdle = 3;
      ACTION = IDLE;
      wakeSkipReady = false;
      AButton = "";
      BButton = "";
      lastButtonPressTime = millis();
      if (randomEventPending && eventState == EVENT_RESULT)
        HandleEventButton();  // one press: skip the cheer and close the result
      return;
    }
  }

  if ((gamePause || millis() - lastTime < 750) && ACTION != HURT)
    return;
  yPos = yPos - 51;
  if (ACTION == IDLE) {
    lastTime = millis();
    if (playerFrames == 3) {
      playerFrames = 0;
      int rand;
      if (MENUSTATE != BATTLE) {
        switch (EMOTION) {
          case HUNGRY:
            emoteFrames = 10;
            rand = random(1, 6);
            if (rand == 5)
              playerFrames = 8;
            break;
          case TIRED:
            emoteFrames = 12;
            rand = random(1, 6);
            if (rand == 5)
              playerFrames = 8;
            break;
          case H_T:
            emoteFrames = 12;
            rand = random(1, 6);
            if (rand == 5)
              playerFrames = 8;
            break;
          case HAPPY:
            emoteFrames = 2;
            rand = random(1, 6);
            if (rand == 5)
              playerFrames = 7;
            break;
          case UNHAPPY:
            emoteFrames = 8;
            rand = random(1, 6);
            if (rand == 5)
              playerFrames = 8;
            break;
          case ANGRY:
            emoteFrames = 7;
            rand = random(1, 6);
            if (rand == 5)
              playerFrames = 6;
            break;
          case ALERT:
            emoteFrames = 3;
            break;
          default:
            emoteFrames = 14;
            break;
        }
      }
    } else {
      playerFrames = 3;
      if (MENUSTATE != BATTLE) {
        switch (EMOTION) {
          case HUNGRY:
            emoteFrames = 11;
            break;
          case TIRED:
            emoteFrames = 13;
            break;
          case H_T:
            emoteFrames = 10;
            break;
          case HAPPY:
            emoteFrames = 14;
            break;
          case UNHAPPY:
            emoteFrames = 9;
            break;
          case ANGRY:
            emoteFrames = 14;
            break;
          case ALERT:
            emoteFrames = 14;
            break;
          default:
            emoteFrames = 14;
            break;
        }
      }
    }
    if (MENUSTATE != BATTLE) {
      if (mirror)
        display.pushImage(xPos, yPos, 48, 48, GetMirrorPet(playerFrames), TFT_BLACK);
      else
        display.pushImage(xPos, yPos, 48, 48, (uint16_t*)pet[playerFrames], TFT_BLACK);
      pushScaled(display, xPos + 48, yPos, 8, 8, (uint16_t*)emotes[emoteFrames], TFT_BLACK);
      display.pushSprite(0, 51, TFT_BLACK);
    } else {
      if (mirror)
        playerSprite.pushImage(0, 0, 48, 48, GetMirrorPet(playerFrames), TFT_BLACK);
      else
        playerSprite.pushImage(0, 0, 48, 48, (uint16_t*)pet[playerFrames], TFT_BLACK);
      playerSprite.pushSprite(0, 60, TFT_BLACK);
    }
  }  //END IDLE
  else if (ACTION == WALKING) {
    lastTime = millis();
    if (playerFrames == 1) {
      playerFrames = 0;
      int rand;
      switch (EMOTION) {
        case HUNGRY:
          emoteFrames = 10;
          rand = random(1, 11);
          if (rand == 10)
            playerFrames = 8;
          break;
        case TIRED:
          emoteFrames = 12;
          rand = random(1, 11);
          if (rand == 10)
            playerFrames = 8;
          break;
        case H_T:
          emoteFrames = 12;
          rand = random(1, 11);
          if (rand == 10)
            playerFrames = 8;
          break;
        case HAPPY:
          emoteFrames = 2;
          rand = random(1, 11);
          if (rand == 10)
            playerFrames = 7;
          break;
        case UNHAPPY:
          emoteFrames = 8;
          rand = random(1, 11);
          if (rand == 10)
            playerFrames = 8;
          break;
        case ANGRY:
          emoteFrames = 7;
          rand = random(1, 11);
          if (rand == 10)
            playerFrames = 6;
          break;
        default:
          emoteFrames = 14;
          break;
      }
    } else {
      playerFrames = 1;
      switch (EMOTION) {
        case HUNGRY:
          emoteFrames = 11;
          break;
        case TIRED:
          emoteFrames = 13;
          break;
        case H_T:
          emoteFrames = 10;
          break;
        case HAPPY:
          emoteFrames = 14;
          break;
        case UNHAPPY:
          emoteFrames = 9;
          break;
        case ANGRY:
          emoteFrames = 14;
          break;
        default:
          emoteFrames = 14;
          break;
      }
    }
    if (mirror)
      display.pushImage(xPos, yPos, 48, 48, GetMirrorPet(playerFrames), TFT_BLACK);
    else
      display.pushImage(xPos, yPos, 48, 48, (uint16_t*)pet[playerFrames], TFT_BLACK);
    if (VENTURESTATE != ENCOUNTER)
      pushScaled(display, xPos + 48, yPos, 8, 8, (uint16_t*)emotes[emoteFrames], TFT_BLACK);
    display.pushSprite(0, 51, TFT_BLACK);
  }  //END WALKING
  else if (ACTION == SLEEPING) {
    //Auto-wake after 8 hours (28800000ms)
    if (millis() - sleepStartTime >= 28800000) {
      animFrames = 0;
      ACTION = WAKE;
      AMENUS = NA;
      tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
      return;
    }
    lastTime = millis();
    if (animFrames >= 2) {
      playerFrames = 4;
      emoteFrames = 0;
      animFrames++;
    } else {
      playerFrames = 5;
      emoteFrames = 1;
      animFrames++;
    }
    if (animFrames == 4)
      animFrames = 0;
    display.pushImage(xPos, yPos, 48, 48, (uint16_t*)pet[playerFrames], TFT_BLACK);
    pushScaled(display, xPos + 48, yPos, 8, 8, (uint16_t*)emotes[emoteFrames], TFT_BLACK);
    display.pushSprite(0, 51, TFT_BLACK);
  }  //END SLEEPING
  else if (ACTION == WAKE) {
    if (animFrames > 4) {
      animFrames = 0;
      emoteFrames = 14;
      randomIdle = 3;
      ACTION = IDLE;
      return;
    }
    lastTime = millis();
    if (playerFrames == 3) {
      playerFrames = 7;
      emoteFrames = 2;
      animFrames++;
    } else {
      playerFrames = 3;
      emoteFrames = 14;
      animFrames++;
    }
    if(MENUSTATE != BATTLE && mirror) {
      display.pushImage(xPos, yPos, 48, 48, GetMirrorPet(playerFrames), TFT_BLACK);
      pushScaled(display, xPos + 48, yPos, 8, 8, (uint16_t*)emotes[emoteFrames], TFT_BLACK);
      display.pushSprite(0, 51, TFT_BLACK);
    } else if (MENUSTATE != BATTLE) {
      display.pushImage(xPos, yPos, 48, 48, (uint16_t*)pet[playerFrames], TFT_BLACK);
      pushScaled(display, xPos + 48, yPos, 8, 8, (uint16_t*)emotes[emoteFrames], TFT_BLACK);
      display.pushSprite(0, 51, TFT_BLACK);
    } else {
      playerSprite.pushImage(0, 0, 48, 48, GetMirrorPet(playerFrames), TFT_BLACK);
      playerSprite.pushSprite(0, 60, TFT_BLACK);
    }
  }  //END WAKE
  else if (ACTION == EATING) {
    if (animFrames > 8) {
      animFrames = 0;
      if (VENTURESTATE == INACTIVE)
        ACTION = IDLE;
      else
        ACTION = WALKING;
    }
    lastTime = millis();
    if (animFrames < 6) {
      if (playerFrames == 3) {
        playerFrames = 2;
        //itemFrames set in UseItem();
        animFrames++;
      } else {
        playerFrames = 3;
        animFrames++;
      }
      if (animFrames == 6)
        itemFrames = 30;

      pushScaled(display, xPos + 48, yPos - 3, 16, 16, (uint16_t*)items[itemFrames], TFT_BLACK);
      display.pushImage(xPos, yPos, 48, 48, GetMirrorPet(playerFrames), TFT_BLACK);
      display.pushSprite(0, 51, TFT_BLACK);
      if (animFrames % 2 == 1)
        itemFrames++;
    } else {
      if (playerFrames == 3) {
        playerFrames = 7;
        emoteFrames = 2;
        animFrames++;
      } else {
        playerFrames = 3;
        emoteFrames = 14;
        animFrames++;
      }
      display.pushImage(xPos, yPos, 48, 48, GetMirrorPet(playerFrames), TFT_BLACK);
      pushScaled(display, xPos + 48, yPos, 8, 8, (uint16_t*)emotes[emoteFrames], TFT_BLACK);
      display.pushSprite(0, 51, TFT_BLACK);
    }
  }  //END EATING
  else if (ACTION == ATTACKING) {
    if (animFrames > 4) {
      animFrames = 0;
      ACTION = IDLE;
    }
    lastTime = millis();
    if (playerFrames == 0) {
      playerFrames = 6;
      animFrames++;
    } else {
      playerFrames = 11;
      animFrames++;
    }
    if (randomEventPending == false){
      playerSprite.pushImage(0, 0, 48, 48, GetMirrorPet(playerFrames), TFT_BLACK);
      playerSprite.pushSprite(0, 60, TFT_BLACK);
    } else {
      playerSprite.pushImage(0, 0, 48, 48, GetMirrorPet(playerFrames), TFT_BLACK);
      playerSprite.pushSprite(xPosition, 60, TFT_BLACK);
    }
  }  //END ATTACKING
  else if (ACTION == REJECT) {
    if (animFrames > 2) {
      animFrames = 0;
      if (VENTURESTATE == INACTIVE)
        if (HP == 0) {
          ACTION = INJURED;
          SavePetStats();
          DrawMainMenu();
        } else
          ACTION = IDLE;
      else
        ACTION = WALKING;
    }
    lastTime = millis();
    if (animFrames % 2 == 0) {
      playerFrames = 6;
      animFrames++;
      display.pushImage(xPos, yPos, 48, 48, (uint16_t*)pet[playerFrames], TFT_BLACK);
      display.pushSprite(0, 51, TFT_BLACK);
    } else {
      playerFrames = 6;
      animFrames++;
      display.pushImage(xPos, yPos, 48, 48, GetMirrorPet(playerFrames), TFT_BLACK);
      display.pushSprite(0, 51, TFT_BLACK);
    }
  }  //END REJECT
  else if (ACTION == INJURED) {
    lastTime = millis();
    if (animFrames < 1) {
      BATTLESTATE = END;
      beginTurn = "defeat";
      Mood = Mood - 50;
      if (Mood < 0)
        Mood = 0;
      actionTime = 0;
      playerFrames = 9;
      animFrames++;
    } else {
      if (animFrames == 1) {
        animFrames = 2;
        if (HP == 0 && countdownSeconds != 0)
          emoteFrames = 5;
        else
          emoteFrames = 4;
      } else if (animFrames == 2) {
        animFrames = 1;
        emoteFrames = 14;
      }
      playerFrames = 10;
    }
    if (MENUSTATE != BATTLE) {
      display.pushImage(xPos, yPos, 48, 48, GetMirrorPet(playerFrames), TFT_BLACK);
      pushScaled(display, xPos + 48, yPos, 8, 8, (uint16_t*)emotes[emoteFrames], TFT_BLACK);
      display.pushSprite(0, 51, TFT_BLACK);
    } else {
      playerSprite.pushImage(0, 0, 48, 48, GetMirrorPet(playerFrames), TFT_BLACK);
      playerSprite.pushSprite(0, 60, TFT_BLACK);
    }
  }  //END INJURED
  //Hurt anim moves at its own speed
  if (ACTION == HURT && millis() - lastTime > 150) {
    lastTime = millis();
    if (animFrames > 4) {
      animFrames = 0;
      if (HP == 0){
        ACTION = INJURED;
        SavePetStats();
      }
      else
        ACTION = IDLE;
      return;
    }
    if (playerFrames == 9) {
      playerFrames = 0;
      animFrames++;
      if (randomEventPending == false){
        tft.fillRect(0, 60, 48, 45, TFT_COLOR4);
        tft.fillRect(0, 105, 48, 3, TFT_COLOR1);
      } else {
        tft.fillRect(xPosition, 60, 48, 45, TFT_COLOR4);
        tft.fillRect(xPosition, 105, 48, 3, TFT_COLOR1);
      }
    } else {
      playerFrames = 9;
      animFrames++;
      if (randomEventPending == false){
        playerSprite.pushImage(0, 0, 48, 48, GetMirrorPet(playerFrames), TFT_BLACK);
        playerSprite.pushSprite(0, 60, TFT_BLACK);
      } else{
        playerSprite.pushImage(0, 0, 48, 48, GetMirrorPet(playerFrames), TFT_BLACK);
        playerSprite.pushSprite(xPosition, 60, TFT_BLACK);
      }
    }
  }  //END HURT
}

void DrawEnemy(int xPos) {
  static unsigned long lastTime = 0;
  if (millis() - lastTime < 750 && enemyAction != "hurt" && enemyAction != "injured") {
    return;
  }
  if (enemyAction == "idle") {
    lastTime = millis();
    if (enemyFrames == 3)
      enemyFrames = 0;
    else
      enemyFrames = 3;
    enemySprite.pushImage(0, 0, 48, 48, (uint16_t*)enemyPet[enemyFrames], TFT_BLACK);
    enemySprite.pushSprite(xPos, 60, TFT_BLACK);
  } else if (enemyAction == "walking") {
    lastTime = millis();
    if (enemyFrames == 1)
      enemyFrames = 0;
    else
      enemyFrames = 1;
    enemySprite.pushImage(0, 0, 48, 48, (uint16_t*)enemyPet[enemyFrames], TFT_BLACK);
    enemySprite.pushSprite(xPos, 60, TFT_BLACK);
  } else if (enemyAction == "attacking") {
    if (enemyAnimFrames > 4) {
      enemyAnimFrames = 0;
      enemyAction = "idle";
    }
    lastTime = millis();
    if (enemyFrames == 0) {
      enemyFrames = 6;
      enemyAnimFrames++;
    } else {
      enemyFrames = 11;
      enemyAnimFrames++;
    }
    enemySprite.pushImage(0, 0, 48, 48, (uint16_t*)enemyPet[enemyFrames], TFT_BLACK);
    enemySprite.pushSprite(xPos, 60, TFT_BLACK);
  }
  //Hurt and injured anim moves at its own speed
  if (enemyAction == "hurt" && millis() - lastTime > 150) {
    lastTime = millis();
    if (enemyAnimFrames > 4) {
      enemyAnimFrames = 0;
      enemyAction = "idle";
    }
    if (enemyFrames == 9) {
      enemyFrames = 0;
      enemyAnimFrames++;
      tft.fillRect(192, 60, 48, 45, TFT_COLOR4);
      tft.fillRect(192, 105, 48, 3, TFT_COLOR1);
      //enemySprite.pushSprite(xPos, 60, TFT_BLACK);
    } else {
      enemyFrames = 9;
      enemyAnimFrames++;
      enemySprite.pushImage(0, 0, 48, 48, (uint16_t*)enemyPet[enemyFrames], TFT_BLACK);
      enemySprite.pushSprite(xPos, 60, TFT_BLACK);
    }
  }  //END HURT
  else if (enemyAction == "injured" && millis() - lastTime > 150) {
    lastTime = millis();
    if (enemyAnimFrames > 8) {
      enemyAnimFrames = 0;
      ACTION = WAKE;
      animFrames = 0;
      tft.fillRect(0, 24, 240, 36, TFT_COLOR4);
      tft.fillRect(48, 24, 192, 51, TFT_COLOR4);
      tft.fillRect(112, 0, 138, 24, TFT_COLOR4);
      tft.fillRect(48, 105, 192, 3, TFT_COLOR1);
      tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
      tft.fillRect(96, 48, 45, 45, TFT_COLOR4);
      BATTLESTATE = END;
      beginTurn = "victory";
      actionTime = 0;
      tft.fillRect(66, 48, 48, 48, TFT_COLOR4);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    }
    if (enemyAnimFrames < 1) {
      enemyFrames = 9;
      enemyAnimFrames++;
    } else {
      enemyFrames = 10;
      enemyAnimFrames++;
    }
    if (enemyAnimFrames % 2 == 1 && enemyAnimFrames != 1) {
      enemySprite.pushImage(0, 0, 48, 48, (uint16_t*)enemyPet[enemyFrames], TFT_BLACK);
      enemySprite.pushSprite(xPos, 60, TFT_BLACK);
    } else {
      tft.fillRect(192, 60, 48, 45, TFT_COLOR4);
      tft.fillRect(192, 105, 48, 3, TFT_COLOR1);
    }
  }
}

void ShowTitleScreen() {
  if (MENUSTATE != TITLE)
    return;
  static unsigned long lastTime = 0;
  PushImageRemap(0, 0, 240, 135, titleScreen[0]);
  testActive = true;
  if (millis() - lastTime > 3000) {
    if (EEPROM.read(0) != 1) {
      MENUSTATE = TUTORIAL;
      lastTime = millis();
      testActive = false;
      tft.fillScreen(TFT_COLOR4);
      if(debug)
        Debug();
    } else {
      lastTime = millis();
      testActive = false;
      tft.fillScreen(TFT_COLOR4);
      DrawBackground();
      DrawMainMenu();
      MENUSTATE = MAINMENU;
    }
  }
}

void drawingBatteryIcon(int imgNum) {
  if (TOOLTIP == ACTIVE)
    return;
  pushScaled(195, 0, 15, 9, Battery[imgNum], TFT_BLACK);
}

void DrawStatusMeters() {
        tft.fillScreen(TFT_COLOR4);
        drawingBatteryIcon(batteryAnim);
        String displayName = "";
        for (int i = 0; i < name.length(); i++) {
          if (name[i] >= 'A' && name[i] <= 'Z') displayName += name[i];
          else break;  // Stop at first non-letter
        }
        int nameBoxWidth = tft.textWidth(displayName) + 3;
        tft.fillRect(3, 3, nameBoxWidth, 27, TFT_COLOR1);
        tft.fillRect(0, 0, nameBoxWidth, 27, TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString(displayName, 3, 3);

        //MOOD (ATK)
        tft.fillRect((1 * 3), (12 * 3), (25 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, (11 * 3), (25 * 3), (9 * 3), TFT_COLOR3);
        tft.fillRect((28 * 3), (15 * 3), (50 * 3), (4 * 3), TFT_COLOR3);
        if (Mood > 30) {
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          tft.drawString(String("MOOD"), (1 * 3), (12 * 3));
          int result = floor(Mood * 50.0 / 100.0);
          tft.fillRect(81, 42, result * 3, 12, TFT_COLOR1);
        }
        if (Mood <= 30) {
          tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
          tft.drawString(String("MOOD"), (1 * 3), (12 * 3));
          int result = floor(Mood * 50.0 / 100.0);
          tft.fillRect(81, 42, result * 3, 12, TFT_COLOR2);
        }
        tft.fillRect(123, 60, 3, 3, TFT_COLOR3);
        tft.fillRect(153, 60, 3, 3, TFT_COLOR3);
        tft.fillRect(183, 60, 3, 3, TFT_COLOR1);
        tft.fillRect(213, 60, 3, 3, TFT_COLOR1);

        //FOOD (DEF)
        tft.fillRect((1 * 3), (23 * 3), (25 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, (22 * 3), (25 * 3), (9 * 3), TFT_COLOR3);
        tft.fillRect((28 * 3), (26 * 3), (50 * 3), (4 * 3), TFT_COLOR3);
        if (Hunger > 30) {
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          tft.drawString(String("FOOD"), (1 * 3), (23 * 3));
          int result = floor(Hunger * 50.0 / 100.0);
          tft.fillRect((27 * 3), (25 * 3), result * 3, (4 * 3), TFT_COLOR1);
        }
        if (Hunger <= 30) {
          tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
          tft.drawString(String("FOOD"), (1 * 3), (23 * 3));
          int result = floor(Hunger * 50.0 / 100.0);
          tft.fillRect((27 * 3), (25 * 3), result * 3, (4 * 3), TFT_COLOR2);
        }
        tft.fillRect(123, 93, 3, 3, TFT_COLOR3);
        tft.fillRect(153, 93, 3, 3, TFT_COLOR3);
        tft.fillRect(183, 93, 3, 3, TFT_COLOR1);
        tft.fillRect(213, 93, 3, 3, TFT_COLOR1);

        //STAM (SPD)
        tft.fillRect((1 * 3), (34 * 3), (25 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, (33 * 3), (25 * 3), (9 * 3), TFT_COLOR3);
        tft.fillRect((28 * 3), (37 * 3), (50 * 3), (4 * 3), TFT_COLOR3);

        tft.fillRect(123, 126, 3, 3, TFT_COLOR3);
        tft.fillRect(153, 126, 3, 3, TFT_COLOR3);
        tft.fillRect(183, 126, 3, 3, TFT_COLOR1);
        tft.fillRect(213, 126, 3, 3, TFT_COLOR1);
        if (Stamina >= 30) {
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          tft.drawString(String("STAM"), (1 * 3), (34 * 3));
          int result = floor(Stamina * 50.0 / 100.0);
          tft.fillRect((27 * 3), (36 * 3), result * 3, (4 * 3), TFT_COLOR1);
        } else if (Stamina < 30) {
          tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
          tft.drawString(String("STAM"), (1 * 3), (34 * 3));
          int result = floor(Stamina * 50.0 / 100.0);
          tft.fillRect((27 * 3), (36 * 3), result * 3, (4 * 3), TFT_COLOR2);
        }
        mainMenuReturn = 0;
}

void ToggleMenus() {
  if (MENUSTATE == TITLE || ACTION == WAKE || VENTURESTATE == ENCOUNTER || OPTIONSSETTINGS == BACKUPWAIT || OPTIONSSETTINGS == RESTOREWAIT)
    return;
  static unsigned long lastTime = 0;
  static bool holdConsumed = false;
  if (AButton != "held") holdConsumed = false;
  //RETURNS TO HOMESCREEN IF BUTTON IS NOT PRESSED IN 15s
  if (millis() - lastTime >= 1000) {  //1 second
    lastTime = millis();
    if ((MENUSTATE != MAINMENU || AMENUS != NA) && !ddGemSelect && !ddGemConfirm && !ddMeritSelect && !ddCardDraft && !ddMeritDiscard && !ddMeritConfirm && !bossAfflictionPopup) {
      if (MENUSTATE == TITLE)
        return;
      mainMenuReturn++;
      if (mainMenuReturn >= 15) {
        AMENUS = NA;
        tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        if (randomEventPending) {
          CenterText("???", 37 * 3);
        } else if (VENTURESTATE == VENTURING && countdownSeconds == 0) {
          tft.drawString(String("BATTLE"), 24 * 3, 37 * 3);
        } else if (HP == 0 && countdownSeconds == 0) {
          tft.drawString(String("REVIVE"), 21 * 3, 37 * 3);
        }
        if (MENUSTATE != MAINMENU && MENUSTATE != BATTLE) {
          // Free enemy sprites if leaving venture menu
          for (int i = 0; i < 12; i++) {
            if (enemyPet[i] != nullptr) {
              delete[] enemyPet[i];
              enemyPet[i] = nullptr;
            }
          }
          enemySpriteCreated = false;  // Reset flag so sprite gets recreated
          MENUSTATE = MAINMENU;
          screenOffTime = 0;  // start the screen-off idle timer fresh at the main menu
          ddGemSelect = false;
          ddGemConfirm = false;
          ddMeritSelect = false;
          ddCardDraft = false;
          statDrill = false;
          ddExitConfirm = false;
          TOOLTIP = RELEASED;
          testActive = false;
          tft.fillScreen(TFT_COLOR4);
          DrawBackground();
          DrawMainMenu();
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          display.fillRect(0, 0, 240, 54, TFT_COLOR4);
          display.fillRect(0, 54, 240, 3, TFT_COLOR1);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          if (randomEventPending) {
            CenterText("???", 37 * 3);
          } else if (VENTURESTATE == VENTURING && countdownSeconds == 0) {
            tft.drawString(String("BATTLE"), 24 * 3, 37 * 3);
          } else if (HP == 0 && countdownSeconds == 0) {
            tft.drawString(String("REVIVE"), 21 * 3, 37 * 3);
          }
          selectedWardrobe = 0;
        }
      }
    }
  }

  //Hold A to return to Main Menu
  if (MENUSTATE != TITLE && MENUSTATE != MAINMENU && MENUSTATE != BATTLE && MENUSTATE != FARM && MENUSTATE != WRDLIST && MENUSTATE != WRDSELECT && MENUSTATE != DECKCARDSELECT && MENUSTATE != DECKLIST && OPTIONSSETTINGS != BACKUPWAIT && OPTIONSSETTINGS != RESTOREWAIT && !ddMeritSelect && !ddCardDraft && !bossAfflictionPopup && !statDrill && !holdConsumed) {
    if (AButton == "held" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (setDck && MENUSTATE == DECKMENU)
        return;
      if (setWrd && MENUSTATE == WARDROBEMENU)
        return;
      if (setVnt && MENUSTATE == VENTUREMENU)
        return;
      if (screenOff == false) {
        // Free enemy sprites if leaving venture menu
        for (int i = 0; i < 12; i++) {
          if (enemyPet[i] != nullptr) {
            delete[] enemyPet[i];
            enemyPet[i] = nullptr;
          }
        }
        enemySpriteCreated = false;  // Reset flag so sprite gets recreated
        AMENUS = NA;
        MENUSTATE = MAINMENU;
        ddGemSelect = false;
        ddGemConfirm = false;
        ddMeritSelect = false;
        ddCardDraft = false;
        statDrill = false;
        ddExitConfirm = false;
        TOOLTIP = RELEASED;
        testActive = false;
        tft.fillScreen(TFT_COLOR4);
        DrawBackground();
        DrawMainMenu();
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        display.fillRect(0, 0, 240, 54, TFT_COLOR4);
        display.fillRect(0, 54, 240, 3, TFT_COLOR1);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        if (randomEventPending) {
          CenterText("???", 37 * 3);
        } else if (VENTURESTATE == VENTURING && countdownSeconds == 0) {
          tft.drawString(String("BATTLE"), 24 * 3, 37 * 3);
        } else if (HP == 0 && countdownSeconds == 0) {
          tft.drawString(String("REVIVE"), 21 * 3, 37 * 3);
        }
        selectedWardrobe = 0;
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
  }
  //RESETS THE AMENU OPTIONS IF THE A BUTTON IS HELD
  if (MENUSTATE == MAINMENU && AMENUS != NA) {
    if (AButton == "held" && BButton != "held" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        AMENUS = NA;
        tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
        if(debug){
          tft.setTextColor(TFT_GREEN, TFT_COLOR3);
          CenterText("DEBUG MODE", 111);
        }
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        if (randomEventPending) {
          CenterText("???", 37 * 3);
        } else if (VENTURESTATE == VENTURING && countdownSeconds == 0) {
          tft.drawString(String("BATTLE"), 24 * 3, 37 * 3);
        } else if (HP == 0 && countdownSeconds == 0) {
          tft.drawString(String("REVIVE"), 21 * 3, 37 * 3);
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
  }
  //A Button: Main Menu choices
  if (MENUSTATE == MAINMENU && ACTION != EATING && ACTION != WAKE && ACTION != REJECT) {
    if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
      if (eventState != EVENT_PROMPT && eventState != EVENT_RESULT) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          switch (AMENUS) {
            case NA:
              AMENUS = INVENTORY;
              if (gamePause || ACTION == SLEEPING)
                tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
              tft.drawString(String("INVENTORY"), 13 * 3, 37 * 3);
              break;
            case INVENTORY:
              AMENUS = DECK;
              if (gamePause)
                tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
              tft.drawString(String("DECK"), 30 * 3, 37 * 3);
              break;
            case DECK:
              AMENUS = VENTURE;
              if (gamePause)
                tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
              tft.drawString(String("VENTURE"), 20 * 3, 37 * 3);
              break;
            case VENTURE:
              AMENUS = SHOP;
              if (gamePause)
                tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
              tft.drawString(String("SHOP"), 30 * 3, 37 * 3);
              break;
            case SHOP:
              AMENUS = WARDROBE;
              if (gamePause)
                tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
              tft.drawString(String("WARDROBE"), 17 * 3, 37 * 3);
              break;
            case WARDROBE:
              AMENUS = SLEEP;
              if (gamePause || VENTURESTATE == VENTURING || ACTION == INJURED || randomEventPending)
                tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
              if (ACTION != SLEEPING)
                tft.drawString(String("SLEEP"), 27 * 3, 37 * 3);
              else
                tft.drawString(String("WAKE"), 30 * 3, 37 * 3);
              break;
            case SLEEP:
              AMENUS = OPTIONS;
              tft.drawString(String("OPTIONS"), 20 * 3, 37 * 3);
              break;
            case OPTIONS:
              AMENUS = NA;
              if (randomEventPending) {
                CenterText("???", 37 * 3);
              } else if (VENTURESTATE == VENTURING && countdownSeconds == 0) {
                tft.drawString(String("BATTLE"), 24 * 3, 37 * 3);
              } else if (HP == 0 && countdownSeconds == 0) {
                tft.drawString(String("REVIVE"), 21 * 3, 37 * 3);
              } else {
                tft.drawString(String(" "), 20 * 3, 37 * 3);
              }
              break;
          }
          mainMenuReturn = 0;
        }
        if (screenOff)
          screenOff = false;
      }
    }
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
      if (!gamePause && randomEventPending && AMENUS == NA && screenOff == false) {
        lastButtonPressTime = millis();
        HandleEventButton();
      }
      //START A BATTLE
      else if (!gamePause && countdownSeconds == 0 && VENTURESTATE == VENTURING && AMENUS == NA && screenOff == false && eventState == EVENT_IDLE) {
        lastButtonPressTime = millis();
        DrawCharacter(xPosition, 60, true);
        ventureTime = 0;
        battleReady = false;
        AMENUS = NA;
        VENTURESTATE = ENCOUNTER;
        tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
        randomIdle = 0;
        mainMenuReturn = 0;
      }
      //REVIVE AFTER INJURY
      else if (!gamePause && countdownSeconds == 0 && HP == 0 && AMENUS == NA && screenOff == false && eventState == EVENT_IDLE) {
        lastButtonPressTime = millis();
        injuryTime = 0;
        AMENUS = NA;
        HP = 1;
        ACTION = WAKE;
        randomIdle = 0;
        DrawMainMenu();
        DrawCharacter(xPosition, 60, true);
        tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
        mainMenuReturn = 0;
      }
    }
    //Do not disturb mode
    else if (AButton == "held" && BButton == "held" && millis() - lastButtonPressTime > 500 && eventState == EVENT_IDLE) {
      lastButtonPressTime = millis();
      if (!setDND) {
        if (!doNotDisturb) {
          doNotDisturb = true;
          tft.fillRect(177, 6, 15, 15, TFT_COLOR3);
          tft.fillRect(180, 12, 9, 3, TFT_COLOR1);
          tft.fillRect(177, 6, 3, 3, TFT_COLOR4);
          tft.fillRect(189, 6, 3, 3, TFT_COLOR4);
          tft.fillRect(177, 18, 3, 3, TFT_COLOR4);
          tft.fillRect(189, 18, 3, 3, TFT_COLOR4);

          strip.setPixelColor(0, 0, 0, 0);  // Turn off the NeoPixel
          strip.show();
          ledActive = false;
          setDND = true;
        } else if (doNotDisturb) {
          doNotDisturb = false;
          tft.fillRect(177, 6, 15, 15, TFT_COLOR4);
          setDND = true;
        }
        // DEBUG: Trigger random event with A+B held
        if (debug && !randomEventPending && ACTION != INJURED) {
          TriggerRandomEvent();
        }
      }
    }
    //Prevents infinite toggles of DND without releasing the buttons
    if (AButton == "released" && BButton == "released" && millis() - lastButtonPressTime > 500 && setDND == true) {
      lastButtonPressTime = millis();
      setDND = false;
    }
  }
#pragma region Inventory
  //(A MENU): Inventory
  if (MENUSTATE == MAINMENU && AMENUS == INVENTORY) {
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500 && !gamePause && ACTION != SLEEPING) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        //INVENTORY SETUP
        MENUSTATE = INVENTORYMENU;
        ManageInventory();
        //invSelect = -1;
        tft.fillScreen(TFT_COLOR4);
        drawingBatteryIcon(batteryAnim);
        tft.fillRect(3, 3, (55 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 0, (55 * 3), (9 * 3), TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString(String("INVENTORY"), 3, 3);
        menuItem = 0;
        menuIndex = 0;
        pageNum = 0;
        CompileListMenu("items");
        pushScaled(18, 33, 5, 5, smallIcons[13], TFT_BLACK);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        tft.fillRect(204, 30, 33, 21, TFT_COLOR4);
        if (inventoryAdr[0] > 0)
          CenterText("NEXT PAGE", 111);
        else
          CenterText("NO ITEMS", 111);
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
  }
  //Inventory Menu (Control)
  if (MENUSTATE == INVENTORYMENU) {
    //A BUTTON
    if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        menuItem++;
        if (menuItem > maxItems)
          menuItem = 0;
        tft.fillRect(18, 39, 189, 15, TFT_COLOR4);
        pushScaled(18 + (menuItem * 54), 33, 5, 5, smallIcons[13], TFT_BLACK);
        tft.fillRect(0, 111, 240, 24, TFT_COLOR4);
        if (menuItem == 0) {
          CenterText("NEXT PAGE", 111);
          tft.fillRect(204, 30, 33, 21, TFT_COLOR4);
        } else {
          CenterText(itemList[menuValues[menuItem - 1]].name1 + " " + itemList[menuValues[menuItem - 1]].name2, 111);
          tft.drawString("x" + String(inventoryQty[(menuItem - 1) + ((pageNum - 1) * 3)]), 68 * 3, 10 * 3);
        }
        //tft.drawString(String(menuItem), 0, 0);
        ManageInventory();
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        display.fillRect(0, 0, 240, 54, TFT_COLOR4);
        display.fillRect(0, 54, 240, 3, TFT_COLOR1);
        if (menuItem != 0) {
          invSelect = (menuItem - 1) + ((pageNum - 1) * 3);
          UseItem();
        }
        else
          CompileListMenu("items");
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON HOLD (Item Tooltip)
    if (BButton == "held" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        if (TOOLTIP == RELEASED && menuItem != 0) {
          TOOLTIP = ACTIVE;
          tft.fillScreen(TFT_COLOR4);
          drawingBatteryIcon(batteryAnim);
          tft.fillRect(63, 6, 129, 51, TFT_COLOR1);
          tft.fillRect(60, 3, 129, 51, TFT_COLOR3);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          pushScaled(6, 3, 16, 16, items[menuValues[menuItem - 1] * 3], TFT_BLACK);
          tft.drawString(itemList[menuValues[menuItem - 1]].name1, 63, 6);
          tft.drawString(itemList[menuValues[menuItem - 1]].name2, 63, 30);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          tft.drawString("x" + String(inventoryQty[(menuItem - 1) + ((pageNum - 1) * 3)]), 204, 30);
          tft.drawString(itemList[menuValues[menuItem - 1]].desc1, 3, 63);
          tft.drawString(itemList[menuValues[menuItem - 1]].desc2, 3, 87);
          tft.drawString(itemList[menuValues[menuItem - 1]].desc3, 3, 111);
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON RELEASE (Item Tooltip)
    if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        TOOLTIP = RELEASED;
        tft.fillScreen(TFT_COLOR4);
        drawingBatteryIcon(batteryAnim);
        tft.fillRect(3, 3, (55 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 0, (55 * 3), (9 * 3), TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString(String("INVENTORY"), 3, 3);
        tft.fillRect(15, 84, 21, 3, TFT_COLOR1);
        pushScaled(18, 81, 5, 5, smallIcons[13], TFT_BLACK);
        pushScaled(18 + (menuItem * 54), 33, 5, 5, smallIcons[13], TFT_BLACK);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        if (pageNum < 10)
          tft.drawString("0" + String(pageNum), 9, 60);
        else
          tft.drawString(String(pageNum), 9, 60);
        if (menuItem != 0) {
          CenterText(itemList[menuValues[menuItem - 1]].name1 + " " + itemList[menuValues[menuItem - 1]].name2, 111);
          tft.drawString("x" + String(inventoryQty[(menuItem - 1) + ((pageNum - 1) * 3)]), 68 * 3, 10 * 3);
        } else if (menuItem == 0 && inventoryAdr[0] > 0)
          CenterText("NEXT PAGE", 111);
        else
          CenterText("NO ITEMS", 111);
        if (EEPROM.read(143 + menuValues[0]) != 0 && EEPROM.read(143 + menuValues[0]) != 255 && menuValues[0] != -1) {
          pushScaled(51, 54, 16, 16, items[(menuValues[0]) * 3], TFT_BLACK);
        }
        if (EEPROM.read(143 + menuValues[1]) != 0 && EEPROM.read(143 + menuValues[1]) != 255 && menuValues[1] != -1) {
          pushScaled(105, 54, 16, 16, items[(menuValues[1]) * 3], TFT_BLACK);
        }
        if (EEPROM.read(143 + menuValues[2]) != 0 && EEPROM.read(143 + menuValues[2]) != 255 && menuValues[2] != -1) {
          pushScaled(159, 54, 16, 16, items[(menuValues[2]) * 3], TFT_BLACK);
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
  }
#pragma endregion

#pragma region Deck
  //(A MENU): Deck
  if (MENUSTATE == MAINMENU && AMENUS == DECK) {
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500 && !gamePause) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        //DECK SETUP
        MENUSTATE = DECKMENU;
        tft.fillScreen(TFT_COLOR4);
        drawingBatteryIcon(batteryAnim);
        tft.fillRect(3, 3, (37 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 0, (37 * 3), (9 * 3), TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        
        // Get equipped deck for current mode
        int currentEquipped = getEquippedDeck();
        equippedDeck = currentEquipped;
        
        // Show different title for DD mode
        if (deepDungeonActive) {
          tft.drawString("DD " + String(currentEquipped + 1), 3, 3);
        } else {
          tft.drawString("DECK " + String(currentEquipped + 1), 3, 3);
        }
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);

        tft.fillRect(126, 12, 15, 15, TFT_COLOR1);
        tft.fillRect(147, 12, 15, 15, TFT_COLOR1);
        tft.fillRect(168, 12, 15, 15, TFT_COLOR1);

        pushScaled(3, 51, 15, 15, abilityFrame[0], TFT_BLACK);
        tft.fillRect(54, 57, 183, 27, TFT_COLOR1);
        tft.fillRect(51, 54, 183, 27, TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        selectedDeck = currentEquipped;
        int baseAddr = getDeckBaseAddr();
        if (selectedDeck == 0) {
          for (int i = 0; i < 10; i++)
            deckIndex[i] = EEPROM.read(baseAddr + i);
        } else if (selectedDeck == 1) {
          for (int i = 0; i < 10; i++)
            deckIndex[i] = EEPROM.read(baseAddr + 10 + i);
        } else if (selectedDeck == 2) {
          for (int i = 0; i < 10; i++)
            deckIndex[i] = EEPROM.read(baseAddr + 20 + i);
        }
        if (currentEquipped == 0) {
          pushScaled(126, -6, 5, 5, smallIcons[13], TFT_BLACK);
          tft.fillRect(126, 12, 15, 15, TFT_COLOR3);
        } else if (currentEquipped == 1) {
          pushScaled(147, -6, 5, 5, smallIcons[13], TFT_BLACK);
          tft.fillRect(147, 12, 15, 15, TFT_COLOR3);
        } else if (currentEquipped == 2) {
          pushScaled(168, -6, 5, 5, smallIcons[13], TFT_BLACK);
          tft.fillRect(168, 12, 15, 15, TFT_COLOR3);
        }
        selectedMenuCard = -1;
        //create each card in the deck
        for (int i = 0; i < 11; i++) {
          if (i < 10) {
            if (i == selectedMenuCard) {
              tft.fillRect(6 + (i * 21), 117, (5 * 3), (5 * 3), TFT_COLOR3);
              tft.fillRect(3 + (i * 21), 114, (5 * 3), (5 * 3), TFT_COLOR1);
            } else {
              tft.fillRect(6 + (i * 21), 117, (5 * 3), (5 * 3), TFT_COLOR1);
              tft.fillRect(3 + (i * 21), 114, (5 * 3), (5 * 3), TFT_COLOR3);
            }
          } else {  //i == 11
            if (i == selectedMenuCard) {
              tft.fillRect(222, 120, 15, 9, TFT_COLOR3);
              tft.fillRect(225, 129, 9, 3, TFT_COLOR3);
              tft.fillRect(219, 117, 15, 9, TFT_COLOR1);
              tft.fillRect(222, 114, 9, 15, TFT_COLOR1);
            } else {
              tft.fillRect(222, 120, 15, 9, TFT_COLOR1);
              tft.fillRect(225, 129, 9, 3, TFT_COLOR1);
              tft.fillRect(219, 117, 15, 9, TFT_COLOR3);
              tft.fillRect(222, 114, 9, 15, TFT_COLOR3);
            }
          }
        }
        //Divider
        tft.fillRect(213, 120, 3, 6, TFT_COLOR1);
        //Repeat counter
        if (selectedMenuCard < 10) {
          pushScaled(9, 57, 11, 11, cardSpriteArray[deckIndex[selectedMenuCard + 1]], TFT_BLACK);
          tft.drawString(cardList[deckIndex[selectedMenuCard + 1]].name, 54, 57);
          tft.fillRect(54, 87, 39, 9, TFT_COLOR4);
          tft.fillRect(57, 90, 15, 6, TFT_COLOR1);
          tft.fillRect(78, 90, 15, 6, TFT_COLOR1);
          tft.fillRect(54, 87, 15, 6, TFT_COLOR3);
          bool repeat = false;
          for (int i = 0; i < 10; i++) {
            if (deckIndex[i] == deckIndex[selectedMenuCard + 1] && i != selectedMenuCard + 1)
              repeat = true;
          }
          if (repeat)
            tft.fillRect(75, 87, 15, 6, TFT_COLOR3);
          mainMenuReturn = 0;
        } else {
          //Hide repeats for gems
          tft.fillRect(54, 87, 39, 9, TFT_COLOR4);
        }
      }
      if (screenOff)
        screenOff = false;
    }
  }
  //Deck Menu (Control)
  if (MENUSTATE == DECKMENU) {
    //A BUTTON
    if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        selectedDeck++;
        if (selectedDeck > 2)
          selectedDeck = 0;
        tft.fillRect(9, 57, 33, 33, TFT_COLOR4);
        tft.fillRect(126, 0, 57, 12, TFT_COLOR4);
        int baseAddr = getDeckBaseAddr();
        if (selectedDeck == 0) {
          pushScaled(126, -6, 5, 5, smallIcons[13], TFT_BLACK);
          for (int i = 0; i < 10; i++)
            deckIndex[i] = EEPROM.read(baseAddr + i);
        } else if (selectedDeck == 1) {
          pushScaled(147, -6, 5, 5, smallIcons[13], TFT_BLACK);
          for (int i = 0; i < 10; i++)
            deckIndex[i] = EEPROM.read(baseAddr + 10 + i);
        } else if (selectedDeck == 2) {
          pushScaled(168, -6, 5, 5, smallIcons[13], TFT_BLACK);
          for (int i = 0; i < 10; i++)
            deckIndex[i] = EEPROM.read(baseAddr + 20 + i);
        }
        if (deepDungeonActive) {
          tft.drawString("DD " + String(selectedDeck + 1), 3, 3);
        } else {
          tft.drawString("DECK " + String(selectedDeck + 1), 3, 3);
        }
        pushScaled(9, 57, 11, 11, cardSpriteArray[deckIndex[selectedMenuCard + 1]], TFT_BLACK);
        tft.fillRect(51, 54, 183, 27, TFT_COLOR3);
        tft.drawString(cardList[deckIndex[selectedMenuCard + 1]].name, 54, 57);
        tft.fillRect(54, 87, 39, 9, TFT_COLOR4);
        if (selectedMenuCard < 10) {
          tft.fillRect(57, 90, 15, 6, TFT_COLOR1);
          tft.fillRect(78, 90, 15, 6, TFT_COLOR1);
          tft.fillRect(54, 87, 15, 6, TFT_COLOR3);
          bool repeat = false;
          for (int i = 0; i < 10; i++) {
            if (deckIndex[i] == deckIndex[selectedMenuCard + 1] && i != selectedMenuCard + 1)
              repeat = true;
          }
          if (repeat)
            tft.fillRect(75, 87, 15, 6, TFT_COLOR3);
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        MENUSTATE = DECKCARDSELECT;
        tft.fillRect(126, 0, 57, 12, TFT_COLOR4);
        int baseAddr = getDeckBaseAddr();
        int currentEquipped = getEquippedDeck();
        if (selectedDeck == 0) {
          tft.fillRect(123, 9, 21, 21, TFT_COLOR2);
          for (int i = 0; i < 10; i++)
            deckIndex[i] = EEPROM.read(baseAddr + i);
        } else if (selectedDeck == 1) {
          tft.fillRect(144, 9, 21, 21, TFT_COLOR2);
          for (int i = 0; i < 10; i++)
            deckIndex[i] = EEPROM.read(baseAddr + 10 + i);
        } else if (selectedDeck == 2) {
          tft.fillRect(165, 9, 21, 21, TFT_COLOR2);
          for (int i = 0; i < 10; i++)
            deckIndex[i] = EEPROM.read(baseAddr + 20 + i);
        }
        tft.fillRect(126, 12, 15, 15, TFT_COLOR1);
        tft.fillRect(147, 12, 15, 15, TFT_COLOR1);
        tft.fillRect(168, 12, 15, 15, TFT_COLOR1);
        //tft.fillRect(0, 111, 240, 24, TFT_COLOR2);

        if (currentEquipped == 0)
          tft.fillRect(126, 12, 15, 15, TFT_COLOR3);
        else if (currentEquipped == 1)
          tft.fillRect(147, 12, 15, 15, TFT_COLOR3);
        else if (currentEquipped == 2)
          tft.fillRect(168, 12, 15, 15, TFT_COLOR3);
        selectedMenuCard = 0;
        for (int i = 0; i < 11; i++) {
          if (i < 10) {
            if (i == selectedMenuCard) {
              tft.fillRect(6 + (i * 21), 117, (5 * 3), (5 * 3), TFT_COLOR3);
              tft.fillRect(3 + (i * 21), 114, (5 * 3), (5 * 3), TFT_COLOR1);
            } else {
              tft.fillRect(6 + (i * 21), 117, (5 * 3), (5 * 3), TFT_COLOR1);
              tft.fillRect(3 + (i * 21), 114, (5 * 3), (5 * 3), TFT_COLOR3);
            }
          } else {  //i == 11
            if (i == selectedMenuCard) {
              tft.fillRect(222, 120, 15, 9, TFT_COLOR3);
              tft.fillRect(225, 129, 9, 3, TFT_COLOR3);
              tft.fillRect(219, 117, 15, 9, TFT_COLOR1);
              tft.fillRect(222, 114, 9, 15, TFT_COLOR1);
            } else {
              tft.fillRect(222, 120, 15, 9, TFT_COLOR1);
              tft.fillRect(225, 129, 9, 3, TFT_COLOR1);
              tft.fillRect(219, 117, 15, 9, TFT_COLOR3);
              tft.fillRect(222, 114, 9, 15, TFT_COLOR3);
            }
          }
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON HOLD
    if (BButton == "held" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        equippedDeck = selectedDeck;
        tft.fillRect(126, 12, 15, 15, TFT_COLOR1);
        tft.fillRect(147, 12, 15, 15, TFT_COLOR1);
        tft.fillRect(168, 12, 15, 15, TFT_COLOR1);

        if (equippedDeck == 0)
          tft.fillRect(126, 12, 15, 15, TFT_COLOR3);
        else if (equippedDeck == 1)
          tft.fillRect(147, 12, 15, 15, TFT_COLOR3);
        else if (equippedDeck == 2)
          tft.fillRect(168, 12, 15, 15, TFT_COLOR3);
        saveEquippedDeck(equippedDeck);
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //A BUTTON RELEASED
    if (AButton == "released" && millis() - lastButtonPressTime > 500 && setDck)
      setDck = false;
  }
  //Deck Card Select
  if (MENUSTATE == DECKCARDSELECT) {
    //A BUTTON
    if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        selectedMenuCard++;
        if (selectedMenuCard > 10)
          selectedMenuCard = 0;
        tft.fillRect(51, 54, 183, 27, TFT_COLOR3);
        if (selectedMenuCard > 9) {
          if(selectedDeck == 0) {
            tft.drawString(gemList[EEPROM.read(128)].name1 + " " + gemList[EEPROM.read(128)].name2, 54, 57);
            pushScaled(0, 48, 16, 16, gems[EEPROM.read(128)], TFT_BLACK);
          } else if(selectedDeck == 1) {
            tft.drawString(gemList[EEPROM.read(129)].name1 + " " + gemList[EEPROM.read(128)].name2, 54, 57);
            pushScaled(0, 48, 16, 16, gems[EEPROM.read(129)], TFT_BLACK);
          } if(selectedDeck == 2) {
            tft.drawString(gemList[EEPROM.read(130)].name1 + " " + gemList[EEPROM.read(128)].name2, 54, 57);
            pushScaled(0, 48, 16, 16, gems[EEPROM.read(130)], TFT_BLACK);
          }
        } else {
          pushScaled(3, 51, 15, 15, abilityFrame[0], TFT_BLACK);
          pushScaled(9, 57, 11, 11, cardSpriteArray[deckIndex[selectedMenuCard]], TFT_BLACK);
          tft.drawString(cardList[deckIndex[selectedMenuCard]].name, 54, 57);
        }
        for (int i = 0; i < 11; i++) {
          if (i < 10) {
            if (i == selectedMenuCard) {
              tft.fillRect(6 + (i * 21), 117, (5 * 3), (5 * 3), TFT_COLOR3);
              tft.fillRect(3 + (i * 21), 114, (5 * 3), (5 * 3), TFT_COLOR1);
            } else {
              tft.fillRect(6 + (i * 21), 117, (5 * 3), (5 * 3), TFT_COLOR1);
              tft.fillRect(3 + (i * 21), 114, (5 * 3), (5 * 3), TFT_COLOR3);
            }
          } else {  //i == 11
            if (i == selectedMenuCard) {
              tft.fillRect(222, 120, 15, 9, TFT_COLOR3);
              tft.fillRect(225, 129, 9, 3, TFT_COLOR3);
              tft.fillRect(219, 117, 15, 9, TFT_COLOR1);
              tft.fillRect(222, 114, 9, 15, TFT_COLOR1);
            } else {
              tft.fillRect(222, 120, 15, 9, TFT_COLOR1);
              tft.fillRect(225, 129, 9, 3, TFT_COLOR1);
              tft.fillRect(219, 117, 15, 9, TFT_COLOR3);
              tft.fillRect(222, 114, 9, 15, TFT_COLOR3);
            }
          }
        }
        //Divider
        tft.fillRect(213, 120, 3, 6, TFT_COLOR1);
        //Repeat counter
        if (selectedMenuCard < 10) {
          tft.fillRect(54, 87, 39, 9, TFT_COLOR4);
          tft.fillRect(57, 90, 15, 6, TFT_COLOR1);
          tft.fillRect(78, 90, 15, 6, TFT_COLOR1);
          tft.fillRect(54, 87, 15, 6, TFT_COLOR3);
          bool repeat = false;
          for (int i = 0; i < 10; i++) {
            if (deckIndex[i] == deckIndex[selectedMenuCard] && i != selectedMenuCard)
              repeat = true;
          }
          if (repeat)
            tft.fillRect(75, 87, 15, 6, TFT_COLOR3);
          mainMenuReturn = 0;
        } else {
          //Hide repeats for gems
          tft.fillRect(54, 87, 39, 9, TFT_COLOR4);
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //A BUTTON HOLD (Return to DECKMENU)
    if (AButton == "held" && millis() - lastButtonPressTime > 500 && !setDck) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        MENUSTATE = DECKMENU;
        setDck = true;
        tft.fillRect(123, 0, 63, 33, TFT_COLOR4);
        tft.fillRect(126, 12, 15, 15, TFT_COLOR1);
        tft.fillRect(147, 12, 15, 15, TFT_COLOR1);
        tft.fillRect(168, 12, 15, 15, TFT_COLOR1);
        tft.fillRect(51, 54, 183, 27, TFT_COLOR3);
        if (equippedDeck == 0)
          tft.fillRect(126, 12, 15, 15, TFT_COLOR3);
        else if (equippedDeck == 1)
          tft.fillRect(147, 12, 15, 15, TFT_COLOR3);
        else if (equippedDeck == 2)
          tft.fillRect(168, 12, 15, 15, TFT_COLOR3);
        selectedMenuCard = -1;
        for (int i = 0; i < 11; i++) {
          if (i < 10) {
            if (i == selectedMenuCard) {
              tft.fillRect(6 + (i * 21), 117, (5 * 3), (5 * 3), TFT_COLOR3);
              tft.fillRect(3 + (i * 21), 114, (5 * 3), (5 * 3), TFT_COLOR1);
            } else {
              tft.fillRect(6 + (i * 21), 117, (5 * 3), (5 * 3), TFT_COLOR1);
              tft.fillRect(3 + (i * 21), 114, (5 * 3), (5 * 3), TFT_COLOR3);
            }
          } else {  //i == 11
            if (i == selectedMenuCard) {
              tft.fillRect(222, 120, 15, 9, TFT_COLOR3);
              tft.fillRect(225, 129, 9, 3, TFT_COLOR3);
              tft.fillRect(219, 117, 15, 9, TFT_COLOR1);
              tft.fillRect(222, 114, 9, 15, TFT_COLOR1);
            } else {
              tft.fillRect(222, 120, 15, 9, TFT_COLOR1);
              tft.fillRect(225, 129, 9, 3, TFT_COLOR1);
              tft.fillRect(219, 117, 15, 9, TFT_COLOR3);
              tft.fillRect(222, 114, 9, 15, TFT_COLOR3);
            }
          }
        }
        //Divider
        tft.fillRect(213, 120, 3, 6, TFT_COLOR1);
        //Repeat counter
        if (selectedMenuCard < 10) {
          tft.fillRect(54, 87, 39, 9, TFT_COLOR4);
          tft.fillRect(57, 90, 15, 6, TFT_COLOR1);
          tft.fillRect(78, 90, 15, 6, TFT_COLOR1);
          tft.fillRect(54, 87, 15, 6, TFT_COLOR3);
          bool repeat = false;
          for (int i = 0; i < 10; i++) {
            if (deckIndex[i] == deckIndex[selectedMenuCard + 1] && i != selectedMenuCard + 1)
              repeat = true;
          }
          if (repeat)
            tft.fillRect(75, 87, 15, 6, TFT_COLOR3);
          mainMenuReturn = 0;
        } else {
          //Hide repeats for gems
          tft.fillRect(54, 87, 39, 9, TFT_COLOR4);
        }
        tft.fillRect(0, 48, 48, 48, TFT_COLOR4);
        pushScaled(3, 51, 15, 15, abilityFrame[0], TFT_BLACK);
        int baseAddr = getDeckBaseAddr();
        if (selectedDeck == 0) {
          pushScaled(126, -6, 5, 5, smallIcons[13], TFT_BLACK);
          pushScaled(9, 57, 11, 11, cardSpriteArray[EEPROM.read(baseAddr)], TFT_BLACK);
          tft.drawString(cardList[EEPROM.read(baseAddr)].name, 54, 57);
        } else if (selectedDeck == 1) {
          pushScaled(147, -6, 5, 5, smallIcons[13], TFT_BLACK);
          pushScaled(9, 57, 11, 11, cardSpriteArray[EEPROM.read(baseAddr + 10)], TFT_BLACK);
          tft.drawString(cardList[EEPROM.read(baseAddr + 10)].name, 54, 57);
        } else if (selectedDeck == 2) {
          pushScaled(168, -6, 5, 5, smallIcons[13], TFT_BLACK);
          pushScaled(9, 57, 11, 11, cardSpriteArray[EEPROM.read(baseAddr + 20)], TFT_BLACK);
          tft.drawString(cardList[EEPROM.read(baseAddr + 20)].name, 54, 57);
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        MENUSTATE = DECKLIST;
        tft.fillRect(0, 30, 240, 84, TFT_COLOR4);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        pushScaled(15, 27, 5, 5, smallIcons[13], TFT_BLACK);
        menuItem = 0;
        menuIndex = 0;
        pageNum = 0;
        CenterText("NEXT PAGE", 90);
        if (selectedMenuCard < 10) {
          pushScaled(0, 42, 15, 15, abilityFrame[0], TFT_BLACK);
          pushScaled(48, 42, 15, 15, abilityFrame[0], TFT_BLACK);
          pushScaled(96, 42, 15, 15, abilityFrame[0], TFT_BLACK);
          pushScaled(144, 42, 15, 15, abilityFrame[0], TFT_BLACK);
          pushScaled(192, 42, 15, 15, abilityFrame[0], TFT_BLACK);
          pushScaled(15, 69, 5, 5, smallIcons[13], TFT_BLACK);
          CompileListMenu("card");
        } else {
          //pageNum = 1;
          CompileListMenu("gems");
          pushScaled(15, 69, 5, 5, smallIcons[13], TFT_BLACK);
        }
        tft.fillRect(12, 72, 21, 3, TFT_COLOR1);
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON HOLD (Card Tooltip)
    if (BButton == "held" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        if (TOOLTIP == RELEASED) {
          TOOLTIP = ACTIVE;
          tft.fillScreen(TFT_COLOR4);
          if(selectedMenuCard < 10){
            pushScaled(3, 3, 15, 15, abilityFrame[0], TFT_BLACK);
            tft.fillRect(51, 3, 183, 27, TFT_COLOR3);
            tft.fillRect(234, 6, 3, 27, TFT_COLOR1);
            tft.fillRect(54, 30, 183, 3, TFT_COLOR1);
            pushScaled(9, 9, 11, 11, cardSpriteArray[deckIndex[selectedMenuCard]], TFT_BLACK);
            tft.drawString(cardList[deckIndex[selectedMenuCard]].name, 54, 6);
            tft.fillRect(54, 39, 15, 6, TFT_COLOR1);
            tft.fillRect(75, 39, 15, 6, TFT_COLOR1);
            tft.fillRect(51, 36, 15, 6, TFT_COLOR3);
            bool repeat = false;
            for (int i = 0; i < 10; i++) {
              if (deckIndex[i] == deckIndex[selectedMenuCard] && i != selectedMenuCard)
                repeat = true;
            }
            if (repeat)
              tft.fillRect(72, 36, 15, 6, TFT_COLOR3);
            tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
            if (cardList[deckIndex[selectedMenuCard]].damage < 100)
              tft.drawString(String(cardList[deckIndex[selectedMenuCard]].damage), 201, 36);
            else
              tft.drawString(String(cardList[deckIndex[selectedMenuCard]].damage), 183, 36);
            tft.fillRect(102, 36, 42, 15, TFT_COLOR4);
            if(cardList[deckIndex[selectedMenuCard]].canReplace)
              pushScaled(102, 36, 5, 5, smallIcons[16], TFT_BLACK);
            if(cardList[deckIndex[selectedMenuCard]].priority)
              pushScaled(129, 36, 5, 5, smallIcons[15], TFT_BLACK);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(cardList[deckIndex[selectedMenuCard]].desc1, 3, 63);
            tft.drawString(cardList[deckIndex[selectedMenuCard]].desc2, 3, 87);
            tft.drawString(cardList[deckIndex[selectedMenuCard]].desc3, 3, 111);
          } else {  //Gem Tooltip
            int gemIndex = 255;
            if(selectedDeck == 0) {
              gemIndex = EEPROM.read(128);
            } else if(selectedDeck == 1) {
              gemIndex = EEPROM.read(129);
            } if(selectedDeck == 2) {
              gemIndex = EEPROM.read(130);
            }
            tft.fillRect(51, 9, 183, 27, TFT_COLOR3);
            tft.fillRect(234, 12, 3, 27, TFT_COLOR1);
            tft.fillRect(54, 36, 183, 3, TFT_COLOR1);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            pushScaled(0, 0, 16, 16, gems[gemIndex], TFT_BLACK);
            tft.drawString(gemList[gemIndex].name1 + " " + gemList[gemIndex].name2, 54, 12);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(gemList[gemIndex].desc1, 3, 63);
            tft.drawString(gemList[gemIndex].desc2, 3, 87);
            tft.drawString(gemList[gemIndex].desc3, 3, 111);
          }
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON RELEASE (Card Tooltip)
    if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        TOOLTIP = RELEASED;
        tft.fillScreen(TFT_COLOR4);
        drawingBatteryIcon(batteryAnim);
        tft.fillRect(3, 3, (37 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 0, (37 * 3), (9 * 3), TFT_COLOR3);
        pushScaled(3, 51, 15, 15, abilityFrame[0], TFT_BLACK);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        if (deepDungeonActive) {
          tft.drawString("DD " + String(selectedDeck + 1), 3, 3);
        } else {
          tft.drawString("DECK " + String(selectedDeck + 1), 3, 3);
        }
        int baseAddr = getDeckBaseAddr();
        if (selectedDeck == 0) {
          tft.fillRect(123, 9, 21, 21, TFT_COLOR2);
          for (int i = 0; i < 10; i++)
            deckIndex[i] = EEPROM.read(baseAddr + i);
        } else if (selectedDeck == 1) {
          tft.fillRect(144, 9, 21, 21, TFT_COLOR2);
          for (int i = 0; i < 10; i++)
            deckIndex[i] = EEPROM.read(baseAddr + 10 + i);
        } else if (selectedDeck == 2) {
          tft.fillRect(165, 9, 21, 21, TFT_COLOR2);
          for (int i = 0; i < 10; i++)
            deckIndex[i] = EEPROM.read(baseAddr + 20 + i);
        }
        tft.fillRect(126, 12, 15, 15, TFT_COLOR1);
        tft.fillRect(147, 12, 15, 15, TFT_COLOR1);
        tft.fillRect(168, 12, 15, 15, TFT_COLOR1);
        tft.fillRect(54, 81, 183, 3, TFT_COLOR1);
        tft.fillRect(234, 57, 3, 27, TFT_COLOR1);
        tft.fillRect(51, 54, 183, 27, TFT_COLOR3);
        tft.fillRect(54, 87, 39, 9, TFT_COLOR4);

        if (equippedDeck == 0)
          tft.fillRect(126, 12, 15, 15, TFT_COLOR3);
        else if (equippedDeck == 1)
          tft.fillRect(147, 12, 15, 15, TFT_COLOR3);
        else if (equippedDeck == 2)
          tft.fillRect(168, 12, 15, 15, TFT_COLOR3);
        for (int i = 0; i < 11; i++) {
          if (i < 10) {
            if (i == selectedMenuCard) {
              tft.fillRect(6 + (i * 21), 117, (5 * 3), (5 * 3), TFT_COLOR3);
              tft.fillRect(3 + (i * 21), 114, (5 * 3), (5 * 3), TFT_COLOR1);
            } else {
              tft.fillRect(6 + (i * 21), 117, (5 * 3), (5 * 3), TFT_COLOR1);
              tft.fillRect(3 + (i * 21), 114, (5 * 3), (5 * 3), TFT_COLOR3);
            }
          } else {  //i == 11
            if (i == selectedMenuCard) {
              tft.fillRect(222, 120, 15, 9, TFT_COLOR3);
              tft.fillRect(225, 129, 9, 3, TFT_COLOR3);
              tft.fillRect(219, 117, 15, 9, TFT_COLOR1);
              tft.fillRect(222, 114, 9, 15, TFT_COLOR1);
            } else {
              tft.fillRect(222, 120, 15, 9, TFT_COLOR1);
              tft.fillRect(225, 129, 9, 3, TFT_COLOR1);
              tft.fillRect(219, 117, 15, 9, TFT_COLOR3);
              tft.fillRect(222, 114, 9, 15, TFT_COLOR3);
            }
          }
        }
        //Divider
        tft.fillRect(213, 120, 3, 6, TFT_COLOR1);
        //Repeat counter
        if (selectedMenuCard < 10) {
          pushScaled(9, 57, 11, 11, cardSpriteArray[deckIndex[selectedMenuCard]], TFT_BLACK);
          tft.drawString(cardList[deckIndex[selectedMenuCard]].name, 54, 57);
          tft.fillRect(54, 87, 39, 9, TFT_COLOR4);
          tft.fillRect(57, 90, 15, 6, TFT_COLOR1);
          tft.fillRect(78, 90, 15, 6, TFT_COLOR1);
          tft.fillRect(54, 87, 15, 6, TFT_COLOR3);
          bool repeat = false;
          for (int i = 0; i < 10; i++) {
            if (deckIndex[i] == deckIndex[selectedMenuCard] && i != selectedMenuCard)
              repeat = true;
          }
          if (repeat)
            tft.fillRect(75, 87, 15, 6, TFT_COLOR3);
          mainMenuReturn = 0;
        } else {
          if(selectedDeck == 0) {
            tft.drawString(gemList[EEPROM.read(128)].name1 + " " + gemList[EEPROM.read(128)].name2, 54, 57);
            pushScaled(0, 48, 16, 16, gems[EEPROM.read(128)], TFT_BLACK);
          } else if(selectedDeck == 1) {
            tft.drawString(gemList[EEPROM.read(129)].name1 + " " + gemList[EEPROM.read(128)].name2, 54, 57);
            pushScaled(0, 48, 16, 16, gems[EEPROM.read(129)], TFT_BLACK);
          } if(selectedDeck == 2) {
            tft.drawString(gemList[EEPROM.read(130)].name1 + " " + gemList[EEPROM.read(128)].name2, 54, 57);
            pushScaled(0, 48, 16, 16, gems[EEPROM.read(130)], TFT_BLACK);
          }
          //Hide repeats for gems
          tft.fillRect(54, 87, 39, 9, TFT_COLOR4);
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //A BUTTON RELEASED
    if (AButton == "released" && millis() - lastButtonPressTime > 500 && setDck)
      setDck = false;
    }
  //Deck Card List
  if (MENUSTATE == DECKLIST) {
    //A BUTTON
    if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        menuItem++;
        if (menuItem > maxItems)
          menuItem = 0;
        tft.fillRect(15, 33, 210, 9, TFT_COLOR4);
        if (selectedMenuCard < 10) {
          pushScaled(15 + (menuItem * 48), 27, 5, 5, smallIcons[13], TFT_BLACK);
        } else {
          pushScaled(15 + (menuItem * 54), 27, 5, 5, smallIcons[13], TFT_BLACK);
        }
        tft.fillRect(0, 87, 240, 27, TFT_COLOR4);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        if (menuItem == 0)
          CenterText("NEXT PAGE", 90);
        else {
          if (selectedMenuCard < 10)
            CenterText(cardList[menuValues[menuItem - 1]].name, 90);
          else
            CenterText(gemList[menuValues[menuItem - 1]].name1 + " " + gemList[menuValues[menuItem - 1]].name2, 90);
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //A BUTTON HOLD (Return to DECKCARDSELECT)
    if (AButton == "held" && millis() - lastButtonPressTime > 500 && !setDck) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        MENUSTATE = DECKCARDSELECT;
        setDck = true;
        tft.fillRect(0, 30, 240, 84, TFT_COLOR4);
        tft.fillRect(54, 57, 183, 27, TFT_COLOR1);
        tft.fillRect(51, 54, 183, 27, TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        if(selectedMenuCard < 10) {
          pushScaled(3, 51, 15, 15, abilityFrame[0], TFT_BLACK);
          pushScaled(9, 57, 11, 11, cardSpriteArray[deckIndex[selectedMenuCard]], TFT_BLACK);
          tft.drawString(cardList[deckIndex[selectedMenuCard]].name, 54, 57);
          tft.fillRect(54, 87, 39, 9, TFT_COLOR4);
          tft.fillRect(57, 90, 15, 6, TFT_COLOR1);
          tft.fillRect(78, 90, 15, 6, TFT_COLOR1);
          tft.fillRect(54, 87, 15, 6, TFT_COLOR3);
          bool repeat = false;
          for (int i = 0; i < 10; i++) {
            if (deckIndex[i] == deckIndex[selectedMenuCard] && i != selectedMenuCard)
              repeat = true;
          }
          if (repeat)
            tft.fillRect(75, 87, 15, 6, TFT_COLOR3);
        } else {
            if(selectedDeck == 0) {
            tft.drawString(gemList[EEPROM.read(128)].name1 + " " + gemList[EEPROM.read(128)].name2, 54, 57);
            pushScaled(0, 48, 16, 16, gems[EEPROM.read(128)], TFT_BLACK);
          } else if(selectedDeck == 1) {
            tft.drawString(gemList[EEPROM.read(129)].name1 + " " + gemList[EEPROM.read(128)].name2, 54, 57);
            pushScaled(0, 48, 16, 16, gems[EEPROM.read(129)], TFT_BLACK);
          } if(selectedDeck == 2) {
            tft.drawString(gemList[EEPROM.read(130)].name1 + " " + gemList[EEPROM.read(128)].name2, 54, 57);
            pushScaled(0, 48, 16, 16, gems[EEPROM.read(130)], TFT_BLACK);
          }
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        if (menuItem == 0) {  //Continue to the next page if they are on the first index
          if (selectedMenuCard < 10)
            CompileListMenu("card");
          else {
            CompileListMenu("gems");
            //tft.fillRect(18, 78, 15, 3, TFT_COLOR1);
          }
          mainMenuReturn = 0;
          return;
        } else {  //Equip selected item
          if (selectedMenuCard < 10) {
            //Checks to see if there are two copies of the card available
            int total = 0;  //if this is -1, it does not exist
            for (int i = 0; i < 10; i++) {
              if (menuValues[menuItem - 1] == deckIndex[i])
                total++;
            }
            if (total >= 2)
              return;
          }
          if (selectedMenuCard < 10)
            deckIndex[selectedMenuCard] = menuValues[menuItem - 1];
          else {
            if (selectedDeck == 0)
              EEPROM.write(128, menuValues[menuItem - 1]);
            if (selectedDeck == 1)
              EEPROM.write(129, menuValues[menuItem - 1]);
            if (selectedDeck == 2)
              EEPROM.write(130, menuValues[menuItem - 1]);
          }
          EEPROM.write((getDeckBaseAddr() + (selectedDeck * 10) + selectedMenuCard), menuValues[menuItem - 1]);
        }
        EEPROM.commit();
        CalculateMaxHP();
        MENUSTATE = DECKCARDSELECT;
        tft.fillRect(0, 30, 240, 84, TFT_COLOR4);
        pushScaled(3, 51, 15, 15, abilityFrame[0], TFT_BLACK);
        tft.fillRect(54, 57, 183, 27, TFT_COLOR1);
        tft.fillRect(51, 54, 183, 27, TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        if (selectedMenuCard < 10){
          pushScaled(9, 57, 11, 11, cardSpriteArray[deckIndex[selectedMenuCard]], TFT_BLACK);
          tft.drawString(cardList[deckIndex[selectedMenuCard]].name, 54, 57);
          tft.fillRect(54, 87, 39, 9, TFT_COLOR4);
          tft.fillRect(57, 90, 15, 6, TFT_COLOR1);
          tft.fillRect(78, 90, 15, 6, TFT_COLOR1);
          tft.fillRect(54, 87, 15, 6, TFT_COLOR3);
          bool repeat = false;
          for (int i = 0; i < 10; i++) {
            if (deckIndex[i] == deckIndex[selectedMenuCard] && i != selectedMenuCard)
              repeat = true;
          }
          if (repeat)
            tft.fillRect(75, 87, 15, 6, TFT_COLOR3);
        } else {
          EEPROM.write((getDeckBaseAddr() + (selectedDeck * 10) + selectedMenuCard), menuValues[menuItem - 1]);
          if(selectedDeck == 0) {
            tft.drawString(gemList[EEPROM.read(128)].name1 + " " + gemList[EEPROM.read(128)].name2, 54, 57);
            pushScaled(0, 48, 16, 16, gems[EEPROM.read(128)], TFT_BLACK);
          } else if(selectedDeck == 1) {
            tft.drawString(gemList[EEPROM.read(129)].name1 + " " + gemList[EEPROM.read(128)].name2, 54, 57);
            pushScaled(0, 48, 16, 16, gems[EEPROM.read(129)], TFT_BLACK);
          } if(selectedDeck == 2) {
            tft.drawString(gemList[EEPROM.read(130)].name1 + " " + gemList[EEPROM.read(128)].name2, 54, 57);
            pushScaled(0, 48, 16, 16, gems[EEPROM.read(130)], TFT_BLACK);
          }
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON HOLD (Card Tooltip)
    if (BButton == "held" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        if (TOOLTIP == RELEASED && menuItem != 0) {
          TOOLTIP = ACTIVE;
          tft.fillScreen(TFT_COLOR4);
          if (selectedMenuCard < 10) {
            pushScaled(3, 3, 15, 15, abilityFrame[0], TFT_BLACK);
            tft.fillRect(51, 3, 183, 27, TFT_COLOR3);
            tft.fillRect(234, 6, 3, 27, TFT_COLOR1);
            tft.fillRect(54, 30, 183, 3, TFT_COLOR1);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            pushScaled(9, 9, 11, 11, cardSpriteArray[menuValues[menuItem - 1]], TFT_BLACK);
            tft.drawString(cardList[menuValues[menuItem - 1]].name, 54, 6);
            tft.fillRect(54, 39, 15, 6, TFT_COLOR1);
            tft.fillRect(75, 39, 15, 6, TFT_COLOR1);
            int exists = -1;  //if this is -1, it does not exist
            for (int i = 0; i < 10; i++) {
              if (menuValues[menuItem - 1] == deckIndex[i])
                exists = i;
            }
            if (exists != -1)
              tft.fillRect(51, 36, 15, 6, TFT_COLOR3);
            bool repeat = false;
            for (int i = 0; i < 10; i++) {
              if (menuValues[menuItem - 1] == deckIndex[i] && i != exists)
                repeat = true;
            }
            if (repeat)
              tft.fillRect(72, 36, 15, 6, TFT_COLOR3);
            tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
            if (cardList[menuValues[menuItem - 1]].damage < 100)
              tft.drawString(String(cardList[menuValues[menuItem - 1]].damage), 201, 36);
            else
              tft.drawString(String(cardList[menuValues[menuItem - 1]].damage), 183, 36);
            tft.fillRect(102, 36, 42, 15, TFT_COLOR4);
            if(cardList[menuValues[menuItem - 1]].canReplace)
              pushScaled(102, 36, 5, 5, smallIcons[16], TFT_BLACK);
            if(cardList[menuValues[menuItem - 1]].priority)
              pushScaled(129, 36, 5, 5, smallIcons[15], TFT_BLACK);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(cardList[menuValues[menuItem - 1]].desc1, 3, 63);
            tft.drawString(cardList[menuValues[menuItem - 1]].desc2, 3, 87);
            tft.drawString(cardList[menuValues[menuItem - 1]].desc3, 3, 111);
          } else {  //Gem Tooltip
            int gemIndex = 255;
            if(selectedDeck == 0) {
              gemIndex = EEPROM.read(128);
            } else if(selectedDeck == 1) {
              gemIndex = EEPROM.read(129);
            } if(selectedDeck == 2) {
              gemIndex = EEPROM.read(130);
            }
            tft.fillRect(51, 9, 183, 27, TFT_COLOR3);
            tft.fillRect(234, 12, 3, 27, TFT_COLOR1);
            tft.fillRect(54, 36, 183, 3, TFT_COLOR1);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            pushScaled(0, 0, 16, 16, gems[menuValues[menuItem - 1]], TFT_BLACK);
            tft.drawString(gemList[menuValues[menuItem - 1]].name1 + " " + gemList[menuValues[menuItem - 1]].name2, 54, 12);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(gemList[menuValues[menuItem - 1]].desc1, 3, 63);
            tft.drawString(gemList[menuValues[menuItem - 1]].desc2, 3, 87);
            tft.drawString(gemList[menuValues[menuItem - 1]].desc3, 3, 111);
          }
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON RELEASE (Card Tooltip)
    if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        TOOLTIP = RELEASED;
        tft.fillScreen(TFT_COLOR4);
        tft.fillRect(3, 3, (37 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 0, (37 * 3), (9 * 3), TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString("DECK " + String(selectedDeck + 1), 3, 3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        if (selectedMenuCard < 10) {
          pushScaled(15 + (menuItem * 48), 27, 5, 5, smallIcons[13], TFT_BLACK);
          pushScaled(0, 42, 15, 15, abilityFrame[0], TFT_BLACK);
          pushScaled(48, 42, 15, 15, abilityFrame[0], TFT_BLACK);
          pushScaled(96, 42, 15, 15, abilityFrame[0], TFT_BLACK);
          pushScaled(144, 42, 15, 15, abilityFrame[0], TFT_BLACK);
          pushScaled(192, 42, 15, 15, abilityFrame[0], TFT_BLACK);
          CompileListMenu("cardTooltip");
        } else {
          pushScaled(15 + (menuItem * 54), 27, 5, 5, smallIcons[13], TFT_BLACK);
          CompileListMenu("gemTooltip");
        }
        tft.fillRect(12, 72, 21, 3, TFT_COLOR1);
        pushScaled(15, 69, 5, 5, smallIcons[13], TFT_BLACK);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        if (menuItem == 0)
          CenterText("NEXT PAGE", 90);
        else {
          if (selectedMenuCard < 10)
            CenterText(cardList[menuValues[menuItem - 1]].name, 90);
          else
            CenterText(gemList[menuValues[menuItem - 1]].name1 + " " + gemList[menuValues[menuItem - 1]].name2, 90);
        }
        if (selectedDeck == 0)
          tft.fillRect(123, 9, 21, 21, TFT_COLOR2);
        else if (selectedDeck == 1)
          tft.fillRect(144, 9, 21, 21, TFT_COLOR2);
        else if (selectedDeck == 2)
          tft.fillRect(165, 9, 21, 21, TFT_COLOR2);
        tft.fillRect(126, 12, 15, 15, TFT_COLOR1);
        tft.fillRect(147, 12, 15, 15, TFT_COLOR1);
        tft.fillRect(168, 12, 15, 15, TFT_COLOR1);
        if (equippedDeck == 0)
          tft.fillRect(126, 12, 15, 15, TFT_COLOR3);
        else if (equippedDeck == 1)
          tft.fillRect(147, 12, 15, 15, TFT_COLOR3);
        else if (equippedDeck == 2)
          tft.fillRect(168, 12, 15, 15, TFT_COLOR3);
        for (int i = 0; i < 11; i++) {
          if (i < 10) {
            if (i == selectedMenuCard) {
              tft.fillRect(6 + (i * 21), 117, (5 * 3), (5 * 3), TFT_COLOR3);
              tft.fillRect(3 + (i * 21), 114, (5 * 3), (5 * 3), TFT_COLOR1);
            } else {
              tft.fillRect(6 + (i * 21), 117, (5 * 3), (5 * 3), TFT_COLOR1);
              tft.fillRect(3 + (i * 21), 114, (5 * 3), (5 * 3), TFT_COLOR3);
            }
          } else {  //i == 11
            if (i == selectedMenuCard) {
              tft.fillRect(222, 120, 15, 9, TFT_COLOR3);
              tft.fillRect(225, 129, 9, 3, TFT_COLOR3);
              tft.fillRect(219, 117, 15, 9, TFT_COLOR1);
              tft.fillRect(222, 114, 9, 15, TFT_COLOR1);
            } else {
              tft.fillRect(222, 120, 15, 9, TFT_COLOR1);
              tft.fillRect(225, 129, 9, 3, TFT_COLOR1);
              tft.fillRect(219, 117, 15, 9, TFT_COLOR3);
              tft.fillRect(222, 114, 9, 15, TFT_COLOR3);
            }
          }
        }
        //Divider
        tft.fillRect(213, 120, 3, 6, TFT_COLOR1);
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
  }
#pragma endregion

#pragma region Venture
  //(A MENU): Venture
  if (MENUSTATE == MAINMENU && AMENUS == VENTURE) {
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500 && !gamePause) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        MENUSTATE = VENTUREMENU;
        ventureCancelConfirm = false;
        tft.fillScreen(TFT_COLOR4);
        drawingBatteryIcon(batteryAnim);
        tft.fillRect(3, 3, (43 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 0, (43 * 3), (9 * 3), TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString(String("VENTURE"), 3, 3);
        tft.fillRect(162, 42, (25 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(159, 39, (25 * 3), (9 * 3), TFT_COLOR3);
        selectedVentureMenu = 0;
        VentureMenu(selectedVentureMenu);
        tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
  }
  //Venture Menu (Control)
  if (MENUSTATE == VENTUREMENU) {
    //Deep Dungeon gem-select screen
    if (ddGemSelect) {
      // Active input (incl. holding the tooltip) keeps the menu alive & resets the 15s timer
      if (AButton == "pressed" || AButton == "held" || BButton == "pressed" || BButton == "held")
        mainMenuReturn = 0;
      //A BUTTON - cycle the three gems
      if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          ddGemSel = (ddGemSel + 1) % 3;
          UpdateDDGemSelect();
        }
        if (screenOff) screenOff = false;
      }
      //B BUTTON HELD - show this gem's tooltip
      if (BButton == "held" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          if (TOOLTIP == RELEASED) {
            TOOLTIP = ACTIVE;
            DrawDDGemTooltip();
          }
        }
        if (screenOff) screenOff = false;
      }
      //B BUTTON RELEASED - close tooltip, back to gem select
      if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          TOOLTIP = RELEASED;
          DrawDDGemSelect();
        }
        if (screenOff) screenOff = false;
      }
      //B BUTTON - open the enter-confirmation
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500 && TOOLTIP == RELEASED) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          ddGemSelect = false;
          ddGemConfirm = true;
          ddGemConfirmYes = false;
          DrawDDGemConfirm();
        }
        if (screenOff) screenOff = false;
      }
    }
    //Deep Dungeon gem enter-confirmation
    else if (ddGemConfirm) {
      if (AButton == "pressed" || BButton == "pressed") mainMenuReturn = 0;
      //A BUTTON - toggle YES/NO
      if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          ddGemConfirmYes = !ddGemConfirmYes;
          tft.fillRect(45, 93, 66, 39, TFT_COLOR4);
          tft.fillRect(120, 93, 75, 39, TFT_COLOR4);
          if (ddGemConfirmYes) {
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
        if (screenOff) screenOff = false;
      }
      //B BUTTON - confirm
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          if (ddGemConfirmYes) {
            activeGem = ddGemChoices[ddGemSel];
            ddGemConfirm = false;
            // Roll 3 distinct merits and open merit selection
            RollMeritDraft();
            ddMeritSel = 0;
            ddMeritSelect = true;
            DrawDDMeritSelect();
          } else {
            // NO -- back to the gem screen
            ddGemConfirm = false;
            ddGemSelect = true;
            DrawDDGemSelect();
          }
        }
        if (screenOff) screenOff = false;
      }
    }
    //Deep Dungeon merit selection
    else if (ddMeritSelect) {
      //A BUTTON - cycle the three merits
      if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          int prev = ddMeritSel;
          ddMeritSel = (ddMeritSel + 1) % 3;
          DrawDDMeritBar(prev);
          DrawDDMeritBar(ddMeritSel);
        }
        if (screenOff) screenOff = false;
      }
      //B BUTTON HELD - show this merit's tooltip
      if (BButton == "held" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          if (TOOLTIP == RELEASED) {
            TOOLTIP = ACTIVE;
            DrawDDMeritTooltip();
          }
        }
        if (screenOff) screenOff = false;
      }
      //B BUTTON RELEASED - close tooltip
      if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          TOOLTIP = RELEASED;
          DrawDDMeritSelect();
        }
        if (screenOff) screenOff = false;
      }
      //B BUTTON - lock merit
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500 && TOOLTIP == RELEASED) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          if (bossMeritDraft) {
            int newMerit = ddMeritChoices[ddMeritSel];
            if (activeMeritCount < GetMeritCap()) {
              activeMerits[activeMeritCount++] = newMerit;
              bossMeritDraft = false;
              ddMeritSelect = false;
              SaveDeepDungeon();
              GrantBossAffliction();   // sets bossAfflictionPopup + message
              DrawBossAfflictionPopup();
            } else {
              // At cap: choose one to discard (current merits + the new draft)
              pendingMerit = newMerit;
              ddMeritSelect = false;
              ddDiscardSel = 0;
              ddMeritDiscard = true;
              DrawMeritDiscard();
            }
          } else {
            activeMerits[0] = ddMeritChoices[ddMeritSel];
            activeMeritCount = 1;
            ddMeritSelect = false;
            ddCardDraft = true;
            ddDraftRound = 0;
            RollDraftChoices();
            DrawDDCardDraft();
          }
        }
        if (screenOff) screenOff = false;
      }
    }
    //Deep Dungeon merit discard (over cap) - pick one to cut
    else if (ddMeritDiscard) {
      if (AButton == "pressed" || BButton == "pressed") mainMenuReturn = 0;
      //A BUTTON - cycle candidates
      if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          ddDiscardSel = (ddDiscardSel + 1) % (activeMeritCount + 1);
          DrawMeritDiscardRows();
        }
        if (screenOff) screenOff = false;
      }
      //B BUTTON HELD - tooltip of the focused merit
      if (BButton == "held" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          if (TOOLTIP == RELEASED) {
            TOOLTIP = ACTIVE;
            int id = (ddDiscardSel < activeMeritCount) ? activeMerits[ddDiscardSel] : pendingMerit;
            DrawMeritTooltipById(id);
          }
        }
        if (screenOff) screenOff = false;
      }
      //B BUTTON RELEASED - close tooltip
      if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          TOOLTIP = RELEASED;
          DrawMeritDiscard();
        }
        if (screenOff) screenOff = false;
      }
      //B BUTTON - choose this merit to remove
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500 && TOOLTIP == RELEASED) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          ddMeritDiscard = false;
          ddMeritConfirm = true;
          ddDiscardConfirmYes = false;
          DrawMeritDiscardConfirm();
        }
        if (screenOff) screenOff = false;
      }
    }
    //Deep Dungeon merit discard confirmation
    else if (ddMeritConfirm) {
      if (AButton == "pressed" || BButton == "pressed") mainMenuReturn = 0;
      //A BUTTON - toggle YES/NO
      if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          ddDiscardConfirmYes = !ddDiscardConfirmYes;
          DrawDiscardYesNo();
        }
        if (screenOff) screenOff = false;
      }
      //B BUTTON - confirm
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          if (ddDiscardConfirmYes) {
            // Existing merit chosen: the new one takes its slot.
            // New merit chosen: keep the current set (decline it).
            if (ddDiscardSel < activeMeritCount) activeMerits[ddDiscardSel] = pendingMerit;
            pendingMerit = -1;
            ddMeritConfirm = false;
            bossMeritDraft = false;
            SaveDeepDungeon();
            GrantBossAffliction();
            DrawBossAfflictionPopup();
          } else {
            ddMeritConfirm = false;
            ddMeritDiscard = true;
            DrawMeritDiscard();
          }
        }
        if (screenOff) screenOff = false;
      }
    }
    //Boss affliction popup - press B to continue into the cooldown
    else if (bossAfflictionPopup) {
      // Active input (incl. holding the tooltip) keeps the menu alive & resets the 15s timer
      if (AButton == "pressed" || AButton == "held" || BButton == "pressed" || BButton == "held")
        mainMenuReturn = 0;
      //B BUTTON HELD - show the affliction's tooltip
      if (BButton == "held" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          if (TOOLTIP == RELEASED) {
            TOOLTIP = ACTIVE;
            DrawAfflictionTooltipById(bossAfflictionId, bossAfflictionLvl);
          }
        }
        if (screenOff) screenOff = false;
      }
      //B BUTTON RELEASED - close tooltip, back to the popup
      if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          TOOLTIP = RELEASED;
          DrawBossAfflictionPopup();
        }
        if (screenOff) screenOff = false;
      }
      //B BUTTON - accept and continue into the cooldown
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500 && TOOLTIP == RELEASED) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          bossAfflictionPopup = false;
          SaveDeepDungeon();
          MENUSTATE = MAINMENU;
          AMENUS = NA;
          ventureTime = debug ? 11 : 1801;
          countdownSeconds = ventureTime;   // post-battle cooldown before the next floor
          battleReady = false;
          VENTURESTATE = VENTURING;
          ACTION = WALKING;
          xPosition = 51;
          randomIdle = 0;
          SavePetStats();
          tft.fillScreen(TFT_COLOR4);
          DrawBackground();
          DrawMainMenu();
          display.fillRect(0, 0, 240, 54, TFT_COLOR4);
          display.fillRect(0, 54, 240, 3, TFT_COLOR1);
          DrawCharacter(xPosition, 60, true);
        }
        if (screenOff) screenOff = false;
      }
    }
    //Card-draft screen handling
    else if (ddCardDraft) {
      //A BUTTON - cycle the three card choices
      if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          ddCardSel = (ddCardSel + 1) % 3;
          UpdateDDCardDraft();
        }
        if (screenOff) screenOff = false;
      }
      //B BUTTON HELD - show this card's tooltip
      if (BButton == "held" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          if (TOOLTIP == RELEASED) {
            TOOLTIP = ACTIVE;
            DrawDDCardDraftTooltip();
          }
        }
        if (screenOff) screenOff = false;
      }
      //B BUTTON RELEASED - close tooltip
      if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          TOOLTIP = RELEASED;
          DrawDDCardDraft();
        }
        if (screenOff) screenOff = false;
      }
      //B BUTTON - pick this card; advance round or finish the draft
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500 && TOOLTIP == RELEASED) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          ddDraftPicks[ddDraftRound] = ddCardChoices[ddCardSel];
          ddDraftRound++;
          if (ddDraftRound < 3) {
            RollDraftChoices();
            DrawDDCardDraft();
          } else {
            ddCardDraft = false;
            playerCredits -= DD_BUYIN;
            SaveCredits();
            ddGemsRolled = false;
            InitializeDeepDungeon();
            randomEnemyGenerated = false;
            enemySpriteCreated = false;
            GenerateRandomEnemy();
            // Enter the run ready to battle, overriding any active regular venture
            VENTURESTATE = VENTURING;
            countdownSeconds = 0;
            ventureTime = debug ? 11 : 1801;
            battleReady = true;
            cooldownDoneNotify = false;
            ventureCancelConfirm = false;
            // Go straight to the home screen where READY shows (cooldown system)
            MENUSTATE = MAINMENU;
            AMENUS = NA;
            ACTION = WALKING;
            xPosition = 51;
            randomIdle = 0;
            tft.fillScreen(TFT_COLOR4);
            DrawBackground();
            DrawMainMenu();
            display.fillRect(0, 0, 240, 54, TFT_COLOR4);
            display.fillRect(0, 54, 240, 3, TFT_COLOR1);
            DrawCharacter(xPosition, 60, true);
          }
        }
        if (screenOff) screenOff = false;
      }
    }
    //Cancel confirmation dialog handling
    else if (ventureCancelConfirm) {
      //A BUTTON - Toggle YES/NO
      if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          ventureCancelSelectYes = !ventureCancelSelectYes;
          
          // Clear and redraw buttons
          tft.fillRect(45, 93, 66, 39, TFT_COLOR4);
          tft.fillRect(138, 93, 66, 39, TFT_COLOR4);
          
          if(ventureCancelSelectYes) {
            tft.fillRect(54, 99, 57, 27, TFT_COLOR1);
            tft.fillRect(51, 96, 57, 27, TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString("YES", 54, 99);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString("NO", 156, 99);
          } else {
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString("YES", 54, 99);
            tft.fillRect(148, 99, 54, 27, TFT_COLOR1);
            tft.fillRect(145, 96, 54, 27, TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString("NO", 156, 99);
          }
          mainMenuReturn = 0;
        }
        if (screenOff) screenOff = false;
      }
      
      //B BUTTON - Confirm selection
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          if(ventureCancelSelectYes) {
            // Cancel the venture
            VENTURESTATE = INACTIVE;
            ventureTime = 0;
            countdownSeconds = 0;
            ventureCancelConfirm = false;
            if (ACTION != INJURED) 
              injuryTime = 0;
            // Free enemy sprites
            for (int i = 0; i < 12; i++) {
              if (enemyPet[i] != nullptr) {
                delete[] enemyPet[i];
                enemyPet[i] = nullptr;
              }
            }
            enemySpriteCreated = false;  // Reset flag so sprite gets recreated
            // Return to main menu
            MENUSTATE = MAINMENU;
            AMENUS = NA;
            tft.fillScreen(TFT_COLOR4);
            DrawBackground();
            DrawMainMenu();
            xPosition = 99;
            display.fillRect(0, 0, 240, 54, TFT_COLOR4);
            display.fillRect(0, 54, 240, 3, TFT_COLOR1);
            DrawCharacter(xPosition, 60, true);
            mainMenuReturn = 0;
          } else {
            // Return to venture menu
            ventureCancelConfirm = false;
            tft.fillScreen(TFT_COLOR4);
            drawingBatteryIcon(batteryAnim);
            tft.fillRect(3, 3, (43 * 3), (9 * 3), TFT_COLOR1);
            tft.fillRect(0, 0, (43 * 3), (9 * 3), TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("VENTURE"), 3, 3);
            VentureMenu(selectedVentureMenu);
          }
        }
        if (screenOff) screenOff = false;
      }
    } else if (ddExitConfirm) {
      // DD Exit confirmation dialog handling
      //A BUTTON - Toggle YES/NO
      if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          ddExitSelectNo = !ddExitSelectNo;
          
          // Clear and redraw buttons
          tft.fillRect(45, 93, 66, 39, TFT_COLOR4);
          tft.fillRect(120, 93, 75, 39, TFT_COLOR4);
          
          if(!ddExitSelectNo) {
            // YES selected
            tft.fillRect(54, 99, 57, 27, TFT_COLOR1);
            tft.fillRect(51, 96, 57, 27, TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString("YES", 54, 99);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString("NO", 144, 99);
          } else {
            // NO selected
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString("YES", 54, 99);
            tft.fillRect(132, 99, 57, 27, TFT_COLOR1);
            tft.fillRect(129, 96, 57, 27, TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString("NO", 144, 99);
          }
          mainMenuReturn = 0;
        }
        if (screenOff) screenOff = false;
      }
      
      //B BUTTON - Confirm selection
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          if(!ddExitSelectNo) {
            // Exit DD mode
            ResetDeepDungeon();
            ddExitConfirm = false;
            
            // Return to venture menu with DD option
            selectedVentureMenu = 0;
            tft.fillScreen(TFT_COLOR4);
            drawingBatteryIcon(batteryAnim);
            tft.fillRect(3, 3, (43 * 3), (9 * 3), TFT_COLOR1);
            tft.fillRect(0, 0, (43 * 3), (9 * 3), TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("VENTURE"), 3, 3);
            VentureMenu(selectedVentureMenu);
            mainMenuReturn = 0;
          } else {
            // Return to venture menu
            ddExitConfirm = false;
            tft.fillScreen(TFT_COLOR4);
            drawingBatteryIcon(batteryAnim);
            tft.fillRect(3, 3, (43 * 3), (9 * 3), TFT_COLOR1);
            tft.fillRect(0, 0, (43 * 3), (9 * 3), TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("VENTURE"), 3, 3);
            VentureMenu(selectedVentureMenu);
          }
        }
        if (screenOff) screenOff = false;
      }
    } else {
      //A BUTTON - cycle menu options
      if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          // Don't allow selecting dungeon when sleeping or injured
          if (ACTION == SLEEPING || ACTION == INJURED) {
            selectedVentureMenu = 0; // Stay on farm
          } else {
            selectedVentureMenu++;
            // Max options depends on DD unlock status
            int maxOptions = deepDungeonUnlocked ? 2 : 1;
            if (selectedVentureMenu > maxOptions) selectedVentureMenu = 0;
          }
          VentureMenu(selectedVentureMenu);
          mainMenuReturn = 0;
        }
        if (screenOff)
          screenOff = false;
      }
      //B BUTTON
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          mainMenuReturn = 0;
          //Farm
          if(selectedVentureMenu == 0){
            MENUSTATE = FARM;
            tft.fillScreen(TFT_COLOR4);
            DrawBackground();
            tft.fillRect(3, 3, 75, 27, TFT_COLOR1);
            tft.fillRect(0, 0, 75, 27, TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString("FARM", 3, 3);
            tft.fillRect(0, 105, 240, 3, TFT_COLOR1);
            tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
            selectedPot = 0;
            harvestMessage = "";
            
            // Draw all farm elements
            DrawFarmArrow();
            for(int i = 0; i < 3; i++) {
              DrawPot(i);
            }
            
            // Show crop name for selected pot
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            if(potCrop[selectedPot] == 0) 
              CenterText("", 111);
            else if(potCrop[selectedPot] == 1) 
              CenterText("SWEET FRUIT", 111);
            else if(potCrop[selectedPot] == 2) 
              CenterText("ENERGY DRINK", 111); 
            else if(potCrop[selectedPot] == 3) 
              CenterText("HEALTH POTION", 111);
            DrawFarm();
          }
          //Dungeon - start venture (or continue a DD floor)
          if(selectedVentureMenu == 1 && VENTURESTATE == INACTIVE && countdownSeconds == 0){
            if (deepDungeonActive || Stamina >= requiredStamina) {
              battleReady = true;
              cooldownDoneNotify = false;
              ventureTime = 1801;
              if(debug)
                ventureTime = 11;
              countdownSeconds = 0;  // battle ready immediately; cooldown runs after the fight
              if (!deepDungeonActive) Stamina = Stamina - requiredStamina;  // DD floors cost no stamina
              MENUSTATE = MAINMENU;
              AMENUS = NA;
              VENTURESTATE = VENTURING;
              ACTION = WALKING;
              xPosition = 51;
              randomIdle = 0;
              tft.fillScreen(TFT_COLOR4);
              DrawBackground();
              DrawMainMenu();
              display.fillRect(0, 0, 240, 54, TFT_COLOR4);
              display.fillRect(0, 54, 240, 3, TFT_COLOR1);
              DrawCharacter(xPosition, 60, true);
            }
          }
          //Dungeon - cancel venture confirmation
          else if(selectedVentureMenu == 1 && VENTURESTATE == VENTURING){
            ventureCancelConfirm = true;
            ventureCancelSelectYes = true;
            
            // Draw cancel confirmation
            tft.fillScreen(TFT_COLOR4);
            tft.fillRect(3, 3, (43 * 3), (9 * 3), TFT_COLOR1);
            tft.fillRect(0, 0, (43 * 3), (9 * 3), TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString("VENTURE", 3, 3);
            
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            CenterText("CANCEL", 39);
            CenterText("VENTURE?", 63);
            
            // YES selected by default
            tft.fillRect(54, 99, 57, 27, TFT_COLOR1);
            tft.fillRect(51, 96, 57, 27, TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString("YES", 54, 99);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString("NO", 156, 99);
          }
          // Deep Dungeon - Enter (option 2, not active)
          else if(selectedVentureMenu == 2 && !deepDungeonActive && deepDungeonUnlocked){
            if (playerCredits < DD_BUYIN) {
              // Can't cover the buy-in -- flash a notice, stay on the menu
              tft.fillRect(0, 105, (80 * 3), (9 * 3), TFT_COLOR3);
              tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
              CenterText("NEED 50", 111);
            } else {
              // Roll 3 distinct gems ONCE; same set persists until a run begins (no reroll cheese)
              if (!ddGemsRolled) {
                ddGemChoices[0] = random(0, 9);
                do { ddGemChoices[1] = random(0, 9); } while (ddGemChoices[1] == ddGemChoices[0]);
                do { ddGemChoices[2] = random(0, 9); } while (ddGemChoices[2] == ddGemChoices[0] || ddGemChoices[2] == ddGemChoices[1]);
                ddGemSel = 0;
                ddGemsRolled = true;
              }
              ddGemSelect = true;
              DrawDDGemSelect();
            }
          }
          // Deep Dungeon - Exit confirmation (option 2, active)
          else if(selectedVentureMenu == 2 && deepDungeonActive){
            ddExitConfirm = true;
            ddExitSelectNo = true; // Default to NO
            
            // Draw exit confirmation
            tft.fillScreen(TFT_COLOR4);
            tft.fillRect(3, 3, (43 * 3), (9 * 3), TFT_COLOR1);
            tft.fillRect(0, 0, (43 * 3), (9 * 3), TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString("VENTURE", 3, 3);
            
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            CenterText("EXIT DEEP", 39);
            CenterText("DUNGEON?", 63);
            
            // NO selected by default
            tft.fillRect(132, 99, 57, 27, TFT_COLOR1);
            tft.fillRect(129, 96, 57, 27, TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString("YES", 54, 99);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString("NO", 144, 99);
          }
        }
        if (screenOff)
          screenOff = false;
      }
      //A BUTTON RELEASED
      if (AButton == "released" && millis() - lastButtonPressTime > 500 && setVnt)
        setVnt = false;
      //B HELD - SHOW ENEMY CARD TOOLTIP (dungeon view only)
      if (BButton == "held" && millis() - lastButtonPressTime > 500 && selectedVentureMenu == 1) {
        lastButtonPressTime = millis();
        if (TOOLTIP == RELEASED) {
          TOOLTIP = ACTIVE;
          // In DD mode or post-stage 5, always use random enemy (ventureEnemy has 5 entries)
          if (deepDungeonActive || stage >= 30)
            tooltipCard = cardList[randomEnemyDeck[0]];
          else
            tooltipCard = cardList[ventureEnemy[stage][4]];
          tft.fillScreen(TFT_COLOR4);
          pushScaled(3, 3, 15, 15, abilityFrame[0], TFT_BLACK);
          tft.fillRect(51, 3, 183, 27, TFT_COLOR3);
          tft.fillRect(234, 6, 3, 27, TFT_COLOR1);
          tft.fillRect(54, 30, 183, 3, TFT_COLOR1);
          pushScaled(9, 9, 11, 11, cardSpriteArray[tooltipCard.cardIndex], TFT_BLACK);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
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
      //B RELEASED - HIDE TOOLTIP
      if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
        lastButtonPressTime = millis();
        TOOLTIP = RELEASED;
        tft.fillScreen(TFT_COLOR4);
        drawingBatteryIcon(batteryAnim);
        tft.fillRect(3, 3, (43 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 0, (43 * 3), (9 * 3), TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString(String("VENTURE"), 3, 3);
        VentureMenu(selectedVentureMenu);
      }
    }
  }
  //VENTURE TO BATTLE
  if (MENUSTATE == MAINMENU && countdownSeconds == 0 && VENTURESTATE == VENTURING && AMENUS == NA && !screenOff) {
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500 && !gamePause) {
      lastButtonPressTime = millis();
      DrawCharacter(xPosition, 60, true);
      ventureTime = 0;
      battleReady = false;
      AMENUS = NA;
      VENTURESTATE = ENCOUNTER;
      ACTION = WALKING;
      randomIdle = 0;
      tft.fillRect(0, 108, 240, 27, TFT_COLOR3);   // wipe the "BATTLE" prompt on start
    }
  }
#pragma endregion

#pragma region Farm
if (MENUSTATE == FARM) {
  // Update farm timers
  UpdateFarm();
  
  //A BUTTON - Cycle between pots
  if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
    lastButtonPressTime = millis();
    if (screenOff == false) {
      selectedPot = (selectedPot + 1) % 3;
      DrawFarmArrow(); 
      harvestMessage = "";
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
      if(potCrop[selectedPot] == 0) {
        CenterText("", 111);
      } else if(potCrop[selectedPot] == 1) {
        int itemAddr = 143;
        if(potTimer[selectedPot] >= 360 && IsInventoryFull(itemAddr))
          tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
        CenterText("SWEET FRUIT", 111);
      } else if(potCrop[selectedPot] == 2) {
        int itemAddr = 144;
        if(potTimer[selectedPot] >= 360 && IsInventoryFull(itemAddr))
          tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
        CenterText("ENERGY DRINK", 111); 
      } else if(potCrop[selectedPot] == 3) {
        int itemAddr = 145;
        if(potTimer[selectedPot] >= 360 && IsInventoryFull(itemAddr))
          tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
        CenterText("HEALTH POTION", 111);
      }
      mainMenuReturn = 0;
    }
    if (screenOff)
      screenOff = false;
  }
  //A BUTTON RELEASED -Return to Venture Menu
    if (AButton == "held" && millis() - lastButtonPressTime > 500 && !setVnt){
      setVnt = true;
      MENUSTATE = VENTUREMENU;
      tft.fillScreen(TFT_COLOR4);
      drawingBatteryIcon(batteryAnim);
      //DisplayBatteryPercentage();
      tft.fillRect(3, 3, (43 * 3), (9 * 3), TFT_COLOR1);
      tft.fillRect(0, 0, (43 * 3), (9 * 3), TFT_COLOR3);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      tft.drawString(String("VENTURE"), 3, 3);
      tft.fillRect(162, 42, (25 * 3), (9 * 3), TFT_COLOR1);
      tft.fillRect(159, 39, (25 * 3), (9 * 3), TFT_COLOR3);
      selectedVentureMenu = 0;
      VentureMenu(selectedVentureMenu);
      tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
      //Venture Options (currently only 2  available - Dungeon and Farm)
      //tft.fillRect(159, 75, 15, 15, TFT_COLOR1);
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
      //tft.fillRect(222, 75, 15, 15, TFT_COLOR1);
      mainMenuReturn = 0;
    }

  //B BUTTON - Interact with selected pot
  if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
    lastButtonPressTime = millis();
    if (screenOff == false) {
      if(potCrop[selectedPot] == 0) {
        // Empty pot - show seed selection
        MENUSTATE = SEEDSELECT;
        ShowSeedSelection();
      }
      else if(potTimer[selectedPot] >= 12) { // 12 hours
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
        HarvestPlant();
      }
      mainMenuReturn = 0;
    }
    if (screenOff)
      screenOff = false;
  }
}

if (MENUSTATE == SEEDSELECT) {
  //A BUTTON - Cycle through seed types
  if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
    lastButtonPressTime = millis();
    if (screenOff == false) {
      menuItem = (menuItem + 1) % 3;
      
      // Update selection arrow
      tft.fillRect(18, 39, 189, 15, TFT_COLOR4);
      pushScaled(39 + (menuItem * 72), 33, 5, 5, smallIcons[13], TFT_BLACK);
      
      // Update description
      tft.fillRect(0, 111, 240, 24, TFT_COLOR4);
      if(menuItem == 0) CenterText("SWEET FRUIT", 111);
      else if(menuItem == 1) CenterText("ENERGY DRINK", 111);
      else if(menuItem == 2) CenterText("HEALTH POTION", 111);

      // Update quantity for the selected seed (Sweet Fruit 143, Energy 144, Potion 145)
      tft.fillRect(198, 27, 42, 24, TFT_COLOR4);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      tft.drawString("x" + String(EEPROM.read(143 + menuItem)), 204, 30);
      
      mainMenuReturn = 0;
    }
    if (screenOff)
      screenOff = false;
  }
  
  //B BUTTON - Select seed type
  if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
    lastButtonPressTime = millis();
    if (screenOff == false) {
      PlantSeed(menuItem + 1);
      mainMenuReturn = 0;
    }
    if (screenOff)
      screenOff = false;
  }
}
#pragma endregion

#pragma region Shop
  //(A MENU): Shop
  if (MENUSTATE == MAINMENU && AMENUS == SHOP) {
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500 && !gamePause) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        //SHOP SETUP
        MENUSTATE = SHOPMENU;
        shopSelection = 0;
        shopPurchaseConfirm = false;
        shopSelectYes = true;
        TOOLTIP = RELEASED;
        
        // Generate shop inventory
        if(shopRefresh){
          shopItemCount = 0;
          for(int i = 0; i < 10; i++) {
            shopInventory[i] = -1;
            shopRareItemPurchased[i] = false;
          }
          for(int i = 0; i < 10; i++) {
            if(i == 0 && deepDungeonActive) continue;
            if(shopItemList[i].availability != 0 && !CanShowRareItem(i)) continue;
            
            bool available = false;
            if(shopItemList[i].availability == 0) 
              available = true;
            else if(shopItemList[i].availability == 1) 
              available = (random(100) < 40);
            else if(shopItemList[i].availability == 2) 
              available = (random(100) < 15);
            if(available) {
              shopInventory[shopItemCount] = i;
              shopItemCount++;
            }
          }
          shopRefresh = false;
        }
        
        // Draw shop screen
        tft.fillScreen(TFT_COLOR4);
        
        // Title
        tft.fillRect(3, 3, (25 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 0, (25 * 3), (9 * 3), TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString("SHOP", 3, 3);
        
        // Credits display
        tft.fillRect((49 * 3), 3, (31 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect((48 * 3), 0, (32 * 3), (9 * 3), TFT_COLOR3);
        pushScaled(147, 6, 5, 5, smallIcons[11], TFT_BLACK);
        String displayCredits = String(playerCredits);
        if(displayCredits.length() == 1) 
          displayCredits = "000" + displayCredits;
        else if(displayCredits.length() == 2) 
          displayCredits = "00" + displayCredits;
        else if(displayCredits.length() == 3) 
          displayCredits = "0" + displayCredits;
        tft.drawString(displayCredits, 165, 3);
        
        // Draw current item
        if(shopItemCount > 0) {
          int itemIdx = shopInventory[shopSelection];
          ShopItem item = shopItemList[itemIdx];
          pushScaled(3, 45, 16, 16, items[item.spriteIndex], TFT_BLACK);
          int actualCost = merchantDiscountActive ? (item.cost / 2) : item.cost;
          bool canAfford = (playerCredits >= actualCost);
          if(canAfford) 
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          else 
            tft.setTextColor(TFT_COLOR2, TFT_COLOR4);
          tft.drawString(item.name1, 54, 45);
          tft.drawString(item.name2, 54, 69);
          
          // Cost box
          tft.fillRect(165, 57, (25 * 3), (9 * 3), TFT_COLOR1);
          tft.fillRect(162, 54, (25 * 3), (9 * 3), TFT_COLOR3);
          pushScaled(165, 60, 5, 5, smallIcons[11], TFT_BLACK);
          if(canAfford) 
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          else 
            tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
          String displayCost = "";
          if(merchantDiscountActive)
            displayCost = String((item.cost)/2);
          else
            displayCost = String(item.cost);
          if(displayCost.length() == 2) 
            displayCost = "0" + displayCost;
          if(displayCost.length() == 1) 
            displayCost = "00" + displayCost;
          tft.drawString(displayCost, 183, 57);
        }
        
        // Position indicators
        int startX = (240 - (shopItemCount * 21)) / 2;
        for(int i = 0; i < shopItemCount; i++) {
          int boxX = startX + (i * 21);
          if(i == shopSelection){
            if(shopItemList[shopInventory[i]].availability != 0){
              tft.fillRect(boxX + 9, 111, 9, 15, TFT_COLOR3);
              tft.fillRect(boxX + 6, 114, 15, 9, TFT_COLOR3);
              tft.fillRect(boxX + 6, 108, 9, 15, TFT_COLOR1);
              tft.fillRect(boxX + 3, 111, 15, 9, TFT_COLOR1);
            } else {
              tft.fillRect(boxX + 6, 111, 15, 15, TFT_COLOR3);
              tft.fillRect(boxX + 3, 108, 15, 15, TFT_COLOR1);
            }
          }
          else {
            if(shopItemList[shopInventory[i]].availability != 0){
              tft.fillRect(boxX + 9, 111, 9, 15, TFT_COLOR1);
              tft.fillRect(boxX + 6, 114, 15, 9, TFT_COLOR1);
              tft.fillRect(boxX + 6, 108, 9, 15, TFT_COLOR3);
              tft.fillRect(boxX + 3, 111, 15, 9, TFT_COLOR3);
            } else {
              tft.fillRect(boxX + 6, 111, 15, 15, TFT_COLOR1);
              tft.fillRect(boxX + 3, 108, 15, 15, TFT_COLOR3);
            }
          }
        }
        
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
  }
  
  //Shop Menu (Control)
  if (MENUSTATE == SHOPMENU) {
    // Handle reward screen dismiss
    if(shopShowingReward) {
      if ((AButton == "pressed" || BButton == "pressed") && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        
        // If screen was off, just wake it up and dismiss reward
        if(screenOff) {
          screenOff = false;
        }
        
        shopShowingReward = false;
        shopPurchaseConfirm = false;
        
        // Redraw shop screen
        tft.fillScreen(TFT_COLOR4);
        tft.fillRect(3, 3, (25 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 0, (25 * 3), (9 * 3), TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString("SHOP", 3, 3);
        
        tft.fillRect((49 * 3), 3, (31 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect((48 * 3), 0, (32 * 3), (9 * 3), TFT_COLOR3);
        pushScaled(147, 6, 5, 5, smallIcons[11], TFT_BLACK);
        String displayCredits = String(playerCredits);
        if(displayCredits.length() == 1) displayCredits = "000" + displayCredits;
        else if(displayCredits.length() == 2) displayCredits = "00" + displayCredits;
        else if(displayCredits.length() == 3) displayCredits = "0" + displayCredits;
        tft.drawString(displayCredits, 165, 3);
        
        if(shopItemCount > 0) {
          int itemIdx = shopInventory[shopSelection];
          ShopItem item = shopItemList[itemIdx];
          pushScaled(3, 45, 16, 16, items[item.spriteIndex], TFT_BLACK);
          int eepromAddr = 143 + item.itemIndex;
          int actualCost = merchantDiscountActive ? (item.cost / 2) : item.cost;
          bool canAfford = (playerCredits >= actualCost);
          bool inventoryFull = (item.availability == 0) ? IsInventoryFull(eepromAddr) : false;
          if(canAfford && !inventoryFull) 
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          else 
            tft.setTextColor(TFT_COLOR2, TFT_COLOR4);
          tft.drawString(item.name1, 54, 45);
          tft.drawString(item.name2, 54, 69);
          
          tft.fillRect(165, 57, (25 * 3), (9 * 3), TFT_COLOR1);
          tft.fillRect(162, 54, (25 * 3), (9 * 3), TFT_COLOR3);
          pushScaled(165, 60, 5, 5, smallIcons[11], TFT_BLACK);
          if(canAfford && !inventoryFull) 
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          else 
            tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
          String displayCost = "";
          if(merchantDiscountActive)
            displayCost = String((item.cost)/2);
          else
            displayCost = String(item.cost);
          if(displayCost.length() == 2) displayCost = "0" + displayCost;
          if(displayCost.length() == 1) displayCost = "00" + displayCost;
          tft.drawString(displayCost, 183, 57);
        } else {
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          CenterText("SOLD OUT", 57);
        }
        
        int startX = (240 - (shopItemCount * 21)) / 2;
        for(int i = 0; i < shopItemCount; i++) {
          int boxX = startX + (i * 21);
          if(i == shopSelection){
            if(shopItemList[shopInventory[i]].availability != 0){
              tft.fillRect(boxX + 9, 111, 9, 15, TFT_COLOR3);
              tft.fillRect(boxX + 6, 114, 15, 9, TFT_COLOR3);
              tft.fillRect(boxX + 6, 108, 9, 15, TFT_COLOR1);
              tft.fillRect(boxX + 3, 111, 15, 9, TFT_COLOR1);
            } else {
              tft.fillRect(boxX + 6, 111, 15, 15, TFT_COLOR3);
              tft.fillRect(boxX + 3, 108, 15, 15, TFT_COLOR1);
            }
          } else {
            if(shopItemList[shopInventory[i]].availability != 0){
              tft.fillRect(boxX + 9, 111, 9, 15, TFT_COLOR1);
              tft.fillRect(boxX + 6, 114, 15, 9, TFT_COLOR1);
              tft.fillRect(boxX + 6, 108, 9, 15, TFT_COLOR3);
              tft.fillRect(boxX + 3, 111, 15, 9, TFT_COLOR3);
            } else {
              tft.fillRect(boxX + 6, 111, 15, 15, TFT_COLOR1);
              tft.fillRect(boxX + 3, 108, 15, 15, TFT_COLOR3);
            }
          }
        }
        mainMenuReturn = 0;
      }
    }
    else if(!shopPurchaseConfirm) {
      // A BUTTON - Move to next item
      if (AButton == "pressed" && millis() - lastButtonPressTime > 500 && TOOLTIP == RELEASED) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          shopSelection++;
          if(shopSelection >= shopItemCount) shopSelection = 0;
          
        // Redraw item
        tft.fillRect(0, 42, (54 * 3), (18 * 3), TFT_COLOR4);
        // Draw current item
        if(shopItemCount > 0) {
          int itemIdx = shopInventory[shopSelection];
          ShopItem item = shopItemList[itemIdx];
          pushScaled(3, 45, 16, 16, items[item.spriteIndex], TFT_BLACK);
          int eepromAddr = 143 + item.itemIndex;
          int actualCost = merchantDiscountActive ? (item.cost / 2) : item.cost;
          bool canAfford = (playerCredits >= actualCost);
          bool inventoryFull = IsInventoryFull(eepromAddr);
          if(canAfford && !inventoryFull) 
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          else 
            tft.setTextColor(TFT_COLOR2, TFT_COLOR4);
          tft.drawString(item.name1, 54, 45);
          tft.drawString(item.name2, 54, 69);
            
          // Cost box
          tft.fillRect(165, 57, (25 * 3), (9 * 3), TFT_COLOR1);
          tft.fillRect(162, 54, (25 * 3), (9 * 3), TFT_COLOR3);
          pushScaled(165, 60, 5, 5, smallIcons[11], TFT_BLACK);
          if(canAfford && !inventoryFull) 
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          else 
            tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
          String displayCost = "";
          if(merchantDiscountActive)
            displayCost = String((item.cost)/2);
          else
            displayCost = String(item.cost);
          if(displayCost.length() == 2) 
            displayCost = "0" + displayCost;
          if(displayCost.length() == 1) 
            displayCost = "00" + displayCost;
          tft.drawString(displayCost, 183, 57);
        }
        
        // Position indicators
        int startX = (240 - (shopItemCount * 21)) / 2;
        for(int i = 0; i < shopItemCount; i++) {
          int boxX = startX + (i * 21);
          if(i == shopSelection){
            if(shopItemList[shopInventory[i]].availability != 0){
              tft.fillRect(boxX + 9, 111, 9, 15, TFT_COLOR3);
              tft.fillRect(boxX + 6, 114, 15, 9, TFT_COLOR3);
              tft.fillRect(boxX + 6, 108, 9, 15, TFT_COLOR1);
              tft.fillRect(boxX + 3, 111, 15, 9, TFT_COLOR1);
            } else {
              tft.fillRect(boxX + 6, 111, 15, 15, TFT_COLOR3);
              tft.fillRect(boxX + 3, 108, 15, 15, TFT_COLOR1);
            }
          }
          else {
            if(shopItemList[shopInventory[i]].availability != 0){
              tft.fillRect(boxX + 9, 111, 9, 15, TFT_COLOR1);
              tft.fillRect(boxX + 6, 114, 15, 9, TFT_COLOR1);
              tft.fillRect(boxX + 6, 108, 9, 15, TFT_COLOR3);
              tft.fillRect(boxX + 3, 111, 15, 9, TFT_COLOR3);
            } else {
              tft.fillRect(boxX + 6, 111, 15, 15, TFT_COLOR1);
              tft.fillRect(boxX + 3, 108, 15, 15, TFT_COLOR3);
            }
          }
        }
          mainMenuReturn = 0;
        }
        if (screenOff) screenOff = false;
      }
      
      // B BUTTON PRESS - Try to purchase
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500 && TOOLTIP == RELEASED) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          if(shopItemCount > 0) {
            int itemIdx = shopInventory[shopSelection];
            ShopItem item = shopItemList[itemIdx];
            int eepromAddr = 143 + item.itemIndex;
            int actualCost = merchantDiscountActive ? (item.cost / 2) : item.cost;
            if(playerCredits >= actualCost && !IsInventoryFull(eepromAddr)) {
              shopPurchaseConfirm = true;
              shopSelectYes = true;
              
              // Draw purchase confirmation
              tft.fillScreen(TFT_COLOR4);
              tft.fillRect(3, 3, (25 * 3), (9 * 3), TFT_COLOR1);
              tft.fillRect(0, 0, (25 * 3), (9 * 3), TFT_COLOR3);
              tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
              tft.drawString("SHOP", 3, 3);
              
              tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
              CenterText("PURCHASE?", 39);
              
              String displayCost = "";
              int costWidth = 0;
              int costX = 0;
              if(merchantDiscountActive){
                costWidth = tft.textWidth(String(item.cost/2)) + 18;
                costX = (240 - costWidth) / 2;
                pushScaled(costX, 66, 5, 5, smallIcons[11], TFT_BLACK);
                tft.drawString(String((item.cost)/2), costX + 18, 63);
              }
              else{
                costWidth = tft.textWidth(String(item.cost)) + 18;
                costX = (240 - costWidth) / 2;
                pushScaled(costX, 66, 5, 5, smallIcons[11], TFT_BLACK);
                tft.drawString(String(item.cost), costX + 18, 63);
              }
              
              // YES selected by default
              tft.fillRect(54, 99, 57, 27, TFT_COLOR1);
              tft.fillRect(51, 96, 57, 27, TFT_COLOR3);
              tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
              tft.drawString("YES", 54, 99);
              tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
              tft.drawString("NO", 156, 99);
            }
          }
          mainMenuReturn = 0;
        }
        if (screenOff) screenOff = false;
      }
      
      // B BUTTON HOLD - Show tooltip
      if (BButton == "held" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          if(TOOLTIP == RELEASED && shopItemCount > 0) {
            TOOLTIP = ACTIVE;
            
            int itemIdx = shopInventory[shopSelection];
            ShopItem item = shopItemList[itemIdx];
            
            tft.fillScreen(TFT_COLOR4);
            tft.fillRect(63, 6, 111, 51, TFT_COLOR1);
            tft.fillRect(60, 3, 111, 51, TFT_COLOR3);
            tft.drawString(item.name1, 63, 6);
            tft.drawString(item.name2, 63, 30);
            pushScaled(6, 3, 16, 16, items[item.spriteIndex], TFT_BLACK);
            
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(item.desc1, 3, 63);
            tft.drawString(item.desc2, 3, 87);
            tft.drawString(item.desc3, 3, 111);
          }
          mainMenuReturn = 0;
        }
        if (screenOff) screenOff = false;
      }
      
      // B BUTTON RELEASE - Return from tooltip
      if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          TOOLTIP = RELEASED;
          
          // Redraw full shop screen
          tft.fillScreen(TFT_COLOR4);
          
          // Title
          tft.fillRect(3, 3, (25 * 3), (9 * 3), TFT_COLOR1);
          tft.fillRect(0, 0, (25 * 3), (9 * 3), TFT_COLOR3);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          tft.drawString("SHOP", 3, 3);
          
          // Credits display
          tft.fillRect((49 * 3), 3, (31 * 3), (9 * 3), TFT_COLOR1);
          tft.fillRect((48 * 3), 0, (32 * 3), (9 * 3), TFT_COLOR3);
          pushScaled(147, 6, 5, 5, smallIcons[11], TFT_BLACK);
          String displayCredits = String(playerCredits);
          if(displayCredits.length() == 1) 
            displayCredits = "000" + displayCredits;
          else if(displayCredits.length() == 2) 
            displayCredits = "00" + displayCredits;
          else if(displayCredits.length() == 3) 
            displayCredits = "0" + displayCredits;
          tft.drawString(displayCredits, 165, 3);
          
          // Draw current item
          if(shopItemCount > 0) {
            int itemIdx = shopInventory[shopSelection];
            ShopItem item = shopItemList[itemIdx];
            pushScaled(3, 45, 16, 16, items[item.spriteIndex], TFT_BLACK);
            int eepromAddr = 143 + item.itemIndex;
          int actualCost = merchantDiscountActive ? (item.cost / 2) : item.cost;
          bool canAfford = (playerCredits >= actualCost);
          bool inventoryFull = IsInventoryFull(eepromAddr);
          if(canAfford && !inventoryFull) 
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          else 
            tft.setTextColor(TFT_COLOR2, TFT_COLOR4);
          tft.drawString(item.name1, 54, 45);
          tft.drawString(item.name2, 54, 69);
            
            // Cost box
            tft.fillRect(165, 57, (25 * 3), (9 * 3), TFT_COLOR1);
            tft.fillRect(162, 54, (25 * 3), (9 * 3), TFT_COLOR3);
            pushScaled(165, 60, 5, 5, smallIcons[11], TFT_BLACK);
            if(canAfford && !inventoryFull) 
              tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            else 
              tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
            String displayCost = "";
            if(merchantDiscountActive)
              displayCost = String((item.cost)/2);
            else
              displayCost = String(item.cost);
            if(displayCost.length() == 2) 
              displayCost = "0" + displayCost;
            if(displayCost.length() == 1) 
              displayCost = "00" + displayCost;
            tft.drawString(displayCost, 183, 57);
          }
          
          // Position indicators
          int startX = (240 - (shopItemCount * 21)) / 2;
          for(int i = 0; i < shopItemCount; i++) {
            int boxX = startX + (i * 21);
            if(i == shopSelection){
              if(shopItemList[shopInventory[i]].availability != 0){
                tft.fillRect(boxX + 9, 111, 9, 15, TFT_COLOR3);
                tft.fillRect(boxX + 6, 114, 15, 9, TFT_COLOR3);
                tft.fillRect(boxX + 6, 108, 9, 15, TFT_COLOR1);
                tft.fillRect(boxX + 3, 111, 15, 9, TFT_COLOR1);
              } else {
                tft.fillRect(boxX + 6, 111, 15, 15, TFT_COLOR3);
                tft.fillRect(boxX + 3, 108, 15, 15, TFT_COLOR1);
              }
            }
            else {
              if(shopItemList[shopInventory[i]].availability != 0){
                tft.fillRect(boxX + 9, 111, 9, 15, TFT_COLOR1);
                tft.fillRect(boxX + 6, 114, 15, 9, TFT_COLOR1);
                tft.fillRect(boxX + 6, 108, 9, 15, TFT_COLOR3);
                tft.fillRect(boxX + 3, 111, 15, 9, TFT_COLOR3);
              } else {
                tft.fillRect(boxX + 6, 111, 15, 15, TFT_COLOR1);
                tft.fillRect(boxX + 3, 108, 15, 15, TFT_COLOR3);
              }
            }
          }
          mainMenuReturn = 0;
        }
        if (screenOff) screenOff = false;
      }
    }
    else {
      // PURCHASE CONFIRMATION MODE
      
      // A BUTTON - Toggle YES/NO
      if (AButton == "pressed" && millis() - lastButtonPressTime > 500 && !shopShowingReward) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          shopSelectYes = !shopSelectYes;
          
          // Clear and redraw buttons
          tft.fillRect(45, 93, 66, 39, TFT_COLOR4);
          tft.fillRect(138, 93, 66, 39, TFT_COLOR4);
          
          if(shopSelectYes) {
            tft.fillRect(54, 99, 57, 27, TFT_COLOR1);
            tft.fillRect(51, 96, 57, 27, TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString("YES", 54, 99);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString("NO", 156, 99);
          } else {
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString("YES", 54, 99);
            tft.fillRect(148, 99, 54, 27, TFT_COLOR1);
            tft.fillRect(145, 96, 54, 27, TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString("NO", 156, 99);
          }
          
          mainMenuReturn = 0;
        }
        if (screenOff) screenOff = false;
      }
      
      // B BUTTON - Confirm selection
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500 && !shopShowingReward) {
        lastButtonPressTime = millis();
        if (screenOff == false) {
          if(shopSelectYes) {
            int itemIdx = shopInventory[shopSelection];
            ShopItem item = shopItemList[itemIdx];
            int eepromAddr = 143 + item.itemIndex;
            
            // Check if it's a rare item (availability != 0)
            if(item.availability != 0) {
              if(SpendCredits(item.cost)) {
                shopRewardIndex = -1;
                shopRewardName = "";
                shopRewardType = 0;
                
                // Card Pack (itemIndex 7)
                if(item.itemIndex == 7) {
                  if(deepDungeonActive) {
                    shopRewardIndex = GrantDDCard(-1);
                  } else {
                    shopRewardIndex = GrantCard(-1);
                  }
                  if(shopRewardIndex != -1) {
                    shopRewardType = 0;
                    shopRewardName = cardList[shopRewardIndex].name;
                  }
                }
                // Gear Drop (itemIndex 8)
                else if(item.itemIndex == 8) {
                  if(random(2) == 0 && HasUnownedHeadGear()) {
                    shopRewardIndex = GrantHeadGear(-1);
                    shopRewardType = 1;
                    if(shopRewardIndex != -1) shopRewardName = headNames[shopRewardIndex];
                  } else if(HasUnownedBodyGear()) {
                    shopRewardIndex = GrantBodyGear(-1);
                    shopRewardType = 2;
                    if(shopRewardIndex != -1) shopRewardName = bodyNames[shopRewardIndex];
                  } else if(HasUnownedHeadGear()) {
                    shopRewardIndex = GrantHeadGear(-1);
                    shopRewardType = 1;
                    if(shopRewardIndex != -1) shopRewardName = headNames[shopRewardIndex];
                  }
                }
                // Random Gem (itemIndex 9)
                else if(item.itemIndex == 9) {
                  shopRewardIndex = GrantGem(-1);
                  shopRewardType = 3;
                  if(shopRewardIndex != -1) {
                    shopRewardName = gemList[shopRewardIndex].name1 + " " + gemList[shopRewardIndex].name2;
                  }
                }
                
                EEPROM.commit();
                shopRareItemPurchased[itemIdx] = true;
                
                // Remove purchased rare item from shop immediately
                for(int i = shopSelection; i < shopItemCount - 1; i++) {
                  shopInventory[i] = shopInventory[i + 1];
                }
                shopInventory[shopItemCount - 1] = -1;
                shopItemCount--;
                if(shopSelection >= shopItemCount && shopItemCount > 0) {
                  shopSelection = shopItemCount - 1;
                }
                
                if(shopRewardIndex != -1) {
                  shopShowingReward = true;
                  tft.fillScreen(TFT_COLOR4);
                  
                  if(shopRewardType == 0) {
                    pushScaled(96, 33, 15, 15, abilityFrame[0], TFT_BLACK);
                    pushScaled(102, 39, 11, 11, cardSpriteArray[shopRewardIndex], TFT_BLACK);
                  } else if(shopRewardType == 1) {
                    pushScaled(96, 33, 15, 15, equipmentFrame[0], TFT_BLACK);
                    pushScaled(102, 39, 11, 11, headGearIcons[shopRewardIndex], TFT_BLACK);
                  } else if(shopRewardType == 2) {
                    pushScaled(96, 33, 15, 15, equipmentFrame[0], TFT_BLACK);
                    pushScaled(102, 39, 11, 11, bodyGearIcons[shopRewardIndex], TFT_BLACK);
                  } else if(shopRewardType == 3) {
                    pushScaled(96, 30, 16, 16, gems[shopRewardIndex], TFT_BLACK);
                  }
                  
                  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
                  CenterText(shopRewardName, 93);
                  mainMenuReturn = 0;
                }
              }
            } else {
              // Regular item - add to inventory
              if(!IsInventoryFull(eepromAddr) && SpendCredits(item.cost)) {
                int currentQty = EEPROM.read(eepromAddr);
                if(currentQty == 255) currentQty = 0;
                currentQty++;
                if(currentQty > 9) currentQty = 9;
                EEPROM.write(eepromAddr, currentQty);
                EEPROM.commit();
                ManageInventory();
              }
            }
          }
          
          // Only redraw shop if not showing reward
          if(!shopShowingReward) {
            shopPurchaseConfirm = false;
            
            tft.fillScreen(TFT_COLOR4);
            tft.fillRect(3, 3, (25 * 3), (9 * 3), TFT_COLOR1);
            tft.fillRect(0, 0, (25 * 3), (9 * 3), TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString("SHOP", 3, 3);
            
            tft.fillRect((49 * 3), 3, (31 * 3), (9 * 3), TFT_COLOR1);
            tft.fillRect((48 * 3), 0, (32 * 3), (9 * 3), TFT_COLOR3);
            pushScaled(147, 6, 5, 5, smallIcons[11], TFT_BLACK);
            String displayCredits = String(playerCredits);
            if(displayCredits.length() == 1) displayCredits = "000" + displayCredits;
            else if(displayCredits.length() == 2) displayCredits = "00" + displayCredits;
            else if(displayCredits.length() == 3) displayCredits = "0" + displayCredits;
            tft.drawString(displayCredits, 165, 3);
            
            if(shopItemCount > 0) {
              int itemIdx = shopInventory[shopSelection];
              ShopItem item = shopItemList[itemIdx];
              pushScaled(3, 45, 16, 16, items[item.spriteIndex], TFT_BLACK);
              int eepromAddr = 143 + item.itemIndex;
              int actualCost = merchantDiscountActive ? (item.cost / 2) : item.cost;
              bool canAfford = (playerCredits >= actualCost);
              bool inventoryFull = IsInventoryFull(eepromAddr);
              if(canAfford && !inventoryFull) 
                tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
              else 
                tft.setTextColor(TFT_COLOR2, TFT_COLOR4);
              tft.drawString(item.name1, 54, 45);
              tft.drawString(item.name2, 54, 69);
              
              tft.fillRect(165, 57, (25 * 3), (9 * 3), TFT_COLOR1);
              tft.fillRect(162, 54, (25 * 3), (9 * 3), TFT_COLOR3);
              pushScaled(165, 60, 5, 5, smallIcons[11], TFT_BLACK);
              if(canAfford && !inventoryFull) 
                tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
              else 
                tft.setTextColor(TFT_COLOR2, TFT_COLOR3);
              String displayCost = "";
              if(merchantDiscountActive)
                displayCost = String((item.cost)/2);
              else
                displayCost = String(item.cost);
              if(displayCost.length() == 2) displayCost = "0" + displayCost;
              if(displayCost.length() == 1) displayCost = "00" + displayCost;
              tft.drawString(displayCost, 183, 57);
            }
            
            int startX = (240 - (shopItemCount * 21)) / 2;
            for(int i = 0; i < shopItemCount; i++) {
              int boxX = startX + (i * 21);
              if(i == shopSelection){
                if(shopItemList[shopInventory[i]].availability != 0){
                  tft.fillRect(boxX + 9, 111, 9, 15, TFT_COLOR3);
                  tft.fillRect(boxX + 6, 114, 15, 9, TFT_COLOR3);
                  tft.fillRect(boxX + 6, 108, 9, 15, TFT_COLOR1);
                  tft.fillRect(boxX + 3, 111, 15, 9, TFT_COLOR1);
                } else {
                  tft.fillRect(boxX + 6, 111, 15, 15, TFT_COLOR3);
                  tft.fillRect(boxX + 3, 108, 15, 15, TFT_COLOR1);
                }
              } else {
                if(shopItemList[shopInventory[i]].availability != 0){
                  tft.fillRect(boxX + 9, 111, 9, 15, TFT_COLOR1);
                  tft.fillRect(boxX + 6, 114, 15, 9, TFT_COLOR1);
                  tft.fillRect(boxX + 6, 108, 9, 15, TFT_COLOR3);
                  tft.fillRect(boxX + 3, 111, 15, 9, TFT_COLOR3);
                } else {
                  tft.fillRect(boxX + 6, 111, 15, 15, TFT_COLOR1);
                  tft.fillRect(boxX + 3, 108, 15, 15, TFT_COLOR3);
                }
              }
            }
          }
          mainMenuReturn = 0;
        }
        if (screenOff) screenOff = false;
      }
    }
  }
#pragma endregion

#pragma region Wardrobe
  //(A MENU): Wardrobe
  if (MENUSTATE == MAINMENU && AMENUS == WARDROBE) {
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        //WARDROBE SETUP
        MENUSTATE = WARDROBEMENU;
        tft.fillScreen(TFT_COLOR4);
        drawingBatteryIcon(batteryAnim);
        tft.fillRect(3, 3, (49 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 0, (49 * 3), (9 * 3), TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString(String("WARDROBE"), 3, 3);

        tft.fillRect(0, 105, 240, 3, TFT_COLOR1);
        tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
        selectedWardrobe = equippedWardrobe;
        CreateWardrobeCharacter(0, headGear[EEPROM.read(135)], bodyGear[EEPROM.read(136)]);
        CreateWardrobeCharacter(1, headGear[EEPROM.read(137)], bodyGear[EEPROM.read(138)]);
        CreateWardrobeCharacter(2, headGear[EEPROM.read(139)], bodyGear[EEPROM.read(140)]);
        tft.pushImage(162, 60, 48, 48, wardrobePet[selectedWardrobe], TFT_BLACK);
        pushScaled(90, 45, 15, 15, equipmentFrame[0], TFT_BLACK);
        pushScaled(21, 45, 15, 15, equipmentFrame[0], TFT_BLACK);
        if (equippedWardrobe == 0) {
          tft.fillRect(159, 39, 15, 15, TFT_COLOR3);
          pushScaled(159, 21, 5, 5, smallIcons[13], TFT_BLACK);
          pushScaled(27, 51, 11, 11, headGearIcons[EEPROM.read(135)], TFT_BLACK);
          pushScaled(96, 51, 11, 11, bodyGearIcons[EEPROM.read(136)], TFT_BLACK);
        } else
          tft.fillRect(159, 39, 15, 15, TFT_COLOR1);
        if (equippedWardrobe == 1) {
          tft.fillRect(180, 39, 15, 15, TFT_COLOR3);
          pushScaled(180, 21, 5, 5, smallIcons[13], TFT_BLACK);
          pushScaled(27, 51, 11, 11, headGearIcons[EEPROM.read(137)], TFT_BLACK);
          pushScaled(96, 51, 11, 11, bodyGearIcons[EEPROM.read(138)], TFT_BLACK);
        } else
          tft.fillRect(180, 39, 15, 15, TFT_COLOR1);
        if (equippedWardrobe == 2) {
          tft.fillRect(201, 39, 15, 15, TFT_COLOR3);
          pushScaled(201, 21, 5, 5, smallIcons[13], TFT_BLACK);
          pushScaled(27, 51, 11, 11, headGearIcons[EEPROM.read(139)], TFT_BLACK);
          pushScaled(96, 51, 11, 11, bodyGearIcons[EEPROM.read(140)], TFT_BLACK);
        } else
          tft.fillRect(201, 39, 15, 15, TFT_COLOR1);
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
  }
  //Wardrobe Menu (Control)
  if (MENUSTATE == WARDROBEMENU) {
    //A BUTTON
    if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        selectedWardrobe++;
        if (selectedWardrobe > 2)
          selectedWardrobe = 0;
        tft.fillRect(27, 51, 33, 33, TFT_COLOR4);
        tft.fillRect(96, 51, 33, 33, TFT_COLOR4);
        if (selectedWardrobe == 0) {
          pushScaled(159, 21, 5, 5, smallIcons[13], TFT_BLACK);
          tft.pushImage(162, 60, 48, 48, wardrobePet[0], TFT_BLACK);
          pushScaled(27, 51, 11, 11, headGearIcons[EEPROM.read(135)], TFT_BLACK);
          pushScaled(96, 51, 11, 11, bodyGearIcons[EEPROM.read(136)], TFT_BLACK);
          tft.fillRect(180, 27, 15, 9, TFT_COLOR4);
          tft.fillRect(201, 27, 15, 9, TFT_COLOR4);
        } else if (selectedWardrobe == 1) {
          pushScaled(180, 21, 5, 5, smallIcons[13], TFT_BLACK);
          tft.pushImage(162, 60, 48, 48, wardrobePet[1], TFT_BLACK);
          pushScaled(27, 51, 11, 11, headGearIcons[EEPROM.read(137)], TFT_BLACK);
          pushScaled(96, 51, 11, 11, bodyGearIcons[EEPROM.read(138)], TFT_BLACK);
          tft.fillRect(159, 27, 15, 9, TFT_COLOR4);
          tft.fillRect(201, 27, 15, 9, TFT_COLOR4);
        } else if (selectedWardrobe == 2) {
          pushScaled(201, 21, 5, 5, smallIcons[13], TFT_BLACK);
          tft.pushImage(162, 60, 48, 48, wardrobePet[2], TFT_BLACK);
          pushScaled(27, 51, 11, 11, headGearIcons[EEPROM.read(139)], TFT_BLACK);
          pushScaled(96, 51, 11, 11, bodyGearIcons[EEPROM.read(140)], TFT_BLACK);
          tft.fillRect(159, 27, 15, 9, TFT_COLOR4);
          tft.fillRect(180, 27, 15, 9, TFT_COLOR4);
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        MENUSTATE = WRDSELECT;
        if (selectedWardrobe == 0) {
          tft.fillRect(156, 36, 21, 21, TFT_COLOR2);
          if (equippedWardrobe == 0)
            tft.fillRect(159, 39, 15, 15, TFT_COLOR3);
          else
            tft.fillRect(159, 39, 15, 15, TFT_COLOR1);
          CenterText(headNames[EEPROM.read(135)], 111);
        }
        if (selectedWardrobe == 1) {
          tft.fillRect(177, 36, 21, 21, TFT_COLOR2);
          if (equippedWardrobe == 1)
            tft.fillRect(180, 39, 15, 15, TFT_COLOR3);
          else
            tft.fillRect(180, 39, 15, 15, TFT_COLOR1);
          CenterText(headNames[EEPROM.read(137)], 111);
        }
        if (selectedWardrobe == 2) {
          tft.fillRect(198, 36, 21, 21, TFT_COLOR2);
          if (equippedWardrobe == 2)
            tft.fillRect(201, 39, 15, 15, TFT_COLOR3);
          else
            tft.fillRect(201, 39, 15, 15, TFT_COLOR1);
          CenterText(headNames[EEPROM.read(139)], 111);
        }
        tft.fillRect(159, 27, 57, 9, TFT_COLOR4);
        pushScaled(36, 30, 5, 5, smallIcons[13], TFT_BLACK);
        wardrobeSlot = "head";
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON HOLD
    if (BButton == "held" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        equippedWardrobe = selectedWardrobe;
        if (equippedWardrobe == 0) {
          tft.fillRect(159, 39, 15, 15, TFT_COLOR3);
          CreateCharacter("player", headGear[EEPROM.read(135)], bodyGear[EEPROM.read(136)]);
        } else
          tft.fillRect(159, 39, 15, 15, TFT_COLOR1);
        if (equippedWardrobe == 1) {
          tft.fillRect(180, 39, 15, 15, TFT_COLOR3);
          CreateCharacter("player", headGear[EEPROM.read(137)], bodyGear[EEPROM.read(138)]);
        } else
          tft.fillRect(180, 39, 15, 15, TFT_COLOR1);
        if (equippedWardrobe == 2) {
          tft.fillRect(201, 39, 15, 15, TFT_COLOR3);
          CreateCharacter("player", headGear[EEPROM.read(139)], bodyGear[EEPROM.read(140)]);
        } else
          tft.fillRect(201, 39, 15, 15, TFT_COLOR1);

        EEPROM.write(134, equippedWardrobe);
        EEPROM.commit();
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //A BUTTON RELEASED
    if (AButton == "released" && millis() - lastButtonPressTime > 500 && setWrd)
      setWrd = false;
  }
  //Wardrobe Select
  if (MENUSTATE == WRDSELECT) {
    //A BUTTON
    if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        if (wardrobeSlot == "head") {
          pushScaled(105, 30, 5, 5, smallIcons[13], TFT_BLACK);
          tft.fillRect(36, 36, 15, 9, TFT_COLOR4);
          wardrobeSlot = "body";
          tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
          if (selectedWardrobe == 0)
            CenterText(bodyNames[EEPROM.read(136)], 111);
          else if (selectedWardrobe == 1)
            CenterText(bodyNames[EEPROM.read(138)], 111);
          else if (selectedWardrobe == 2)
            CenterText(bodyNames[EEPROM.read(140)], 111);
        } else if (wardrobeSlot == "body") {
          pushScaled(36, 30, 5, 5, smallIcons[13], TFT_BLACK);
          tft.fillRect(105, 36, 15, 9, TFT_COLOR4);
          wardrobeSlot = "head";
          tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
          if (selectedWardrobe == 0)
            CenterText(headNames[EEPROM.read(135)], 111);
          else if (selectedWardrobe == 1)
            CenterText(headNames[EEPROM.read(137)], 111);
          else if (selectedWardrobe == 2)
            CenterText(headNames[EEPROM.read(149)], 111);
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //A BUTTON HOLD (Return to WARDROBEMENU)
    if (AButton == "held" && millis() - lastButtonPressTime > 500 && !setWrd) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        MENUSTATE = WARDROBEMENU;
        setWrd = true;
        tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
        tft.fillRect(36, 36, 84, 9, TFT_COLOR4);
        tft.fillRect(156, 36, 63, 24, TFT_COLOR4);
        tft.fillRect(159, 39, 15, 15, TFT_COLOR1);
        tft.fillRect(180, 39, 15, 15, TFT_COLOR1);
        tft.fillRect(201, 39, 15, 15, TFT_COLOR1);
        if (equippedWardrobe == 0)
          tft.fillRect(159, 39, 15, 15, TFT_COLOR3);
        if (equippedWardrobe == 1)
          tft.fillRect(180, 39, 15, 15, TFT_COLOR3);
        if (equippedWardrobe == 2)
          tft.fillRect(201, 39, 15, 15, TFT_COLOR3);
        wardrobeSlot = "head";
        if (selectedWardrobe == 0)
          pushScaled(159, 21, 5, 5, smallIcons[13], TFT_BLACK);
        else if (selectedWardrobe == 1)
          pushScaled(180, 21, 5, 5, smallIcons[13], TFT_BLACK);
        else if (selectedWardrobe == 2)
          pushScaled(201, 21, 5, 5, smallIcons[13], TFT_BLACK);
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        MENUSTATE = WRDLIST;
        tft.fillScreen(TFT_COLOR4);
        drawingBatteryIcon(batteryAnim);
        tft.fillRect(3, 3, (49 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 0, (49 * 3), (9 * 3), TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString(String("WARDROBE"), 3, 3);

        pushScaled(0, 48, 15, 15, equipmentFrame[0], TFT_BLACK);
        pushScaled(48, 48, 15, 15, equipmentFrame[0], TFT_BLACK);
        pushScaled(96, 48, 15, 15, equipmentFrame[0], TFT_BLACK);
        pushScaled(144, 48, 15, 15, equipmentFrame[0], TFT_BLACK);
        pushScaled(192, 48, 15, 15, equipmentFrame[0], TFT_BLACK);
        pushScaled(15, 33, 5, 5, smallIcons[13], TFT_BLACK);

        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        //tft.drawString(String("01"), 6, 54);
        tft.fillRect(12, 78, 21, 3, TFT_COLOR1);
        pushScaled(15, 75, 5, 5, smallIcons[13], TFT_BLACK);
        menuItem = 0;
        menuIndex = 0;
        pageNum = 1;
        tft.fillRect(0, 99, 240, 123, TFT_COLOR4);
        CenterText("NEXT PAGE", 102);
        if (wardrobeSlot == "head")
          CompileListMenu("head");
        else if (wardrobeSlot == "body")
          CompileListMenu("body");
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //A BUTTON RELEASE
    if (AButton == "released" && millis() - lastButtonPressTime > 500 && setWrd)
      setWrd = false;
  }
  //Wardrobe List
  if (MENUSTATE == WRDLIST) {
    //A BUTTON
    if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        menuItem++;
        if (menuItem > maxItems)
          menuItem = 0;
        tft.fillRect(15, 39, 210, 9, TFT_COLOR4);
        pushScaled(15 + (menuItem * 48), 33, 5, 5, smallIcons[13], TFT_BLACK);
        tft.fillRect(0, 99, 240, 123, TFT_COLOR4);
        if (menuItem == 0)
          CenterText("NEXT PAGE", 102);
        else {
          if (wardrobeSlot == "head")
            CenterText(headNames[menuValues[menuItem - 1]], 102);
          else if (wardrobeSlot == "body")
            CenterText(bodyNames[menuValues[menuItem - 1]], 102);
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //A BUTTON HOLD (Return to WRDSELECT)
    if (AButton == "held" && millis() - lastButtonPressTime > 500 && !setWrd) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        MENUSTATE = WRDSELECT;
        setWrd = true;
        tft.fillScreen(TFT_COLOR4);
        drawingBatteryIcon(batteryAnim);
        tft.fillRect(3, 3, (49 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 0, (49 * 3), (9 * 3), TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString(String("WARDROBE"), 3, 3);

        tft.fillRect(0, 105, 240, 3, TFT_COLOR1);
        tft.fillRect(0, 108, 240, 123, TFT_COLOR3);

        tft.pushImage(162, 60, 48, 48, wardrobePet[selectedWardrobe], TFT_BLACK);
        pushScaled(90, 45, 15, 15, equipmentFrame[0], TFT_BLACK);
        pushScaled(21, 45, 15, 15, equipmentFrame[0], TFT_BLACK);
        if (selectedWardrobe == 0) {
          pushScaled(27, 51, 11, 11, headGearIcons[EEPROM.read(135)], TFT_BLACK);
          pushScaled(96, 51, 11, 11, bodyGearIcons[EEPROM.read(136)], TFT_BLACK);
          tft.fillRect(156, 36, 21, 21, TFT_COLOR2);
        }
        if (selectedWardrobe == 1) {
          tft.fillRect(177, 36, 21, 21, TFT_COLOR2);
          pushScaled(27, 51, 11, 11, headGearIcons[EEPROM.read(137)], TFT_BLACK);
          pushScaled(96, 51, 11, 11, bodyGearIcons[EEPROM.read(138)], TFT_BLACK);
        }
        if (selectedWardrobe == 2) {
          tft.fillRect(198, 36, 21, 21, TFT_COLOR2);
          pushScaled(27, 51, 11, 11, headGearIcons[EEPROM.read(139)], TFT_BLACK);
          pushScaled(96, 51, 11, 11, bodyGearIcons[EEPROM.read(140)], TFT_BLACK);
        }
        tft.fillRect(159, 39, 15, 15, TFT_COLOR1);
        tft.fillRect(180, 39, 15, 15, TFT_COLOR1);
        tft.fillRect(201, 39, 15, 15, TFT_COLOR1);
        if (selectedWardrobe == 0)
          tft.fillRect(159, 39, 15, 15, TFT_COLOR3);
        if (selectedWardrobe == 1)
          tft.fillRect(180, 39, 15, 15, TFT_COLOR3);
        if (selectedWardrobe == 2)
          tft.fillRect(201, 39, 15, 15, TFT_COLOR3);
        tft.fillRect(159, 27, 57, 9, TFT_COLOR4);
        if (wardrobeSlot == "head") {
          pushScaled(36, 30, 5, 5, smallIcons[13], TFT_BLACK);
           tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
          if (equippedWardrobe == 0)
            CenterText(headNames[EEPROM.read(135)], 111);
          else if (equippedWardrobe == 1)
            CenterText(headNames[EEPROM.read(137)], 111);
          else if (equippedWardrobe == 2)
            CenterText(headNames[EEPROM.read(149)], 111);
        }
        else if (wardrobeSlot == "body") {
          pushScaled(105, 30, 5, 5, smallIcons[13], TFT_BLACK);
          tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
          if (selectedWardrobe == 0)
            CenterText(bodyNames[EEPROM.read(136)], 111);
          else if (selectedWardrobe == 1)
            CenterText(bodyNames[EEPROM.read(138)], 111);
          else if (selectedWardrobe == 2)
            CenterText(bodyNames[EEPROM.read(140)], 111);
        }
        mainMenuReturn = 0;
      }
      mainMenuReturn = 0;
    }
    if (screenOff)
      screenOff = false;
    //B BUTTON
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        if (menuItem == 0) {  //Continue to the next page if they are on the first index
          if (wardrobeSlot == "head")
            CompileListMenu("head");
          else if (wardrobeSlot == "body")
            CompileListMenu("body");
        } else {  //Equip selected item
          if (wardrobeSlot == "head") {
            if (selectedWardrobe == 0) {
              EEPROM.write(135, menuValues[menuItem - 1]);
              CreateWardrobeCharacter(0, headGear[EEPROM.read(135)], bodyGear[EEPROM.read(136)]);
              if (selectedWardrobe == equippedWardrobe)
                CreateCharacter("player", headGear[EEPROM.read(135)], bodyGear[EEPROM.read(136)]);
            } else if (selectedWardrobe == 1) {
              EEPROM.write(137, menuValues[menuItem - 1]);
              CreateWardrobeCharacter(1, headGear[EEPROM.read(137)], bodyGear[EEPROM.read(138)]);
              if (selectedWardrobe == equippedWardrobe)
                CreateCharacter("player", headGear[EEPROM.read(137)], bodyGear[EEPROM.read(138)]);
            } else if (selectedWardrobe == 2) {
              EEPROM.write(139, menuValues[menuItem - 1]);
              CreateWardrobeCharacter(2, headGear[EEPROM.read(139)], bodyGear[EEPROM.read(140)]);
              if (selectedWardrobe == equippedWardrobe)
                CreateCharacter("player", headGear[EEPROM.read(139)], bodyGear[EEPROM.read(140)]);
            }
          } else if (wardrobeSlot == "body") {
            if (selectedWardrobe == 0) {
              EEPROM.write(136, menuValues[menuItem - 1]);
              CreateWardrobeCharacter(0, headGear[EEPROM.read(135)], bodyGear[EEPROM.read(136)]);
              if (selectedWardrobe == equippedWardrobe)
                CreateCharacter("player", headGear[EEPROM.read(135)], bodyGear[EEPROM.read(136)]);
            } else if (selectedWardrobe == 1) {
              EEPROM.write(138, menuValues[menuItem - 1]);
              CreateWardrobeCharacter(1, headGear[EEPROM.read(137)], bodyGear[EEPROM.read(138)]);
              if (selectedWardrobe == equippedWardrobe)
                CreateCharacter("player", headGear[EEPROM.read(137)], bodyGear[EEPROM.read(138)]);
            } else if (selectedWardrobe == 2) {
              EEPROM.write(140, menuValues[menuItem - 1]);
              CreateWardrobeCharacter(2, headGear[EEPROM.read(139)], bodyGear[EEPROM.read(140)]);
              if (selectedWardrobe == equippedWardrobe)
                CreateCharacter("player", headGear[EEPROM.read(139)], bodyGear[EEPROM.read(140)]);
            }
          }
          EEPROM.commit();
          MENUSTATE = WRDSELECT;
          tft.fillScreen(TFT_COLOR4);
          drawingBatteryIcon(batteryAnim);
          tft.fillRect(3, 3, (49 * 3), (9 * 3), TFT_COLOR1);
          tft.fillRect(0, 0, (49 * 3), (9 * 3), TFT_COLOR3);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          tft.drawString(String("WARDROBE"), 3, 3);

          tft.fillRect(0, 105, 240, 3, TFT_COLOR1);
          tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
          tft.pushImage(162, 60, 48, 48, wardrobePet[selectedWardrobe], TFT_BLACK);
          pushScaled(90, 45, 15, 15, equipmentFrame[0], TFT_BLACK);
          pushScaled(21, 45, 15, 15, equipmentFrame[0], TFT_BLACK);
          if (wardrobeSlot == "head")
            pushScaled(36, 30, 5, 5, smallIcons[13], TFT_BLACK);
          else if (wardrobeSlot == "body")
            pushScaled(105, 30, 5, 5, smallIcons[13], TFT_BLACK);
          if (selectedWardrobe == 0) {
            tft.fillRect(156, 36, 21, 21, TFT_COLOR2);
            tft.fillRect(159, 39, 15, 15, TFT_COLOR1);
            pushScaled(27, 51, 11, 11, headGearIcons[EEPROM.read(135)], TFT_BLACK);
            pushScaled(96, 51, 11, 11, bodyGearIcons[EEPROM.read(136)], TFT_BLACK);
          } else
            tft.fillRect(159, 39, 15, 15, TFT_COLOR1);
          if (selectedWardrobe == 1) {
            tft.fillRect(177, 36, 21, 21, TFT_COLOR2);
            tft.fillRect(180, 39, 15, 15, TFT_COLOR1);
            pushScaled(27, 51, 11, 11, headGearIcons[EEPROM.read(137)], TFT_BLACK);
            pushScaled(96, 51, 11, 11, bodyGearIcons[EEPROM.read(138)], TFT_BLACK);
          } else
            tft.fillRect(180, 39, 15, 15, TFT_COLOR1);
          if (selectedWardrobe == 2) {
            tft.fillRect(198, 36, 21, 21, TFT_COLOR2);
            tft.fillRect(201, 39, 15, 15, TFT_COLOR1);
            pushScaled(27, 51, 11, 11, headGearIcons[EEPROM.read(139)], TFT_BLACK);
            pushScaled(96, 51, 11, 11, bodyGearIcons[EEPROM.read(140)], TFT_BLACK);
          } else
            tft.fillRect(201, 39, 15, 15, TFT_COLOR1);

          if (equippedWardrobe == 0)
            tft.fillRect(159, 39, 15, 15, TFT_COLOR3);
          else if (equippedWardrobe == 1)
            tft.fillRect(180, 39, 15, 15, TFT_COLOR3);
          else if (equippedWardrobe == 2)
            tft.fillRect(201, 39, 15, 15, TFT_COLOR3);
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
  }
#pragma endregion

#pragma region Sleep
  //(A MENU): Sleep/Wake
  if (MENUSTATE == MAINMENU && AMENUS == SLEEP) {
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500 && !gamePause && VENTURESTATE == INACTIVE && ACTION != INJURED && !randomEventPending) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        if (ACTION == SLEEPING) {
          animFrames = 0;
          ACTION = WAKE;
          AMENUS = NA;
          tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
        } else if (ACTION != SLEEPING) {
          ACTION = SLEEPING;
          sleepStartTime = millis();
          AMENUS = NA;
          xPosition = 99;
          display.fillRect(0, 0, 240, 54, TFT_COLOR4);
          display.fillRect(0, 54, 240, 3, TFT_COLOR1);
          tft.fillRect(0, 108, 240, 123, TFT_COLOR3);
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
  }
#pragma endregion

#pragma region Options
  //(A MENU): Options
  if (MENUSTATE == MAINMENU && AMENUS == OPTIONS) {
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        MENUSTATE = OPTIONSMENU;
        OPTIONSSETTINGS = PAUSE;
        tft.fillScreen(TFT_COLOR4);
        drawingBatteryIcon(batteryAnim);
        //DisplayBatteryPercentage();
        tft.fillRect(3, 3, (43 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 0, (43 * 3), (9 * 3), TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString("OPTIONS", 3, 3);

        //Highlight
        //tft.fillRect(0, 51, 240, 36, TFT_COLOR2);
        //Options Icon
        tft.fillRect(9, 57, 54, 54, TFT_COLOR1);
        tft.fillRect(6, 54, 54, 54, TFT_COLOR3);
        //Options Setting Name
        tft.fillRect(72, 57, 165, 27, TFT_COLOR1);
        tft.fillRect(69, 54, 165, 27, TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        if (gamePause) {
          tft.drawString(String("RESUME"), 72, 57);
          pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[8], TFT_BLACK);
        } else {
          tft.drawString("PAUSE", 72, 57);
          pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[0], TFT_BLACK);
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
  }
  //Options Menu (Control)
  if (MENUSTATE == OPTIONSMENU) {
    //A BUTTON
    if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        tft.fillRect(6, 54, 54, 54, TFT_COLOR3);
        tft.fillRect(69, 54, 165, 27, TFT_COLOR3);
        switch (OPTIONSSETTINGS) {
          case PAUSE:
            OPTIONSSETTINGS = BACKLIGHT;
            tft.drawString(String("BACKLIGHT"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[1], TFT_BLACK);
            break;
          case PAUSECANCEL:
            OPTIONSSETTINGS = PAUSECONFIRM;
            if (gamePause) {
              tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
              tft.drawString(String("RESUME   "), 87, 90);
              tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
              tft.drawString(String("RESUME"), 72, 57);
              pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[8], TFT_BLACK);
            } else if (gamePause == false) {
              tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
              tft.drawString(String("PAUSE   "), 87, 90);
              tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
              tft.drawString(String("PAUSE"), 72, 57);
              pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[0], TFT_BLACK);
            }
            break;
          case PAUSECONFIRM:
            OPTIONSSETTINGS = PAUSECANCEL;
            if (gamePause) {
              tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
              tft.drawString(String("CANCEL   "), 87, 90);
              tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
              tft.drawString(String("RESUME"), 72, 57);
              pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[8], TFT_BLACK);
            } else if (gamePause == false) {
              tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
              tft.drawString(String("CANCEL   "), 87, 90);
              tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
              tft.drawString(String("PAUSE"), 72, 57);
              pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[0], TFT_BLACK);
            }
            break;
          case BACKLIGHT:
            OPTIONSSETTINGS = LED;
            tft.drawString(String("LED"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[2], TFT_BLACK);
            break;
          case BLLOW:
            backlightPercent += 10;
            if (backlightPercent > 100) backlightPercent = 0;
            ApplyBacklight();
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            tft.fillRect(69, 90, 3, 21, TFT_COLOR1);
            tft.fillRect(72, 93, 3, 15, TFT_COLOR1);
            tft.fillRect(75, 96, 3, 9, TFT_COLOR1);
            tft.fillRect(78, 99, 3, 3, TFT_COLOR1);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String(backlightPercent) + "%", 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("BACKLIGHT"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[1], TFT_BLACK);
            break;
          case BLMED:
            OPTIONSSETTINGS = BLHIGH;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String("HIGH        "), 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("BACKLIGHT"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[1], TFT_BLACK);
            screenBrightness = 220;
            ledcWrite(0, screenBrightness);
            break;
          case BLHIGH:
            OPTIONSSETTINGS = BLLOW;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String("LOW       "), 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("BACKLIGHT"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[1], TFT_BLACK);
            screenBrightness = 10;
            ledcWrite(0, screenBrightness);
            break;
          case LED:
            OPTIONSSETTINGS = TIMEOUT;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("SCRN TIME"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[14], TFT_BLACK);
            break;
          case LEDLOW:
            ledPercent += 10;
            if (ledPercent > 100) ledPercent = 0;
            ApplyLed();
            testActive = true;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            tft.fillRect(69, 90, 3, 21, TFT_COLOR1);
            tft.fillRect(72, 93, 3, 15, TFT_COLOR1);
            tft.fillRect(75, 96, 3, 9, TFT_COLOR1);
            tft.fillRect(78, 99, 3, 3, TFT_COLOR1);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String(ledPercent) + "%", 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("LED"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[2], TFT_BLACK);
            break;
          case LEDMED:
            OPTIONSSETTINGS = LEDHIGH;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String("HIGH       "), 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("LED"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[2], TFT_BLACK);
            if (ledBrightness != 255) {
              ledBrightness = 255;
              strip.setBrightness(ledBrightness);
            }
            testActive = true;
            break;
          case LEDHIGH:
            OPTIONSSETTINGS = LEDOFF;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String("OFF       "), 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("LED"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[2], TFT_BLACK);
            if (ledBrightness != 0) {
              ledBrightness = 0;
              strip.setBrightness(ledBrightness);
              strip.setPixelColor(0, 0, 0, 0);
              strip.show();
            }
            testActive = true;
            break;
          case LEDOFF:
            OPTIONSSETTINGS = LEDLOW;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String("LOW       "), 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("LED"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[2], TFT_BLACK);
            if (ledBrightness != 10) {
              ledBrightness = 10;
              strip.setBrightness(ledBrightness);
            }
            testActive = true;
            break;
          case TIMEOUT:
            OPTIONSSETTINGS = LEDTIME;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("LED TIME"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[15], TFT_BLACK);
            break;
          case TIMEOUTSET:
            screenTimeout += 15;
            if (screenTimeout > 60) screenTimeout = 15;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            tft.fillRect(69, 90, 3, 21, TFT_COLOR1);
            tft.fillRect(72, 93, 3, 15, TFT_COLOR1);
            tft.fillRect(75, 96, 3, 9, TFT_COLOR1);
            tft.fillRect(78, 99, 3, 3, TFT_COLOR1);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String(screenTimeout) + "S", 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("SCRN TIME"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[14], TFT_BLACK);
            break;
          case LEDTIME:
            OPTIONSSETTINGS = BACKUP;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("BACKUP"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[10], TFT_BLACK);
            break;
          case LEDTIMESET:
            if (ledTimeout == 0) ledTimeout = 1;
            else if (ledTimeout == 1) ledTimeout = 5;
            else if (ledTimeout == 5) ledTimeout = 10;
            else if (ledTimeout == 10) ledTimeout = 15;
            else ledTimeout = 0;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            tft.fillRect(69, 90, 3, 21, TFT_COLOR1);
            tft.fillRect(72, 93, 3, 15, TFT_COLOR1);
            tft.fillRect(75, 96, 3, 9, TFT_COLOR1);
            tft.fillRect(78, 99, 3, 3, TFT_COLOR1);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(ledTimeout == 0 ? String("ALWAYS") : String(ledTimeout) + "MIN", 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("LED TIME"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[15], TFT_BLACK);
            break;
          case BACKUP:
            OPTIONSSETTINGS = RESTORE;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("RESTORE"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[11], TFT_BLACK);
            break;
          case BACKUPCANCEL:
            OPTIONSSETTINGS = BACKUPCONFIRM;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("BACKUP"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[10], TFT_BLACK);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String("CONFIRM       "), 87, 90);
            break;
          case BACKUPCONFIRM:
            OPTIONSSETTINGS = BACKUPCANCEL;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("BACKUP"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[10], TFT_BLACK);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String("CANCEL       "), 87, 90);
            tft.fillRect(69, 90, 3, 21, TFT_COLOR1);
            tft.fillRect(72, 93, 3, 15, TFT_COLOR1);
            tft.fillRect(75, 96, 3, 9, TFT_COLOR1);
            tft.fillRect(78, 99, 3, 3, TFT_COLOR1);
            break;
          case BACKUPWAIT:
            tft.fillRect(9, 57, 54, 54, TFT_COLOR1);
            tft.fillRect(6, 54, 54, 54, TFT_COLOR2);
            tft.fillRect(72, 57, 165, 27, TFT_COLOR1);
            tft.fillRect(69, 54, 165, 27, TFT_COLOR2);
            tft.setTextColor(TFT_COLOR3, TFT_COLOR2);
            tft.drawString(String("WAITING"), 72, 57);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[13], TFT_BLACK);
            break;
          case RESTORE:
            OPTIONSSETTINGS = POWEROFF;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("POWER OFF"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[5], TFT_BLACK);
            break;
          case RESTORECANCEL:
            OPTIONSSETTINGS = RESTORECONFIRM;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("RESTORE"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[11], TFT_BLACK);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String("CONFIRM       "), 87, 90);
            break;
          case RESTORECONFIRM:
            OPTIONSSETTINGS = RESTORECANCEL;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("RESTORE"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[11], TFT_BLACK);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String("CANCEL       "), 87, 90);
            tft.fillRect(69, 90, 3, 21, TFT_COLOR1);
            tft.fillRect(72, 93, 3, 15, TFT_COLOR1);
            tft.fillRect(75, 96, 3, 9, TFT_COLOR1);
            tft.fillRect(78, 99, 3, 3, TFT_COLOR1);
            break;
          case RESTOREWAIT:
            tft.fillRect(9, 57, 54, 54, TFT_COLOR1);
            tft.fillRect(6, 54, 54, 54, TFT_COLOR2);
            tft.fillRect(72, 57, 165, 27, TFT_COLOR1);
            tft.fillRect(69, 54, 165, 27, TFT_COLOR2);
            tft.setTextColor(TFT_COLOR3, TFT_COLOR2);
            tft.drawString(String("WAITING"), 72, 57);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[12], TFT_BLACK);
            break;
          case POWEROFF:
            OPTIONSSETTINGS = FACTORYRESET;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("RESET ALL"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[6], TFT_BLACK);
            break;
          case POWEROFFCANCEL:
            OPTIONSSETTINGS = POWEROFFCONFIRM;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("POWER OFF"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[5], TFT_BLACK);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String("CONFIRM       "), 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            break;
          case POWEROFFCONFIRM:
            OPTIONSSETTINGS = POWEROFFCANCEL;
            tft.drawString(String("POWER OFF"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[5], TFT_BLACK);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String("CANCEL       "), 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);

            break;
          case FACTORYRESET:
            OPTIONSSETTINGS = ABOUT;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("ABOUT"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[7], TFT_BLACK);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(buildName, 75, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            break;
          case FRCANCEL:
            OPTIONSSETTINGS = FRCONFIRM;
            tft.fillRect(9, 57, 54, 54, TFT_COLOR1);
            tft.fillRect(6, 54, 54, 54, TFT_COLOR2);
            //Options Setting Name
            tft.fillRect(72, 57, 165, 27, TFT_COLOR1);
            tft.fillRect(69, 54, 165, 27, TFT_COLOR2);
            tft.setTextColor(TFT_COLOR3, TFT_COLOR2);
            tft.drawString(String("DELETE?"), 72, 57);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[9], TFT_BLACK);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String("CONFIRM       "), 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            break;
          case FRCONFIRM:
            OPTIONSSETTINGS = FRCANCEL;
            tft.fillRect(9, 57, 54, 54, TFT_COLOR1);
            tft.fillRect(6, 54, 54, 54, TFT_COLOR2);
            //Options Setting Name
            tft.fillRect(72, 57, 165, 27, TFT_COLOR1);
            tft.fillRect(69, 54, 165, 27, TFT_COLOR2);
            tft.setTextColor(TFT_COLOR3, TFT_COLOR2);
            tft.drawString(String("DELETE?"), 72, 57);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[9], TFT_BLACK);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String("CANCEL       "), 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            break;
          case ABOUT:
            OPTIONSSETTINGS = PAUSE;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            if (gamePause) {
              tft.drawString(String("RESUME"), 72, 57);
              pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[8], TFT_BLACK);
            } else {
              tft.drawString(String("PAUSE"), 72, 57);
              pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[0], TFT_BLACK);
            }
            break;
        }
        mainMenuReturn = 0;
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      if (screenOff == false) {
        mainMenuReturn = 0;
        switch (OPTIONSSETTINGS) {
          case PAUSE:
            OPTIONSSETTINGS = PAUSECANCEL;
            tft.fillRect(69, 90, 3, 21, TFT_COLOR1);
            tft.fillRect(72, 93, 3, 15, TFT_COLOR1);
            tft.fillRect(75, 96, 3, 9, TFT_COLOR1);
            tft.fillRect(78, 99, 3, 3, TFT_COLOR1);
            if (gamePause) {
              tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
              tft.drawString(String("CANCEL        "), 87, 90);
              tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            } else if (gamePause == false) {
              tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
              tft.drawString(String("CANCEL        "), 87, 90);
              tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            }
            break;
          case PAUSECANCEL:
            OPTIONSSETTINGS = PAUSE;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            break;
          case PAUSECONFIRM:
            OPTIONSSETTINGS = PAUSE;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            if (gamePause) {
              tft.fillRect(9, 57, 54, 54, TFT_COLOR1);
              tft.fillRect(6, 54, 54, 54, TFT_COLOR3);
              //Options Setting Name
              tft.fillRect(72, 57, 165, 27, TFT_COLOR1);
              tft.fillRect(69, 54, 165, 27, TFT_COLOR3);
              tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
              tft.drawString(String("PAUSE"), 72, 57);
              gamePause = false;
              pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[0], TFT_BLACK);
              tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            } else if (gamePause == false) {
              tft.fillRect(9, 57, 54, 54, TFT_COLOR1);
              tft.fillRect(6, 54, 54, 54, TFT_COLOR3);
              //Options Setting Name
              tft.fillRect(72, 57, 165, 27, TFT_COLOR1);
              tft.fillRect(69, 54, 165, 27, TFT_COLOR3);
              tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
              gamePause = true;
              tft.drawString(String("RESUME"), 72, 57);
              pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[8], TFT_BLACK);
              tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            }
            break;
          case BACKLIGHT:
            OPTIONSSETTINGS = BLLOW;
            ApplyBacklight();
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            tft.fillRect(69, 90, 3, 21, TFT_COLOR1);
            tft.fillRect(72, 93, 3, 15, TFT_COLOR1);
            tft.fillRect(75, 96, 3, 9, TFT_COLOR1);
            tft.fillRect(78, 99, 3, 3, TFT_COLOR1);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String(backlightPercent) + "%", 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            break;
          case BLLOW:
            OPTIONSSETTINGS = BACKLIGHT;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            EEPROM.write(220, backlightPercent);
            EEPROM.commit();
            break;
          case BLMED:
            OPTIONSSETTINGS = BACKLIGHT;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            break;
          case BLHIGH:
            OPTIONSSETTINGS = BACKLIGHT;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            break;
          case LED:
            OPTIONSSETTINGS = LEDLOW;
            ApplyLed();
            testActive = true;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            tft.fillRect(69, 90, 3, 21, TFT_COLOR1);
            tft.fillRect(72, 93, 3, 15, TFT_COLOR1);
            tft.fillRect(75, 96, 3, 9, TFT_COLOR1);
            tft.fillRect(78, 99, 3, 3, TFT_COLOR1);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String(ledPercent) + "%", 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            break;
          case LEDLOW:
            OPTIONSSETTINGS = LED;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            testActive = false;
            EEPROM.write(221, ledPercent);
            EEPROM.commit();
            break;
          case LEDMED:
            OPTIONSSETTINGS = LED;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            testActive = false;
            break;
          case LEDHIGH:
            OPTIONSSETTINGS = LED;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            testActive = false;
            break;
          case LEDOFF:
            OPTIONSSETTINGS = LED;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            testActive = false;
            break;
          case TIMEOUT:
            OPTIONSSETTINGS = TIMEOUTSET;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            tft.fillRect(69, 90, 3, 21, TFT_COLOR1);
            tft.fillRect(72, 93, 3, 15, TFT_COLOR1);
            tft.fillRect(75, 96, 3, 9, TFT_COLOR1);
            tft.fillRect(78, 99, 3, 3, TFT_COLOR1);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String(screenTimeout) + "S", 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            break;
          case TIMEOUTSET:
            OPTIONSSETTINGS = TIMEOUT;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            EEPROM.write(222, screenTimeout);
            EEPROM.commit();
            break;
          case LEDTIME:
            OPTIONSSETTINGS = LEDTIMESET;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            tft.fillRect(69, 90, 3, 21, TFT_COLOR1);
            tft.fillRect(72, 93, 3, 15, TFT_COLOR1);
            tft.fillRect(75, 96, 3, 9, TFT_COLOR1);
            tft.fillRect(78, 99, 3, 3, TFT_COLOR1);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(ledTimeout == 0 ? String("ALWAYS") : String(ledTimeout) + "MIN", 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            break;
          case LEDTIMESET:
            OPTIONSSETTINGS = LEDTIME;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            EEPROM.write(223, ledTimeout);
            EEPROM.commit();
            break;
          case BACKUP:
            OPTIONSSETTINGS = BACKUPCANCEL;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            tft.fillRect(9, 57, 54, 54, TFT_COLOR1);
            tft.fillRect(6, 54, 54, 54, TFT_COLOR3);
            //Options Setting Name
            tft.fillRect(72, 57, 165, 27, TFT_COLOR1);
            tft.fillRect(69, 54, 165, 27, TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("BACKUP"), 72, 57);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String("CANCEL       "), 87, 90);
            tft.fillRect(69, 90, 3, 21, TFT_COLOR1);
            tft.fillRect(72, 93, 3, 15, TFT_COLOR1);
            tft.fillRect(75, 96, 3, 9, TFT_COLOR1);
            tft.fillRect(78, 99, 3, 3, TFT_COLOR1);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[10], TFT_BLACK);
            break;
          case BACKUPCANCEL:
            OPTIONSSETTINGS = BACKUP;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            break;
          case BACKUPCONFIRM:
            CopyEEPROMToFlash();
            BButton = "released";
            tft.fillRect(9, 57, 54, 54, TFT_COLOR1);
            tft.fillRect(6, 54, 54, 54, TFT_COLOR2);
            tft.fillRect(72, 57, 165, 27, TFT_COLOR1);
            tft.fillRect(69, 54, 165, 27, TFT_COLOR2);
            tft.setTextColor(TFT_COLOR3, TFT_COLOR2);
            tft.drawString(String("READY!"), 72, 57);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[13], TFT_BLACK);
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            tft.drawString(String("USE PNUT"), 69, 90);
            OPTIONSSETTINGS = BACKUP;
            break;
          case BACKUPWAIT:
            ESP.restart();
            break;
          case RESTORE:
            OPTIONSSETTINGS = RESTORECANCEL;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            tft.fillRect(9, 57, 54, 54, TFT_COLOR1);
            tft.fillRect(6, 54, 54, 54, TFT_COLOR3);
            //Options Setting Name
            tft.fillRect(72, 57, 165, 27, TFT_COLOR1);
            tft.fillRect(69, 54, 165, 27, TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("RESTORE"), 72, 57);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String("CANCEL       "), 87, 90);
            tft.fillRect(69, 90, 3, 21, TFT_COLOR1);
            tft.fillRect(72, 93, 3, 15, TFT_COLOR1);
            tft.fillRect(75, 96, 3, 9, TFT_COLOR1);
            tft.fillRect(78, 99, 3, 3, TFT_COLOR1);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[11], TFT_BLACK);
            break;
          case RESTORECANCEL:
            OPTIONSSETTINGS = RESTORE;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            break;
          case RESTORECONFIRM:
            BButton = "released";
            tft.fillRect(9, 57, 54, 54, TFT_COLOR1);
            tft.fillRect(6, 54, 54, 54, TFT_COLOR2);
            tft.fillRect(72, 57, 165, 27, TFT_COLOR1);
            tft.fillRect(69, 54, 165, 27, TFT_COLOR2);
            tft.setTextColor(TFT_COLOR3, TFT_COLOR2);
            tft.drawString(String("READY!"), 72, 57);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[12], TFT_BLACK);
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            tft.drawString(String("USE PNUT"), 69, 90);
            OPTIONSSETTINGS = RESTORE;
            break;
          case RESTOREWAIT:
            ESP.restart();
            break;
          case POWEROFF:
            OPTIONSSETTINGS = POWEROFFCANCEL;
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            tft.drawString(String("CANCEL       "), 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.fillRect(69, 90, 3, 21, TFT_COLOR1);
            tft.fillRect(72, 93, 3, 15, TFT_COLOR1);
            tft.fillRect(75, 96, 3, 9, TFT_COLOR1);
            tft.fillRect(78, 99, 3, 3, TFT_COLOR1);
            break;
          case POWEROFFCANCEL:
            OPTIONSSETTINGS = POWEROFF;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            break;
          case POWEROFFCONFIRM:
            //TO DO: Research press and hold to power back on or tap both buttons
            strip.setPixelColor(0, 0, 0, 0);
            strip.show();
            // Wait for buttons to be released before sleeping
            while (digitalRead(35) == LOW || digitalRead(0) == LOW) {
              delay(10);
            }
            delay(100);
            esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
            esp_deep_sleep_start();
            break;
          case FACTORYRESET:
            OPTIONSSETTINGS = FRCANCEL;
            tft.fillRect(9, 57, 54, 54, TFT_COLOR1);
            tft.fillRect(6, 54, 54, 54, TFT_COLOR2);
            //Options Setting Name
            tft.fillRect(72, 57, 165, 27, TFT_COLOR1);
            tft.fillRect(69, 54, 165, 27, TFT_COLOR2);
            tft.setTextColor(TFT_COLOR3, TFT_COLOR2);
            tft.drawString(String("DELETE?"), 72, 57);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[9], TFT_BLACK);
            tft.drawString(String("CANCEL       "), 87, 90);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.fillRect(69, 90, 3, 21, TFT_COLOR1);
            tft.fillRect(72, 93, 3, 15, TFT_COLOR1);
            tft.fillRect(75, 96, 3, 9, TFT_COLOR1);
            tft.fillRect(78, 99, 3, 3, TFT_COLOR1);
            break;
          case FRCANCEL:
            OPTIONSSETTINGS = FACTORYRESET;
            tft.fillRect(69, 90, 171, 45, TFT_COLOR4);
            tft.fillRect(9, 57, 54, 54, TFT_COLOR1);
            tft.fillRect(6, 54, 54, 54, TFT_COLOR3);
            //Options Setting Name
            tft.fillRect(72, 57, 165, 27, TFT_COLOR1);
            tft.fillRect(69, 54, 165, 27, TFT_COLOR3);
            tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
            tft.drawString(String("RESET ALL"), 72, 57);
            pushScaled(9, 57, 16, 16, (uint16_t*)OptionsArray[6], TFT_BLACK);
            break;
          case FRCONFIRM:
            //TO DO:Add a wipe command that deletes EEPROM and goes to startup screen
            FormatData();
            delay(1000);
            ESP.restart();
            break;
          case ABOUT:
            //Easter egg?
            break;
        }
      }
      if (screenOff)
        screenOff = false;
    }
  }
#pragma endregion

#pragma region Status
  //B Button: Main Menu >> Status 1
  if (MENUSTATE == MAINMENU && AMENUS == NA && ACTION != EATING && ACTION != WAKE && ACTION != REJECT && eventState == EVENT_IDLE) {
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {  //STATS1
      lastButtonPressTime = millis();
      if (screenOff == false) {
        MENUSTATE = STATS1;
        DrawStatusMeters();
      }
      if (screenOff)
        screenOff = false;
    }
  }
  //A Button: Status 1 (meters) >> Status 2 (stats)
  if (MENUSTATE == STATS1 && AMENUS == NA) {
    if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {  //STATS2
      lastButtonPressTime = millis();
      if (screenOff == false) {
        MENUSTATE = STATS2;
        tft.fillScreen(TFT_COLOR4);
        drawingBatteryIcon(batteryAnim);
        //DisplayBatteryPercentage();
        tft.fillRect(3, 3, (49 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 0, (49 * 3), (9 * 3), TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        String displayHP = String(HP);
        String displayMaxHP = String(maxHP);
        if (HP < 10)
          displayHP = "0" + String(HP);
        if (maxHP < 10)
          displayMaxHP = "0" + String(maxHP);
        tft.drawString(String("HP " + displayHP + "/" + displayMaxHP), 3, 3);

        //ATK
        tft.fillRect(3, 36, (43 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 33, (43 * 3), (9 * 3), TFT_COLOR3);
        tft.drawString("ATK " + String(Attack), 3, 36);

        //DEF
        tft.fillRect(3, 69, (43 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 66, (43 * 3), (9 * 3), TFT_COLOR3);
        tft.drawString("DEF " + String(Defense), 3, 69);

        //SPD
        tft.fillRect(3, 102, (43 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 99, (43 * 3), (9 * 3), TFT_COLOR3);
        tft.drawString("SPD " + String(Speed), 3, 102);

        //BEST DD FLOOR SCORE
        if(deepDungeonUnlocked){
          tft.fillRect(141, 36, (31 * 3), (9 * 3), TFT_COLOR1);
          tft.fillRect(138, 33, (31 * 3), (9 * 3), TFT_COLOR3);
          tft.drawString("*" + String(deepDungeonHighScore), 141, 36);
        }

        //FLOOR NUMBER
        tft.fillRect(141, 69, (31 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(138, 66, (31 * 3), (9 * 3), TFT_COLOR3);
        if (deepDungeonActive) {
          tft.drawString("D" + String(deepDungeonStage), 141, 69);
        } else {
          if(stage < 30)
            tft.drawString("F" + String(stage), 141, 69);
          else
            tft.drawString("F" + String(stage), 141, 69);
        }

        //CREDITS
        tft.fillRect(141, 102, (31 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(138, 99, (31 * 3), (9 * 3), TFT_COLOR3);
        pushScaled(141, 105, 5, 5, smallIcons[11], TFT_BLACK);
        tft.drawString(String(playerCredits), 159, 102);
        mainMenuReturn = 0;
      }
    }
    if (screenOff)
      screenOff = false;
  }
  //A Button: Status 2 (stats) >> merits (in a run) or back to meters
  if (MENUSTATE == STATS2 && AMENUS == NA) {
    if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      mainMenuReturn = 0;
      if (screenOff == false) {
        if (deepDungeonActive) {
          MENUSTATE = STATS3;
          DrawMeritScreen();
        } else {
          MENUSTATE = STATS1;
          DrawStatusMeters();
        }
      }
      if (screenOff)
        screenOff = false;
    }
  }
  //Merits (STATS3) and Afflictions (STATS4) panels + drill-down list
  if ((MENUSTATE == STATS3 || MENUSTATE == STATS4) && AMENUS == NA) {
    //A BUTTON - cycle the list when drilled, else advance to the next panel
    if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
      lastButtonPressTime = millis();
      mainMenuReturn = 0;
      if (screenOff == false) {
        if (statDrill) {
          int count = (MENUSTATE == STATS3) ? (activeMeritCount + 1) : activeAfflictionCount;
          int prev = statDrillSel;
          if (count > 0) statDrillSel = (statDrillSel + 1) % count;
          if (statDrillSel != prev) {
            if (MENUSTATE == STATS3) {
              // page 1 = gem + merit 0 (sel 0,1); list pages = sel 2+ in 3s.
              // Only flip the page on a real page change; otherwise just move
              // the highlight in place so there's no flicker.
              int pidPrev = (prev <= 1) ? 0 : 1 + (prev - 2) / 3;
              int pidNew = (statDrillSel <= 1) ? 0 : 1 + (statDrillSel - 2) / 3;
              if (pidPrev != pidNew) DrawMeritPage();
              else DrawMeritRows();
            } else {
              // afflictions: full redraw only when the 3-row window changes
              if (prev / 3 != statDrillSel / 3) DrawAfflictionPage();
              else DrawAfflictionRows();
            }
          }
        } else if (MENUSTATE == STATS3) {
          MENUSTATE = STATS4;
          DrawAfflictionScreen();
        } else {
          MENUSTATE = STATS1;
          DrawStatusMeters();
        }
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON - drill into the list
    if (BButton == "pressed" && millis() - lastButtonPressTime > 500 && !statDrill && TOOLTIP == RELEASED) {
      lastButtonPressTime = millis();
      mainMenuReturn = 0;
      if (screenOff == false) {
        statDrill = true;
        statDrillSel = 0;
        if (MENUSTATE == STATS3) DrawMeritScreen();
        else DrawAfflictionScreen();
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON HELD - tooltip of the focused entry
    if (BButton == "held" && millis() - lastButtonPressTime > 500 && statDrill) {
      lastButtonPressTime = millis();
      mainMenuReturn = 0;
      if (screenOff == false) {
        if (TOOLTIP == RELEASED) {
          TOOLTIP = ACTIVE;
          if (MENUSTATE == STATS3) {
            if (statDrillSel == 0) {
              if (activeGem >= 0) DrawGemTooltipById(activeGem);
            } else if (activeMeritCount > 0) {
              DrawMeritTooltipById(activeMerits[statDrillSel - 1]);
            }
          } else {
            if (activeAfflictionCount > 0)
              DrawAfflictionTooltipById(activeAfflictions[statDrillSel], activeAfflictionLevels[statDrillSel]);
          }
        }
      }
      if (screenOff)
        screenOff = false;
    }
    //B BUTTON RELEASED - close tooltip
    if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
      lastButtonPressTime = millis();
      mainMenuReturn = 0;
      if (screenOff == false) {
        TOOLTIP = RELEASED;
        if (MENUSTATE == STATS3) DrawMeritScreen();
        else DrawAfflictionScreen();
      }
      if (screenOff)
        screenOff = false;
    }
    //A BUTTON HELD - back out of the drill to the panel
    if (AButton == "held" && millis() - lastButtonPressTime > 500 && statDrill) {
      lastButtonPressTime = millis();
      mainMenuReturn = 0;
      if (screenOff == false) {
        statDrill = false;
        holdConsumed = true;
        TOOLTIP = RELEASED;
        if (MENUSTATE == STATS3) DrawMeritScreen();
        else DrawAfflictionScreen();
      }
      if (screenOff)
        screenOff = false;
    }
  }
#pragma endregion
}

void ScreenOff() {
  static unsigned long lastTime = 0;
  if (millis() - lastTime >= 1000 && screenOff < 30) {
    lastTime = millis();
    screenOffTime++;
  }
  //TURN SCREEN OFF
  if (screenOffTime >= screenTimeout && screenOff == false) {  //idle timeout (seconds)
    if (MENUSTATE == MAINMENU && eventState != EVENT_PROMPT && eventState != EVENT_RESULT && OPTIONSSETTINGS != BACKUPWAIT && OPTIONSSETTINGS != RESTOREWAIT) {
      digitalWrite(4, LOW);
      tft.writecommand(ST7789_DISPOFF);
      tft.writecommand(ST7789_SLPIN);
      ledcWrite(0, 0);
      digitalWrite(27, LOW);
      screenOff = true;
    }
  }
  //Change CPU frequency when screen is off if LED activates

  if (screenOff && !ledActive && cpu != 10) {
    strip.setPin(27);
    strip.setPin(-1);
    strip.begin();
    setCpuFrequencyMhz(10);
    cpu = 10;
  } else if (screenOff && ledActive && cpu != 80) {
    strip.setPin(27);
    strip.begin();
    setCpuFrequencyMhz(80);
    cpu = 80;
  }

  //SCREEN ON WITH BUTTON PRESS
  if (AButton == "pressed" || BButton == "pressed") {
    if (screenOff) {
      if (cpu < 80) {              // boost FIRST so the wake redraw runs at full speed, not 10MHz
        strip.setPin(27);
        strip.begin();
        setCpuFrequencyMhz(80);
        cpu = 80;
      }
      digitalWrite(4, HIGH);
      tft.writecommand(ST7789_DISPON);
      tft.writecommand(ST7789_SLPOUT);
      ledcWrite(0, screenBrightness);
      digitalWrite(27, 70);
      if (ddGemSelect) DrawDDGemSelect();
      else if (ddMeritSelect) DrawDDMeritSelect();
      else if (ddCardDraft) DrawDDCardDraft();
      else if (bossAfflictionPopup) DrawBossAfflictionPopup();
      else DrawScene();
      screenOff = false;
    }
    screenOffTime = 0;            // always reset the idle timer on a press
  }
  //SCREEN ON WHEN PLUGGED IN
  static int plugInReadings = 0;

  if (BL.getBatteryVolts() >= 4.5 && isCharging == false) {
    plugInReadings++;
    if (plugInReadings >= 10) {  // Require 10 consecutive readings
      digitalWrite(4, HIGH);
      tft.writecommand(ST7789_DISPON);
      tft.writecommand(ST7789_SLPOUT);
      ledcWrite(0, 70);
      digitalWrite(27, 70);
      screenOffTime = 0;
      screenOff = false;
      chargeNotification = true;
      strip.setPixelColor(0, 0, 0, 0);
      strip.show();
      DrawScene();
      isCharging = true;
      plugInReadings = 0;
      if (cpu < 80) {
        strip.setPin(27);
        strip.begin();
        setCpuFrequencyMhz(80);
        cpu = 80;
      }
    }
  } else {
    plugInReadings = 0;
  }
}

void CombineImages(const uint16_t* bottomImage, const uint16_t* middleImage, const uint16_t* topImage, uint16_t* combinedImage, bool bodyBehindHead) {
  // Combine 16x16 sprites and scale to 48x48
  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      uint16_t bottomColor = pgm_read_word(&bottomImage[y * 16 + x]);
      uint16_t middleColor = pgm_read_word(&middleImage[y * 16 + x]);
      uint16_t topColor = pgm_read_word(&topImage[y * 16 + x]);

      uint16_t finalColor;
      if (bodyBehindHead) {
        // Layer order: base -> body -> head
        if (middleColor != TFT_BLACK) finalColor = middleColor;
        else if (topColor != TFT_BLACK) finalColor = topColor;
        else finalColor = bottomColor;
      } else {
        // Layer order: base -> head -> body (default)
        if (topColor != TFT_BLACK) finalColor = topColor;
        else if (middleColor != TFT_BLACK) finalColor = middleColor;
        else finalColor = bottomColor;
      }
      finalColor = RemapColor(finalColor);
      // Scale 3x
      for (int dy = 0; dy < 3; dy++) {
        for (int dx = 0; dx < 3; dx++) {
          combinedImage[(y * 3 + dy) * 48 + (x * 3 + dx)] = finalColor;
        }
      }
    }
  }

  for (int i = 0; i < 48 * 48; i++) {
    tempImage[i] = combinedImage[i];
  }
  for (int y = 0; y < 48; y++) {
    for (int x = 0; x < 48; x++) {
      int i = y * 48 + x;
      // Add outline
      if (tempImage[i] != TFT_BLACK) {
        // Draw to the right
        for (int k = 1; k <= 3 && x + k < 48; k++) {
          if (tempImage[i + k] == TFT_BLACK)
            combinedImage[i + k] = TFT_COLOR1;
        }

        // Draw below
        for (int k = 1; k <= 3 && y + k < 48; k++) {
          if (tempImage[i + k * 48] == TFT_BLACK)
            combinedImage[i + k * 48] = TFT_COLOR1;
        }

        // Draw to the left if the left pixel is black
        for (int k = 1; k <= 3 && x - k >= 0; k++) {
          if (tempImage[i - k] == TFT_BLACK)
            combinedImage[i - k] = TFT_COLOR1;
        }

        // Draw above if the top pixel is black
        for (int k = 1; k <= 3 && y - k >= 0; k++) {
          if (tempImage[i - k * 48] == TFT_BLACK)
            combinedImage[i - k * 48] = TFT_COLOR1;
        }
      }
    }
  }
  // Fill the blank areas with TFT_COLOR4
  for (int i = 0; i < 48 * 48; i++) {
    if (combinedImage[i] == TFT_BLACK) {
      if (!(i >= 48 * (48 - 3))) {
        combinedImage[i] = TFT_COLOR4;
      }
    }
  }

  // Set the bottom three rows of pixels to TFT_COLOR1
  for (int i = 48 * (48 - 3); i < 48 * 48; i++) {
    if (combinedImage[i] == TFT_BLACK) {
      combinedImage[i] = TFT_COLOR1;
    }
  }

  // Mirror the image and store it for future use
  MirrorHorizontally(combinedImage, combinedMirrorImage);
}

void CreateCharacter(String _pet, const uint16_t* headSprites[], const uint16_t* bodySprites[]) {
  // Single buffer outside loop to avoid stack overflow
  uint16_t combinedImage[48 * 48];
  bool layerBehind = false;
  int totalBody = getTotalBodyGear();
  for (int b = 0; b < totalBody; b++) {
    if (bodyGear[b][0] == bodySprites[0]) {
      layerBehind = bodyPriority[b];
      break;
    }
  }
  for (int i = 0; i < 12; i++) {
    // Create new character data
    if (_pet == "player") {
      CombineImages(petBody[i], headSprites[i], bodySprites[i], combinedImage, layerBehind);
      if (pet[i] != nullptr) {
        delete[] pet[i];
        pet[i] = nullptr;
      }
      if (mirrorPet[i] != nullptr) {
        delete[] mirrorPet[i];
        mirrorPet[i] = nullptr;
      }
      pet[i] = new uint16_t[48 * 48];
      memcpy(pet[i], combinedImage, sizeof(uint16_t) * 48 * 48);

      // Only allocate mirrorPet if enemy sprites don't exist (memory constraint)
      // GetMirrorPet() has a fallback that generates on-the-fly if mirrorPet is null
      if (enemyPet[0] == nullptr) {
        mirrorPet[i] = new uint16_t[48 * 48];
        MirrorHorizontally(combinedImage, mirrorPet[i]);
      }

      // Display playerSprite using pet[i]
      playerSprite.pushImage(0, 0, 48, 48, pet[i]);
    } else if (_pet == "enemy") {
      // Free mirrorPet[i] to make room for enemyPet[i]
      if (mirrorPet[i] != nullptr) {
        delete[] mirrorPet[i];
        mirrorPet[i] = nullptr;
      }
      
      CombineImages(petBody[i], headSprites[i], bodySprites[i], combinedImage, layerBehind);
      if (enemyPet[i] != nullptr) {
        delete[] enemyPet[i];
        enemyPet[i] = nullptr;
      }
      if (mirrorEnemyPet[i] != nullptr) {
        delete[] mirrorEnemyPet[i];
        mirrorEnemyPet[i] = nullptr;
      }

      enemyPet[i] = new uint16_t[48 * 48];
      memcpy(enemyPet[i], combinedImage, sizeof(uint16_t) * 48 * 48);
    }
  }
}

void CreateWardrobeCharacter(int wardrobe, const uint16_t* headSprites[], const uint16_t* bodySprites[]) {
  bool layerBehind = false;
  int totalBody = getTotalBodyGear();
  for (int b = 0; b < totalBody; b++) {
    if (bodyGear[b][0] == bodySprites[0]) {
      layerBehind = bodyPriority[b];
      break;
    }
  }
  uint16_t* combinedImage = new uint16_t[48 * 48];
  CombineImages(petBody[0], headSprites[0], bodySprites[0], combinedImage, layerBehind);
  if (wardrobePet[wardrobe] != NULL) {
    delete[] wardrobePet[wardrobe];  // Deallocate memory if it was dynamically allocated
    wardrobePet[wardrobe] = NULL;    // Set the pointer to NULL to avoid dangling pointers
  }
  wardrobePet[wardrobe] = combinedImage;
}

void MirrorHorizontally(const uint16_t* combinedImage, uint16_t* mirroredImage) {
  for (int row = 0; row < 48; row++) {
    for (int col = 0; col < 48; col++) {
      mirroredImage[row * 48 + col] = combinedImage[row * 48 + (47 - col)];
    }
  }
}

// Get mirrored player sprite - uses mirrorPet if available, otherwise generates on-the-fly
uint16_t* GetMirrorPet(int frame) {
  if (mirrorPet[frame] != nullptr) {
    return mirrorPet[frame];
  }
  // Generate on-the-fly into combinedMirrorImage buffer
  if (pet[frame] != nullptr) {
    MirrorHorizontally(pet[frame], combinedMirrorImage);
    return combinedMirrorImage;
  }
  return nullptr;
}

void WalkPet() {
  static unsigned long lastTime = 0;
  if (millis() - lastTime >= 1000) {  // update sprites once per second
    lastTime = millis();
    if (randomIdle > 0) {
      randomIdle--;
      if (moveLeft)
        DrawCharacter(xPosition, 60, false);
      else
        DrawCharacter(xPosition, 60, true);
      if (randomIdle == 0)
        ACTION = WALKING;
    } else if (ACTION == WALKING) {
      if (moveLeft) {
        xPosition -= 9;

        // Check if it has reached the left boundary
        if (xPosition < 27) {
          xPosition = 27;    // Prevent going past the boundary
          moveLeft = false;  // Change direction
        }
      } else {
        // Move the pet to the right
        xPosition += 9;

        // Check if it has reached the right boundary
        if (xPosition + 48 > 210) {
          xPosition = 210 - 48;  // Prevent going past the boundary
          moveLeft = true;       // Change direction
        }
      }

      // Occasionally change direction randomly
      if (random(100) < 2) {   // 2% chance to change direction
        moveLeft = !moveLeft;  // Invert the current direction
      }
      // Occasionally stand still
      if (random(100) < 10 && randomIdle == 0) {  // 10% chance to idle
        randomIdle = random(3, 11);               //Idles for 3-10 frames
        ACTION = IDLE;
      }
      display.fillRect(0, 0, 240, 54, TFT_COLOR4);
      display.fillRect(0, 54, 240, 3, TFT_COLOR1);
      if (moveLeft)
        DrawCharacter(xPosition, 60, false);
      else
        DrawCharacter(xPosition, 60, true);
    }
  }
}

void VentureWalk() {
  static unsigned long lastTime = 0;
  static int currentWallIndex = 0;

  if (millis() - lastTime >= 1000) {  // update sprites once per second
    lastTime = millis();

    if (countdownSeconds > 0) {
      display.fillRect(0, 0, 240, 54, TFT_COLOR4);
      display.fillRect(0, 54, 240, 3, TFT_COLOR1);
    }
    xPosition = 51;   // keep in sync so a random event starts from the pet's actual spot
    DrawCharacter(51, 60, true);
  }
}

void VentureEncounter() {
  static unsigned long lastTime = 0;
  
  // Safety check - ensure player sprites are allocated before proceeding
  if (pet[0] == nullptr) {
    return;  // Player sprites not ready, skip this frame
  }
  
  ACTION = WALKING;
  enemyAction = "walking";
  if (millis() - lastTime >= 1000 && xPosition > 0) {  // update sprites once per second
    if (xPosition == 51) {
      // In DD mode or stage 5+ always use random enemy (ventureEnemy has 5 entries)
      InitializeEnemy(stage, deepDungeonActive || stage >= 30);
      tft.fillRect(0, 27, 240, 27, TFT_COLOR4);  // Clear READY text and progress bar area
    }
    lastTime = millis();
    display.fillRect(0, 0, 240, 54, TFT_COLOR4);
    display.fillRect(0, 54, 240, 3, TFT_COLOR1);
    xPosition -= 9;
    if (xPosition < 0)
      xPosition = 0;
    DrawCharacter(xPosition, 60, true);
    DrawEnemy(xPosition + 192);
  }
  if (xPosition <= 0) {
    ACTION = ATTACKING;
    enemyAction = "attacking";
    DrawEnemy(xPosition + 192);
    // Use DD deck when in DD mode
    if (deepDungeonActive) {
      int ddEquippedDeck = EEPROM.read(321);
      CreateDDDeck(ddEquippedDeck);
    } else {
      CreateDeck(equippedDeck);
    }
    VENTURESTATE = INACTIVE;
    MENUSTATE = BATTLE;
    beginTurn = "";
  }
}

void TutorialManager() {
  switch (tutorialPage) {
    case 0:
      CenterText("WELCOME,", 42);
      CenterText("NAVIGATOR.", 69);
      //A Arrow
      tft.fillRect(234, 18, 3, 3, TFT_COLOR1);
      tft.fillRect(231, 15, 3, 9, TFT_COLOR1);
      tft.fillRect(228, 12, 3, 15, TFT_COLOR1);
      //A Button Icon
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      //tft.fillRect(204, 6, 21, 27, TFT_COLOR1);
      //tft.fillRect(201, 3, 21, 27, TFT_COLOR3);
      tft.drawString("A", 204, 6);
      //B Arrow
      tft.fillRect(234, 117, 3, 3, TFT_COLOR1);
      tft.fillRect(231, 114, 3, 9, TFT_COLOR1);
      tft.fillRect(228, 111, 3, 15, TFT_COLOR1);
      //B Button Icon
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      tft.fillRect(204, 105, 21, 27, TFT_COLOR1);
      tft.fillRect(201, 102, 21, 27, TFT_COLOR3);
      tft.drawString("B", 204, 105);

      tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
      tft.drawString("CONTINUE", 3, 105);
      tutorialPage++;
      break;
    case 1:
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        tutorialPage++;
        tft.fillScreen(TFT_COLOR4);
      }
      if (AButton == "held" && BButton == "held" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        // Wait for buttons to be released before sleeping
        while (digitalRead(35) == LOW || digitalRead(0) == LOW) {
          delay(10);
        }
        delay(100);  // Extra debounce delay
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
        esp_deep_sleep_start();
      }
      break;
    case 2:
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      CenterText("PRESS B TO", 42);
      CenterText("CONFIRM", 69);
      tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
      tft.drawString("B", 138, 42);
      //A Arrow
      tft.fillRect(234, 18, 3, 3, TFT_COLOR1);
      tft.fillRect(231, 15, 3, 9, TFT_COLOR1);
      tft.fillRect(228, 12, 3, 15, TFT_COLOR1);
      //A Button Icon
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      tft.drawString("A", 204, 6);
      //B Arrow
      tft.fillRect(234, 117, 3, 3, TFT_COLOR1);
      tft.fillRect(231, 114, 3, 9, TFT_COLOR1);
      tft.fillRect(228, 111, 3, 15, TFT_COLOR1);
      //B Button Icon
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      tft.fillRect(204, 105, 21, 27, TFT_COLOR1);
      tft.fillRect(201, 102, 21, 27, TFT_COLOR3);
      tft.drawString("B", 204, 105);

      tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
      tft.drawString("CONFIRM", 3, 105);
      tutorialPage++;
      break;
    case 3:
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        tutorialPage++;
        tft.fillScreen(TFT_COLOR4);
      }
      break;
    case 4:
      tft.fillRect(204, 6, 21, 27, TFT_COLOR4);
      tft.fillRect(201, 3, 21, 27, TFT_COLOR4);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      CenterText("PRESS A TO", 42);
      CenterText("CYCLE OPTIONS", 69);
      tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
      tft.drawString("A", 138, 42);
      //A Arrow
      tft.fillRect(234, 18, 3, 3, TFT_COLOR1);
      tft.fillRect(231, 15, 3, 9, TFT_COLOR1);
      tft.fillRect(228, 12, 3, 15, TFT_COLOR1);
      //A Button Icon
      tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
      tft.fillRect(204, 6, 21, 27, TFT_COLOR1);
      tft.fillRect(201, 3, 21, 27, TFT_COLOR3);
      tft.drawString("A", 204, 6);
      //B Arrow
      tft.fillRect(234, 117, 3, 3, TFT_COLOR1);
      tft.fillRect(231, 114, 3, 9, TFT_COLOR1);
      tft.fillRect(228, 111, 3, 15, TFT_COLOR1);
      //B Button Icon
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      //tft.fillRect(204, 105, 21, 27, TFT_COLOR1);
      //tft.fillRect(201, 102, 21, 27, TFT_COLOR3);
      tft.drawString("B", 204, 105);

      tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
      tft.drawString("PRESS A", 3, 6);
      tutorialPage++;
      break;
    case 5:
      if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
        tft.fillRect(0, 102, 201, 33, TFT_COLOR4);
        tft.drawString("ONCE MORE", 3, 6);
        tutorialPage++;
      }
      break;
    case 6:
      if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        tft.fillRect(204, 6, 21, 27, TFT_COLOR4);
        tft.fillRect(201, 3, 21, 27, TFT_COLOR4);
        tft.drawString("A", 204, 6);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.fillRect(204, 105, 21, 27, TFT_COLOR1);
        tft.fillRect(201, 102, 21, 27, TFT_COLOR3);
        tft.drawString("B", 204, 105);
        tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
        tft.fillRect(0, 3, 201, 33, TFT_COLOR4);
        tft.fillRect(0, 102, 201, 33, TFT_COLOR4);
        tft.drawString("GOOD JOB!", 3, 105);
        tutorialPage++;
      }
      break;
    case 7:
      if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        tft.fillRect(204, 6, 21, 27, TFT_COLOR4);
        tft.fillRect(201, 3, 21, 27, TFT_COLOR4);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.fillRect(204, 6, 21, 27, TFT_COLOR1);
        tft.fillRect(201, 3, 21, 27, TFT_COLOR3);
        tft.drawString("A", 204, 6);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        tft.fillRect(204, 105, 21, 27, TFT_COLOR4);
        tft.fillRect(201, 102, 21, 27, TFT_COLOR4);
        tft.drawString("B", 204, 105);
        tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
        tft.fillRect(0, 3, 201, 33, TFT_COLOR4);
        tft.fillRect(0, 102, 201, 33, TFT_COLOR4);
        tft.drawString("TOO FAR", 3, 6);
        tutorialPage = 8;
      }
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        tft.fillScreen(TFT_COLOR4);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        CenterText("HOLD B TO", 42);
        CenterText("SEE MORE", 69);
        //A Arrow
        tft.fillRect(234, 18, 3, 3, TFT_COLOR1);
        tft.fillRect(231, 15, 3, 9, TFT_COLOR1);
        tft.fillRect(228, 12, 3, 15, TFT_COLOR1);
        //A Button Icon
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        tft.drawString("A", 204, 6);
        //B Arrow
        tft.fillRect(234, 117, 3, 3, TFT_COLOR1);
        tft.fillRect(231, 114, 3, 9, TFT_COLOR1);
        tft.fillRect(228, 111, 3, 15, TFT_COLOR1);
        //B Button Icon
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.fillRect(204, 105, 21, 27, TFT_COLOR1);
        tft.fillRect(201, 102, 21, 27, TFT_COLOR3);
        tft.drawString("B", 204, 105);
        tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
        tft.fillRect(0, 102, 201, 33, TFT_COLOR4);
        tft.drawString("HOLD B", 3, 105);
        tft.drawString("B", 129, 42);
        tutorialPage = 9;
      }
      break;
    case 8:
      if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        tutorialPage = 4;
      }
      break;
    case 9:
      if (BButton == "held" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        tft.fillScreen(TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        CenterText("RELEASE TO", 42);
        CenterText("STOP READING", 69);
        TOOLTIP = ACTIVE;
        tutorialPage++;
      }
      break;
    case 10:
      if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
        lastButtonPressTime = millis();
        TOOLTIP = RELEASED;
        tft.fillScreen(TFT_COLOR4);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        CenterText("HOLD A TO GO", 42);
        CenterText("BACK ONE MENU", 69);
        //A Arrow
        tft.fillRect(234, 18, 3, 3, TFT_COLOR1);
        tft.fillRect(231, 15, 3, 9, TFT_COLOR1);
        tft.fillRect(228, 12, 3, 15, TFT_COLOR1);
        //A Button Icon
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.fillRect(204, 6, 21, 27, TFT_COLOR1);
        tft.fillRect(201, 3, 21, 27, TFT_COLOR3);
        tft.drawString("A", 204, 6);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        tft.fillRect(234, 117, 3, 3, TFT_COLOR1);
        tft.fillRect(231, 114, 3, 9, TFT_COLOR1);
        tft.fillRect(228, 111, 3, 15, TFT_COLOR1);
        tft.drawString("B", 204, 105);
        tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
        tft.fillRect(0, 102, 201, 33, TFT_COLOR4);
        tft.drawString("HOLD A", 3, 6);
        tft.drawString("A", 102, 42);
        tutorialPage++;
      }
      break;
    case 11:
      if (AButton == "held" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        tft.fillRect(0, 3, 201, 33, TFT_COLOR4);
        tft.fillRect(0, 102, 201, 33, TFT_COLOR4);
        tft.fillRect(204, 6, 21, 27, TFT_COLOR4);
        tft.fillRect(201, 3, 21, 27, TFT_COLOR4);
        tft.fillRect(240, 69, 0, 33, TFT_COLOR4);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        CenterText("LET'S CHOOSE", 42);
        CenterText("YOUR NEW PICO", 69);
        //A Arrow
        tft.fillRect(234, 18, 3, 3, TFT_COLOR1);
        tft.fillRect(231, 15, 3, 9, TFT_COLOR1);
        tft.fillRect(228, 12, 3, 15, TFT_COLOR1);
        //A Button Icon
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        tft.drawString("A", 204, 6);
        //B Arrow
        tft.fillRect(234, 117, 3, 3, TFT_COLOR1);
        tft.fillRect(231, 114, 3, 9, TFT_COLOR1);
        tft.fillRect(228, 111, 3, 15, TFT_COLOR1);
        //B Button Icon
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.fillRect(204, 105, 21, 27, TFT_COLOR1);
        tft.fillRect(201, 102, 21, 27, TFT_COLOR3);
        tft.drawString("B", 204, 105);

        tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
        tft.drawString("LET'S GO!", 3, 105);
        tutorialPage++;
      }
      break;
    case 12:
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        tft.fillScreen(TFT_COLOR4);
        drawingBatteryIcon(batteryAnim);
        tft.fillRect(3, 3, (43 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 0, (43 * 3), (9 * 3), TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString(String("STARTER"), 3, 3);
        DrawBackground();
        CreateWardrobeCharacter(0, headGear[1], bodyGear[1]);
        CreateWardrobeCharacter(1, headGear[2], bodyGear[2]);
        CreateWardrobeCharacter(2, headGear[3], bodyGear[3]);
        tft.pushImage(162, 60, 48, 48, wardrobePet[0], TFT_BLACK);
        pushScaled(3, 57, 16, 16, gems[1], TFT_BLACK);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        tft.drawString("+10%", 54, 57);
        tft.drawString("ATK", 66, 81);
        tft.fillRect(159, 39, 15, 15, TFT_COLOR1);
        tft.fillRect(180, 39, 15, 15, TFT_COLOR1);
        tft.fillRect(201, 39, 15, 15, TFT_COLOR1);
        pushScaled(159, 21, 5, 5, smallIcons[13], TFT_BLACK);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        CenterText("HOLD FOR INFO", 111);
        tutorialPage++;
        name = "1";
      }
      break;
    case 13:
      //A BUTTON PRESSED FOR NAVIGATION
      if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        drawingBatteryIcon(batteryAnim);
        if (name == "C1" || name == "C2" || name == "C3")
          return;
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString(String("STARTER"), 3, 3);
        CenterText("HOLD FOR INFO", 111);
        tft.fillRect(159, 39, 15, 15, TFT_COLOR1);
        tft.fillRect(180, 39, 15, 15, TFT_COLOR1);
        tft.fillRect(201, 39, 15, 15, TFT_COLOR1);
        if (name == "1") {
          pushScaled(180, 21, 5, 5, smallIcons[13], TFT_BLACK);
          tft.fillRect(159, 27, 15, 9, TFT_COLOR4);
          tft.pushImage(162, 60, 48, 48, wardrobePet[1], TFT_BLACK);
          pushScaled(3, 57, 16, 16, gems[2], TFT_BLACK);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          tft.drawString("+10%", 54, 57);
          tft.drawString("DEF", 66, 81);
          name = "2";
        } else if (name == "2") {
          pushScaled(201, 21, 5, 5, smallIcons[13], TFT_BLACK);
          tft.fillRect(180, 27, 15, 9, TFT_COLOR4);
          tft.pushImage(162, 60, 48, 48, wardrobePet[2], TFT_BLACK);
          pushScaled(3, 57, 16, 16, gems[3], TFT_BLACK);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          tft.drawString("+10%", 54, 57);
          tft.drawString("SPD", 66, 81);
          name = "3";
        } else if (name == "3") {
          pushScaled(159, 21, 5, 5, smallIcons[13], TFT_BLACK);
          tft.fillRect(201, 27, 15, 9, TFT_COLOR4);
          tft.pushImage(162, 60, 48, 48, wardrobePet[0], TFT_BLACK);
          pushScaled(3, 57, 16, 16, gems[1], TFT_BLACK);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          tft.drawString("+10%", 54, 57);
          tft.drawString("ATK", 66, 81);
          name = "1";
        }
      }  //end A button
      //A BUTTON HELD TO CANCEL PICO SELECTION
      if (AButton == "held" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString(String("STARTER"), 3, 3);
        CenterText("HOLD FOR INFO", 111);
        tft.fillRect(159, 39, 15, 15, TFT_COLOR1);
        tft.fillRect(180, 39, 15, 15, TFT_COLOR1);
        tft.fillRect(201, 39, 15, 15, TFT_COLOR1);
        if (name == "C1")
          name = "1";
        if (name == "C2")
          name = "2";
        if (name == "C3")
          name = "3";
      }
      //B BUTTON HELD FOR STARTER PICO TOOLTIP
      if (BButton == "held" && millis() - lastButtonPressTime > 500 && TOOLTIP == RELEASED) {
        lastButtonPressTime = millis();
        if (name == "C1" || name == "C2" || name == "C3")
          return;
        tft.fillScreen(TFT_COLOR4);
        pushScaled(3, 3, 15, 15, abilityFrame[0], TFT_BLACK);
        tft.fillRect(51, 3, 183, 27, TFT_COLOR3);
        tft.fillRect(234, 6, 3, 27, TFT_COLOR1);
        tft.fillRect(54, 30, 183, 3, TFT_COLOR1);
        if (name == "1") {
          pushScaled(9, 9, 11, 11, cardSpriteArray[0], TFT_BLACK);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          tft.drawString("ATK FOCUS", 54, 6);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          tft.drawString("EMPHASIS ON", 3, 63);
          tft.drawString("DEALING BIG", 3, 87);
          tft.drawString("DAMAGE", 3, 111);
        } else if (name == "2") {
          pushScaled(9, 9, 11, 11, cardSpriteArray[1], TFT_BLACK);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          tft.drawString("DEF FOCUS", 54, 6);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          tft.drawString("STAY HEALTHY", 3, 63);
          tft.drawString("AND OUTLAST", 3, 87);
          tft.drawString("YOUR ENEMIES", 3, 111);
        } else if (name == "3") {
          pushScaled(9, 9, 11, 11, cardSpriteArray[5], TFT_BLACK);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
          tft.drawString("SPD FOCUS", 54, 6);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          tft.drawString("MANIPULATE", 3, 63);
          tft.drawString("TIME AND", 3, 87);
          tft.drawString("COUNTERS", 3, 111);
        }
        TOOLTIP = ACTIVE;
      }
      //B BUTTON RELEASED TO RETURN TO STARTER NAVIGATION
      if (BButton == "released" && millis() - lastButtonPressTime > 500 && TOOLTIP == ACTIVE) {
        lastButtonPressTime = millis();
        TOOLTIP = RELEASED;
        tft.fillScreen(TFT_COLOR4);
        drawingBatteryIcon(batteryAnim);
        tft.fillRect(3, 3, (43 * 3), (9 * 3), TFT_COLOR1);
        tft.fillRect(0, 0, (43 * 3), (9 * 3), TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString(String("STARTER"), 3, 3);
        DrawBackground();
        tft.fillRect(159, 39, 15, 15, TFT_COLOR1);
        tft.fillRect(180, 39, 15, 15, TFT_COLOR1);
        tft.fillRect(201, 39, 15, 15, TFT_COLOR1);
        CenterText("HOLD FOR INFO", 111);
        if (name == "1") {
          pushScaled(159, 21, 5, 5, smallIcons[13], TFT_BLACK);
          tft.fillRect(201, 27, 15, 9, TFT_COLOR4);
          tft.pushImage(162, 60, 48, 48, wardrobePet[0], TFT_BLACK);
          pushScaled(3, 57, 16, 16, gems[0], TFT_BLACK);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          tft.drawString("+10%", 54, 57);
          tft.drawString("ATK", 66, 81);
        } else if (name == "2") {
          pushScaled(180, 21, 5, 5, smallIcons[13], TFT_BLACK);
          tft.fillRect(159, 27, 15, 9, TFT_COLOR4);
          tft.pushImage(162, 60, 48, 48, wardrobePet[1], TFT_BLACK);
          pushScaled(3, 57, 16, 16, gems[1], TFT_BLACK);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          tft.drawString("+10%", 54, 57);
          tft.drawString("DEF", 66, 81);
        } else if (name == "3") {
          pushScaled(201, 21, 5, 5, smallIcons[13], TFT_BLACK);
          tft.fillRect(180, 27, 15, 9, TFT_COLOR4);
          tft.pushImage(162, 60, 48, 48, wardrobePet[2], TFT_BLACK);
          pushScaled(3, 57, 16, 16, gems[2], TFT_BLACK);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          tft.drawString("+10%", 54, 57);
          tft.drawString("SPD", 66, 81);
        }
      }
      //B BUTTON PRESSED TO SELECT AND CONFIRM SELECTION
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString(String("CONFIRM"), 3, 3);
        tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
        CenterText("B = CONFIRM", 111);

        if (name == "1") {
          tft.fillRect(159, 39, 15, 15, TFT_COLOR3);
          name = "C1";
        } else if (name == "2") {
          tft.fillRect(180, 39, 15, 15, TFT_COLOR3);
          name = "C2";
        } else if (name == "3") {
          tft.fillRect(201, 39, 15, 15, TFT_COLOR3);
          name = "C3";
        } else if (name == "C1" || name == "C2" || name == "C3") {
          tutorialPage++;
          tft.fillScreen(TFT_COLOR4);
          tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
          CenterText("GIVE YOUR NEW", 42);
          CenterText("PICO A NAME", 69);
        }
      }
      break;
    case 14:
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        tutorialPage++;
        tft.fillScreen(TFT_COLOR4);
        DrawBackground();
        tft.setTextColor(TFT_COLOR3, TFT_COLOR4);
        tft.drawString("H.A = BACK ", 3, 51);
        tft.drawString("H.B = DONE", 3, 78);
        
        int gearIndex = 0;
        int gemIndex = 0;
        
        // Character selection and setup
        if (name == "C1") {
          gearIndex = 1;
          gemIndex = 1;
          tft.pushImage(189, 60, 48, 48, wardrobePet[0], TFT_BLACK);
          CreateCharacter("player", headGear[1], bodyGear[1]);
          
          // Set Deck 1 cards for C1 (ATK focus)
          EEPROM.write(97, 0);
          EEPROM.write(98, 0);
          EEPROM.write(99, 7);
          EEPROM.write(100, 7);
          EEPROM.write(101, 9);
          EEPROM.write(102, 9);
          EEPROM.write(103, 3);
          EEPROM.write(104, 3);
          EEPROM.write(105, 12);
          EEPROM.write(106, 18);
          
        } else if (name == "C2") {
          gearIndex = 2;
          gemIndex = 2;
          tft.pushImage(189, 60, 48, 48, wardrobePet[1], TFT_BLACK);
          CreateCharacter("player", headGear[2], bodyGear[2]);
          
          // Set Deck 1 cards for C2 (DEF focus)
          EEPROM.write(97, 1);
          EEPROM.write(98, 1);
          EEPROM.write(99, 2);
          EEPROM.write(100, 2);
          EEPROM.write(101, 7);
          EEPROM.write(102, 7);
          EEPROM.write(103, 9);
          EEPROM.write(104, 9);
          EEPROM.write(105, 12);
          EEPROM.write(106, 18);
          
        } else if (name == "C3") {
          gearIndex = 3;
          gemIndex = 3;
          tft.pushImage(189, 60, 48, 48, wardrobePet[2], TFT_BLACK);
          CreateCharacter("player", headGear[3], bodyGear[3]);
          
          // Set Deck 1 cards for C3 (SPD focus)
          EEPROM.write(97, 3);
          EEPROM.write(98, 3);
          EEPROM.write(99, 2);
          EEPROM.write(100, 2);
          EEPROM.write(101, 9);
          EEPROM.write(102, 9);
          EEPROM.write(103, 7);
          EEPROM.write(104, 7);
          EEPROM.write(105, 18);
          EEPROM.write(106, 18);
        }
        
        // Set ownership bits in the RAM arrays
        bitSet(headOwnership[gearIndex / 8], gearIndex % 8);
        bitSet(bodyOwnership[gearIndex / 8], gearIndex % 8);
        
        // Write ownership arrays to EEPROM
        EEPROM.put(33, headOwnership);
        EEPROM.put(65, bodyOwnership);
        
        // Write equipped gear indices
        EEPROM.write(135, gearIndex);
        EEPROM.write(136, gearIndex);
        
        // Write gem data
        EEPROM.write(199, 1); //NO GEM
        EEPROM.write(199 + gemIndex, 1); //SELECTED GEM
        EEPROM.write(128, gemIndex);
        
        // Commit everything
        EEPROM.commit();
        
        pushScaled(3, -3, 5, 5, smallIcons[13], TFT_BLACK);
        tft.fillRect(3, 18, 210, 27, TFT_COLOR1);
        tft.fillRect(0, 15, 210, 27, TFT_COLOR3);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
        tft.drawString("A", 3, 18);
      }
      break;
    case 15:
      // Check A button press (move forward through charSet)
      if (AButton == "pressed" && millis() - lastButtonPressTime > 500 && confirmName == false) {
        lastButtonPressTime = millis();
        currentChar = NextChar(currentChar);  // Move to next character in the set
        tft.drawString(String(currentChar), 3 + (charIndex * 21), 18);
      }

      // Check B button press (select character)
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500 && confirmName == false) {
        lastButtonPressTime = millis();
        if (charIndex < 9) {  // Max 10 characters
          tft.fillRect(3 + (charIndex * 21), -3, 15, 15, TFT_COLOR4);
          tempName[charIndex] = currentChar;  // Add the current character to the tempName
          charIndex++;                        // Move to the next position
          currentChar = 'A';
          tft.drawString(String(currentChar), 3 + (charIndex * 21), 18);
          pushScaled(3 + (charIndex * 21), -3, 5, 5, smallIcons[13], TFT_BLACK);
        } else if (charIndex == 9) {
          tft.fillRect(3 + (charIndex * 21), -3, 15, 15, TFT_COLOR4);
          tempName[charIndex] = currentChar;  // Add the current character to the tempName
          charIndex++;                        // Move to the next position
          currentChar = 'A';
          tft.fillRect(192, -3, 15, 15, TFT_COLOR4);
        }
      }
      // Check B button press (Confirm name)
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500 && confirmName) {
        lastButtonPressTime = millis();
        tutorialPage++;
        name = tempName;
        tft.fillScreen(TFT_COLOR4);
        tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
        CenterText("LOOK OVER", 15);
        CenterText(name, 42);
        CenterText("AND TAKE CARE", 69);
        CenterText("OF THEM", 96);
      }

      // Check if A button is held (delete last character)
      if (AButton == "held" && millis() - lastButtonPressTime > 500 && confirmName == false) {
        lastButtonPressTime = millis();
        if (charIndex > 0) {
          tft.fillRect(3 + (charIndex * 21), -3, 15, 15, TFT_COLOR4);
          if (charIndex != 10)
            tft.fillRect(3 + (charIndex * 21), 15, 18, 24, TFT_COLOR3);
          charIndex--;                 // Move back one character
          tempName[charIndex] = '\0';  // Remove the last character
          currentChar = 'A';
          tft.drawString(String(currentChar), 3 + (charIndex * 21), 18);
          pushScaled(3 + (charIndex * 21), -3, 5, 5, smallIcons[13], TFT_BLACK);
        }
      }
      // Check if A button is held (Cancel name confirmation)
      if (AButton == "held" && millis() - lastButtonPressTime > 500 && confirmName) {
        lastButtonPressTime = millis();
        confirmName = false;
        if(charIndex < 9){
          tft.drawString(String(currentChar), 3 + (charIndex * 21), 18);
          pushScaled(3 + (charIndex * 21), -3, 5, 5, smallIcons[13], TFT_BLACK);
        }
      }

      // Check if B button is held (confirm tempName)
      if (BButton == "held" && millis() - lastButtonPressTime > 500 && confirmName == false && charIndex > 0) {
        lastButtonPressTime = millis();
        confirmName = true;
        tft.fillRect(3 + (charIndex * 21), -3, 15, 15, TFT_COLOR4);
        if(charIndex < 10)
          tft.fillRect(3 + (charIndex * 21), 15, 18, 24, TFT_COLOR3);
        else
          tft.fillRect(3 + (charIndex * 21), 15, 18, 24, TFT_COLOR4);
        CenterText("B = CONFIRM", 111);
      }
      break;
    case 16:
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        tutorialPage++;
      }
      break;
    case 17:
      tutorialPage++;
      tft.fillScreen(TFT_COLOR4);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      CenterText("NOW GO FORTH", 15);
      CenterText("FOR LOOT AND", 42);
      CenterText("ADVENTURE,", 69);
      CenterText("NAVIGATOR.", 96);
      break;
    case 18:
      if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
        lastButtonPressTime = millis();
        testActive = false;
        xPosition = 99;
        tft.fillScreen(TFT_COLOR4);
        DrawBackground();
        DrawMainMenu();
        EEPROM.put(175, name);
        EEPROM.write(0, 1);  // Mark tutorial as completed
        EEPROM.commit();
        ReadSaveData();
        MENUSTATE = MAINMENU;
      }
      break;
  }
}
