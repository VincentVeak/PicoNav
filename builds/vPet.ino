#include <Pangodream_18650_CL.h>
#include <TFT_eSPI.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>
#include "globals.h"
#include "sprites.h"
#include "cards.h"
#include "headGear.h"
#include "bodyGear.h"
#include "ventureInfo.h"

///////////////Pet Stuff//////////////////////
#pragma region PetVars

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite display = TFT_eSprite(&tft);
TFT_eSprite playerSprite = TFT_eSprite(&tft);
TFT_eSprite emoteSprite = TFT_eSprite(&tft);
TFT_eSprite itemSprite = TFT_eSprite(&tft);
uint16_t combinedImage[48 * 48];
uint16_t combinedMirrorImage[48 * 48];
uint16_t tempImage[48 * 48];
uint16_t* pet[12] = { nullptr };
uint16_t* mirrorPet[12] = { nullptr };
int xPosition = 99;     //Character x position;
bool moveLeft = false;  //Used for walking around the homescreen
int randomIdle = 5;     //Used for randomly standing still while walking around the homescreen
String name = "DAVE";
int Hunger = 100;
int hungerCycles = 0;
int hungerRegenCycles = 0;
int Stamina = 100;
int staminaCycles = 0;
unsigned long sleepStartTime = 0;
int Mood = 100;
int moodCycles = 0;
unsigned long lastSaveTime = 0;
int maxHP = 25;
int HP = 25;
int baseAttack = 10;
int baseDefense = 10;
int baseSpeed = 10;
int Attack = 10;
int Defense = 10;
int Speed = 10;
int attackStage = 0;
int defenseStage = 0;
int speedStage = 0;
int gemsIndex[3] = { 0, 1, 2 };

