#include "globals.h"
#include <EEPROM.h>

#define NATURE_ADDR       380   // 380-387 today's points, 388-395 scores
#define NATURE_CUR_ADDR   396
#define NATURE_DAY_ADDR   397   // 397-398 active minutes into the day
#define NATURE_VENT_ADDR  399
#define NATURE_FLAG_ADDR  400   // today's event flags (thoughts and dreams)
#define NATURE_MAGIC_ADDR 401
#define ENEMY_NAME_ADDR   402   // 402-412 last enemy fought, null-terminated
#define ENEMY_MEM_ADDR    413   // 413-414 minutes left to remember that enemy
#define NATURE_MAGIC      0xB7

#define DAY_MINUTES       1440  // one nature day = 24 hours of unpaused time
#define DAILY_POINTS      10    // points handed out per day, split by share
#define EARN_SCORE        25    // score needed to earn or switch to a nature
#define EARN_LEAD         5     // lead over second place needed from NEUTRAL
#define SWITCH_LEAD       15    // lead over the current nature needed to switch
#define NEUTRAL_SCORE     10    // current nature fades to NEUTRAL below this
#define ENEMY_MEMORY      480   // active minutes a fought enemy's name stays usable in thoughts

uint8_t natureToday[8];
uint8_t natureScore[8];
uint16_t natureDayMinutes = 0;
uint8_t venturesToday = 0;
bool natureMsgPending = false;
bool lazyRefusedWake = false;
int lazyGrumble = 0;
char lastEnemyName[11] = "";
uint16_t enemyMemoryMinutes = 0;

static int cheerMinutes = 0;
static int grumpyMinutes = 0;
static int lazyMinutes = 0;
static int creditsFoundToday = 0;

const char* NatureName(int n) {
  static const char* natureNames[] = { "NEUTRAL", "CHEERFUL", "PLAYFUL", "GLUTTON", "LAZY", "BRAVE", "GREEDY", "TIMID", "GRUMPY" };
  return (n >= 0 && n <= 8) ? natureNames[n] : natureNames[0];
}

void AddNaturePoints(uint8_t nature, uint8_t pts) {
  if (nature < 1 || nature > 8) return;
  int v = natureToday[nature - 1] + pts;
  natureToday[nature - 1] = (v > 255) ? 255 : v;
}

void SaveNatures() {
  for (int i = 0; i < 8; i++) {
    EEPROM.write(NATURE_ADDR + i, natureToday[i]);
    EEPROM.write(NATURE_ADDR + 8 + i, natureScore[i]);
  }
  EEPROM.write(NATURE_CUR_ADDR, currentNature);
  EEPROM.put(NATURE_DAY_ADDR, natureDayMinutes);
  EEPROM.write(NATURE_VENT_ADDR, venturesToday);
  EEPROM.write(NATURE_FLAG_ADDR, dayFlags);
  EEPROM.write(NATURE_MAGIC_ADDR, NATURE_MAGIC);
  for (int i = 0; i < 11; i++)
    EEPROM.write(ENEMY_NAME_ADDR + i, lastEnemyName[i]);
  EEPROM.put(ENEMY_MEM_ADDR, enemyMemoryMinutes);
}

// Remembers the venture enemy just fought so thoughts and dreams can mention it
void BankEnemyName() {
  if (eventBattle) return;
  strncpy(lastEnemyName, enemyName.c_str(), 10);
  lastEnemyName[10] = 0;
  enemyMemoryMinutes = ENEMY_MEMORY;
}

// Clears everything back to a NEUTRAL Pico (new game or missing block)
void ResetNatures() {
  for (int i = 0; i < 8; i++) {
    natureToday[i] = 0;
    natureScore[i] = 0;
  }
  currentNature = 0;
  natureDayMinutes = 0;
  venturesToday = 0;
  dayFlags = 0;
  natureMsgPending = false;
  lastEnemyName[0] = 0;
  enemyMemoryMinutes = 0;
  SaveNatures();
}

