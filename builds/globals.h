#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>
// ---- card / counter / item classes (was card.h) ----
/////////////CARD DEFINITION//////////////////
#pragma region Cards
class Card {
public:
  String name;           // Name of the card
  int cardIndex;         // Index of card sprite in the cardSpriteArray
  String type;           // Type of the card (e.g. fire, water, electric)
  int damage;            // Base damage value
  bool priority;         // Whether the card has priority over other moves
  String desc1;           // Description line 1 of the card's effect
  String desc2;           // Description line 2 of the card's effect
  String desc3;           // Description line 3 of the card's effect
  String selfCounter;    // Name of counter placed on the player
  String oppCounter;     // Name of counter placed on the opponent
  String arenaCounter;   // Name of counter placed on the arena
  bool canReplace;       // Whether or not the counter can replace existing counters

  // Constructor
  Card(String _name, int _cardIndex, String _type, int _damage, bool _priority, String _desc1, String _desc2, String _desc3, String _selfCounter, String _oppCounter, String _arenaCounter, bool _canReplace) {
    name = _name;
    cardIndex = _cardIndex;
    type = _type;
    damage = _damage;
    priority = _priority;
    desc1 = _desc1;
    desc2 = _desc2;
    desc3 = _desc3;
    selfCounter = _selfCounter;
    oppCounter = _oppCounter;
    arenaCounter = _arenaCounter;
    canReplace = _canReplace;
  }
};
#pragma endregion

/////////////COUNTER DEFINITION//////////////////
#pragma region Counters

class Counter {
public:
  String name;            // Name of the counter
  String abbreviation;    // Abbreviation of the counter (i.e. FI)
  int counterIndex;       // Index of counter sprite in the counterArray
  String type;            // Type of the counter (e.g. fire, water, electric)
  String desc1;           // Description line 1 of the counter's effect
  String desc2;           // Description line 2 of the counter's effect
  String desc3;           // Description line 3 of the counter's effect

  // Constructor
  Counter(String _name, String _abbreviation, int _counterIndex, String _type, String _desc1, String _desc2, String _desc3) {
    name = _name;
    abbreviation = _abbreviation;
    counterIndex = _counterIndex;
    type = _type;
    desc1 = _desc1;
    desc2 = _desc2;
    desc3 = _desc3;
  }
};
#pragma endregion

/////////////ITEM DEFINITION//////////////////
#pragma region Items

class Item {
public:
  String name1;            // Line 1 of the item name
  String name2;            // Line 2 of the item name
  int itemIndex;          // Index of counter sprite in the counterArray
  String desc1;           // Description line 1 of the counter's effect
  String desc2;           // Description line 2 of the counter's effect
  String desc3;           // Description line 3 of the counter's effect

  // Constructor
  Item(String _name1, String _name2, int _itemIndex, String _desc1, String _desc2, String _desc3) {
    name1 = _name1;
    name2 = _name2;
    itemIndex = _itemIndex;
    desc1 = _desc1;
    desc2 = _desc2;
    desc3 = _desc3;
  }
};
#pragma endregion
#include <Adafruit_NeoPixel.h>
#include <Pangodream_18650_CL.h>
// ---- hardware / UI config (was config.h) ----
#define ICON_WIDTH 45
#define ICON_HEIGHT 27
#define STATUS_HEIGHT_BAR ICON_HEIGHT
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define MIN_USB_VOL 4.9
#define ADC_PIN 34
#define CONV_FACTOR 1.8
#define READS 20
#define SCREEN_OFF_TIME 1000
#define BUTTON_HOLD_TIME 500     // press shorter than this = tap; longer = hold (also the auto-repeat start delay)
#define BUTTON_REPEAT_RATE 120   // ms between auto-repeat steps while a button is held
#define DEBOUNCE_TIME 30         // ms contact debounce (stops one physical tap registering twice)


/////////////SHARED TYPES//////////////////////
struct SortedCard {
  int index;
  String name;
};

struct ShopItem {
  int itemIndex;
  int spriteIndex;
  String name1;
  String name2;
  int cost;
  int availability;
  String desc1;
  String desc2;
  String desc3;
};