int inventoryAdr[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  //EEPROM address of the iventory item
int inventoryQty[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  //Available quantity of inventory item
int invSelect = 0;

int menuItem = 0;    //Current selected index being hovered
int maxItems = 0;    //Total number of items visible in the list. Should be 1-4
int pageNum = 0;     //Menu page number
int menuIndex = 0;   //Index of the ownership array
int menuValues[4];   //Array of stored menu indexes that are currently being displayed onscreen
bool empty = false;  //Used to prevent empty menu pages from populating

int selectedVentureMenu = 0;

byte headOwnership[32];
byte bodyOwnership[32];
uint16_t* wardrobePet[3];
int equippedWardrobe = 0;
int selectedWardrobe = 0;
String wardrobeSlot = "head";

//CARDS//
byte cardOwnership[32];
int selectedDeck = 0;
int equippedDeck = 0;
int selectedMenuCard = -1;  // Selected card inside the deck (slot 0 - 7)
int deckIndex[10];          //list of card indexes stored of selected deck;
Card playerDeck[10] = {
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false)
};

//ENEMY VARIABLES//
#pragma region Enemy Variables
TFT_eSprite enemySprite = TFT_eSprite(&tft);
uint16_t combinedEnemyImage[48 * 48];
uint16_t combinedMirrorEnemyImage[48 * 48];
uint16_t* enemyPet[12] = { nullptr };
uint16_t* mirrorEnemyPet[12] = { nullptr };

String enemyName = "EVIL DAVE";
int enemyMaxHP = 25;
int enemyHP = 25;
int enemyLevel = 1;
int enemyAttack = 10;
int enemyDefense = 10;
int enemySpeed = 10;
int enemyAttackStage = 0;
int enemyDefenseStage = 0;
int enemySpeedStage = 0;
int enemyGems[3] = { 0, 0, 0 };
Card enemyDeck[10] = {
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false),
  Card("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false)
};


#pragma endregion

//SHOP STUFF//
#pragma region Shop
//SHOP VARIABLES//
int playerCredits = 0;         // Current player credits
int shopSelection = 0;         // Currently selected shop item (0-based)
int shopItemCount = 0;         // Total items currently available in shop
int shopInventory[10];          // Indices of available shop items
bool shopPurchaseConfirm = false;  // Whether showing purchase confirmation
bool shopSelectYes = true;     // YES/NO selection (true = YES)
bool shopRefresh = true;
bool shopRareItemPurchased[10] = {false, false, false, false, false, false, false, false, false, false};
bool shopShowingReward = false;
int shopRewardType = 0;       // 0=card, 1=headgear, 2=bodygear, 3=gem
int shopRewardIndex = -1;
String shopRewardName = "";


ShopItem shopItemList[10] = {
  {3, 9, "MED", "KIT", 50, 0, "INSTANTLY","REVITALIZE","INJURED PICO"},
  {5, 15, "SCRY", "GLASS", 30, 0, "INC DROP", "CHANCE IN", "NEXT DUNGEON"},
  {6, 18, "FLOOR", "MAP", 25, 0, "REMOVES WAIT", "TIME FOR NEXT", "VENTURE"},
  {7, 21, "CARD", "PACK", 75, 1, "GRANTS A", "RANDOM NEW", "CARD"},
  {8, 24, "GEAR", "DROP", 100, 1, "GRANTS A", "RANDOM NEW", "GEAR PIECE"},
  {9, 27, "RANDOM", "GEM", 250, 2, "GRANTS A", "RANDOM NEW", "GEM"},
  {4, 12, "TASTY", "CAKE", 30, 0, "INCREASE MOOD","+40 AND","FOOD +10"},
  {0, 0, "SWEET", "FRUIT", 10, 0, "FEED PICO &","FILL FOOD","METER +40PTS"},
  {1, 3, "ENERGY", "DRINK", 20, 0, "REPLENISH","STAMINA","METER +20PTS"},
  {2, 6, "HEALTH", "POTION", 15, 0, "REPLENISH","50% OF MAX","HEALTH POINTS"}
};

#pragma endregion

//VENTURE MODE//
#pragma region Venture
int ventureRewardCards[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
int ventureRewardBody[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
int ventureRewardHead[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
int maxStage = 0;         //Latest unlocked dungeon stage
int stage = 0;            //current selected stage broken down for UI
int ventureOpponent = 0;  //current selected opponent index
int countdownSeconds = 0;
int ventureTime = 0;
int injuryTime = 0;  //time it takes to revive injured Pico
int requiredStamina = 0;
//Random enemy (post stage 30)
bool randomEnemyGenerated = false;
bool enemySpriteCreated = false;
int randomEnemyHead = 0;
int randomEnemyBody = 0;
int randomEnemyDeck[10] = {0,0,0,0,0,0,0,0,0,0};
//Venture cancel confirmation
bool ventureCancelConfirm = false;
bool ventureCancelSelectYes = false;
//Deep Dungeon Mode
int deepDungeonHighScore = 0;
bool deepDungeonUnlocked = false;
bool deepDungeonActive = false;
uint16_t deepDungeonStage = 0;
byte ddCardOwnership[32];
int activeGem = -1;                                 // DD run: chosen gem (0-8), -1 = none
bool ddReviveUsed = false;                          // DD run: REVIVE gem consumed (once per run)
String effectMsgQueue[8];                           // battle effect messages waiting to show
int effectMsgTargetQ[8];                            // arrow target per message (0=player,1=arena,2=enemy)
int effectMsgCount = 0;
int effectMsgIndex = 0;
int activeMerits[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };   // DD run: active merit ids
int activeMeritCount = 0;                           // DD run: number of active merits
int activeAfflictions[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };
int activeAfflictionLevels[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int activeAfflictionCount = 0;
bool statDrill = false;
int statDrillSel = 0;
bool ddGemSelect = false;                           // DD gem-select screen active
int ddGemChoices[3] = { 0, 1, 2 };                  // the 3 rolled gem ids offered
int ddGemSel = 0;                                   // highlighted choice (0-2)
bool ddGemsRolled = false;                          // gems rolled? persists until a run begins
bool ddGemConfirm = false;                          // gem enter-confirmation screen active
bool ddGemConfirmYes = false;                        // confirm selection (false = NO default)
bool ddMeritSelect = false;                          // DD merit-select screen active
int ddMeritChoices[3] = { 0, 1, 2 };                 // the 3 rolled merit ids offered
int ddMeritSel = 0;                                  // highlighted merit (0-2)
bool ddCardDraft = false;                            // DD card-draft screen active
int ddCardChoices[3] = { 0, 1, 2 };                  // the 3 rolled card ids offered this round
int ddCardSel = 0;                                   // highlighted card (0-2)
int ddDraftRound = 0;                                // current draft round (0-2)
int ddDraftPicks[3] = { -1, -1, -1 };                // the 3 cards picked across the draft
bool ddExitConfirm = false;
bool ddExitSelectNo = true;
bool ddUnlockShown = true;   // shown only in the battle where DD is first unlocked
//Item Effects
bool scryGlassActive = false;  // Doubles gear drop chance for next battle
bool floorMapActive = false;   // Skip venture wait time
bool battleReady = false;      // a battle is queued -> show READY; cleared once it starts
int battleAutoAdvance = 3;     // 500ms steps before a battle message auto-advances (3 = 1.5s)
String battleArrowUser = "";   // who the attack arrow points from ("player"/"enemy"), for tooltip restore
int battleArrowTarget = -1;    // counter-effect arrow slot (0=player,1=arena,2=enemy), for tooltip restore
int randomBossIndex = 0;       // chosen boss from the pool on a boss floor
bool bossRewardPending = false;  // boss defeated: owe a merit draft + affliction
bool bossMeritDraft = false;     // the merit-select screen is a boss reward, not run start
bool ddMeritDiscard = false;     // over-cap: choosing a merit to discard
bool ddMeritConfirm = false;     // over-cap: confirm the discard
int ddDiscardSel = 0;
bool ddDiscardConfirmYes = false;
int pendingMerit = -1;           // merit awaiting placement when at cap
bool bossAfflictionPopup = false;  // showing the boss's affliction grant popup
String bossAfflictionMsg = "";
int bossAfflictionId = 0;          // granted affliction (for the popup tooltip)
int bossAfflictionLvl = 1;
#pragma endregion

//FARM STUFF//
#pragma region Farm
int selectedPot = 0;    // Currently selected pot (0-2)
int potCrop[3] = {0, 0, 0};  // Crop type in each pot (0=empty, 1=fruit, 2=drink, 3=health)
int potStage[3] = {0, 0, 0};  // Growth stage (0=empty, 1-2=sprout, 3-4=bud, 5=grown)
int potTimer[3] = {0, 0, 0};  // Timer for each pot (in minutes)
unsigned long lastFarmUpdate = 0;
int farmAnimFrame = 0;
String harvestMessage = "";
unsigned long harvestMessageStart = 0;
#pragma endregion

//BATTLE STUFF//
#pragma region Battle
BattleState BATTLESTATE;
String beginTurn = "Waiting";
int actionTime = 0;
int selectedBattleCard = 0;
int enemyBattleCard = 0;
String selectionMode = "Cards";
int selectedCounter = 0;
String playerCounter = "";
String enemyCounter = "";
String arenaCounter = "";
String firstPlayer = "";
String secondPlayer = "";
String battleMessage = "";
int equippedGem = 0;
int battleTurnCount = 0;
int pressureUsageCount = 0;
bool payoutUsed = false;
bool playerUsedBlock = false;
bool enemyUsedBlock = false;
bool playerLifesteal = false;
bool whiplashPrimed = false;
bool stackedDeckUsed = false;
bool enemyLifesteal = false;
int playerLastDamage = 0;
int enemyLastDamage = 0;
Counter tooltipCounter("Name", "Abbreviation", 0, "Type", "Desc1", "Desc2", "Desc3");
Card tooltipCard("Fireball", 0, "Fire", 70, false, "Burns enemy on contact", " ", " ", "", "Fire", "", false);
bool concedeMenuActive = false;
bool concedeSelectNo = true;
#pragma endregion

//RANDOM EVENT STUFF//
#pragma region RandomEvents
bool randomEventPending = false;
EventType pendingEventType = EVENT_NONE;
EventState eventState = EVENT_IDLE;
unsigned long lastEventCheck = 0;
unsigned long eventWalkStart = 0;
int eventTargetX = 51;
String eventResultText1 = "";
String eventResultText2 = "";
String eventResultText3 = "";
bool eventShowCharacter = false;
int eventCharacterAction = 0;
int eventCharacterX = 99;
bool chargedUpActive = false;
bool downerActive = false;
bool merchantDiscountActive = false;
bool woundedEnemyActive = false;
int eventFreebieItem = -1;

#pragma endregion

ActionState ACTION;
Emotion EMOTION;
int randomPos = 0;
int playerFrames = 0;
int enemyFrames = 0;
String enemyAction = "idle";
int emoteFrames = 0;
int itemFrames = 0;
#pragma endregion

///////////////UI////////////////////////////
int animFrames = 0;
int enemyAnimFrames = 0;
int batteryFrames = 0;
int tutorialPage = 0;  //Used to remember what part of the tutorial the player is on
extern const char charSet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_-+=<>,.?";
extern const int charSetLength = sizeof(charSet) - 1;  // Length of the character set
char currentChar = 'A';                         // Start from first character in the set
char tempName[11] = "";                         // Temporary name string, max 10 characters + null terminator
int charIndex = 0;                              // Index for the tempName string
bool confirmName = false;
String buildName = "v2.0B1";
bool debug = false;
unsigned long lastButtonPressTime = 0;
int mainMenuReturn = 0;
AppMenu AMENUS;
MenuState MENUSTATE;
OptionsSetting OPTIONSSETTINGS;
VentureState VENTURESTATE;
TooltipState TOOLTIP;
/////////////////////////////////////////////

/////////////HARDWARE STUFF//////////////////
#pragma region HARDWARE
//***LED STUFF
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, 27);
BlinkState state = OFF;
unsigned long previousMillis = 0;  // Previous time in milliseconds
int blinkCount = 0;
int ledBrightness = 50;
int ledPercent = 20;  // 0..100 in steps of 10 (scales ledBrightness in ApplyLed)
bool ledActive = false;
bool cooldownDoneNotify = false;      // brief rainbow ping when a venture cooldown ends
unsigned long cooldownDoneTime = 0;
String ledColor = "Off";
bool ledTimedOut = false;   // true while a timed-out notification is suppressed (nothing drives the strip)
bool chargeNotification = false;
bool doNotDisturb = false;
bool cropReady = false;
bool setDND = false;
bool setWrd = false;
bool setDck = false;
bool setVnt = false;
bool testActive = false;
//***
//***BATTERY STUFF
int batteryAnim = 0;
Pangodream_18650_CL BL(ADC_PIN, CONV_FACTOR, READS);
bool isCharging = true;
int percentage = 0;
int chargeInitLoops = 10;  //used to calibrate battery percentage
int cpu = 80;
bool exportTriggered = false;
//***
//***SCREEN STUFF
int screenOffTime = 0;
bool screenOff = false;
int screenBrightness = 70;
int backlightPercent = 30;  // 0..100 in steps of 10 (scales screenBrightness in ApplyBacklight)
int screenTimeout = 15;     // seconds of idle before the screen sleeps (15/30/45/60)
int ledTimeout = 0;         // minutes the LED may run continuously before auto-off; 0 = NONE
//***
//***FOR PNUT REFERENCE ONLY--COLORS
String COLOR1 = "0xE758";
String COLOR2 = "0xFE72";
String COLOR3 = "0xFBAE";
String COLOR4 = "0x0187";
//***
//***BUTTON STUFF
int button1State = LOW;
int button2State = LOW;
String AButton = "";
String BButton = "";
unsigned long button1PressTime = 0;
unsigned long button2PressTime = 0;
unsigned long button1RepeatTime = 0;
unsigned long button2RepeatTime = 0;
bool eatSkipReady = false;
int skipCheerStage = 0;
unsigned long emergencyShutdownStart = 0;  // Track A button hold for emergency shutdown
//***CLOCK AND GAME TIME
char timeStr[6];
bool twelveHr = true;
bool gamePause = false;
//***
//***SERIAL STUFF
unsigned long lastUpdateTime = 0;  // To track time for the countdown update
int countdown = 60;                // Starting countdown value
bool countdownComplete = false;    // Flag to track if countdown is complete
//***

#pragma endregion

void setup() {
  delay(250);
  Serial.begin(115200);
  pinoutInit();
  displayInit();
  EEPROM.begin(512);
  CheckAndLoadRestore();
  SaveDataInit(); // Initialize EEPROM data BEFORE reading it (critical for new units)
  ReadSaveData();
  // Stamp running firmware version into save data (bytes 500-511) so backups carry it
  bool _verChanged = false;
  for (int i = 0; i < 12; i++) { byte _c = (i < buildName.length()) ? (byte)buildName[i] : 0; if (EEPROM.read(500 + i) != _c) { EEPROM.write(500 + i, _c); _verChanged = true; } }
  if (_verChanged) EEPROM.commit();
  LoadPetStats();
  ACTION = IDLE;
  AMENUS = NA;
  EMOTION = NORMAL;
  BATTLESTATE = COMMENCED;
  MENUSTATE = TITLE;
  OPTIONSSETTINGS = PAUSE;
  TOOLTIP = RELEASED;
  CalculateMaxHP();
  CreateDeck(equippedDeck);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  tft.fillScreen(TFT_COLOR4);
  tft.pushImage(0, 0, 240, 135, titleScreen[0]);
  lastFarmUpdate = millis();
  //setTime(20, 52, 0, 1, 1, 1995);
  if(debug)
    Debug();
}

void loop() {
  if (Serial.available()) {
    String _cmd = Serial.readStringUntil('\n');
    _cmd.trim();
    if (_cmd == "VERSION") Serial.println(buildName);
  }
  //tft.drawString(String(randomEventPending), 0, 0);
  //playMelody();
  
  // Emergency shutdown: Hold A for 30 seconds
  if (AButton == "held") {
    if (emergencyShutdownStart == 0) {
      emergencyShutdownStart = millis();
    } else if (millis() - emergencyShutdownStart >= 30000) {
      // Wait for button to be released before sleeping
      while (digitalRead(35) == LOW) {
        delay(10);
      }
      delay(100);
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
      esp_deep_sleep_start();
    }
  } else {
    emergencyShutdownStart = 0;
  }
  
  LEDManager();
  ScreenOff();
  CheckRandomEvent();
  UpdateEventWalk();

  ButtonManager();
  AutoSave();
  CalculateHunger();
  CalculateStamina();
  CalculateMood();
  EmotionState();
  ShowTitleScreen();
  UpdateFarm();
  if (MENUSTATE != TITLE) {
    DrawScene();
    if (ACTION != INJURED)
      DisplayVentureTime();
    else
      DisplayInjuryTime();
  }
  if (MENUSTATE != BATTLE && MENUSTATE != SHOPMENU && MENUSTATE != TITLE && MENUSTATE != TUTORIAL
      && !(randomEventPending && (eventState == EVENT_PROMPT || eventState == EVENT_RESULT)))
    BatteryInfo();
}

#pragma region GameFunctions

void Debug(){
  EEPROM.write(141, 3);
  
  EEPROM.write(190, 1);
  for(int i = 0; i < 56; i++)
    bitSet(cardOwnership[i / 8], i % 8);
  //bitClear(bodyOwnership[9 / 8], 9 % 8);
  EEPROM.put(1, cardOwnership);
  /*
  for(int i = 0; i < 31; i++)
    bitSet(bodyOwnership[i / 8], i % 8);
  EEPROM.put(65, bodyOwnership);
  for(int i = 0; i < 51; i++)
    bitSet(headOwnership[i / 8], i % 8);
  EEPROM.put(33, headOwnership);
  //give all gems
  for(int i = 0; i < 11; i++)
    EEPROM.write(199 + i, 1);
  */
  playerCredits = 44;
  EEPROM.commit();
  //DEBUG - Full Stats
  HP = 1;
  Hunger = 100;
  Stamina = 100;
  Mood = 100;
  ACTION = IDLE;
  injuryTime = 0;
  SavePetStats();
}

int getTotalCards() {
  return sizeof(cardList) / sizeof(cardList[0]);
}

int getTotalBosses() {
  return sizeof(bossEnemy) / sizeof(bossEnemy[0]);
}

int getTotalHeadGear() {
  return sizeof(headGear) / sizeof(headGear[0]);
}

int getTotalBodyGear() {
  return sizeof(bodyGear) / sizeof(bodyGear[0]);
}

int getTotalGems() {
  return sizeof(gemList) / sizeof(gemList[0]);
}

int getTotalCounters() {
  return sizeof(counterList) / sizeof(counterList[0]);
}

bool HasUnownedCards() {
  int total = getTotalCards();
  byte* ownership = deepDungeonActive ? ddCardOwnership : cardOwnership;
  for (int i = 0; i < total; i++) {
    if (!bitRead(ownership[i / 8], i % 8)) return true;
  }
  return false;
}

bool HasUnownedHeadGear() {
  int total = getTotalHeadGear();
  for (int i = 0; i < total; i++) {
    if (!bitRead(headOwnership[i / 8], i % 8)) return true;
  }
  return false;
}

bool HasUnownedBodyGear() {
  int total = getTotalBodyGear();
  for (int i = 0; i < total; i++) {
    if (!bitRead(bodyOwnership[i / 8], i % 8)) return true;
  }
  return false;
}

bool HasUnownedGems() {
  int total = getTotalGems();
  for (int i = 0; i < total; i++) {
    if (EEPROM.read(199 + i) != 1) return true;
  }
  return false;
}

bool CanShowRareItem(int shopItemListIndex) {
  if (shopItemListIndex == 3) return HasUnownedCards();      // Card Pack
  if (shopItemListIndex == 4) return (HasUnownedHeadGear() || HasUnownedBodyGear());  // Gear Drop
  if (shopItemListIndex == 5) return HasUnownedGems();       // Random Gem
  return true;
}

// Grant card (index or -1 for random)
int GrantCard(int cardIndex) {
  int total = getTotalCards();
  
  if (cardIndex == -1) {
    int unowned = 0;
    for (int i = 0; i < total; i++) {
      if (!bitRead(cardOwnership[i / 8], i % 8)) unowned++;
    }
    if (unowned == 0) return -1;
    
    int pick = random(unowned);
    int current = 0;
    for (int i = 0; i < total; i++) {
      if (!bitRead(cardOwnership[i / 8], i % 8)) {
        if (current == pick) {
          cardIndex = i;
          break;
        }
        current++;
      }
    }
  }
  
  if (cardIndex < 0 || cardIndex >= total) return -1;
  if (bitRead(cardOwnership[cardIndex / 8], cardIndex % 8)) return -1;
  
  bitSet(cardOwnership[cardIndex / 8], cardIndex % 8);
  EEPROM.put(1, cardOwnership);
  //EEPROM.commit();
  return cardIndex;
}

// Grant head gear (index or -1 for random)
int GrantHeadGear(int gearIndex) {
  int total = getTotalHeadGear();
  
  if (gearIndex == -1) {
    int unowned = 0;
    for (int i = 0; i < total; i++) {
      if (!bitRead(headOwnership[i / 8], i % 8)) unowned++;
    }
    if (unowned == 0) return -1;
    
    int pick = random(unowned);
    int current = 0;
    for (int i = 0; i < total; i++) {
      if (!bitRead(headOwnership[i / 8], i % 8)) {
        if (current == pick) {
          gearIndex = i;
          break;
        }
        current++;
      }
    }
  }
  
  if (gearIndex < 0 || gearIndex >= total) return -1;
  if (bitRead(headOwnership[gearIndex / 8], gearIndex % 8)) return -1;
  
  bitSet(headOwnership[gearIndex / 8], gearIndex % 8);
  EEPROM.put(33, headOwnership);
  //EEPROM.commit();
  return gearIndex;
}

// Grant body gear (index or -1 for random)
int GrantBodyGear(int gearIndex) {
  int total = getTotalBodyGear();
  
  if (gearIndex == -1) {
    int unowned = 0;
    for (int i = 0; i < total; i++) {
      if (!bitRead(bodyOwnership[i / 8], i % 8)) unowned++;
    }
    if (unowned == 0) return -1;
    
    int pick = random(unowned);
    int current = 0;
    for (int i = 0; i < total; i++) {
      if (!bitRead(bodyOwnership[i / 8], i % 8)) {
        if (current == pick) {
          gearIndex = i;
          break;
        }
        current++;
      }
    }
  }
  
  if (gearIndex < 0 || gearIndex >= total) return -1;
  if (bitRead(bodyOwnership[gearIndex / 8], gearIndex % 8)) return -1;
  
  bitSet(bodyOwnership[gearIndex / 8], gearIndex % 8);
  EEPROM.put(65, bodyOwnership);
  EEPROM.commit();
  return gearIndex;
}

// Grant gem (index or -1 for random)
int GrantGem(int gemIndex) {
  int total = getTotalGems();
  
  if (gemIndex == -1) {
    int unowned = 0;
    for (int i = 0; i < total; i++) {
      if (EEPROM.read(199 + i) != 1) unowned++;
    }
    if (unowned == 0) return -1;
    
    int pick = random(unowned);
    int current = 0;
    for (int i = 0; i < total; i++) {
      if (EEPROM.read(199 + i) != 1) {
        if (current == pick) {
          gemIndex = i;
          break;
        }
        current++;
      }
    }
  }
  
  if (gemIndex < 0 || gemIndex >= total) return -1;
  if (EEPROM.read(199 + gemIndex) == 1) return -1;
  
  EEPROM.write(199 + gemIndex, 1);
  EEPROM.commit();
  return gemIndex;
}

// Grant DD card (index or -1 for random)
int GrantDDCard(int cardIndex) {
  int total = getTotalCards();
  
  if (cardIndex == -1) {
    int unowned = 0;
    for (int i = 0; i < total; i++) {
      if (!bitRead(ddCardOwnership[i / 8], i % 8)) unowned++;
    }
    if (unowned == 0) return -1;
    
    int pick = random(unowned);
    int current = 0;
    for (int i = 0; i < total; i++) {
      if (!bitRead(ddCardOwnership[i / 8], i % 8)) {
        if (current == pick) {
          cardIndex = i;
          break;
        }
        current++;
      }
    }
  }
  
  if (cardIndex < 0 || cardIndex >= total) return -1;
  if (bitRead(ddCardOwnership[cardIndex / 8], cardIndex % 8)) return -1;
  
  bitSet(ddCardOwnership[cardIndex / 8], cardIndex % 8);
  EEPROM.put(259, ddCardOwnership);
  return cardIndex;
}

// Initialize Deep Dungeon with starter cards and decks
void InitializeDeepDungeon() {
  deepDungeonActive = true;
  deepDungeonStage = 0;
  
  // Clear DD card ownership
  for (int i = 0; i < 32; i++) ddCardOwnership[i] = 0;
  
  // Drafted starting deck: 3 picks + guaranteed PIERCE (7) and BLOCK (12)
  int draftDeck[5] = { ddDraftPicks[0], ddDraftPicks[1], ddDraftPicks[2], 7, 12 };
  for (int i = 0; i < 5; i++) bitSet(ddCardOwnership[draftDeck[i] / 8], draftDeck[i] % 8);
  
  // Lay the deck into all three 10-slot DD deck slots (padded by repeat for now)
  for (int d = 0; d < 3; d++)
    for (int i = 0; i < 10; i++)
      EEPROM.write(291 + (d * 10) + i, draftDeck[i % 5]);
  
  // Set DD equipped deck to 0
  EEPROM.write(321, 0);
  
  // Save DD state
  EEPROM.write(255, deepDungeonUnlocked ? 1 : 0);
  EEPROM.write(256, 1); // deepDungeonActive
  EEPROM.put(257, deepDungeonStage);
  EEPROM.put(259, ddCardOwnership);
  ddReviveUsed = false;
  activeAfflictionCount = 0;   // runs start with no afflictions (GAMBLE / bosses add them)

  // GAMBLE gem: one extra random affliction for the run (cost of doubled drops)
  if (activeGem == 5 && activeAfflictionCount < GetAfflictionCap()) {
    int id;
    bool dup;
    do {
      id = random(DD_AFFLICTION_COUNT);
      dup = false;
      for (int i = 0; i < activeAfflictionCount; i++)
        if (activeAfflictions[i] == id) dup = true;
    } while (dup);
    activeAfflictions[activeAfflictionCount] = id;
    activeAfflictionLevels[activeAfflictionCount] = 1;
    activeAfflictionCount++;
  }

  // Persist the locked-in run state (gem, merits, afflictions): 344-374
  EEPROM.write(344, (byte)(activeGem & 0xFF));
  EEPROM.write(345, (byte)activeMeritCount);
  for (int i = 0; i < 9; i++) EEPROM.write(346 + i, (byte)(activeMerits[i] & 0xFF));
  EEPROM.write(355, (byte)activeAfflictionCount);
  for (int i = 0; i < 9; i++) EEPROM.write(356 + i, (byte)(activeAfflictions[i] & 0xFF));
  for (int i = 0; i < 9; i++) EEPROM.write(365 + i, (byte)activeAfflictionLevels[i]);
  EEPROM.write(374, (byte)(ddReviveUsed ? 1 : 0));
  EEPROM.commit();
}

// Reset Deep Dungeon (on exit or defeat)
void ResetDeepDungeon() {
  deepDungeonActive = false;
  deepDungeonStage = 0;
  randomEnemyGenerated = false;
  enemySpriteCreated = false;
  
  // Clear DD card ownership
  for (int i = 0; i < 32; i++) ddCardOwnership[i] = 0;
  
  // Wipe per-run gem + merits (cards/merits don't carry between runs)
  activeGem = -1;
  ddReviveUsed = false;
  activeMeritCount = 0;
  for (int i = 0; i < 9; i++) activeMerits[i] = -1;
  for (int i = 0; i < 9; i++) { activeAfflictions[i] = -1; activeAfflictionLevels[i] = 0; }
  activeAfflictionCount = 0;
  statDrill = false;
  statDrillSel = 0;
  ddGemsRolled = false;   // fresh gem roll for the next attempt
  ddMeritSelect = false;
  ddMeritDiscard = false;
  ddMeritConfirm = false;
  pendingMerit = -1;
  ddCardDraft = false;
  ddDraftRound = 0;
  for (int i = 0; i < 3; i++) ddDraftPicks[i] = -1;
  
  // Save DD state
  EEPROM.write(256, 0); // deepDungeonActive
  EEPROM.put(257, deepDungeonStage);
  EEPROM.put(259, ddCardOwnership);
  EEPROM.commit();
}

// Save Deep Dungeon progress
void SaveDeepDungeon() {
  EEPROM.put(253, (uint16_t)deepDungeonHighScore);
  EEPROM.write(255, deepDungeonUnlocked ? 1 : 0);
  EEPROM.write(256, deepDungeonActive ? 1 : 0);
  EEPROM.put(257, deepDungeonStage);
  EEPROM.put(259, ddCardOwnership);
  EEPROM.write(344, (byte)(activeGem & 0xFF));
  EEPROM.write(345, (byte)activeMeritCount);
  for (int i = 0; i < 9; i++) EEPROM.write(346 + i, (byte)(activeMerits[i] & 0xFF));
  EEPROM.write(355, (byte)activeAfflictionCount);
  for (int i = 0; i < 9; i++) EEPROM.write(356 + i, (byte)(activeAfflictions[i] & 0xFF));
  for (int i = 0; i < 9; i++) EEPROM.write(365 + i, (byte)activeAfflictionLevels[i]);
  EEPROM.write(374, (byte)(ddReviveUsed ? 1 : 0));
  EEPROM.commit();
}

// Create deck for DD mode
void CreateDDDeck(int deckNumber) {
  int baseAddr = 291 + (deckNumber * 10);
  for (int i = 0; i < 10; i++) {
    playerDeck[i] = cardList[EEPROM.read(baseAddr + i)];
  }
}

// Helper functions for mode-dependent EEPROM addresses
int getDeckBaseAddr() {
  return deepDungeonActive ? 291 : 97;
}

int getEquippedDeckAddr() {
  return deepDungeonActive ? 321 : 127;
}

// Check if player owns a card (mode-dependent)
bool ownsCard(int cardIndex) {
  if (deepDungeonActive) {
    return bitRead(ddCardOwnership[cardIndex / 8], cardIndex % 8);
  } else {
    return bitRead(cardOwnership[cardIndex / 8], cardIndex % 8);
  }
}

// Get equipped deck for current mode
int getEquippedDeck() {
  return EEPROM.read(getEquippedDeckAddr());
}

// Save equipped deck for current mode
void saveEquippedDeck(int deck) {
  EEPROM.write(getEquippedDeckAddr(), deck);
  EEPROM.commit();
}


void SavePetStats() {
  EEPROM.write(322, 0xAA);
  EEPROM.write(323, maxHP);
  EEPROM.write(324, HP);
  byte actionState = 0;
  if(ACTION == SLEEPING) actionState = 1;
  else if(ACTION == INJURED) actionState = 2;
  EEPROM.write(325, actionState);
  EEPROM.write(326, Hunger);
  EEPROM.write(327, Stamina);
  EEPROM.write(328, Mood);
  EEPROM.put(329, injuryTime);
  EEPROM.put(333, countdownSeconds);
  EEPROM.commit();
}

void LoadPetStats() {
  if(EEPROM.read(322) == 0xAA) {
    maxHP = EEPROM.read(323);
    HP = EEPROM.read(324);
    byte actionState = EEPROM.read(325);
    if(actionState == 1) {
      ACTION = SLEEPING;
      sleepStartTime = millis();  // Reset timer on load
    }
    else if(actionState == 2) ACTION = INJURED;
    else ACTION = IDLE;
    Hunger = EEPROM.read(326);
    Stamina = EEPROM.read(327);
    Mood = EEPROM.read(328);
    EEPROM.get(329, injuryTime);
    EEPROM.get(333, countdownSeconds);
    
    // Safety: Fix corrupted injury state
    if (ACTION == INJURED && injuryTime == 0) {
      injuryTime = 3600;
      countdownSeconds = 0;
    }
    // Safety: If injured but HP > 0, clear injury state
    if (ACTION == INJURED && HP > 0) {
      ACTION = IDLE;
      injuryTime = 0;
      countdownSeconds = 0;
    }
    // Safety: Clear stale injury/venture timers if not actively injured/venturing
    // This fixes A button lockout after force reset during battle
    if (ACTION != INJURED) {
      injuryTime = 0;
    }
    // VENTURESTATE isn't persisted, so clear venture countdown on load
    // (ventureTime is 0 on boot, so countdownSeconds is orphaned data)
    if (ventureTime == 0) {
      countdownSeconds = 0;
    }
  }
}

void AutoSave() {
  if(millis() - lastSaveTime >= 60000) {
    SavePetStats();
    lastSaveTime = millis();
  }
}

void CalculateMaxHP() {
  int baseMaxHP = 25;  // Base HP without level scaling
  maxHP = ceil(baseMaxHP * GetGemHPMult());
  if (HP > maxHP) HP = maxHP;
}

void CalculateHunger() {
  if (gamePause)
    return;
  static unsigned long lastTime = 0;
  if (micros() - lastTime >= 1000000) {  // check if one second has elapsed
    lastTime = micros();
    hungerCycles++;
    hungerRegenCycles++;
  }
  //Drain hunger points if awake
  if (hungerCycles >= 216 && ACTION != SLEEPING) {  //checks every 216 seconds
    if (Hunger > 0) {
      Hunger = Hunger - 1;
      hungerCycles = 0;
    } else {  //Decrease Health by 5% when Pico is starving and Hunger = 0
      if (HP > 1 && MENUSTATE != BATTLE) {
        // Calculate 5% of maxHP
        int regenHealth = (int)ceil(0.05 * maxHP);
        HP = HP - regenHealth;
        if (HP < 1)
          HP = 1;
        SavePetStats();
        if (MENUSTATE == MAINMENU) {
          //HEALTH BAR
          tft.fillRect(27, 3, 63, 18, TFT_COLOR4);
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
            if (result == 0 && HP > 0)
              result = 1;
            tft.fillRect(27, 6, (result * 3), 12, TFT_COLOR2);
          }
        }
      }
      hungerCycles = 0;
    }
  }
  //Drain hunger points slower if asleep
  if (hungerCycles >= 1296 && ACTION == SLEEPING) {  //checks every 1296 seconds
    if (Hunger > 0) {
      Hunger = Hunger - 1;
      hungerCycles = 0;
    } else {  //Decrease Health by 5% when Pico is starving and Hunger = 0
      if (HP > 1 && MENUSTATE != BATTLE) {
        // Calculate 5% of maxHP
        int regenHealth = (int)ceil(0.05 * maxHP);
        HP = HP - regenHealth;
        if (HP < 1)
          HP = 1;
        if (MENUSTATE == MAINMENU) {
          //HEALTH BAR
          tft.fillRect(27, 3, 63, 18, TFT_COLOR4);
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
      hungerCycles = 0;
    }
  }
  //Regenerate HP when not in battle every X minutes if alive
  if (hungerRegenCycles >= 300 && MENUSTATE != BATTLE && Hunger > 25 && HP > 0) {  //checks every 5 minutes
    int regenAmt = 1;
    if (deepDungeonActive && activeGem == 7) regenAmt = 2;  // MARATHON: 2x passive regen
    if (deepDungeonActive && activeGem == 4) regenAmt = 0;  // GLASS: no passive regen
    if (HP < maxHP && regenAmt > 0) {
      HP = HP + regenAmt;
      if (HP > maxHP) HP = maxHP;
      if (MENUSTATE == MAINMENU && eventState != EVENT_PROMPT && eventState != EVENT_RESULT) {
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
      }
    }
    hungerRegenCycles = 0;
  }
  if(Hunger > 90){
    Defense = ceil((baseDefense + (baseDefense * 0.25)) * GetGemDefMult());
  } else if(Hunger <= 90 && Hunger > 70){
    Defense = ceil((baseDefense + (baseDefense * 0.1)) * GetGemDefMult());
  } else if(Hunger <= 70 && Hunger > 50){
    Defense = ceil(baseDefense * GetGemDefMult());
  } else if(Hunger <= 50 && Hunger > 30){
    Defense = ceil((baseDefense - (baseDefense * 0.1)) * GetGemDefMult());
  } else if(Hunger <= 30){
    Defense = ceil((baseDefense - (baseDefense * 0.25)) * GetGemDefMult());
  }
  ClampStats();
}

void CalculateStamina() {
  if (gamePause)
    return;
  static unsigned long lastTime = 0;
  if (micros() - lastTime >= 1000000) {  // check if one second has elapsed
    lastTime = micros();
    staminaCycles++;
  }
  //Drain stamina if awake
  if (staminaCycles >= 864 && ACTION != SLEEPING) {  //checks every 864 seconds
    if (Stamina > 0) {
      Stamina = Stamina - 1;
    } else {  //Decrease Mood if Stamina hits 0
      if (Mood > 0)
        Mood = Mood - 1;
    }
    staminaCycles = 0;
  }
  //Regain stamina if asleep
  if (staminaCycles >= 288 && ACTION == SLEEPING) {  //checks every 288 seconds
    if (Stamina < 100) {
      Stamina = Stamina + 1;
    }
    staminaCycles = 0;
  }
  if(Stamina > 90){
    Speed = ceil((baseSpeed + (baseSpeed * 0.25)) * GetGemSpdMult());
  } else if(Stamina <= 90 && Stamina > 70){
    Speed = ceil((baseSpeed + (baseSpeed * 0.1)) * GetGemSpdMult());
  } else if(Stamina <= 70 && Stamina > 50){
    Speed = ceil(baseSpeed * GetGemSpdMult());
  } else if(Stamina <= 50 && Stamina > 30){
    Speed = ceil((baseSpeed - (baseSpeed * 0.1)) * GetGemSpdMult());
  } else if(Stamina <= 30){
    Speed = ceil((baseSpeed - (baseSpeed * 0.25)) * GetGemSpdMult());
  }
  ClampStats();
}

void CalculateMood() {
  if (gamePause)
    return;
  static unsigned long lastTime = 0;
  if (micros() - lastTime >= 1000000) {  // check if one second has elapsed
    lastTime = micros();
    moodCycles++;
  }

  //Greatly increase Mood if out of Venture and awake
  if (moodCycles >= 288 && ACTION != SLEEPING && MENUSTATE != BATTLE && VENTURESTATE == INACTIVE) {  //checks every 288 seconds
    if (EMOTION != HUNGRY && EMOTION != TIRED && EMOTION != H_T) {                                   //Also checks to ensure Pico is not hungry and/or tired
      if (Mood < 100) {
        Mood = Mood + 3;
      } else if (Mood > 100)
        Mood = 100;
    }
    moodCycles = 0;
  }

  //Increase Mood if out of Venture and asleep
  if (moodCycles >= 288 && ACTION == SLEEPING && MENUSTATE != BATTLE && VENTURESTATE == INACTIVE) {  //checks every 288 seconds
    if (EMOTION != HUNGRY && EMOTION != TIRED && EMOTION != H_T) {
      if (Mood < 100) {
        Mood = Mood + 1;
      } else if (Mood > 100)
        Mood = 100;
    }
    moodCycles = 0;
  }

  //Decrease Mood if Venturing
  if (moodCycles >= 288 && ACTION != SLEEPING && MENUSTATE != BATTLE && VENTURESTATE == VENTURING) {  //checks every 288 seconds
    if (Mood > 0) {
      Mood = Mood - 1;
    }
    moodCycles = 0;
  }

  if(Mood > 90){
    Attack = ceil((baseAttack + (baseAttack * 0.25)) * GetGemAtkMult());;
  } else if(Mood <= 90 && Mood > 70){
    Attack = ceil((baseAttack + (baseAttack * 0.1)) * GetGemAtkMult());;
  } else if(Mood <= 70 && Mood > 50){
    Attack = ceil(baseAttack * GetGemAtkMult());;
  } else if(Mood <= 50 && Mood > 30){
    Attack = ceil((baseAttack - (baseAttack * 0.1)) * GetGemAtkMult());;
  } else if(Mood <= 30){
    Attack = ceil((baseAttack - (baseAttack * 0.25)) * GetGemAtkMult());;
  }
  ClampStats();
}

void EmotionState() {
  if (Hunger <= 30 && Stamina > 30)
    EMOTION = HUNGRY;
  else if (Hunger > 30 && Stamina <= 30)
    EMOTION = TIRED;
  else if (Hunger <= 30 && Stamina <= 30)
    EMOTION = H_T;
  else
    EMOTION = NORMAL;

  if (EMOTION != HUNGRY && EMOTION != TIRED && EMOTION != H_T) {
    if (Mood >= 90)
      EMOTION = HAPPY;
    else if (Mood < 90 && Mood >= 70)
      EMOTION = NORMAL;
    else if (Mood < 70 && Mood >= 50)
      EMOTION = NORMAL;
    else if (Mood < 50 && Mood >= 30)
      EMOTION = UNHAPPY;
    else
      EMOTION = ANGRY;
  }
  if (VENTURESTATE == VENTURING && countdownSeconds == 0)
    EMOTION = ALERT;
  if (randomEventPending && eventState != EVENT_RESULT)
    EMOTION = ALERT;
  else if (randomEventPending && eventState == EVENT_RESULT){
    if(eventCharacterAction == WAKE)
      EMOTION = HAPPY;
    else
      EMOTION = NORMAL;
  }
  if (HP == 0 && countdownSeconds != 0)
    EMOTION = SKULL;
  if (HP == 0 && countdownSeconds == 0)
    EMOTION = REVIVE;
}

float GetGemAtkMult() {
  if (deepDungeonActive) {
    float m = 1.0;
    if (activeGem == 1) m *= 1.25;  // BLOODLUST
    if (activeGem == 4) m *= 1.30;  // GLASS
    if (HasMerit(MERIT_BOLSTER)) m *= 1.10;
    if (HasMerit(MERIT_TEMPER)) m *= 1.25;
    if (HasMerit(MERIT_SCHOLAR)) m *= (1.0 + min(25, GetDDCardCount()) / 100.0);
    return m;
  }
  int gem = (equippedDeck == 0) ? EEPROM.read(128) : (equippedDeck == 1) ? EEPROM.read(129) : EEPROM.read(130);
  if (gem == 1) return 1.1;   // ATK Gem
  if (gem == 7) return 1.25;  // Blood Gem
  if (gem == 8) return 0.75;  // Armor Gem
  return 1.0;
}

float GetGemDefMult() {
  if (deepDungeonActive) {
    float m = 1.0;
    if (activeGem == 0) m *= 0.75;  // PROSPECT
    if (HasMerit(MERIT_BOLSTER)) m *= 1.10;
    if (HasMerit(MERIT_TEMPER)) m *= 1.25;
    if (HasMerit(MERIT_SCHOLAR)) m *= (1.0 + min(25, GetDDCardCount()) / 100.0);
    return m;
  }
  int gem = (equippedDeck == 0) ? EEPROM.read(128) : (equippedDeck == 1) ? EEPROM.read(129) : EEPROM.read(130);
  if (gem == 2) return 1.1;   // DEF Gem
  if (gem == 5 || gem == 6 || gem == 9) return 0.75;  // Pricey, Rare, Swift
  if (gem == 8) return 1.25;  // Armor Gem
  return 1.0;
}

float GetGemSpdMult() {
  if (deepDungeonActive) {
    float m = 1.0;
    if (HasMerit(MERIT_HASTE)) m *= 1.20;
    if (HasMerit(MERIT_SCHOLAR)) m *= (1.0 + min(25, GetDDCardCount()) / 100.0);
    return m;
  }
  int gem = (equippedDeck == 0) ? EEPROM.read(128) : (equippedDeck == 1) ? EEPROM.read(129) : EEPROM.read(130);
  return (gem == 3) ? 1.1 : 1.0;  // SPD Gem
}

float GetGemHPMult() {
  if (deepDungeonActive) {
    float m = 1.0;
    if (activeGem == 1) m *= 0.67;  // BLOODLUST
    if (HasMerit(MERIT_VITALITY)) m *= 1.25;
    return m;
  }
  int gem = (equippedDeck == 0) ? EEPROM.read(128) : (equippedDeck == 1) ? EEPROM.read(129) : EEPROM.read(130);
  if (gem == 4) return 1.1;   // HP Gem
  if (gem == 7) return 0.67;  // Blood Gem
  if (gem == 10) return 1.25; // Vital Gem
  return 1.0;
}

// DD merit / affliction ceilings. OVERLOAD (gem 6) raises both to 9; base is 6.
// The boss-reward grant system reads these when handing out merits/afflictions.
int GetMeritCap() {
  return (deepDungeonActive && activeGem == 6) ? 9 : 6;
}
int GetAfflictionCap() {
  return (deepDungeonActive && activeGem == 6) ? 9 : 6;
}

// Level of an active DD affliction (0 if not present). Effects scale with this.
int GetAfflictionLevel(int id) {
  if (!deepDungeonActive) return 0;
  for (int i = 0; i < activeAfflictionCount; i++)
    if (activeAfflictions[i] == id) return activeAfflictionLevels[i];
  return 0;
}

// True if the given DD merit is active this run.
bool HasMerit(int id) {
  if (!deepDungeonActive) return false;
  for (int i = 0; i < activeMeritCount; i++)
    if (activeMerits[i] == id) return true;
  return false;
}

// Number of cards owned in the DD collection (used by SCHOLAR).
int GetDDCardCount() {
  int n = 0;
  for (int i = 0; i < 56; i++)
    if (bitRead(ddCardOwnership[i / 8], i % 8)) n++;
  return n;
}

void ButtonManager() {
  int newButton1State = digitalRead(35);
  int newButton2State = digitalRead(0);
  unsigned long now = millis();

  // Setting lastButtonPressTime to (now - 1000) makes the consumer cooldown
  // checks (millis() - lastButtonPressTime > 500 / > 250) pass on this frame,
  // letting one action through. The consumer then resets it to now, which
  // "uses up" the event until ButtonManager opens the gate again.

  // ---- Button A (GPIO35) ----
  if (newButton1State == LOW && button1State == HIGH) {            // press down
    if (now - button1PressTime > DEBOUNCE_TIME) {
      button1PressTime = now;
      button1RepeatTime = now;
    }
    AButton = "";                                                 // wait for release (tap) or hold
  } else if (newButton1State == LOW && button1State == LOW) {      // being held
    if (now - button1PressTime > BUTTON_HOLD_TIME) {
      AButton = "held";
      if (now - button1RepeatTime >= BUTTON_REPEAT_RATE) {         // auto-repeat tick
        button1RepeatTime = now;
        lastButtonPressTime = now - 1000;
      }
    } else {
      AButton = "";                                               // pressed, not long enough to be a hold yet
    }
  } else if (newButton1State == HIGH && button1State == LOW) {     // release
    AButton = (now - button1PressTime <= BUTTON_HOLD_TIME) ? "pressed" : "released";
    lastButtonPressTime = now - 1000;                             // fire the tap/release immediately
  } else {                                                        // idle (up)
    AButton = "released";
  }

  // ---- Button B (GPIO0) ----
  if (newButton2State == LOW && button2State == HIGH) {
    if (now - button2PressTime > DEBOUNCE_TIME) {
      button2PressTime = now;
      button2RepeatTime = now;
    }
    BButton = "";
  } else if (newButton2State == LOW && button2State == LOW) {
    if (now - button2PressTime > BUTTON_HOLD_TIME) {
      BButton = "held";
      if (now - button2RepeatTime >= BUTTON_REPEAT_RATE) {
        button2RepeatTime = now;
        lastButtonPressTime = now - 1000;
      }
    } else {
      BButton = "";
    }
  } else if (newButton2State == HIGH && button2State == LOW) {
    BButton = (now - button2PressTime <= BUTTON_HOLD_TIME) ? "pressed" : "released";
    lastButtonPressTime = now - 1000;
  } else {
    BButton = "released";
  }

  // Update the button states
  button1State = newButton1State;
  button2State = newButton2State;
}

void CenterText(String text, int yCord) {
  int displayWidth = tft.width();  // Get the display width
  int textWidth = tft.textWidth(text);
  int centerX = (displayWidth - textWidth) / 2;
  tft.drawString(text, centerX, yCord);
}




void ClampStats() {
  if (HP > maxHP) HP = maxHP;
  if (HP < 0) HP = 0;
  if (Hunger > 100) Hunger = 100;
  if (Hunger < 0) Hunger = 0;
  if (Stamina > 100) Stamina = 100;
  if (Stamina < 0) Stamina = 0;
  if (Mood > 100) Mood = 100;
  if (Mood < 0) Mood = 0;
}

void ManageInventory() {
  int q = 0;
  for (int i = 0; i < 10; i++) {
    inventoryQty[i] = 0;
    inventoryAdr[i] = 0;
    int val = EEPROM.read(143 + i);
    if (val != 0 && val != 255) {
      inventoryQty[q] = val;
      inventoryAdr[q] = 143 + i;
      q++;
    }
  }
}

void ManageGemInventory() {
  //FOR TESTING ONLY
  if (EEPROM.read(199) == 255 || EEPROM.read(199) == 0 || EEPROM.read(200) == 0 || EEPROM.read(201) == 0) {
    EEPROM.write(199, 6);
    EEPROM.write(200, 4);
    EEPROM.write(201, 2);
    EEPROM.commit();
  }
  int q = 0;
  for (int i = 0; i < 10; i++) {
    inventoryQty[q] = 0;
    inventoryAdr[q] = 0;
    int val = EEPROM.read(199 + i);
    if (val != 0 && val != 255) {
      inventoryQty[q] = val;
      inventoryAdr[q] = 199 + i;
      q++;
    }
  }

  //tft.fillScreen(TFT_COLOR4);
  tft.fillRect(0, 60, 240, 75, TFT_COLOR4);
  tft.fillRect(0, 0, 24, 6, TFT_COLOR4);
  tft.fillRect(0, 0, 6, 24, TFT_COLOR4);
  //tft.fillRect(0, 0, 54, 54, TFT_COLOR4);
  tft.fillRect(51, 3, 9, 30, TFT_COLOR4);
  tft.fillRect(57, 24, 3, 24, TFT_COLOR4);
  drawingBatteryIcon(batteryAnim);
  for (int i = 0; i < 10; i++) {
    pushScaled(21 + (i * 21), (39 * 3), 5, 5, smallIcons[10]);
    if (inventoryQty[i] != 0 && inventoryQty[i] != 255) {
      if (i == invSelect) {
        tft.fillRect(21 + (i * 21), (39 * 3), (5 * 3), (5 * 3), TFT_COLOR3);
        tft.fillRect(18 + (i * 21), (38 * 3), (5 * 3), (5 * 3), TFT_COLOR1);
      } else {
        tft.fillRect(21 + (i * 21), (39 * 3), (5 * 3), (5 * 3), TFT_COLOR1);
        tft.fillRect(18 + (i * 21), (38 * 3), (5 * 3), (5 * 3), TFT_COLOR3);
      }
    }
  }
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.fillRect(63, 54, 126, 3, TFT_COLOR1);
  tft.fillRect(189, 6, 3, 51, TFT_COLOR1);
  tft.fillRect(60, 3, 129, 51, TFT_COLOR3);
  switch (inventoryAdr[invSelect]) {
    case 199:  // ATK GEM
      tft.drawString(String("ATK"), 63, 6);
      tft.drawString(String("GEM"), 63, 30);
      pushScaled(6, 6, 16, 16, gems[0]);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      tft.drawString("x" + String(inventoryQty[invSelect]), 68 * 3, 10 * 3);
      tft.drawString(String("+10% ATK"), 6, 63);
      tft.drawString(String(""), 6, 87);
      break;
    case 200:  // DEF GEM
      tft.drawString(String("DEF"), 63, 6);
      tft.drawString(String("GEM"), 63, 30);
      pushScaled(6, 6, 16, 16, gems[1]);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      tft.drawString("x" + String(inventoryQty[invSelect]), 68 * 3, 10 * 3);
      tft.drawString(String("+10% DEF"), 6, 63);
      tft.drawString(String(""), 6, 87);
      break;
    case 201:  // SPD GEM
      tft.drawString(String("SPD"), 63, 6);
      tft.drawString(String("GEM"), 63, 30);
      pushScaled(6, 6, 16, 16, gems[2]);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      tft.drawString("x" + String(inventoryQty[invSelect]), 68 * 3, 10 * 3);
      tft.drawString(String("+10% SPD"), 6, 63);
      tft.drawString(String(""), 6, 87);
      break;
    case 202:  // HP GEM
      tft.drawString(String("HP"), 63, 6);
      tft.drawString(String("GEM"), 63, 30);
      pushScaled(6, 6, 16, 16, gems[2]);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      tft.drawString("x" + String(inventoryQty[invSelect]), 68 * 3, 10 * 3);
      tft.drawString(String("+10% MAX HP"), 6, 63);
      tft.drawString(String(""), 6, 87);
      break;
    case 203:  // PRICEY GEM
      tft.drawString(String("PRICEY"), 63, 6);
      tft.drawString(String("GEM"), 63, 30);
      pushScaled(6, 6, 16, 16, gems[2]);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      tft.drawString("x" + String(inventoryQty[invSelect]), 68 * 3, 10 * 3);
      tft.drawString(String("-25% DEF"), 6, 63);
      tft.drawString(String("+50% MONEY"), 6, 87);
      break;
    case 204:  // RARE GEM
      tft.drawString(String("RARE"), 63, 6);
      tft.drawString(String("GEM"), 63, 30);
      pushScaled(6, 6, 16, 16, gems[2]);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      tft.drawString("x" + String(inventoryQty[invSelect]), 68 * 3, 10 * 3);
      tft.drawString(String("-25% DEF +12%"), 6, 63);
      tft.drawString(String("DROP UPGRADE"), 6, 87);
      break;
    case 205:  // VITAL GEM
      tft.drawString(String("VITAL"), 63, 6);
      tft.drawString(String("GEM"), 63, 30);
      pushScaled(6, 6, 16, 16, gems[2]);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      tft.drawString("x" + String(inventoryQty[invSelect]), 68 * 3, 10 * 3);
      tft.drawString(String("+25% MAX HP"), 6, 63);
      tft.drawString(String("NO PRIORITY"), 6, 87);
      break;
    case 206:  // BLOOD GEM
      tft.drawString(String("BLOOD"), 63, 6);
      tft.drawString(String("GEM"), 63, 30);
      pushScaled(6, 6, 16, 16, gems[2]);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      tft.drawString("x" + String(inventoryQty[invSelect]), 68 * 3, 10 * 3);
      tft.drawString(String("-33% MAX HP"), 6, 63);
      tft.drawString(String("+25% ATK"), 6, 87);
      break;
    case 207:  // ARMOR GEM
      tft.drawString(String("ARMOR"), 63, 6);
      tft.drawString(String("GEM"), 63, 30);
      pushScaled(6, 6, 16, 16, gems[2]);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      tft.drawString("x" + String(inventoryQty[invSelect]), 68 * 3, 10 * 3);
      tft.drawString(String("+25% DEF"), 6, 63);
      tft.drawString(String("-25% ATK"), 6, 87);
      break;
    case 208:  // SWIFT GEM
      tft.drawString(String("SWIFT"), 63, 6);
      tft.drawString(String("GEM"), 63, 30);
      pushScaled(6, 6, 16, 16, gems[2]);
      tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
      tft.drawString("x" + String(inventoryQty[invSelect]), 68 * 3, 10 * 3);
      tft.drawString(String("0DMG+PRIORITY"), 6, 63);
      tft.drawString(String("-25% DEF"), 6, 87);
      break;
  }
}

bool IsInventoryFull(int itemAddress) {
  for(int i = 0; i < 10; i++) {
    if(inventoryAdr[i] == itemAddress) {
      return inventoryQty[i] >= 9;
    }
  }
  return false;
}

void ApplyBacklight() {
  if (backlightPercent < 0) backlightPercent = 0;
  if (backlightPercent > 100) backlightPercent = 100;
  screenBrightness = (backlightPercent * 255) / 100;
  ledcWrite(0, screenBrightness);
}

void ApplyLed() {
  if (ledPercent < 0) ledPercent = 0;
  if (ledPercent > 100) ledPercent = 100;
  ledBrightness = (ledPercent * 51) / 100;  // 51 = PWM at 100%; lower this to dim further
  strip.setBrightness(ledBrightness);
}

void UseItem() {
  eatSkipReady = false;  // the press that confirms the item must not also skip its animation
  skipCheerStage = 0;
  switch (inventoryAdr[(menuItem - 1) + ((pageNum - 1) * 3)]) {
    case 143:  // SWEET FRUIT
      if (Hunger < 100 && EMOTION != SKULL && EMOTION != REVIVE) {
        Hunger = Hunger + 40;
        if (Hunger > 100)
          Hunger = 100;
        hungerCycles = 0;
        hungerRegenCycles = 0;
        itemFrames = 0;
        playerFrames = 3;
        animFrames = 0;
        ACTION = EATING;
        inventoryQty[invSelect] = inventoryQty[invSelect] - 1;
        EEPROM.write(inventoryAdr[invSelect], inventoryQty[invSelect]);
        EEPROM.commit();
      } else {
        animFrames = 0;
        ACTION = REJECT;
      }
      MENUSTATE = MAINMENU;
      AMENUS = NA;
      tft.fillScreen(TFT_COLOR4);
      DrawBackground();
      DrawMainMenu();
      break;
    case 144:  // ENERGY DRINK
      if (Stamina < 100 && EMOTION != SKULL && EMOTION != REVIVE) {
        Stamina = Stamina + 40;
        if (Stamina > 100)
          Stamina = 100;
        staminaCycles = 0;
        itemFrames = 3;
        playerFrames = 3;
        animFrames = 0;
        ACTION = EATING;
        inventoryQty[invSelect] = inventoryQty[invSelect] - 1;
        EEPROM.write(inventoryAdr[invSelect], inventoryQty[invSelect]);
        EEPROM.commit();
      } else {
        animFrames = 0;
        ACTION = REJECT;
      }
      MENUSTATE = MAINMENU;
      AMENUS = NA;
      tft.fillScreen(TFT_COLOR4);
      DrawBackground();
      DrawMainMenu();
      break;
    case 145:  // HEALTH POTION
      if (HP < maxHP && EMOTION != SKULL && EMOTION != REVIVE) {
        HP = HP + (maxHP / 2);
        if (HP > maxHP)
          HP = maxHP;
        itemFrames = 6;
        playerFrames = 3;
        animFrames = 0;
        ACTION = EATING;
        inventoryQty[invSelect] = inventoryQty[invSelect] - 1;
        EEPROM.write(inventoryAdr[invSelect], inventoryQty[invSelect]);
        EEPROM.commit();
      } else {
        animFrames = 0;
        ACTION = REJECT;
      }
      MENUSTATE = MAINMENU;
      AMENUS = NA;
      tft.fillScreen(TFT_COLOR4);
      DrawBackground();
      DrawMainMenu();
      break;
    case 146:  // MED KIT
      if (HP == 0 && (EMOTION == SKULL || EMOTION == REVIVE)) {
        itemFrames = 9;
        playerFrames = 3;
        animFrames = 0;
        HP = maxHP;
        injuryTime = 0;
        countdownSeconds = 0;
        ACTION = EATING;
        EMOTION = NORMAL;
        inventoryQty[invSelect] = inventoryQty[invSelect] - 1;
        EEPROM.write(inventoryAdr[invSelect], inventoryQty[invSelect]);
        EEPROM.commit();
        SavePetStats();
      } else {
        animFrames = 0;
        ACTION = REJECT;
      }
      MENUSTATE = MAINMENU;
      AMENUS = NA;
      tft.fillScreen(TFT_COLOR4);
      DrawBackground();
      DrawMainMenu();
      break;
    case 147:  // TASTY CAKE
      if (Mood < 100 && EMOTION != SKULL && EMOTION != REVIVE) {
        Mood = Mood + 40;
        if (Mood > 100)
          Mood = 100;
        moodCycles = 0;
        if (Hunger < 100 && EMOTION != SKULL && EMOTION != REVIVE) {
          Hunger = Hunger + 10;
          if (Hunger > 100)
            Hunger = 100;
          hungerCycles = 0;
          hungerRegenCycles = 0;
        }
        itemFrames = 12;
        playerFrames = 3;
        animFrames = 0;
        ACTION = EATING;
        inventoryQty[invSelect] = inventoryQty[invSelect] - 1;
        EEPROM.write(inventoryAdr[invSelect], inventoryQty[invSelect]);
        EEPROM.commit();
      } else {
        animFrames = 0;
        ACTION = REJECT;
      }
      MENUSTATE = MAINMENU;
      AMENUS = NA;
      tft.fillScreen(TFT_COLOR4);
      DrawBackground();
      DrawMainMenu();
      break;
    case 148:  // SCRY GLASS
      if (!scryGlassActive) {
        scryGlassActive = true;
        itemFrames = 15;
        playerFrames = 3;
        animFrames = 0;
        ACTION = EATING;
        inventoryQty[invSelect] = inventoryQty[invSelect] - 1;
        EEPROM.write(inventoryAdr[invSelect], inventoryQty[invSelect]);
        EEPROM.commit();
      } else {
        animFrames = 0;
        ACTION = REJECT;
      }
      MENUSTATE = MAINMENU;
      AMENUS = NA;
      tft.fillScreen(TFT_COLOR4);
      DrawBackground();
      DrawMainMenu();
      break;
    case 149:  // FLOOR MAP
      // If in venture and ventureTime > 1, skip to 1
      if (VENTURESTATE == VENTURING && countdownSeconds > 1) {
        countdownSeconds = 1;
        itemFrames = 18;
        playerFrames = 3;
        animFrames = 0;
        ACTION = EATING;
        inventoryQty[invSelect] = inventoryQty[invSelect] - 1;
        EEPROM.write(inventoryAdr[invSelect], inventoryQty[invSelect]);
        EEPROM.commit();
      } 
      // If not in venture, set flag for next venture
      else if (VENTURESTATE == INACTIVE && !floorMapActive) {
        floorMapActive = true;
        itemFrames = 18;
        playerFrames = 3;
        animFrames = 0;
        ACTION = EATING;
        inventoryQty[invSelect] = inventoryQty[invSelect] - 1;
        EEPROM.write(inventoryAdr[invSelect], inventoryQty[invSelect]);
        EEPROM.commit();
      } else {
        animFrames = 0;
        ACTION = REJECT;
      }
      MENUSTATE = MAINMENU;
      AMENUS = NA;
      tft.fillScreen(TFT_COLOR4);
      DrawBackground();
      DrawMainMenu();
      break;
  }
}

void sortCardsByName(SortedCard arr[], int size) {
  for (int i = 0; i < size - 1; i++) {
    for (int j = 0; j < size - i - 1; j++) {
      if (arr[j].name > arr[j + 1].name) {
        SortedCard temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
    }
  }
}

void CompileListMenu(String menu) {
  int displayed = 0;
  if (menu == "head") {
    if (menuIndex == 0 || empty)
      pageNum = 0;
    pageNum++;
    tft.fillRect(3, 51, 39, 27, TFT_COLOR4);
    if (pageNum < 10)
      tft.drawString("0" + String(pageNum), 6, 54);
    else
      tft.drawString(String(pageNum), 6, 54);
    tft.fillRect(54, 54, 33, 33, TFT_COLOR4);
    tft.fillRect(102, 54, 33, 33, TFT_COLOR4);
    tft.fillRect(150, 54, 33, 33, TFT_COLOR4);
    tft.fillRect(198, 54, 33, 33, TFT_COLOR4);

    for (int i = 0; i < 4; i++) {
      menuValues[i] = -1;
    }

    SortedCard sortedHead[256];
    int totalHead = 0;
    // Add NONE first if owned
    if (bitRead(headOwnership[0], 0)) {
      sortedHead[totalHead].index = 0;
      sortedHead[totalHead].name = headNames[0];
      totalHead++;
    }
    // Add rest and sort alphabetically
    int startSort = totalHead;
    for (int i = 1; i < getTotalHeadGear(); i++) {
      if (bitRead(headOwnership[i / 8], i % 8)) {
        sortedHead[totalHead].index = i;
        sortedHead[totalHead].name = headNames[i];
        totalHead++;
      }
    }
    sortCardsByName(&sortedHead[startSort], totalHead - startSort);

    for (; menuIndex < totalHead && displayed < 4; menuIndex++) {
      menuValues[displayed] = sortedHead[menuIndex].index;
      pushScaled(54 + (displayed * 48), 54, 11, 11, headGearIcons[sortedHead[menuIndex].index], TFT_BLACK);
      displayed++;
    }
    if (displayed < 4 || menuIndex >= totalHead) {
      maxItems = displayed;
      for (int i = displayed; i < 4; i++) {
        menuValues[i] = -1;
        tft.fillRect(54 + (i * 48), 54, 33, 33, TFT_COLOR4);
      }
      menuIndex = 0;
    } else
      maxItems = 4;
  } else if (menu == "body") {
    if (menuIndex == 0 || empty)
      pageNum = 0;
    pageNum++;
    tft.fillRect(3, 51, 39, 27, TFT_COLOR4);
    if (pageNum < 10)
      tft.drawString("0" + String(pageNum), 6, 54);
    else
      tft.drawString(String(pageNum), 6, 54);
    tft.fillRect(54, 54, 33, 33, TFT_COLOR4);
    tft.fillRect(102, 54, 33, 33, TFT_COLOR4);
    tft.fillRect(150, 54, 33, 33, TFT_COLOR4);
    tft.fillRect(198, 54, 33, 33, TFT_COLOR4);

    for (int i = 0; i < 4; i++) {
      menuValues[i] = -1;
    }

    SortedCard sortedBody[256];
    int totalBody = 0;
    // Add NONE first if owned
    if (bitRead(bodyOwnership[0], 0)) {
      sortedBody[totalBody].index = 0;
      sortedBody[totalBody].name = bodyNames[0];
      totalBody++;
    }
    // Add rest and sort alphabetically
    int startSort = totalBody;
    for (int i = 1; i < getTotalBodyGear(); i++) {
      if (bitRead(bodyOwnership[i / 8], i % 8)) {
        sortedBody[totalBody].index = i;
        sortedBody[totalBody].name = bodyNames[i];
        totalBody++;
      }
    }
    sortCardsByName(&sortedBody[startSort], totalBody - startSort);

    for (; menuIndex < totalBody && displayed < 4; menuIndex++) {
      menuValues[displayed] = sortedBody[menuIndex].index;
      pushScaled(54 + (displayed * 48), 54, 11, 11, bodyGearIcons[sortedBody[menuIndex].index], TFT_BLACK);
      displayed++;
    }
    if (displayed < 4 || menuIndex >= totalBody) {
      maxItems = displayed;
      for (int i = displayed; i < 4; i++) {
        menuValues[i] = -1;
        tft.fillRect(54 + (i * 48), 54, 33, 33, TFT_COLOR4);
      }
      menuIndex = 0;
    } else
      maxItems = 4;
  } else if (menu == "card") {
    if (menuIndex == 0 || empty)
      pageNum = 0;
    pageNum++;
    if (pageNum < 10)
      tft.drawString("0" + String(pageNum), 6, 48);
    else
      tft.drawString(String(pageNum), 6, 48);
    tft.fillRect(12, 72, 21, 3, TFT_COLOR1);
    tft.fillRect(54, 48, 33, 3, TFT_COLOR4);
    tft.fillRect(102, 48, 33, 33, TFT_COLOR4);
    tft.fillRect(150, 48, 33, 33, TFT_COLOR4);
    tft.fillRect(198, 48, 33, 33, TFT_COLOR4);
    tft.fillRect(63, 84, 15, 3, TFT_COLOR4);
    tft.fillRect(111, 84, 15, 3, TFT_COLOR4);
    tft.fillRect(159, 84, 15, 3, TFT_COLOR4);
    tft.fillRect(207, 84, 15, 3, TFT_COLOR4);

    for (int i = 0; i < 4; i++) {
      menuValues[i] = -1;
    }

    SortedCard sortedCards[256];
    int totalCards = 0;
    for (int i = 0; i < 256; i++) {
      if (ownsCard(i)) {
        sortedCards[totalCards].index = i;
        sortedCards[totalCards].name = cardList[i].name;
        totalCards++;
      }
    }
    sortCardsByName(sortedCards, totalCards);

    for (menuIndex; menuIndex < totalCards && displayed < 4; menuIndex++) {
      menuValues[displayed] = sortedCards[menuIndex].index;
      pushScaled(54 + (displayed * 48), 48, 11, 11, cardSpriteArray[sortedCards[menuIndex].index], TFT_BLACK);
      tft.fillRect(63 + (displayed * 48), 84, 15, 3, TFT_COLOR4);
      int exists = -1;
      for (int i = 0; i < 10; i++) {
        if (menuValues[displayed] == deckIndex[i])
          exists = i;
      }
      if (exists > -1)
        tft.fillRect(63 + (displayed * 48), 84, 6, 3, TFT_COLOR3);
      bool repeat = false;
      for (int i = 0; i < 10; i++) {
        if (menuValues[displayed] == deckIndex[i] && i != exists)
          repeat = true;
      }
      if (repeat)
        tft.fillRect(72 + (displayed * 48), 84, 6, 3, TFT_COLOR3);
      displayed++;
    }
    if (displayed < 4 || menuIndex >= totalCards) {
      maxItems = displayed;
      for (int i = displayed; i < 4; i++) {
        menuValues[i] = -1;
        tft.fillRect(54 + (i * 48), 48, 33, 33, TFT_COLOR4);
      }
      menuIndex = 0;
    } else {
      maxItems = 4;
    }
  } else if (menu == "cardTooltip") {
    int tempIndex = menuIndex;
    displayed = 0;
    if (pageNum < 10)
      tft.drawString("0" + String(pageNum), 6, 48);
    else
      tft.drawString(String(pageNum), 6, 48);
    tft.fillRect(12, 72, 21, 3, TFT_COLOR1);
    tft.fillRect(54, 48, 33, 33, TFT_COLOR4);
    tft.fillRect(102, 48, 33, 33, TFT_COLOR4);
    tft.fillRect(150, 48, 33, 33, TFT_COLOR4);
    tft.fillRect(198, 48, 33, 33, TFT_COLOR4);
    for (int i; i < maxItems && displayed < 4; i++) {
      pushScaled(54 + (displayed * 48), 48, 11, 11, cardSpriteArray[menuValues[displayed]], TFT_BLACK);
      tft.fillRect(63 + (displayed * 48), 84, 15, 3, TFT_COLOR4);
      int exists = -1;  //if this is -1, it does not exist
      for (int i = 0; i < 8; i++) {
        if (menuValues[displayed] == deckIndex[i])
          exists = i;
      }
      if (exists > -1)
        tft.fillRect(63 + (displayed * 48), 84, 6, 3, TFT_COLOR3);
      bool repeat = false;
      for (int i = 0; i < 8; i++) {
        if (menuValues[displayed] == deckIndex[i] && i != exists)
          repeat = true;
      }
      if (repeat)
        tft.fillRect(72 + (displayed * 48), 84, 6, 3, TFT_COLOR3);
      displayed++;
    }
    if (displayed < 4) {
      maxItems = displayed;
      for (int i = displayed; i < 4; i++) {
        //menuValues[displayed] = 0;
        displayed = i;
        tft.fillRect(54 + (i * 48), 54, 33, 33, TFT_COLOR4);
      }
    } else
      maxItems = 4;
  } else if (menu == "gems") {
    tft.fillRect(6, 48, 39, 27, TFT_COLOR4);
    if (menuIndex == 0 || empty)
      pageNum = 0;
    pageNum++;
    if (pageNum < 10)
      tft.drawString("0" + String(pageNum), 6, 48);
    else
      tft.drawString(String(pageNum), 6, 48);
    tft.fillRect(12, 72, 21, 3, TFT_COLOR1);
    // Clear gem display areas (only 3 slots for gems)
    tft.fillRect(54, 42, 48, 48, TFT_COLOR4);
    tft.fillRect(108, 42, 48, 48, TFT_COLOR4);
    tft.fillRect(162, 42, 48, 48, TFT_COLOR4);

    // Iterate through the 10 available gems (indices 0-10)
    for (menuIndex; menuIndex < 11 && displayed < 3; menuIndex++) {
      int gemQuantity = EEPROM.read(199 + menuIndex);
      if (gemQuantity > 0 && gemQuantity != 255) {
        menuValues[displayed] = menuIndex;
        pushScaled(51 + (displayed * 54), 42, 16, 16, gems[menuIndex], TFT_BLACK);
        displayed++;
      }
    }

    // Handle end of gem list
    if (displayed < 3 && menuIndex >= 11) {
      maxItems = displayed;
      for (int i = displayed; i < 3; i++) {
        tft.fillRect(54 + (i * 48), 54, 33, 33, TFT_COLOR4);
      }
      menuIndex = 0;
      pageNum = 1;
    } else {
      maxItems = 3;
    }
  } else if (menu == "gemTooltip") {
    tft.fillRect(6, 48, 39, 27, TFT_COLOR4);
    displayed = 0;
    if (pageNum < 10)
      tft.drawString("0" + String(pageNum), 6, 48);
    else
      tft.drawString(String(pageNum), 6, 48);
    tft.fillRect(12, 72, 21, 3, TFT_COLOR1);
    // Clear gem display areas (only 3 slots for gems)
    tft.fillRect(54, 42, 48, 48, TFT_COLOR4);
    tft.fillRect(108, 42, 48, 48, TFT_COLOR4);
    tft.fillRect(162, 42, 48, 48, TFT_COLOR4);

    // Redraw the same gems that were displayed before tooltip
    for (int i = 0; i < maxItems && i < 3; i++) {
      int gemIndex = menuValues[i];  // Use the previously stored gem indices
      pushScaled(54 + (i * 48), 42, 16, 16, gems[gemIndex], TFT_BLACK);
      displayed++;
    }

    // Handle end of gem list
    if (displayed < 3) {
      maxItems = displayed;
      for (int i = displayed; i < 3; i++) {
        tft.fillRect(54 + (i * 48), 54, 33, 33, TFT_COLOR4);
      }
    } else {
      maxItems = 3;
    }
  } else if (menu == "items") {
    if (menuIndex == 0)
      pageNum = 0;
    pageNum++;
    tft.fillRect(6, 66, 39, 27, TFT_COLOR4);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
    if (pageNum < 10)
      tft.drawString("0" + String(pageNum), 9, 60);
    else
      tft.drawString(String(pageNum), 9, 60);
    tft.fillRect(15, 84, 21, 3, TFT_COLOR1);
    pushScaled(18, 81, 5, 5, smallIcons[13], TFT_BLACK);
    tft.fillRect(45, 54, 195, 48, TFT_COLOR4);  //clear items
    for (int i = 0; i < 3; i++) {               //clear the visible menuValues values on screen
      menuValues[i] = -1;
    }
    for (menuIndex; menuIndex < 20 && displayed < 3; menuIndex++) {  //20 = current total of all items available in game
      if (EEPROM.read(143 + menuIndex) != 0 && EEPROM.read(143 + menuIndex) != 255) {
        menuValues[displayed] = menuIndex;
        pushScaled(54 + (displayed * 57), 54, 16, 16, items[menuIndex * 3], TFT_BLACK);
        displayed++;
      }
    }
    if (displayed < 3 && menuIndex >= 20) {
      maxItems = displayed;
      for (int i = displayed; i < 3; i++) {
        displayed = i;
        tft.fillRect(48 + (displayed * 57), 54, 48, 48, TFT_COLOR4);
      }
      menuIndex = 0;
    } else
      maxItems = 3;
  } else if (menu == "itemsTooltip") {
    tft.fillRect(6, 66, 39, 27, TFT_COLOR4);
    tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
    if (pageNum < 10)
      tft.drawString("0" + String(pageNum), 9, 60);
    else
      tft.drawString(String(pageNum), 9, 60);
    tft.fillRect(15, 84, 21, 3, TFT_COLOR1);
    pushScaled(18, 81, 5, 5, smallIcons[13], TFT_BLACK);
    for (menuIndex; menuIndex < 20 && displayed < 3; menuIndex++) {  //20 = current total of all items available in game
      if (EEPROM.read(143 + menuIndex) != 0 && EEPROM.read(143 + menuIndex) != 255) {
        menuValues[displayed] = menuIndex;
        pushScaled(54 + (displayed * 57), 54, 16, 16, items[menuIndex * 3], TFT_BLACK);
        displayed++;
      }
    }
    if (displayed < 3 && menuIndex >= 20) {
      maxItems = displayed;
      for (int i = displayed; i < 3; i++) {
        displayed = i;
        tft.fillRect(48 + (displayed * 57), 54, 48, 48, TFT_COLOR4);
      }
      menuIndex = 0;
    } else
      maxItems = 4;
  }
}

int OwnershipCount(byte ownership[32]) {
  //Used to return the number of items owned in a particular byte array
  int totalOwned = 0;
  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 8; j++) {
      if (bitRead(ownership[i], j) != 0) {
        totalOwned++;
      }
    }
  }
  return totalOwned;
}

void ManageGems(String _pet, bool remove) {
  if (_pet == "player") {
    //Get base stats before calculating any stat changes with gems
    if (remove) {
      int nonZeroCount = 0;
      for (int i = 0; i < 3; ++i) {
        if (gemsIndex[i] > 0) {
          nonZeroCount++;
        }
      }
      // If there are non-zero elements, randomly select one to reset to zero
      if (nonZeroCount > 0) {
        int randomIndex = random(nonZeroCount);  // Generate a random index from 0 to nonZeroCount - 1

        // Reset the randomly selected non-zero element to zero
        int zeroCount = 0;
        for (int i = 0; i < 3; ++i) {
          if (gemsIndex[i] > 0) {
            if (randomIndex == 0) {
              gemsIndex[i] = 0;
              break;
            } else if (zeroCount > 0) {
              // Swap the non-zero element with the first zero element
              gemsIndex[i - zeroCount] = gemsIndex[i];
              gemsIndex[i] = 0;
            }
            randomIndex--;
          }
        }
      }
    }  // end remove
    float atkCount = 0;
    float defCount = 0;
    float spdCount = 0;
    for (int i = 0; i < 3; ++i) {
      if (gemsIndex[i] == 0)
        atkCount++;
      if (gemsIndex[i] == 1)
        defCount++;
      if (gemsIndex[i] == 2)
        spdCount++;
    }
    Attack = Attack + (Attack * (0.1 * atkCount));
    Defense = Defense + (Defense * (0.1 * defCount));
    Speed = Speed + (Speed * (0.1 * spdCount));
  }
}

char NextChar(char current) {
  for (int i = 0; i < charSetLength; i++) {
    if (charSet[i] == current) {
      return charSet[(i + 1) % charSetLength];  // Move to the next character, loop back to first if at the end
    }
  }
  return charSet[0];  // Default to first character if none match
}

void playMelody() {
  // Melody notes (frequency in Hz)
  int melody[] = { 262, 294, 330, 349, 392, 440, 494, 523 };

  // Note durations (in milliseconds)
  int noteDurations[] = { 500, 500, 500, 500, 500, 500, 500, 500 };

  static unsigned long startTime = 0;
  static int noteIndex = 0;

  // Check if it's time to play the next note
  if (millis() - startTime >= noteDurations[noteIndex]) {
    // Move to the next note
    noteIndex++;

    // Check if all notes have been played
    if (noteIndex < 8) {
      // Play the next note
      tone(26, melody[noteIndex]);

      // Update the start time for the next note
      startTime = millis();
    } else {
      // All notes have been played, reset for the next iteration
      noteIndex = 0;
    }
  }
}





bool HasHarvestableCrop() {
  for(int i = 0; i < 3; i++) {
    if(potCrop[i] > 0 && potTimer[i] >= 360) {
      int itemAddress = 0;
      if(potCrop[i] == 1) itemAddress = 143;
      else if(potCrop[i] == 2) itemAddress = 144;
      else if(potCrop[i] == 3) itemAddress = 145;
      
      // Check EEPROM directly instead of cached inventory
      int qty = EEPROM.read(itemAddress);
      if(qty == 255) qty = 0;
      if(qty < 9) return true;
    }
  }
  return false;
}

void SaveCredits() {
  uint16_t credits = (uint16_t)playerCredits;
  EEPROM.put(193, credits);
  EEPROM.commit();
}

void AddCredits(int amount) {
  playerCredits += amount;
  if(playerCredits > 9999) playerCredits = 9999;
  SaveCredits();
}

bool SpendCredits(int amount) {
  // Apply merchant discount from random event
  if (merchantDiscountActive) {
    amount = amount / 2;
    merchantDiscountActive = false;
    SaveEventFlags();
  }
  if(playerCredits >= amount) {
    playerCredits -= amount;
    SaveCredits();
    return true;
  }
  return false;
}

#pragma endregion


#pragma region Initialization

void pinoutInit() {
  pinMode(14, OUTPUT);
  digitalWrite(14, HIGH);
  pinMode(35, INPUT_PULLUP);
  pinMode(0, INPUT_PULLUP);
  pinMode(4, OUTPUT);
  pinMode(26, OUTPUT);
  //NEOPIXEL
  strip.begin();
  strip.show();
  strip.setBrightness(ledBrightness);
}

void displayInit() {
  tft.init();
  tft.setRotation(1);
  //lower screen brightness (70,100,130,160,200,220)
  ledcSetup(0, 5000, 8);
  ledcAttachPin(TFT_BL, 0);
  ledcWrite(0, screenBrightness);
  ledcAttachPin(26, 2);
  ledcWrite(2, 0);
  tft.fillScreen(TFT_COLOR4);
  tft.setSwapBytes(true);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.setTextSize(3);
  display.createSprite(240, 57);
  display.setSwapBytes(true);
  playerSprite.createSprite(48, 48);
  playerSprite.setSwapBytes(true);
  enemySprite.createSprite(48, 48);
  enemySprite.setSwapBytes(true);
  emoteSprite.createSprite(24, 24);
  emoteSprite.setSwapBytes(true);
  itemSprite.createSprite(48, 48);
  itemSprite.setSwapBytes(true);
}

void ReadSaveData() {
  EEPROM.get(1, cardOwnership);
  EEPROM.get(33, headOwnership);
  EEPROM.get(65, bodyOwnership);
  equippedWardrobe = EEPROM.read(134);
  if (EEPROM.read(134) == 255) {
    EEPROM.write(134, 0);
    EEPROM.commit();
  }

  if (equippedWardrobe == 0) {
    CreateCharacter("player", headGear[EEPROM.read(135)], bodyGear[EEPROM.read(136)]);
  } else if (equippedWardrobe == 1) {
    CreateCharacter("player", headGear[EEPROM.read(137)], bodyGear[EEPROM.read(138)]);
  } else if (equippedWardrobe == 2) {
    CreateCharacter("player", headGear[EEPROM.read(139)], bodyGear[EEPROM.read(140)]);
  }

  //Deck contents
  for (int i = 97; i < 127; i++) {
    EEPROM.read(i);
  }
  //Equipped Deck
  equippedDeck = EEPROM.read(127);
  stage = EEPROM.read(141);  //get Venture stage

  if(debug)
    stage = 29;
  EEPROM.write(141, stage);
  EEPROM.commit();

  if(EEPROM.read(141) != 255)
    ventureOpponent = EEPROM.read(141);
  else
    ventureOpponent = 0;

  // ===== V1.1 HOTFIX - DELETE NEXT PATCH =====
  // Auto-unlock card 51 for users at stage 6+
    if (stage >= 6 && !bitRead(cardOwnership[51 / 8], 51 % 8)) {
      bitSet(cardOwnership[51 / 8], 51 % 8);
      EEPROM.put(1, cardOwnership);
      EEPROM.commit();
    }
  // ===== END V1.1 HOTFIX =====

  //Pico Name
  EEPROM.get(175, tempName);
  for (int i = 0; i < 10; i++) {
    if (tempName[i] == 255) {
      tempName[i] = 0;
    }
  }
  name = String(tempName);

  //Farm data
  for(int i = 0; i < 3; i++) {
    potCrop[i] = EEPROM.read(230 + i);
    potStage[i] = EEPROM.read(233 + i);
    EEPROM.get(236 + (i * 4), potTimer[i]);
    
    // Validate data
    if(potCrop[i] == 255) potCrop[i] = 0;
    if(potStage[i] == 255) potStage[i] = 0;
    if(potTimer[i] == -1) potTimer[i] = 0;
  }
  bool tempCropReady = false;
  for(int i = 0; i < 3; i++) {
    if(potCrop[i] > 0 && potTimer[i] >= 360) {
      tempCropReady = true;
      break;
    }
  }
  cropReady = tempCropReady;

  // Read player credits (2 bytes: 193 and 194)
  uint16_t tempCredits;
  EEPROM.get(193, tempCredits);
  if(tempCredits == 0xFFFF) tempCredits = 0;  // Default to 0 if uninitialized
  playerCredits = tempCredits;

  // Load Deep Dungeon state
  deepDungeonUnlocked = EEPROM.read(255) == 1;
  deepDungeonActive = EEPROM.read(256) == 1;
  EEPROM.get(257, deepDungeonStage);
  if (deepDungeonStage < 0 || deepDungeonStage > 999) deepDungeonStage = 0;
  EEPROM.get(259, ddCardOwnership);

  // Restore locked-in run state only while a run is active (344-374)
  if (deepDungeonActive) {
    int g = EEPROM.read(344);
    activeGem = (g == 255) ? -1 : g;
    activeMeritCount = EEPROM.read(345);
    if (activeMeritCount > 9) activeMeritCount = 0;
    for (int i = 0; i < 9; i++) { int v = EEPROM.read(346 + i); activeMerits[i] = (v == 255) ? -1 : v; }
    activeAfflictionCount = EEPROM.read(355);
    if (activeAfflictionCount > 9) activeAfflictionCount = 0;
    for (int i = 0; i < 9; i++) { int v = EEPROM.read(356 + i); activeAfflictions[i] = (v == 255) ? -1 : v; }
    for (int i = 0; i < 9; i++) activeAfflictionLevels[i] = EEPROM.read(365 + i);
    ddReviveUsed = (EEPROM.read(374) == 1);
  }
  
  // Load Deep Dungeon high score
  uint16_t tempHighScore;
  EEPROM.get(253, tempHighScore);
  if(tempHighScore == 0xFFFF) tempHighScore = 0;
  deepDungeonHighScore = tempHighScore;

  // Load random event flags
  LoadEventFlags();

  // Screen backlight + LED brightness (stored as 0-100 percent)
  backlightPercent = EEPROM.read(220);
  if (backlightPercent > 100) backlightPercent = 30;   // uninitialized -> default
  backlightPercent = (backlightPercent / 10) * 10;     // snap to nearest 10
  ApplyBacklight();

  ledPercent = EEPROM.read(221);
  if (ledPercent > 100) ledPercent = 20;
  ledPercent = (ledPercent / 10) * 10;
  ApplyLed();

  screenTimeout = EEPROM.read(222);
  if (screenTimeout != 15 && screenTimeout != 30 && screenTimeout != 45 && screenTimeout != 60)
    screenTimeout = 15;  // uninitialized/invalid -> default

  ledTimeout = EEPROM.read(223);
  if (ledTimeout != 1 && ledTimeout != 5 && ledTimeout != 10 && ledTimeout != 15)
    ledTimeout = 0;  // uninitialized/invalid -> NONE

  Serial.println(buildName);
  for (int i = 0; i < 255; i++) {
    byte value = EEPROM.read(i);
    Serial.println(value);
  }
}

void SaveDataInit() {
  // EEPROM address 338 is used to determine if EEPROM data has been initialized
  if (EEPROM.read(338) != 0xCD) {
    FormatData();
    
    // Clear ownership arrays in RAM before setting starter items
    memset(cardOwnership, 0, sizeof(cardOwnership));
    memset(headOwnership, 0, sizeof(headOwnership));
    memset(bodyOwnership, 0, sizeof(bodyOwnership));
    
    bitSet(cardOwnership[0 / 8], 0 % 8);
    bitSet(cardOwnership[1 / 8], 1 % 8);
    bitSet(cardOwnership[2 / 8], 2 % 8);
    bitSet(cardOwnership[3 / 8], 3 % 8);
    bitSet(cardOwnership[7 / 8], 7 % 8);
    bitSet(cardOwnership[9 / 8], 9 % 8);
    bitSet(cardOwnership[12 / 8], 12 % 8);
    bitSet(cardOwnership[18/ 8], 18 % 8);
    EEPROM.put(1, cardOwnership);

    bitSet(headOwnership[0 / 8], 0 % 8);
    //bitSet(headOwnership[1 / 8], 1 % 8);
    //bitSet(headOwnership[2 / 8], 2 % 8);
    //bitSet(headOwnership[3 / 8], 3 % 8);
    //bitSet(headOwnership[4 / 8], 4 % 8);
    EEPROM.put(33, headOwnership);

    bitSet(bodyOwnership[0 / 8], 0 % 8);
    //bitSet(bodyOwnership[1 / 8], 1 % 8);
    //bitSet(bodyOwnership[2 / 8], 2 % 8);
    //bitSet(bodyOwnership[3 / 8], 3 % 8);
    //bitSet(bodyOwnership[4 / 8], 4 % 8);
    EEPROM.put(65, bodyOwnership);

    EEPROM.write(134, 0);  //Set equipped wardrobe
    EEPROM.write(135, 0);  //Wardrobe headGear 1
    EEPROM.write(136, 0);  //Wardrobe bodyGear 1
    EEPROM.write(137, 0);  //Wardrobe headGear 2
    EEPROM.write(138, 0);  //Wardrobe bodyGear 2
    EEPROM.write(139, 0);  //Wardrobe headGear 3
    EEPROM.write(140, 0);  //Wardrobe bodyGear 3

    //Deck 1
    EEPROM.write(97, 0);   //Deck 1 card 1
    EEPROM.write(98, 0);   //Deck 1 card 2
    EEPROM.write(99, 1);   //Deck 1 card 3
    EEPROM.write(100, 1);  //Deck 1 card 4
    EEPROM.write(101, 2);  //Deck 1 card 5
    EEPROM.write(102, 2);  //Deck 1 card 6
    EEPROM.write(103, 3);  //Deck 1 card 7
    EEPROM.write(104, 3);  //Deck 1 card 8
    EEPROM.write(105, 4);  //Deck 1 card 9
    EEPROM.write(106, 4);  //Deck 1 card 10
    //Gems 1
    EEPROM.write(128, 0);  //Deck 1 Gem 1

    //Deck 2
    EEPROM.write(107, 0);  //Deck 2 card 1
    EEPROM.write(108, 0);  //Deck 2 card 2
    EEPROM.write(109, 1);  //Deck 2 card 3
    EEPROM.write(110, 1);  //Deck 2 card 4
    EEPROM.write(111, 2);  //Deck 2 card 5
    EEPROM.write(112, 2);  //Deck 2 card 6
    EEPROM.write(113, 3);  //Deck 2 card 7
    EEPROM.write(114, 3);  //Deck 2 card 8
    EEPROM.write(115, 4);  //Deck 2 card 9
    EEPROM.write(116, 4);  //Deck 2 card 10
    //Gems 2
    EEPROM.write(129, 0);  //Deck 2 Gem 1

    //Deck 3
    EEPROM.write(117, 0);  //Deck 3 card 1
    EEPROM.write(118, 0);  //Deck 3 card 2
    EEPROM.write(119, 1);  //Deck 3 card 3
    EEPROM.write(120, 1);  //Deck 3 card 4
    EEPROM.write(121, 2);  //Deck 3 card 5
    EEPROM.write(122, 2);  //Deck 3 card 6
    EEPROM.write(123, 3);  //Deck 3 card 7
    EEPROM.write(124, 3);  //Deck 3 card 8
    EEPROM.write(125, 4);  //Deck 3 card 9
    EEPROM.write(126, 4);  //Deck 3 card 10
    //Gems 3
    EEPROM.write(130, 0);  //Deck 3 Gem 1

    EEPROM.write(127, 0);  //Set equipped deck

    EEPROM.write(141, 0);  //Set cleared Venture stages to 0 
    EEPROM.write(142, 0);  //Set seleted Venture opponent to the first

    //Initialize Gems
    for(int i = 0; i < 11; i++)
      EEPROM.write(199 + i, 0);
    
    //Grant starting inventory items
    EEPROM.write(143, 2);
    EEPROM.write(144, 2);
    EEPROM.write(145, 2);
    EEPROM.write(146, 1);

    // Initialize farm data
    for(int i = 0; i < 3; i++) {
      EEPROM.write(230 + i, 0);  // potCrop
      EEPROM.write(233 + i, 0);  // potStage  
      EEPROM.put(236 + (i * 4), 0);  // potTimer
    }
    // Initialize player credits
    uint16_t startingCredits = 50;
    EEPROM.put(193, startingCredits);

    EEPROM.write(338, 0xCD);  // Mark data as initialized
    EEPROM.commit();

    CreateDeck(equippedDeck);
    LoadEventFlags();
  }
}

void FormatData() {
  for (int i = 0; i < 512; i++) {  //512 addresses being used
    EEPROM.write(i, 255);          //default value
  }
  for (int i = 1; i < 143; i++)
    EEPROM.write(i, 0);
  EEPROM.commit();
}

// Base64 decode helper for PNUT restore protocol
int base64_decode(uint8_t* output, const char* input, int inputLen) {
  static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  int outputLen = 0;
  uint32_t val = 0;
  int valb = -8;
  
  for (int i = 0; i < inputLen; i++) {
    char c = input[i];
    if (c == '=') break;
    const char* p = strchr(b64, c);
    if (p == NULL) continue;
    val = (val << 6) + (p - b64);
    valb += 6;
    if (valb >= 0) {
      output[outputLen++] = (val >> valb) & 0xFF;
      valb -= 8;
    }
  }
  return outputLen;
}

// CRC32 helper for PNUT restore protocol
uint32_t crc32(uint8_t* data, size_t len) {
  uint32_t crc = 0xFFFFFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
    }
  }
  return ~crc;
}


#define SAVE_STAGING_ADDR 0xFFF000
#define SAVE_MAGIC 0x53415645

void CopyEEPROMToFlash() {
  uint32_t buffer[129];
  buffer[0] = SAVE_MAGIC;
  for (int i = 0; i < 512; i++) {
    ((uint8_t*)&buffer[1])[i] = EEPROM.read(i);
  }
  ESP.flashEraseSector(SAVE_STAGING_ADDR / 0x1000);
  ESP.flashWrite(SAVE_STAGING_ADDR, buffer, 516);
}

void CheckAndLoadRestore() {
  uint32_t magic;
  ESP.flashRead(SAVE_STAGING_ADDR, &magic, 4);
  if (magic != SAVE_MAGIC) return;
  
  uint32_t buffer[128];
  ESP.flashRead(SAVE_STAGING_ADDR + 4, buffer, 512);
  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, ((uint8_t*)buffer)[i]);
  }
  EEPROM.commit();
  ESP.flashEraseSector(SAVE_STAGING_ADDR / 0x1000);
}

