#include "globals.h"

#define SHOW_UNBUILT 0   // 1 = list unbuilt activities as grayed placeholders

enum ActivityId { ACT_FARM, ACT_PET, ACT_HIDE, ACT_SLOTS, ACT_BLACKJACK, ACT_COUNT };

static const char* activityNames[ACT_COUNT] = { "FARM", "PET", "HIDE & SEEK", "SLOTS", "BLACKJACK" };
static int activitySel = 0;
static int visibleIds[ACT_COUNT];
static int visibleCount = 0;

// Activities that are built and can be opened (the rest are placeholders for now)
static bool ActivityReady(int id) {
  return id == ACT_FARM;
}

static void BuildVisibleList() {
  visibleCount = 0;
  for (int i = 0; i < ACT_COUNT; i++)
    if (SHOW_UNBUILT || ActivityReady(i))
      visibleIds[visibleCount++] = i;
}

// Three bordered rows like the merit list; unbuilt activities show grayed text
static void DrawActivityRows() {
  int top = (activitySel / 3) * 3;
  for (int r = 0; r < 3; r++) {
    int row = top + r;
    int by = 33 + r * 33;
    if (row >= visibleCount) {
      tft.fillRect(0, by, 240, 30, TFT_COLOR4);
      continue;
    }
    int id = visibleIds[row];
    bool sel = row == activitySel;
    uint16_t bg = sel ? TFT_COLOR2 : TFT_COLOR3;
    uint16_t fg = sel ? TFT_COLOR3 : (ActivityReady(id) ? TFT_COLOR1 : TFT_COLOR2);
    tft.fillRect(3, by + 3, 234, 27, TFT_COLOR1);
    tft.fillRect(0, by, 234, 27, bg);
    tft.setTextColor(fg, bg);
    CenterText(activityNames[id], by + 3);
  }
}

void DrawActivitiesMenu() {
  MENUSTATE = ACTIVITIESMENU;
  BuildVisibleList();
  tft.fillScreen(TFT_COLOR4);
  drawingBatteryIcon(batteryAnim);
  int w = tft.textWidth("ACTIVITIES") + 3;
  tft.fillRect(3, 3, w, (9 * 3), TFT_COLOR1);
  tft.fillRect(0, 0, w, (9 * 3), TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString("ACTIVITIES", 3, 3);
  DrawActivityRows();
  mainMenuReturn = 0;
}

void OpenActivities() {
  activitySel = 0;
  DrawActivitiesMenu();
}

// A cycles the list, B opens the selected activity, hold A returns home
void ActivitiesInput() {
  if (AButton == "released" && setVnt)
    setVnt = false;
  if (AButton == "pressed" && millis() - lastButtonPressTime > 500) {
    lastButtonPressTime = millis();
    if (screenOff == false) {
      int prevTop = activitySel / 3;
      activitySel = (activitySel + 1) % visibleCount;
      if (activitySel / 3 != prevTop)
        tft.fillRect(0, 28, 240, 107, TFT_COLOR4);
      DrawActivityRows();
      mainMenuReturn = 0;
    }
    if (screenOff)
      screenOff = false;
  }
  if (BButton == "pressed" && millis() - lastButtonPressTime > 500) {
    lastButtonPressTime = millis();
    if (screenOff == false) {
      mainMenuReturn = 0;
      if (visibleIds[activitySel] == ACT_FARM)
        OpenFarm();
    }
    if (screenOff)
      screenOff = false;
  }
}
