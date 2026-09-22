#include "globals.h"
#include <EEPROM.h>

void SaveFarmData() {
  for(int i = 0; i < 3; i++) {
    EEPROM.write(230 + i, potCrop[i]);
    EEPROM.write(233 + i, potStage[i]);
    EEPROM.put(236 + (i * 4), potTimer[i]);
  }
  EEPROM.commit();
}

void UpdateFarm() {
  if(millis() - lastFarmUpdate < 60000 ) // Check every minute (60000ms)
    return;
  
  lastFarmUpdate = millis();
  bool farmChanged = false;
  
  for(int i = 0; i < 3; i++) {
    if(potCrop[i] > 0) {
      potTimer[i]++; // Increment minute timer
      
      // Update growth stages based on 6-hour system (360 minutes)
      if(potTimer[i] >= 360) {
        potStage[i] = 5;
      }
      else if(potTimer[i] >= 180) { // 180 minutes = 3 hours = bud stage
        potStage[i] = 3;
      }
      else { // 0-180 minutes = sprout stage
        potStage[i] = 1;
      }
      farmChanged = true;
    }
  }

  bool tempCropReady = false;
  for(int i = 0; i < 3; i++) {
    if(potCrop[i] > 0 && potTimer[i] >= 360) {
      tempCropReady = true;
      break;
    }
  }
  cropReady = tempCropReady;
  
  if(farmChanged) {
    SaveFarmData();
    if(MENUSTATE == FARM) {
      for(int i = 0; i < 3; i++) {
        DrawPot(i);
      }
      
    } else if(MENUSTATE == VENTUREMENU && selectedVentureMenu == 0){
      tft.fillRect(0, 30, 153, 78, TFT_COLOR4);
      tft.fillRect(0, 105, 240, 3, TFT_COLOR1);
      for(int i = 0; i < 3; i++) {
        int potX = 3 + (i * 51);
        int potY = 60;
        
        tft.fillRect(potX + 6, 48, 30, 9, TFT_COLOR3); // Background bar
        if(potCrop[i] > 0) {
          int progressPercent = (potTimer[i] * 100) / 360;
          if(progressPercent > 100) 
            progressPercent = 100;
          int progressWidth = (progressPercent * 30) / 100;
          progressWidth = (progressWidth / 3) * 3;
          if(progressWidth > 0) {
            if(progressPercent < 100)
              tft.fillRect(potX + 9, 45, progressWidth, 9, TFT_COLOR1);
            else
              tft.fillRect(potX + 9, 45, progressWidth, 9, TFT_COLOR2);
          }
        }
        
        // Draw plant sprites
        if(potStage[i] == 0) {
          pushScaled(potX, potY, 16, 16, FoodFarm[0], TFT_BLACK);
        }
        else if(potTimer[i] < 240) { //sprout
          pushScaled(potX, potY, 16, 16, FoodFarm[1], TFT_BLACK);
        }
        else if(potTimer[i] < 360) { //bud
          pushScaled(potX, potY, 16, 16, FoodFarm[3], TFT_BLACK);
        }
        else if(potStage[i] == 5) {
          if(potCrop[i] == 1) pushScaled(potX, potY, 16, 16, FoodFarm[5], TFT_BLACK);
          else if(potCrop[i] == 2) pushScaled(potX, potY, 16, 16, FoodFarm[7], TFT_BLACK);
          else if(potCrop[i] == 3) pushScaled(potX, potY, 16, 16, FoodFarm[9], TFT_BLACK);
        }
      }
    }
  }
}