enum BattleState { COMMENCED, CARDSELECT, ACTION1, ACTION2, ARENA, END };
enum EventState { EVENT_IDLE, EVENT_WALKING, EVENT_ALERT, EVENT_PROMPT, EVENT_RESULT };
enum EventType {
  EVENT_NONE = -1,
  EVENT_SPARE_CHANGE,
  EVENT_REFRESHING,
  EVENT_CHARGED_UP,
  EVENT_DOWNER,
  EVENT_MERCHANT,
  EVENT_WOUNDED_ENEMY,
  EVENT_HARVEST_BOUNTY,
  EVENT_AMBUSH,
  EVENT_PICKPOCKET,
  EVENT_FREEBIE,
  EVENT_COUNT
};
enum ActionState { IDLE, WALKING, SLEEPING, WAKE, EATING, ATTACKING, REJECT, HURT, INJURED };
enum Emotion { NORMAL, TIRED, HUNGRY, H_T, ANGRY, UNHAPPY, HAPPY, ALERT, SKULL, REVIVE };
enum AppMenu { NA, INVENTORY, DECK, VENTURE, SHOP, WARDROBE, SLEEP, OPTIONS };
enum MenuState {
  TITLE, TUTORIAL, MAINMENU, STATS1, STATS2, INVENTORYMENU, DECKMENU,
  DECKCARDSELECT, DECKLIST, VENTUREMENU, FARM, SEEDSELECT, SHOPMENU,
  WARDROBEMENU, WRDSELECT, WRDLIST, OPTIONSMENU, BATTLE, STATS3, STATS4
};
enum OptionsSetting {
  PAUSE, PAUSECANCEL, PAUSECONFIRM, BACKLIGHT, BLLOW, BLMED, BLHIGH,
  LED, LEDLOW, LEDMED, LEDHIGH, LEDOFF, TIMEOUT, TIMEOUTSET, LEDTIME, LEDTIMESET, BACKUP, BACKUPCONFIRM, BACKUPCANCEL,
  BACKUPWAIT, RESTORE, RESTORECONFIRM, RESTORECANCEL, RESTOREWAIT, CLOCK,
  POWEROFF, POWEROFFCONFIRM, POWEROFFCANCEL, FACTORYRESET, FRCANCEL, FRCONFIRM, ABOUT
};
enum VentureState { INACTIVE, VENTURING, ENCOUNTER };
enum TooltipState { ACTIVE, RELEASED };
enum BlinkState { OFF, BLINK_ON, BLINK_OFF, WAIT };

///////////////Pet Stuff//////////////////////
extern TFT_eSPI tft;
extern TFT_eSprite display;
extern TFT_eSprite playerSprite;
extern TFT_eSprite emoteSprite;
extern TFT_eSprite itemSprite;
extern uint16_t combinedImage[48 * 48];
extern uint16_t combinedMirrorImage[48 * 48];
extern uint16_t tempImage[48 * 48];
extern uint16_t* pet[12];
extern uint16_t* mirrorPet[12];
extern int xPosition;
extern bool moveLeft;
extern int randomIdle;
extern String name;
extern int Hunger;
extern int hungerCycles;
extern int hungerRegenCycles;
extern int Stamina;
extern int staminaCycles;
extern unsigned long sleepStartTime;
extern int Mood;
extern int moodCycles;
extern unsigned long lastSaveTime;
extern int maxHP;
extern int HP;
extern int baseAttack;
extern int baseDefense;
extern int baseSpeed;
extern int Attack;
extern int Defense;
extern int Speed;
extern int attackStage;
extern int defenseStage;
extern int speedStage;
extern int gemsIndex[3];
extern int inventoryAdr[10];
extern int inventoryQty[10];
extern int invSelect;
extern int menuItem;
extern int maxItems;
extern int pageNum;
extern int menuIndex;
extern int menuValues[4];
extern bool empty;
extern int selectedVentureMenu;
extern byte headOwnership[32];
extern byte bodyOwnership[32];
extern uint16_t* wardrobePet[3];
extern int equippedWardrobe;
extern int selectedWardrobe;
extern String wardrobeSlot;
extern byte cardOwnership[32];
extern int selectedDeck;
extern int equippedDeck;
extern int selectedMenuCard;
extern int deckIndex[10];
extern Card playerDeck[10];