#pragma endregion

#pragma region LEDsAndBattery

void LEDManager() {
  // LED update timing control - only update every 50ms (20Hz instead of 60+ Hz)
  static unsigned long lastLEDUpdate = 0;
  const unsigned long LED_UPDATE_INTERVAL = 50; // 50ms = 20Hz
  
  bool needsUpdate = (millis() - lastLEDUpdate >= LED_UPDATE_INTERVAL);

  // LED auto-off streak tracking (reset on every early-out where the LED is forced off)
  static unsigned long ledOnStart = 0;
  static String prevLedColor = "Off";
  
  //TO DO: Add states that determine what LED should be flashing
  //LEDBlink(100, 2000); //test low battery //TO-DO: Add the color you want the LED to flash instead of just red
  //LEDBreath(strip.Color(100,255,0), 2000, ledBrightness); //Orange color for hunger
  //LEDBreath(strip.Color(0,255,255, 0), 2000, ledBrightness); //Purple for sleep
  //LEDBreath(strip.Color(0,255,0, 0), 750, ledBrightness); //Red for injured
  //LEDBreath(strip.Color(255,55,0), 2000, ledBrightness); //Turquoise color for crops
  //LEDRainbow(200);

  //Skips the function and prevents the clock speed from increasing when asleep
  if (ledBrightness == 0) {
    if (ledActive) {           // clear the strip once instead of holding the last frame
      strip.setPixelColor(0, 0, 0, 0);
      strip.show();
      ledActive = false;
    }
    prevLedColor = "Off";
    ledTimedOut = false;
    return;
  }

  if (doNotDisturb && !testActive) {
    if (needsUpdate) {
      strip.setPixelColor(0, 0, 0, 0);
      strip.show();
      lastLEDUpdate = millis();
    }
    prevLedColor = "Off";
    ledTimedOut = false;
    return;
  }
  
  // Only proceed with LED updates if it's time to update
  if (!needsUpdate) {
    return;
  }
  
  // Update timestamp since we're proceeding with LED update
  lastLEDUpdate = millis();
  
  // Boost CPU frequency for LED operations if needed
  bool cpuBoosted = false;
  if (screenOff && cpu != 80 && !ledTimedOut) {
    strip.setPin(27);
    strip.begin();
    setCpuFrequencyMhz(80);
    cpu = 80;
    cpuBoosted = true;
  }
  
  // Cooldown-done ping is an away notification: it only glows while the screen is off,
  // and clears the moment the player wakes the screen. Duration is bounded by ledTimeout.
  if (cooldownDoneNotify && !screenOff)
    cooldownDoneNotify = false;

  //For testing brightness in the Options menu
  if (testActive) {
    LEDRainbow(200);
    ledActive = true;
    ledColor = "Rainbow";
  }
  //CHARGE PULSE NOTIFICATION
  else if (chargeNotification && !testActive) {
    ledActive = true;
    ledColor = "Green";
    LEDPulseOnce(strip.Color(255, 0, 0, 0), 500, ledBrightness);
  }
  //LOW BATTERY NOTIFICATION
  else if (BL.getBatteryChargeLevel() < 5 && !testActive) {
    ledActive = true;
    ledColor = "Red";
    LEDBlink(100, 2000, 100);
  }
  //REVIVE READY NOTIFICATION
  else if (ACTION == INJURED && EMOTION == REVIVE && MENUSTATE != BATTLE && !testActive && !gamePause) {
    ledActive = true;
    ledColor = "Red";
    LEDBreath(strip.Color(0, 255, 0, 0), 1500, ledBrightness);  // Red pulse
  }
  //RANDOM EVENT NOTIFICATION
  else if (randomEventPending && eventState == EVENT_ALERT && MENUSTATE != BATTLE && !testActive && !gamePause) {
    ledActive = true;
    ledColor = "White";
    LEDBreath(strip.Color(255, 255, 255, 0), 1500, ledBrightness);
  }
  //VENTURE READY
  else if (VENTURESTATE == VENTURING && countdownSeconds == 0) {
    LEDRainbow(200);
    ledActive = true;
    ledColor = "Rainbow";
  }
  //COOLDOWN FINISHED - brief rainbow ping that you're free to battle/venture again
  else if (cooldownDoneNotify && screenOff && countdownSeconds == 0 && MENUSTATE != BATTLE && !testActive && !gamePause) {
    LEDRainbow(200);
    ledActive = true;
    ledColor = "Rainbow";
  }
  //STAMINA+HUNGER NOTIFICATION
  else if (Stamina <= 25 && Hunger <= 25 && ACTION != SLEEPING && ACTION != INJURED && MENUSTATE != BATTLE && !testActive && !gamePause) {
    ledActive = true;
    ledColor = "Pink";
    LEDBreath(strip.Color(75, 255, 100, 0), 1000, ledBrightness);
  }
  //STAMINA NOTIFICATION
  else if (Stamina <= 25 && ACTION != SLEEPING && ACTION != INJURED && MENUSTATE != BATTLE && !testActive && !gamePause) {
    ledActive = true;
    ledColor = "Purple";
    LEDBreath(strip.Color(100, 220, 255, 0), 2000, ledBrightness);
  }
  //HUNGER NOTIFICATION
  else if (Hunger <= 25 && ACTION != SLEEPING && ACTION != INJURED && MENUSTATE != BATTLE && !testActive && !gamePause) {
    ledActive = true;
    ledColor = "Orange";
    LEDBreath(strip.Color(100, 255, 0), 2000, ledBrightness);
  }
  //CROP NOTIFICATION
  else if (cropReady && HasHarvestableCrop() && MENUSTATE != BATTLE && ACTION != SLEEPING && !testActive && !gamePause) {
    ledActive = true;
    ledColor = "Turquoise";
    LEDBreath(strip.Color(255, 0, 75, 0), 2000, ledBrightness);
  }
  else if (ledColor != "Off" && MENUSTATE != TITLE) {
    ledActive = false;
    if (!ledTimedOut) {        // already dark while suppressed, and the clock may be at 10MHz
      strip.setPixelColor(0, 0, 0, 0);
      strip.show();
    }
    if (BL.getBatteryChargeLevel() > 10)
      ledActive = false;
    ledColor = "Off";
  }
  
  // LED auto-off: once a notification has run continuously past the limit, suppress the strip until
  // it clears. ledTimeout is in minutes; 0 = NONE. The boost, the four render functions, and the
  // cleanup draw above are all gated on ledTimedOut, so while suppressed nothing touches the WS2812
  // and the clock is free to fall to 10MHz. We drive the strip dark only on the one cycle the limit
  // is crossed (still at 80MHz then); the cheap condition checks keep running to detect the clear.
  bool notifying = (ledColor != "Off" && !testActive);
  if (notifying && prevLedColor == "Off")
    ledOnStart = millis();
  prevLedColor = ledColor;
  bool justTimedOut = false;
  if (!notifying)
    ledTimedOut = false;
  else if (ledTimeout > 0 && !ledTimedOut && millis() - ledOnStart >= (unsigned long)ledTimeout * 60000UL) {
    ledTimedOut = true;
    justTimedOut = true;
  }
  if (justTimedOut) {     // crossed the limit this cycle; clock is still at 80MHz, so dark the strip once
    strip.setPixelColor(0, 0, 0, 0);
    strip.show();
  }
  if (ledTimedOut)
    ledActive = false;    // strip released; the CPU-drop below can now take us to 10MHz

  // Drop CPU frequency back down immediately after LED update if we boosted it
  if (cpuBoosted && screenOff && !ledActive) {
    strip.setPin(27);
    strip.setPin(-1);
    strip.begin();
    setCpuFrequencyMhz(10);
    cpu = 10;
  }
}