void LoadNatures() {
  if (EEPROM.read(NATURE_MAGIC_ADDR) != NATURE_MAGIC) {
    ResetNatures();
    EEPROM.commit();
    return;
  }
  for (int i = 0; i < 8; i++) {
    natureToday[i] = EEPROM.read(NATURE_ADDR + i);
    natureScore[i] = min((int)EEPROM.read(NATURE_ADDR + 8 + i), 100);
  }
  currentNature = EEPROM.read(NATURE_CUR_ADDR);
  if (currentNature > 8) currentNature = 0;
  EEPROM.get(NATURE_DAY_ADDR, natureDayMinutes);
  if (natureDayMinutes >= DAY_MINUTES) natureDayMinutes = 0;
  venturesToday = EEPROM.read(NATURE_VENT_ADDR);
  dayFlags = EEPROM.read(NATURE_FLAG_ADDR);
  for (int i = 0; i < 11; i++)
    lastEnemyName[i] = EEPROM.read(ENEMY_NAME_ADDR + i);
  lastEnemyName[10] = 0;
  EEPROM.get(ENEMY_MEM_ADDR, enemyMemoryMinutes);
  bool valid = enemyMemoryMinutes <= ENEMY_MEMORY && lastEnemyName[0] != 0;
  for (int i = 0; valid && lastEnemyName[i]; i++)
    if (!(lastEnemyName[i] >= 'A' && lastEnemyName[i] <= 'Z') && lastEnemyName[i] != ' ') valid = false;
  if (!valid) {
    lastEnemyName[0] = 0;
    enemyMemoryMinutes = 0;
  }
}

// End of a nature day: split the day's points by share, fade old scores, pick the nature
static void NatureRollover() {
  if (playerCredits >= 2000) AddNaturePoints(NAT_GREEDY, 3);
  if (venturesToday == 0) AddNaturePoints(NAT_LAZY, 4);

  int sum = 0;
  for (int i = 0; i < 8; i++) sum += natureToday[i];
  for (int i = 0; i < 8; i++) {
    int share = (sum > 0) ? (natureToday[i] * DAILY_POINTS + sum / 2) / sum : 0;
    natureScore[i] = min(natureScore[i] * 7 / 8 + share, 100);
  }

  int cur = currentNature - 1;
  int top = (cur >= 0) ? cur : 0;
  for (int i = 0; i < 8; i++)
    if (natureScore[i] > natureScore[top]) top = i;
  int second = -1;
  for (int i = 0; i < 8; i++)
    if (i != top && (second < 0 || natureScore[i] > natureScore[second])) second = i;

  uint8_t before = currentNature;
  if (cur < 0) {
    if (natureScore[top] >= EARN_SCORE && natureScore[top] - natureScore[second] >= EARN_LEAD)
      currentNature = top + 1;
  } else if (top != cur && natureScore[top] >= EARN_SCORE && natureScore[top] >= natureScore[cur] + SWITCH_LEAD) {
    currentNature = top + 1;
  } else if (natureScore[cur] < NEUTRAL_SCORE) {
    currentNature = 0;
  }
  if (currentNature != before) natureMsgPending = true;

  for (int i = 0; i < 8; i++) natureToday[i] = 0;
  natureDayMinutes = 0;
  venturesToday = 0;
  dayFlags = 0;
  creditsFoundToday = 0;
  SaveNatures();
  EEPROM.commit();
}