//ENEMY VARIABLES//
extern TFT_eSprite enemySprite;
extern uint16_t combinedEnemyImage[48 * 48];
extern uint16_t combinedMirrorEnemyImage[48 * 48];
extern uint16_t* enemyPet[12];
extern uint16_t* mirrorEnemyPet[12];
extern String enemyName;
extern int enemyMaxHP;
extern int enemyHP;
extern int enemyLevel;
extern int enemyAttack;
extern int enemyDefense;
extern int enemySpeed;
extern int enemyAttackStage;
extern int enemyDefenseStage;
extern int enemySpeedStage;
extern int enemyGems[3];
extern Card enemyDeck[10];

//SHOP STUFF//
extern int playerCredits;
extern int shopSelection;
extern int shopItemCount;
extern int shopInventory[10];
extern bool shopPurchaseConfirm;
extern bool shopSelectYes;
extern bool shopRefresh;
extern bool shopRareItemPurchased[10];
extern bool shopShowingReward;
extern int shopRewardType;
extern int shopRewardIndex;
extern String shopRewardName;
extern ShopItem shopItemList[10];

//VENTURE MODE//
extern int ventureRewardCards[10];
extern int ventureRewardBody[10];
extern int ventureRewardHead[10];
extern int maxStage;
extern int stage;
extern int ventureOpponent;
extern int countdownSeconds;
extern int ventureTime;
extern int injuryTime;
extern int requiredStamina;
extern bool randomEnemyGenerated;
extern bool enemySpriteCreated;
extern int randomEnemyHead;
extern int randomEnemyBody;
extern int randomEnemyDeck[10];
extern bool ventureCancelConfirm;
extern bool ventureCancelSelectYes;
extern int deepDungeonHighScore;
extern bool deepDungeonUnlocked;
extern bool deepDungeonActive;
extern uint16_t deepDungeonStage;
extern byte ddCardOwnership[32];
extern int activeGem;            // DD run: chosen gem id (0-8), -1 = none
extern bool ddReviveUsed;        // DD run: REVIVE gem consumed
extern String effectMsgQueue[8];
extern int effectMsgTargetQ[8];
extern int effectMsgCount;
extern int effectMsgIndex;
extern int activeMerits[9];      // DD run: active merit ids (-1 = empty slot)
extern int activeMeritCount;     // DD run: number of active merits
extern int activeAfflictions[9];     // DD run: active affliction ids (-1 = empty)
extern int activeAfflictionLevels[9];// DD run: level (1-3) of each active affliction
extern int activeAfflictionCount;    // DD run: number of active afflictions
extern bool statDrill;               // status menu: drilled into merit/affliction list
extern int statDrillSel;             // status menu: focused list index
#define DD_BUYIN 50              // flat credit cost to enter a Deep Dungeon run
#define DD_MERIT_COUNT 53        // number of merits available to draft from

// Merit ids (match the ddMeritNames/Desc array order).
enum Merit {
  MERIT_WINDRIDER, MERIT_INFLOW, MERIT_SCORCHING, MERIT_TECTONICS, MERIT_HIGH_VOLTAGE,
  MERIT_FIRE_AMP, MERIT_WATER_AMP, MERIT_WIND_AMP, MERIT_EARTH_AMP,
  MERIT_FIRE_SURGE, MERIT_WATER_SURGE, MERIT_WIND_SURGE, MERIT_EARTH_SURGE,
  MERIT_FIRE_TD, MERIT_WATER_TD, MERIT_WIND_TD, MERIT_EARTH_TD,
  MERIT_CONDUIT, MERIT_OVERCHARGE, MERIT_EXECUTE, MERIT_WHIPLASH, MERIT_BLOODRUSH,
  MERIT_SHARPEN, MERIT_BOLSTER, MERIT_TEMPER, MERIT_HASTE, MERIT_KEEN_EYE,
  MERIT_LEECH, MERIT_DESPERATION, MERIT_SCHOLAR, MERIT_BERSERK, MERIT_GUTS,
  MERIT_TECHNICIAN, MERIT_SPEED_BOOST, MERIT_SECOND_WIND, MERIT_REGEN, MERIT_VITALITY,
  MERIT_SPIKES, MERIT_AMBUSH, MERIT_PHANTOM, MERIT_DEEP_FREEZE, MERIT_WARD,
  MERIT_OMEN, MERIT_FROSTGUARD, MERIT_INTIMIDATE, MERIT_REFORGE, MERIT_STACKED_DECK,
  MERIT_SCAVENGE, MERIT_BOUNTY, MERIT_REPRIEVE, MERIT_MOXIE, MERIT_PURGE, MERIT_WINDFALL
};