void LEDBlink(int blinkDelay, int waitDelay, int onTime) {
  if (ledTimedOut) return;   // suppressed: leave the strip dark, no driving at low clock
  unsigned long currentMillis = millis();   // Current time in milliseconds
  static unsigned long blinkOnTime = 0;     // Time the LED turned on
  static byte state = OFF;                  // Initial state
  static byte blinkCount = 0;               // Blink counter
  static unsigned long previousMillis = 0;  // Previous time
  ledActive = true;
  
  switch (state) {
    case OFF:
      if (currentMillis - previousMillis >= waitDelay) {
        state = BLINK_ON;                // Switch to blink on state
        blinkCount = 0;                  // Reset blink counter
        previousMillis = currentMillis;  // Update previous time
      }
      break;
    case BLINK_ON:
      if (blinkCount < 2) {
        if (currentMillis - previousMillis >= blinkDelay) {
          strip.setBrightness(ledBrightness * 3);
          strip.setPixelColor(0, 0, 255, 0);  // Set NeoPixel color to red
          strip.show();                       // Show the NeoPixel
          if (blinkOnTime == 0) {             // If this is the first time the LED turns on
            blinkOnTime = currentMillis;      // Set the time the LED turned on
          }
          if (currentMillis - blinkOnTime >= onTime) {  // If the LED has been on for onTime ms
            state = BLINK_OFF;                          // Switch to blink off state
            previousMillis = currentMillis;             // Update previous time
            blinkOnTime = 0;                            // Reset blinkOnTime
          }
        }
      } else {
        state = WAIT;                    // Switch to wait state
        previousMillis = currentMillis;  // Update previous time
      }
      break;
    case BLINK_OFF:
      if (currentMillis - previousMillis >= blinkDelay) {
        strip.setBrightness(ledBrightness);
        strip.setPixelColor(0, 0, 0, 0);  // Turn off the NeoPixel
        strip.show();                     // Show the NeoPixel
        state = BLINK_ON;                 // Switch to blink on state
        blinkCount++;                     // Increment blink counter
        previousMillis = currentMillis;   // Update previous time
        blinkOnTime = 0;                  // Reset blinkOnTime
      }
      break;
    case WAIT:
      if (currentMillis - previousMillis >= waitDelay) {
        state = BLINK_ON;                // Switch to blink on state
        blinkCount = 0;                  // Reset blink counter
        previousMillis = currentMillis;  // Update previous time
      }
      break;
  }
}

