#include "globals.h"
#include "dialogue.h"

#define THOUGHT_ODDS      1   // 1 in N chance per active minute while awake
#define THOUGHT_LIFETIME  30    // minutes a thought bubble stays up before fading
#define THOUGHT_COOLDOWN  1    // minutes after a thought before another can roll
#define DREAM_CHANCE      40    // percent chance a sleep has a dream (max one per sleep)
#define DREAM_MIN         30    // earliest dream, in minutes after falling asleep
#define DREAM_MAX         180   // latest dream, in minutes after falling asleep
#define RECENT_SIZE       8     // recently shown lines that won't repeat

bool thoughtActive = false;
uint8_t dayFlags = 0;
uint8_t currentNature = 0;  // 0 = NEUTRAL

static bool thoughtIsDream = false;
static int thoughtAge = 0;
static int thoughtCooldown = 0;
static int sleepMinutes = 0;
static int dreamAt = -1;
static uint16_t recentLines[RECENT_SIZE] = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };
static uint8_t recentPos = 0;

void SetDayFlag(uint8_t flag) {
  dayFlags |= flag;
}

// Anything more important than a thought hides the bubble
static bool ThoughtBlocked() {
  return randomEventPending || VENTURESTATE == ENCOUNTER
      || (VENTURESTATE == VENTURING && countdownSeconds == 0)
      || (HP == 0 && countdownSeconds == 0)
      || MENUSTATE == BATTLE || MENUSTATE == TITLE || MENUSTATE == TUTORIAL;
}

// Rolls thoughts while awake and maybe schedules a dream per sleep
void UpdateThoughts() {
  static unsigned long lastTick = millis();
  static bool wasSleeping = false;
  bool sleeping = (ACTION == SLEEPING);

  if (sleeping && !wasSleeping) {
    sleepMinutes = 0;
    dreamAt = (random(100) < DREAM_CHANCE) ? random(DREAM_MIN, DREAM_MAX + 1) : -1;
    thoughtActive = false;
  } else if (!sleeping && wasSleeping) {
    dreamAt = -1;
    thoughtActive = false;
  }
  wasSleeping = sleeping;

  if (gamePause) {
    lastTick = millis();
    return;
  }
  bool blocked = ThoughtBlocked();
  if (blocked)
    thoughtActive = false;
  if (millis() - lastTick < 60000)
    return;
  lastTick += 60000;

  if (sleeping) {
    sleepMinutes++;
    if (!blocked && !thoughtActive && dreamAt >= 0 && sleepMinutes >= dreamAt) {
      thoughtActive = true;
      thoughtIsDream = true;
      dreamAt = -1;
    }
    return;
  }
  if (thoughtActive) {
    if (++thoughtAge >= THOUGHT_LIFETIME) {
      thoughtActive = false;
      thoughtCooldown = THOUGHT_COOLDOWN;
    }
    return;
  }
  if (thoughtCooldown > 0) {
    thoughtCooldown--;
    return;
  }
  if (!blocked && random(THOUGHT_ODDS) == 0) {
    thoughtActive = true;
    thoughtIsDream = false;
    thoughtAge = 0;
  }
}

// Swaps the Pico's emote for the thought bubble while one is waiting on the home screen.
// Called once per Pico frame, so flipping here keeps the bubble in step with the body.
int ThoughtEmote(int current) {
  static bool bigBubble = false;
  if (!thoughtActive || MENUSTATE != MAINMENU)
    return current;
  bigBubble = !bigBubble;
  return bigBubble ? 9 : 8;
}