// Merit draft rarity tiers. R_NONE keeps a merit out of the draft pool
// (used for merits not yet implemented).
enum MeritRarity { R_NONE, R_COMMON, R_UNCOMMON, R_RARE };

#define DD_AFFLICTION_COUNT 15   // number of afflictions

// Affliction ids (match the ddAfflictionNames/Desc array order).
enum Affliction {
  AFFL_FEROCITY, AFFL_STALWART, AFFL_VIGOR, AFFL_FRENZY, AFFL_RUSH,
  AFFL_ENRAGE, AFFL_RENEW, AFFL_DRAIN, AFFL_DISPEL, AFFL_TOXIC,
  AFFL_SLOTH, AFFL_RECOIL, AFFL_BLIGHT, AFFL_DREAD, AFFL_VOLATILE
};
extern bool ddExitConfirm;
extern bool ddGemSelect;         // DD gem-select screen active
extern int ddGemChoices[3];      // the 3 rolled gem ids offered
extern int ddGemSel;             // highlighted choice (0-2)
extern bool ddGemsRolled;        // true once the 3 gems are rolled (persists until a run begins)
extern bool ddGemConfirm;        // gem enter-confirmation screen active
extern bool ddGemConfirmYes;     // confirm selection (false = NO default)
extern bool ddMeritSelect;       // DD merit-select screen active
extern int ddMeritChoices[3];    // the 3 rolled merit ids offered
extern bool ddMeritDiscard;      // over-cap: choosing a merit to discard
extern bool ddMeritConfirm;      // over-cap: confirm the discard
extern int ddDiscardSel;         // highlighted discard candidate
extern bool ddDiscardConfirmYes; // discard confirm YES/NO toggle
extern int pendingMerit;         // merit awaiting placement when at cap
extern int ddMeritSel;           // highlighted merit (0-2)
extern bool ddCardDraft;         // DD card-draft screen active
extern int ddCardChoices[3];     // the 3 rolled card ids offered this round
extern int ddCardSel;            // highlighted card (0-2)
extern int ddDraftRound;         // current draft round (0-2)
extern int ddDraftPicks[3];      // the 3 cards picked across the draft
extern bool ddExitSelectNo;
extern bool ddUnlockShown;
extern bool scryGlassActive;
extern bool floorMapActive;
extern bool battleReady;
extern int battleAutoAdvance;
extern String battleArrowUser;
extern int battleArrowTarget;
extern bool cooldownDoneNotify;
extern unsigned long cooldownDoneTime;

//FARM STUFF//
extern int selectedPot;
extern int potCrop[3];
extern int potStage[3];
extern int potTimer[3];
extern unsigned long lastFarmUpdate;
extern int farmAnimFrame;
extern String harvestMessage;
extern unsigned long harvestMessageStart;

//BATTLE STUFF//
extern BattleState BATTLESTATE;
extern String beginTurn;
extern int actionTime;
extern int selectedBattleCard;
extern int enemyBattleCard;
extern String selectionMode;
extern int selectedCounter;
extern String playerCounter;
extern String enemyCounter;
extern String arenaCounter;
extern String firstPlayer;
extern String secondPlayer;
extern String battleMessage;
extern int equippedGem;
extern int battleTurnCount;
extern int pressureUsageCount;
extern bool payoutUsed;
extern bool playerUsedBlock;
extern bool enemyUsedBlock;
extern bool playerLifesteal;
extern bool whiplashPrimed;
extern bool stackedDeckUsed;
extern bool enemyLifesteal;
extern int playerLastDamage;
extern int enemyLastDamage;
extern Counter tooltipCounter;
extern Card tooltipCard;
extern bool concedeMenuActive;
extern bool concedeSelectNo;

//RANDOM EVENT STUFF//
extern bool randomEventPending;
extern EventType pendingEventType;
extern EventState eventState;
extern unsigned long lastEventCheck;
extern unsigned long eventWalkStart;
extern int eventTargetX;
extern String eventResultText1;
extern String eventResultText2;
extern String eventResultText3;
extern bool eventShowCharacter;
extern int eventCharacterAction;
extern int eventCharacterX;
extern bool chargedUpActive;
extern bool downerActive;
extern bool merchantDiscountActive;
extern bool woundedEnemyActive;
extern int eventFreebieItem;