void LEDRainbow(uint8_t wait) {
  if (ledTimedOut) return;   // suppressed: leave the strip dark, no driving at low clock
  static uint32_t startTime = 0;
  uint32_t currTime = millis();
  uint32_t elapsed = currTime - startTime;
  uint8_t r, g, b;

  // Determine the phase of the color cycle
  uint8_t phase = (elapsed / wait) % 6;

  // Calculate the color values based on the current phase
  switch (phase) {
    case 0:  // Red to Yellow
      r = 255;
      g = (elapsed % wait) * 255 / wait;
      b = 0;
      break;
    case 1:  // Yellow to Green
      r = 255 - (elapsed % wait) * 255 / wait;
      g = 255;
      b = 0;
      break;
    case 2:  // Green to Cyan
      r = 0;
      g = 255;
      b = (elapsed % wait) * 255 / wait;
      break;
    case 3:  // Cyan to Blue
      r = 0;
      g = 255 - (elapsed % wait) * 255 / wait;
      b = 255;
      break;
    case 4:  // Blue to Magenta
      r = (elapsed % wait) * 255 / wait;
      g = 0;
      b = 255;
      break;
    case 5:  // Magenta to Red
      r = 255;
      g = 0;
      b = 255 - (elapsed % wait) * 255 / wait;
      break;
  }

  // Set the color of the LED
  strip.setPixelColor(0, r, g, b);
  strip.show();

  // Reset the timer when a full cycle has completed
  if (elapsed >= wait * 6) {
    startTime = currTime;
  }
}

