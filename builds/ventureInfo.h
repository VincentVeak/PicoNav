#if defined(__AVR__)
    #include <avr/pgmspace.h>
#elif defined(__PIC32MX__)
    #define PROGMEM
#elif defined(__arm__)
    #define PROGMEM
#endif

#pragma region ENEMIES
// Create an array of enemies
extern const int ventureEnemy[][14] = {
//headGear, bodyGear, equippedGem, deck
//{rP, hG, bG, eG, d0, d1, d2, d3, d4, d5, d6, d7, d8, d9},
  {0, 3, 3, 0, 5, 3, 3, 5, 9, 9, 7, 7, 18, 2},
  {0, 30, 2, 0, 4, 1, 1, 7, 7, 18, 12, 3, 2, 2},
  {0, 8, 0, 0, 35, 2, 2, 35, 3, 9, 9, 7, 7, 12},
  {0, 1, 1, 0, 8, 0, 0, 8, 12, 12, 9, 9, 3, 3},
  {0, 21, 14, 0, 47, 0, 0, 47, 1, 1, 12, 7, 7, 4},
  {0, 50, 30, 0, 51, 2, 2, 51, 35, 35, 12, 18, 47, 47},
  {0, 9, 0, 0, 27, 1, 1, 27, 4, 12, 12, 18, 47, 7},
  {0, 35, 0, 0, 20, 47, 0, 0, 8, 3, 3, 5, 18, 9},
  {0, 12, 0, 0, 33, 2, 2, 33, 35, 18, 9, 9, 12, 3},
  {0, 24, 4, 0, 44, 3, 3, 44, 5, 5, 9, 9, 0, 47},
  {0, 40, 28, 0, 29, 1, 1, 29, 18, 12, 12, 0, 27, 47},
  {0, 28, 15, 0, 11, 0, 0, 11, 8, 12, 47, 9, 9, 3},
  {0, 27, 24, 0, 6, 35, 35, 6, 33, 33, 2, 2, 12, 47},
  {0, 41, 11, 0, 24, 18, 18, 24, 1, 1, 29, 20, 12, 12},
  {0, 29, 22, 0, 41, 3, 3, 41, 44, 5, 5, 9, 9, 18},
  {0, 20, 5, 0, 21, 1, 1, 4, 4, 21, 12, 12, 20, 18},
  {0, 18, 9, 0, 40, 6, 21, 1, 1, 33, 33, 40, 6, 35},
  {0, 46, 20, 0, 30, 29, 1, 1, 30, 27, 18, 47, 9},
  {0, 15, 4, 0, 19, 27, 1, 1, 19, 30, 30, 27, 12, 12},
  {0, 16, 0, 0, 42, 3, 3, 44, 0, 0, 47, 30, 30, 9},
  {0, 10, 10, 0, 39, 2, 2, 39, 6, 35, 3, 40, 30, 33},
  {0, 31, 3, 0, 13, 1, 1, 4, 13, 27, 30, 24, 47, 18},
  {0, 14, 17, 0, 23, 24, 1, 1, 23, 30, 47, 21, 8, 8},
  {0, 39, 9, 0, 17, 11, 11, 17, 3, 3, 12, 44, 47, 18},
  {0, 4, 19, 0, 16, 17, 11, 11, 17, 16, 3, 3, 12, 44},
  {0, 17, 15, 0, 49, 23, 24, 19, 19, 23, 47, 47, 1, 1},
  {0, 13, 19, 0, 32, 12, 12, 32, 1, 1, 13, 30, 4, 27},
  {0, 34, 21, 0, 10, 21, 21, 10, 1, 1, 8, 8, 40, 40},
  {0, 44, 0, 0, 43, 44, 3, 3, 5, 5, 44, 43, 42, 41},
  {0, 11, 0, 0, 36, 7, 7, 36, 9, 9, 47, 47, 12, 12},
};

#pragma endregion

#pragma region BOSSES
// Boss decks. Same 14-column format as ventureEnemy: {_, headGear, bodyGear, gem, d0..d9}.
// One boss is picked at random on each boss floor (every 10 floors). Their stats are
// scaled automatically (see InitializeEnemy) so only gear + deck are defined here.
// Add as many rows as you like; edit the placeholders below.
extern const int bossEnemy[][14] = {
//{rP, hG, bG, eG, d0, d1, d2, d3, d4, d5, d6, d7, d8, d9},
  {0, 33, 21, 0, 5, 5, 3, 3, 9, 9, 47, 47, 12, 18},   // placeholder - EDIT
  {0, 44, 30, 0, 51, 51, 35, 35, 12, 12, 18, 18, 47, 2}, // placeholder - EDIT
};