// Runs every loop: minute tick for the timed points, the day clock, and the change message
void UpdateNatures() {
  static unsigned long lastTick = millis();

  if (natureMsgPending && MENUSTATE == MAINMENU && AMENUS == NA && !screenOff && !randomEventPending
      && eventState == EVENT_IDLE && (ACTION == IDLE || ACTION == WALKING) && HP > 0
      && !(VENTURESTATE == VENTURING && countdownSeconds == 0)) {
    natureMsgPending = false;
    String petName = name;
    petName.trim();
    String lines[3] = { petName + "'S", "NATURE IS NOW", NatureName(currentNature) };
    DrawDialogueScreen(lines, 3);
  }

  if (gamePause) {
    lastTick = millis();
    return;
  }
  if (millis() - lastTick < 60000)
    return;
  lastTick += 60000;

  if (ACTION != SLEEPING && MENUSTATE != BATTLE) {
    if (Hunger >= 50 && Mood >= 50 && Stamina >= 50 && ++cheerMinutes >= 240) {
      cheerMinutes = 0;
      AddNaturePoints(NAT_CHEERFUL, 1);
    }
    if ((Hunger < 30 || Mood < 30) && ++grumpyMinutes >= 60) {
      grumpyMinutes = 0;
      AddNaturePoints(NAT_GRUMPY, 1);
    }
    if (Stamina > 70 && VENTURESTATE == INACTIVE && ++lazyMinutes >= 120) {
      lazyMinutes = 0;
      AddNaturePoints(NAT_LAZY, 1);
    }
    if (currentNature == NAT_GREEDY && creditsFoundToday < 10 && random(90) == 0) {
      int found = random(1, 4);
      AddCredits(found);
      creditsFoundToday += found;
    }
  }
  if (enemyMemoryMinutes > 0)
    enemyMemoryMinutes--;
  if (++natureDayMinutes >= DAY_MINUTES)
    NatureRollover();
}

// Awake drain interval for hunger or stamina, scaled by nature
int NatureDrain(int base, int meter, bool isHunger) {
  int pct = 100;
  if (currentNature == NAT_CHEERFUL) pct = 115;
  if (currentNature == NAT_GLUTTON && isHunger) pct = 85;
  if (currentNature == NAT_GRUMPY && meter < 30) pct = 125;
  return base * pct / 100;
}

// Mood change per tick while resting awake out of a venture (normally +3)
int NatureRestMood() {
  if (currentNature == NAT_PLAYFUL && millis() - lastButtonPressTime > 7200000UL) return 1;
  if (currentNature == NAT_BRAVE && venturesToday == 0 && natureDayMinutes >= 720) return -1;
  return 3;
}

int NatureVentureCooldown() {
  if (debug) return 11;
  return (currentNature == NAT_TIMID) ? 2070 : 1800;
}

// Home screen roaming: tick speed, turn chance, idle chance and idle length
void NatureWalkParams(int &tickMs, int &turnPct, int &idlePct, int &idleMin, int &idleMax) {
  tickMs = 1000; turnPct = 2; idlePct = 10; idleMin = 3; idleMax = 10;
  switch (currentNature) {
    case NAT_PLAYFUL: tickMs = 700; turnPct = 4; idlePct = 5; idleMin = 2; idleMax = 5; break;
    case NAT_LAZY: tickMs = 1400; idlePct = 25; idleMin = 8; idleMax = 20; break;
    case NAT_GREEDY: turnPct = 12; break;
    case NAT_TIMID: tickMs = 1200; idlePct = 12; idleMin = 4; idleMax = 12; break;
    case NAT_GRUMPY: tickMs = 1200; idlePct = 15; idleMin = 5; idleMax = 12; break;
    default: break;
  }
}

// TIMID only stops to idle near the screen edges
bool NatureCanIdleHere(int x) {
  return currentNature != NAT_TIMID || x < 60 || x > 114;
}

// Occasional nature flavor frame while idling with a normal or happy emotion
void NatureIdleFrame(int &frame, int &emote) {
  if (random(5) != 0) return;
  switch (currentNature) {
    case NAT_CHEERFUL: frame = 7; emote = 6; break;
    case NAT_PLAYFUL: frame = 7; break;
    case NAT_GLUTTON: frame = 2; break;
    case NAT_LAZY: frame = 4; break;
    case NAT_BRAVE: frame = random(2) ? 11 : 6; break;
    case NAT_GREEDY: emote = 2; break;
    case NAT_TIMID: frame = 8; break;
    case NAT_GRUMPY: frame = 6; emote = 7; break;
    default: break;
  }
}