void PlantSeed(int seedType) {
  potCrop[selectedPot] = seedType;
  potStage[selectedPot] = 1; // Start with sprout stage
  potTimer[selectedPot] = 0;
  bool tempCropReady = false;
  for(int i = 0; i < 3; i++) {
    if(potCrop[i] > 0 && potTimer[i] >= 360) {
      tempCropReady = true;
      break;
    }
  }
  cropReady = tempCropReady;
  SaveFarmData();
  EEPROM.commit();
  
  // Return to farm view
  MENUSTATE = FARM;
  tft.fillScreen(TFT_COLOR4);
  DrawBackground();
  tft.fillRect(3, 3, 75, 27, TFT_COLOR1);
  tft.fillRect(0, 0, 75, 27, TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString("FARM", 3, 3);
  tft.fillRect(0, 105, 240, 3, TFT_COLOR1);
  tft.fillRect(0, 108, 240, 27, TFT_COLOR3);
  DrawFarm();
  DrawFarmArrow();
  for(int i = 0; i < 3; i++) {
    DrawPot(i);
  }
  if(potCrop[selectedPot] == 1) 
    CenterText("SWEET FRUIT", 111);
  else if(potCrop[selectedPot] == 2) 
    CenterText("ENERGY DRINK", 111); 
  else if(potCrop[selectedPot] == 3) 
    CenterText("HEALTH POTION", 111);
}

void HarvestPlant() {
  if(potCrop[selectedPot] > 0 && potTimer[selectedPot] >= 360) {
    int itemAddress = 0;
    if(potCrop[selectedPot] == 1) itemAddress = 143; // Sweet Fruit
    else if(potCrop[selectedPot] == 2) itemAddress = 144; // Energy Drink
    else if(potCrop[selectedPot] == 3) itemAddress = 145; // Health Potion
    
    // Check if inventory is full for this item
    if(IsInventoryFull(itemAddress)) {
      harvestMessage = "INVENTORY FULL";
      harvestMessageStart = millis();
      return;
    }
    
    // Determine harvest amount (1-3 items)
    int harvestAmount = random(1, 4);
    
    // Find inventory slot or add to existing
    bool itemAdded = false;
    for(int i = 0; i < 10; i++) {
      if(inventoryAdr[i] == itemAddress) {
        inventoryQty[i] += harvestAmount;
        if(inventoryQty[i] > 9) inventoryQty[i] = 9;
        EEPROM.write(itemAddress, inventoryQty[i]);
        itemAdded = true;
        break;
      }
      else if(inventoryAdr[i] == 0) {
        inventoryAdr[i] = itemAddress;
        inventoryQty[i] = harvestAmount;
        if(inventoryQty[i] > 9) inventoryQty[i] = 9;
        EEPROM.write(itemAddress, inventoryQty[i]);
        itemAdded = true;
        break;
      }
    }
    
    // Set harvest message
    harvestMessage = "HARVESTED " + String(harvestAmount);
    harvestMessageStart = millis();
    
    // Clear the pot data
    potCrop[selectedPot] = 0;
    potStage[selectedPot] = 0;
    potTimer[selectedPot] = 0;
    if(selectedPot == 0)
      tft.fillRect(9, 36, 9, 60, TFT_COLOR4);
    if(selectedPot == 1)
      tft.fillRect(81, 36, 9, 60, TFT_COLOR4);
    if(selectedPot == 2)
      tft.fillRect(153, 36, 9, 60, TFT_COLOR4);
    DrawPot(selectedPot);

    bool tempCropReady = false;
    for(int i = 0; i < 3; i++) {
      if(potCrop[i] > 0 && potTimer[i] >= 360) {
        tempCropReady = true;
        break;
      }
    }
    cropReady = tempCropReady;
    SaveFarmData();
    EEPROM.commit();
    DrawFarm();
  }
}

void DrawFarm() {
  static unsigned long lastTime = 0;
  if (millis() - lastTime < 750) {
    DrawHarvestMessage();
    return;
  }
  
  lastTime = millis();
  farmAnimFrame = (farmAnimFrame + 1) % 2;
  for(int i = 0; i < 3; i++) {
    if(potCrop[i] > 0) {
      DrawPot(i);
    }
  }
  DrawHarvestMessage();
}

void DrawPot(int potIndex) {
  int potX = 24 + (potIndex * 72);
  int potY = 60;
  
  // Always redraw progress bar (no static tracking)
  tft.fillRect(potX - 12, 39, 9, 60, TFT_COLOR3);
  if(potCrop[potIndex] > 0) {
    int progressPercent = (potTimer[potIndex] * 100) / 360;
    if(potStage[potIndex] == 5)
      progressPercent = 100;
    if(progressPercent > 100) progressPercent = 100;
    int progressHeight = (progressPercent * 60) / 100;
    progressHeight = (progressHeight / 3) * 3;
    
    if(progressHeight > 0) {
      if(progressPercent != 100)
        tft.fillRect(potX - 15, 96 - progressHeight, 9, progressHeight, TFT_COLOR1);
      else
        tft.fillRect(potX - 15, 96 - progressHeight, 9, progressHeight, TFT_COLOR2);
    }
  }
  
  if(potStage[potIndex] == 0) {
    pushScaled(potX, potY, 16, 16, FoodFarm[0], TFT_BLACK);
  }
  else if(potTimer[potIndex] < 240) {
    int spriteIndex = 1 + farmAnimFrame;
    pushScaled(potX, potY, 16, 16, FoodFarm[spriteIndex], TFT_BLACK);
  }
  else if(potTimer[potIndex] < 360) {
    int spriteIndex = 3 + farmAnimFrame;
    pushScaled(potX, potY, 16, 16, FoodFarm[spriteIndex], TFT_BLACK);
  }
  else if(potStage[potIndex] == 5) {
    if(potCrop[potIndex] == 1) {
      int spriteIndex = 5 + farmAnimFrame;
      pushScaled(potX, potY, 16, 16, FoodFarm[spriteIndex], TFT_BLACK);
    } else if(potCrop[potIndex] == 2) {
      int spriteIndex = 7 + farmAnimFrame;
      pushScaled(potX, potY, 16, 16, FoodFarm[spriteIndex], TFT_BLACK);
    } else if(potCrop[potIndex] == 3) {
      int spriteIndex = 9 + farmAnimFrame;
      pushScaled(potX, potY, 16, 16, FoodFarm[spriteIndex], TFT_BLACK);
    }
  }
}

void DrawFarmArrow() {
  // Clear all arrow positions first
  tft.fillRect(39, 33, 15, 15, TFT_COLOR4);
  tft.fillRect(111, 33, 15, 15, TFT_COLOR4);
  tft.fillRect(183, 33, 15, 15, TFT_COLOR4);
  int arrowX = 39 + (selectedPot * 72);
  pushScaled(arrowX, 33, 5, 5, smallIcons[13], TFT_BLACK);
}

void DrawHarvestMessage() {
  if(harvestMessage != "" && millis() - harvestMessageStart < 2000) {
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    CenterText(harvestMessage, 111);
  }
  else if(harvestMessage != "" && millis() - harvestMessageStart >= 2000) {
    harvestMessage = "";
    tft.fillRect(0, 111, 240, 24, TFT_COLOR3);
    
    // Restore plant name for selected pot after clearing harvest message
    tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
    if(potCrop[selectedPot] == 0) 
      CenterText("", 111);
    else if(potCrop[selectedPot] == 1) 
      CenterText("SWEET FRUIT", 111);
    else if(potCrop[selectedPot] == 2) 
      CenterText("ENERGY DRINK", 111); 
    else if(potCrop[selectedPot] == 3) 
      CenterText("HEALTH POTION", 111);
  }
}

void ShowSeedSelection() {
  tft.fillScreen(TFT_COLOR4);
  drawingBatteryIcon(batteryAnim);
  
  // Title
  tft.fillRect(3, 3, 93, 27, TFT_COLOR1);
  tft.fillRect(0, 0, 93, 27, TFT_COLOR3);
  tft.setTextColor(TFT_COLOR1, TFT_COLOR3);
  tft.drawString("SEEDS", 3, 3);
  
  // Reset menu state
  menuItem = 0;
  maxItems = 3;
  
  // Draw seed options (following your item display pattern)
  pushScaled(24, 54, 16, 16, items[0], TFT_BLACK); // Sweet Fruit sprite
  pushScaled(96, 54, 16, 16, items[3], TFT_BLACK); // Energy Drink sprite
  pushScaled(165, 54, 16, 16, items[6], TFT_BLACK); // Health Potion sprite
  
  // Selection arrow
  tft.fillRect(18, 39, 189, 15, TFT_COLOR4);
  pushScaled(39 + (menuItem * 72), 33, 5, 5, smallIcons[13], TFT_BLACK);
  
  // Show initial name
  tft.setTextColor(TFT_COLOR1, TFT_COLOR4);
  tft.fillRect(0, 111, 240, 24, TFT_COLOR4);
  CenterText("SWEET FRUIT", 111);
  // Quantity of the selected seed (same style/spot as the inventory)
  tft.drawString("x" + String(EEPROM.read(143)), 204, 30);
}