const char* bossNames[] = {
  "OVERSEER",
  "WARDEN",
};
#pragma endregion

#pragma region NAMES

const char* names[] = {
  "DAVE",
  "MILO",
  "CLEMENTINE",
  "ZAHRA",
  "CASSIUS",
  "SEBASTIAN",
  "TAHLIA",
  "LLOYD",
  "MACIE",
  "RHEA",
  "MARV",
  "DEACON",
  "JAGO",
  "STU",
  "VINCENZO",
  "GAVIN",
  "BEATRIX",
  "ARUN",
  "LILLI",
  "FABIAN",
  "GIOVANNI",
  "RAIHAN",
  "JULIUS",
  "FREDERICK",
  "IVY",
  "RUSSELL",
  "CLARK",
  "SIMON",
  "FREYA",
  "LARZ",
  "OVILIA",
  "AVA",
  "RILEY",
  "EVIL DAVE",
  "MATTHIAS",
  "FERGUSON",
  "GABE",
  "DOMINIK",
  "MEL",
  "THEODORE",
  "ALESHA",
  "PHUONG",
  "ZEKE",
  "METTE",
  "TITO",
  "NAOMI",
  "FRANCIS",
  "MIMI",
  "IORI",
  "NORIS",
  "BALTHAZAR",
  "MEKKI",
  "OUS",
  "OTIS",
  "PEARL",
  "MAVIS",
  "GLADYS",
  "CHESTER",
  "HILDA",
  "MORRIS",
  "ENID",
  "WILBUR",
  "DAPHNE",
  "RUFUS",
  "MAUDE",
  "GIDEON",
  "LORNA",
  "ULYSSES",
  "PRUDENCE",
  "HORACE",
  "BERNARD",
  "MABEL",
  "ROSCOE",
  "EDITH",
  "LUTHER",
  "OPAL",
  "AMOS",
  "HARRIET",
  "PERCIVAL",
  "WINNIE",
  "ANGUS",
  "GERTRUDE",
  "BERTHA",
  "FORREST"
  "CECIL",
  "MYRTLE",
  "ATSU",
  "CORNELIUS",
  "BLANCHE",
  "BARNABY",
  "KIJANA",
  "LORENZO",
  "AGNES",
  "HUMPHREY",
  "IRMA",
  "DORIAN",
  "ESTHER",
  "CLYDE",
  "MAXINE",
  "WOLFGANG",
  "MARGO",
  "HERBERT",
  "ARCHIE",
  "VELMA",
  "WINSTON",
  "ETHEL",
  "CLARENCE",
  "WINFRED",
  "EUGENE",
  "ZELMA",
  "BAXTER",
  "MILDRED",
  "ERNIE",
  "REGINALD",
  "GERALD",
  "VANESSA",
  "MORTIMER",
  "DOROTHY",
  "RUPERT",
  "LUCILLE",
  "CEDRIC",
  "BERDIE",
  "IGNACIO",
  "VIOLA",
  "MONA",
  "WALDEN",
  "PHYLLIS",
  "AMBROSE",
  "DELILAH",
  "SHERA",
  "ESTELLE",
  "WINSTON",
  "SIMON",
  "ALISTAIR",
  "IRIS",
  "MAGNUS",
  "VIRGIL",
  "SYLVIA",
  "JASPER",
  "OMAR",
  "RAMONA",
  "SILAS",
  "WANDA",
  "BASIL",
  "VERA",
  "HECTOR",
  "LOUISA",
  "MURDOC",
  "MONTY",
  "CONSTANCE",
  "MILLICENT",
  "ELMER",
  "VIVIAN",
  "CORDELIA",
  "PERCY",
  "GEORGINA",
  "ROLAND",
  "BENEDICT",
  "SEYMOUR",
  "FLORENCE",
  "OSWALD",
  "ALBERTA",
  "EMMETT",
  "MILTON",
  "LAVINIA",
  "AUGUSTINE",
  "CORINNE",
  "BARTHOL",
  "MEREDITH",
  "LEOPOLD",
  "QUINCY",
  "SHELDON",
  "WERNER",
  "ODETTE",
  "ROWAN",
  "CYRUS",
  "MIRANDA",
  "LENORA",
  "ORVILLE",
  "DORINDA",
  "WALFRED",
  "FRANCINE",
  "NELL",
  "CLAUDE",
  "DOLORES",
  "BORAN",
  "EPHRAIM",
  "DARCY"

  

  // Add more strings as needed
};

#pragma endregion