extern ActionState ACTION;
extern Emotion EMOTION;
extern int randomPos;
extern int playerFrames;
extern int enemyFrames;
extern String enemyAction;
extern int emoteFrames;
extern int itemFrames;

///////////////UI////////////////////////////
extern int animFrames;
extern int enemyAnimFrames;
extern int batteryFrames;
extern int tutorialPage;
extern const char charSet[];
extern const int charSetLength;
extern char currentChar;
extern char tempName[11];
extern int charIndex;
extern bool confirmName;
extern String buildName;
extern bool debug;
extern unsigned long lastButtonPressTime;
extern int mainMenuReturn;
extern AppMenu AMENUS;
extern MenuState MENUSTATE;
extern OptionsSetting OPTIONSSETTINGS;
extern VentureState VENTURESTATE;
extern TooltipState TOOLTIP;

//***HARDWARE
extern Adafruit_NeoPixel strip;
extern BlinkState state;
extern unsigned long previousMillis;
extern int blinkCount;
extern int ledBrightness;
extern int ledPercent;
extern bool ledActive;
extern String ledColor;
extern bool chargeNotification;
extern bool doNotDisturb;
extern bool cropReady;
extern bool setDND;
extern bool setWrd;
extern bool setDck;
extern bool setVnt;
extern bool testActive;
extern int batteryAnim;
extern Pangodream_18650_CL BL;
extern bool isCharging;
extern int percentage;
extern int chargeInitLoops;
extern int cpu;
extern bool exportTriggered;
extern int screenOffTime;
extern bool screenOff;
extern int screenBrightness;
extern int backlightPercent;
extern int screenTimeout;
extern int ledTimeout;
extern String COLOR1;
extern String COLOR2;
extern String COLOR3;
extern String COLOR4;
extern int button1State;
extern int button2State;
extern String AButton;
extern String BButton;
extern unsigned long button1PressTime;
extern unsigned long button2PressTime;
extern unsigned long button1RepeatTime;
extern unsigned long button2RepeatTime;
extern bool eatSkipReady;
extern int skipCheerStage;
extern unsigned long emergencyShutdownStart;
extern char timeStr[6];
extern bool twelveHr;
extern bool gamePause;
extern unsigned long lastUpdateTime;
extern int countdown;
extern bool countdownComplete;


// Sprite arrays (defined once via sprites.h in vPet.ino; declared here for system files)
extern const uint16_t* items[];
extern const uint16_t* smallIcons[];
extern const uint16_t* FoodFarm[];
extern const uint16_t* Battery[];
extern const uint16_t* cropReadyIcon[];
extern const uint16_t* emotes[];
extern const uint16_t* titleScreen[];

// Card / gear / menu data arrays (defined once in their data headers, included only in vPet.ino)
extern Card cardList[];
extern const uint16_t* cardSpriteArray[];
extern const uint16_t* gems[];
extern const uint16_t* headGearIcons[];
extern const uint16_t* bodyGearIcons[];
extern const char* headNames[];
extern const char* bodyNames[];
extern const uint16_t* abilityFrame[];
extern const uint16_t* equipmentFrame[];
extern const uint16_t* OptionsArray[];
extern Item itemList[];
extern Item gemList[];
extern const uint16_t* headGear[][12];
extern const uint16_t* bodyGear[][12];
extern const int ventureEnemy[][14];
extern const int bossEnemy[][14];
extern const char* bossNames[];
extern int randomBossIndex;
extern bool bossRewardPending;
extern bool bossMeritDraft;
extern bool bossAfflictionPopup;
extern String bossAfflictionMsg;
extern int bossAfflictionId;
extern int bossAfflictionLvl;

// Deep Dungeon stat scaling (HP is always fixed at 25; only the non-HP total scales).
#define DD_STAT_TIER_STEP 4     // non-HP base total added per cleared boss (every 10 floors)
#define DD_BOSS_STAT_MULT 1.3   // boss base-30 multiplier: bossBST = (30 * mult) + floor bonus
extern Counter counterList[];
extern const uint16_t* counterArray[];
extern const char* names[];
extern const uint16_t* blank[];
extern const uint16_t* petBody[];
extern const bool bodyPriority[];

#include "declarations.h"