// Half the time a matching situation, a quarter the nature, the rest general
static int PickPool(bool dream) {
  int ctx[24];
  int n = 0;
  if (dream) {
    const int flagPools[8] = { P_DR_WON, P_DR_LOST, P_DR_BOSS, P_DR_JACKPOT, P_DR_GEAR, P_DR_HARVEST, P_DR_GOOD_EVENT, P_DR_BAD_EVENT };
    if (Hunger <= 30) ctx[n++] = P_DR_HUNGRY;
    for (int i = 0; i < 8; i++)
      if (dayFlags & (1 << i)) ctx[n++] = flagPools[i];
  } else {
    const int flagPools[8] = { P_TH_WON, P_TH_LOST, P_TH_BOSS, P_TH_JACKPOT, P_TH_GEAR, P_TH_HARVEST, P_TH_GOOD_EVENT, P_TH_BAD_EVENT };
    if (Hunger <= 30) ctx[n++] = P_TH_HUNGRY;
    if (Stamina <= 30) ctx[n++] = P_TH_TIRED;
    if (Mood < 30) ctx[n++] = P_TH_ANGRY;
    else if (Mood < 50) ctx[n++] = P_TH_SAD;
    else if (Mood >= 90) ctx[n++] = P_TH_HAPPY;
    if (ACTION == INJURED) ctx[n++] = P_TH_INJURED;
    else if (HP < maxHP / 2) ctx[n++] = P_TH_LOW_HP;
    if (deepDungeonActive) ctx[n++] = P_TH_DEEP_DUNGEON;
    if (VENTURESTATE == VENTURING && countdownSeconds > 0) ctx[n++] = P_TH_COOLDOWN;
    if (cropReady) ctx[n++] = P_TH_CROPS_READY;
    if (playerCredits >= 2000) ctx[n++] = P_TH_RICH;
    else if (playerCredits < 50) ctx[n++] = P_TH_BROKE;
    for (int i = 0; i < 8; i++)
      if (dayFlags & (1 << i)) ctx[n++] = flagPools[i];
  }
  int roll = random(100);
  if (n > 0 && roll < 50)
    return ctx[random(n)];
  if (currentNature >= 1 && currentNature <= 8 && roll < 75)
    return (dream ? P_DR_CHEERFUL : P_TH_CHEERFUL) + currentNature - 1;
  return dream ? P_DR_GENERAL : P_TH_GENERAL;
}

// Enemy name lines need a recently fought enemy to talk about
static bool LineAllowed(const char* line) {
  return enemyMemoryMinutes > 0 || strstr(line, "{E}") == nullptr;
}

static const char* PickLine(bool dream) {
  int pool = PickPool(dream);
  int idx = -1;
  uint16_t key = 0;
  for (int tries = 0; tries < 20; tries++) {
    int i = random(dialoguePools[pool].count);
    if (!LineAllowed(dialoguePools[pool].lines[i])) continue;
    idx = i;
    key = pool * 1000 + idx;
    bool seen = false;
    for (int r = 0; r < RECENT_SIZE; r++)
      if (recentLines[r] == key) seen = true;
    if (!seen) break;
  }
  if (idx < 0) {
    pool = dream ? P_DR_GENERAL : P_TH_GENERAL;
    do {
      idx = random(dialoguePools[pool].count);
    } while (!LineAllowed(dialoguePools[pool].lines[idx]));
    key = pool * 1000 + idx;
  }
  recentLines[recentPos] = key;
  recentPos = (recentPos + 1) % RECENT_SIZE;
  return dialoguePools[pool].lines[idx];
}

// Picks a thought or dream and cuts to it
void OpenThought() {
  String text = PickLine(thoughtIsDream);
  String petName = name;
  petName.trim();
  thoughtActive = false;
  if (!thoughtIsDream)
    thoughtCooldown = THOUGHT_COOLDOWN;

  String lines[4];
  int count = 0;
  int start = 0;
  while (count < 4) {
    int bar = text.indexOf('|', start);
    String line = (bar < 0) ? text.substring(start) : text.substring(start, bar);
    line.replace("{P}", petName);
    line.replace("{E}", lastEnemyName);
    lines[count++] = line;
    if (bar < 0) break;
    start = bar + 1;
  }

  DrawDialogueScreen(lines, count);
}

// Full-screen message: up to 4 centered lines, any button returns home
void DrawDialogueScreen(String lines[], int count) {
  MENUSTATE = THOUGHT;
  mainMenuReturn = 0;
  tft.fillScreen(TFT_COLOR4);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  int y = (135 - (count * 30 - 6)) / 2;
  for (int i = 0; i < count; i++)
    CenterText(lines[i], y + i * 30);
}

void CloseThought() {
  MENUSTATE = MAINMENU;
  AMENUS = NA;
  mainMenuReturn = 0;
  tft.fillScreen(TFT_COLOR4);
  DrawBackground();
  DrawMainMenu();
  display.fillRect(0, 0, 240, 54, TFT_COLOR4);
  display.fillRect(0, 54, 240, 3, TFT_COLOR1);
}