void LEDBreath(uint32_t color, uint16_t duration, uint8_t brightness) {
  if (ledTimedOut) return;   // suppressed: leave the strip dark, no driving at low clock
  static uint32_t startTime = 0;
  static uint8_t direction = 1;  // 1 for increasing, -1 for decreasing
  uint32_t currTime = millis();
  uint32_t elapsed = currTime - startTime;

  // Ensure the brightness stays within bounds
  if (brightness > 255) {
    brightness = 255;
  }

  // Calculate the elapsed phase in a range of 0 to duration
  elapsed = elapsed % duration;

  // Determine the current phase within the duration
  float phase = static_cast<float>(elapsed) / duration;

  // Calculate brightness based on the phase
  brightness = static_cast<uint8_t>(255 * (0.5 - abs(0.5 - phase)) * 2);

  // Update LED color based on the calculated brightness
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;
  r = (r * brightness) / 255;
  g = (g * brightness) / 255;
  b = (b * brightness) / 255;

  // Set the color of the LED
  strip.setPixelColor(0, r, g, b);
  strip.show();

  // Reset the timer when a full cycle has completed
  if (elapsed >= duration) {
    startTime = currTime;
  }
}

void LEDPulseOnce(uint32_t color, uint16_t duration, uint8_t brightness) {
  if (ledTimedOut) return;   // suppressed: leave the strip dark, no driving at low clock
  static uint32_t startTime = 0;
  static int8_t state = 0;  // 0 for off, 1 for ramping up, 2 for ramping down
  uint32_t currTime = millis();
  uint32_t elapsed = currTime - startTime;

  if (chargeNotification) {
    if (doNotDisturb)
      chargeNotification = false;
    if (state == 0) {
      // Start the pulse, ramping up from 0
      state = 1;
      brightness = 0;
      startTime = currTime;
    } else if (state == 1) {
      if (elapsed >= duration / 2) {
        // Reached full brightness, start ramping down
        state = 2;
        brightness = 255;
        startTime = currTime;
      } else {
        // Calculate brightness based on the elapsed time (ramping up)
        brightness = map(elapsed, 0, duration / 2, 0, 255);
      }
    } else if (state == 2) {
      if (elapsed >= duration) {
        // Ramping down complete, turn off the LED
        strip.setPixelColor(0, 0);
        strip.show();
        chargeNotification = false;
        ledActive = false;
        ledColor = "Off";  // Notification is complete
        state = 0;         // Reset state for the next cycle
        return;
      } else {
        // Calculate brightness based on the elapsed time (ramping down)
        brightness = map(elapsed, 0, duration, 255, 0);
      }
    }

    // Update LED color based on the calculated brightness
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    r = (r * brightness) / 255;
    g = (g * brightness) / 255;
    b = (b * brightness) / 255;

    // Set the color of the LED
    strip.setPixelColor(0, r, g, b);
    strip.show();
  }
}

void BatteryInfo() {
  static unsigned long lastTime = 0;
  static unsigned long chgTime = 0;
  static int stopChargeReadings = 0;
  
  if (!ddGemSelect && MENUSTATE != STATS3 && MENUSTATE != STATS4)
    drawingBatteryIcon(batteryAnim);
  
  float voltage = BL.getBatteryVolts();
  
  if (voltage >= 4.5) {
    stopChargeReadings = 0;  // Reset stop counter
    if (millis() - lastTime >= 750) {
      lastTime = millis();
      batteryAnim++;
    }
    if (batteryAnim >= 4) {
      batteryAnim = 0;
    }
  } else {
    stopChargeReadings++;
    if (stopChargeReadings >= 10 && isCharging) {
      isCharging = false;
    }
    if (millis() - lastTime >= 1000) {
      lastTime = millis();
      if (percentage >= 75) {
        batteryAnim = 7;
      } else if (percentage < 75 && percentage > 50) {
        batteryAnim = 6;
      } else if (percentage < 50 && percentage > 25) {
        batteryAnim = 5;
      } else if (percentage <= 25) {
        batteryAnim = 4;
      }
    }
    if (millis() - chgTime >= 600) {
      chgTime = millis();
      percentage = BL.getBatteryChargeLevel();
    }
  }
}

#pragma endregion











