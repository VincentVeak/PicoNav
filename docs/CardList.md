---
layout: default
title: Card Reference Database
nav_order: 2
---

# Card Reference Database

Complete reference for all cards in PicoNav. Click column headers to sort, or use filters and search below.

<div class="stats-box">
  <strong>Total Available Cards:</strong> <span id="total-cards">50</span> | 
  <strong>Showing:</strong> <span id="showing-cards">50</span>
</div>

<div class="filter-controls">
  <input type="text" class="card-search" id="card-search" placeholder="🔍 Search cards by name, description, or tags...">
  
  <div class="filter-buttons">
    <button class="filter-btn active">All</button>
    <button class="filter-btn aspect-fire">Fire</button>
    <button class="filter-btn aspect-earth">Earth</button>
    <button class="filter-btn aspect-water">Water</button>
    <button class="filter-btn aspect-wind">Wind</button>
    <button class="filter-btn aspect-null">Neutral</button>
  </div>
</div>

---

<table class="card-table" id="card-table">
<thead>
  <tr>
    <th class="sortable">Icon <span class="sort-arrow">⇅</span></th>
    <th class="sortable">Name <span class="sort-arrow">⇅</span></th>
    <th class="sortable">Description <span class="sort-arrow">⇅</span></th>
    <th class="sortable">Aspect <span class="sort-arrow">⇅</span></th>
    <th class="sortable">Series <span class="sort-arrow">⇅</span></th>
    <th class="sortable">Damage <span class="sort-arrow">⇅</span></th>
    <th class="sortable">Tags <span class="sort-arrow">⇅</span></th>
  </tr>
</thead>
<tbody>
  <!-- FIRE CARDS -->
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_00.png" alt="Fireball"></td>
    <td><strong>Fireball</strong></td>
    <td>Adds a Fire counter that deals 12.5% damage per turn</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td data-value="70">70</td>
    <td>Fire, Burst Damage, DOT</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_08.png" alt="Flare"></td>
    <td><strong>Flare</strong></td>
    <td>Removes Fire counter on opponent for 3x damage</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td data-value="35">35</td>
    <td>Fire, Burst Damage</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_21.png" alt="Inferno"></td>
    <td><strong>Inferno</strong></td>
    <td>Adds a Fire Counter to both players and a Blaze counter to the arena. All players with Fire counters take 2x damage</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td data-value="50">50</td>
    <td>Fire</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_22.png" alt="Combustion"></td>
    <td><strong>Combustion</strong></td>
    <td>Does 4x damage if Blaze counter is active</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td data-value="25">25</td>
    <td>Fire</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_10.png" alt="Explosion"></td>
    <td><strong>Explosion</strong></td>
    <td>Removes Fire counter on self to deal 5x damage</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td data-value="30">30</td>
    <td>Fire</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_20.png" alt="Overheat"></td>
    <td><strong>Overheat</strong></td>
    <td>Lowers the user's defense stat by 2 stages</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td data-value="100">100</td>
    <td>Fire</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_19.png" alt="Incinerate"></td>
    <td><strong>Incinerate</strong></td>
    <td>Removes a counter on the opponent</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td data-value="55">55</td>
    <td>Fire</td>
  </tr>

  <!-- FIRE/LIGHTNING CARDS -->
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_11.png" alt="Lightning"></td>
    <td><strong>Lightning</strong></td>
    <td>Adds or increases Lightning counter. Damage is multiplied by number of counters</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td><span class="aspect-lightning">Lightning</span></td>
    <td data-value="40">40</td>
    <td>Fire, Lightning</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_16.png" alt="Discharge"></td>
    <td><strong>Discharge</strong></td>
    <td>Removes Lightning counters from opponent for 3x damage</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td><span class="aspect-lightning">Lightning</span></td>
    <td data-value="40">40</td>
    <td>Fire, Lightning</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_17.png" alt="Storm"></td>
    <td><strong>Storm</strong></td>
    <td>Adds a Storm counter to the arena that adds or increases Lightning on both players</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td><span class="aspect-lightning">Lightning</span></td>
    <td data-value="25">25</td>
    <td>Fire, Lightning</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_14.png" alt="Thunder"></td>
    <td><strong>Thunder</strong></td>
    <td>Does 4x damage if Storm counter is active</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td><span class="aspect-lightning">Lightning</span></td>
    <td data-value="25">25</td>
    <td>Fire, Lightning</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_15.png" alt="Recharge"></td>
    <td><strong>Recharge</strong></td>
    <td>Removes Lightning counters from opponent to deal 3x damage and heal damage dealt</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td><span class="aspect-lightning">Lightning</span></td>
    <td data-value="20">20</td>
    <td>Fire, Lightning, Heal</td>
  </tr>

  <!-- EARTH CARDS -->
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_01.png" alt="Stonewall"></td>
    <td><strong>Stonewall</strong></td>
    <td>Adds an Earth counter that reduces damage by 25%. Maximum of 3 stacks</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td>Earth</td>
    <td data-value="70">70</td>
    <td>Earth, Defense</td>
  </tr>
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_04.png" alt="Fortify"></td>
    <td><strong>Fortify</strong></td>
    <td><b>Priority move.</b> Upgrades Earth to Metal counter, which on removal grants an Earth counter. Instantly maxes out Earth counter stacks to 3</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td>Earth</td>
    <td data-value="15">15</td>
    <td>Earth, Defense, Priority</td>
  </tr>
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_26.png" alt="Aftershock"></td>
    <td><strong>Aftershock</strong></td>
    <td>Add a Slow counter to opponent if you have an Earth counter</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td>Earth</td>
    <td data-value="70">70</td>
    <td>Earth</td>
  </tr>
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_27.png" alt="Stone Rain"></td>
    <td><strong>Stone Rain</strong></td>
    <td>Add a Sandstorm counter to the arena. Does not damage Earth counter. Sandstorm deals 12.5% per turn</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td>Earth</td>
    <td data-value="40">40</td>
    <td>Earth, DOT</td>
  </tr>

  <!-- EARTH/NATURE CARDS -->
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_29.png" alt="Terraform"></td>
    <td><strong>Terraform</strong></td>
    <td><b>Priority move.</b>Upgrades Earth Counter to Nature Counter, which heals 12.5% each turn</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td><span class="aspect-nature">Nature</span></td>
    <td data-value="15">15</td>
    <td>Earth, Nature, Heal, Priority</td>
  </tr>
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_30.png" alt="Naturalize"></td>
    <td><strong>Naturalize</strong></td>
    <td>Remove opponent counter and heal 12.5%. Doubles damage if counter is removed</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td><span class="aspect-nature">Nature</span></td>
    <td data-value="50">50</td>
    <td>Earth, Nature, Heal</td>
  </tr>
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_31.png" alt="Life Leech"></td>
    <td><strong>Life Leech</strong></td>
    <td>Remove Nature Counter from opponent to deal 3x damage and heal the damage dealt</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td><span class="aspect-nature">Nature</span></td>
    <td data-value="20">20</td>
    <td>Earth, Nature, Heal</td>
  </tr>
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_34.png" alt="Synthesis"></td>
    <td><strong>Synthesis</strong></td>
    <td>Heals double healing if user has Nature counter</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td><span class="aspect-nature">Nature</span></td>
    <td data-value="0">0</td>
    <td>Earth, Nature, Heal</td>
  </tr>
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_32.png" alt="Overgrowth"></td>
    <td><strong>Overgrowth</strong></td>
    <td>Adds/Replaces Overgrowth Counter to arena. Grants Nature counter to both players at end of turn</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td><span class="aspect-nature">Nature</span></td>
    <td data-value="25">25</td>
    <td>Earth, Nature, Heal</td>
  </tr>
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_33.png" alt="Solar Beam"></td>
    <td><strong>Solar Beam</strong></td>
    <td>Does 4x damage if Overgrowth counter is active</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td><span class="aspect-nature">Nature</span></td>
    <td data-value="25">25</td>
    <td>Earth, Nature</td>
  </tr>

  <!-- WATER CARDS -->
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_02.png" alt="Bubble"></td>
    <td><strong>Bubble</strong></td>
    <td>Adds a Water counter that heals 12.5% per turn</td>
    <td><span class="aspect-water">Water</span></td>
    <td>Water</td>
    <td data-value="70">70</td>
    <td>Water, Heal</td>
  </tr>
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_06.png" alt="Current"></td>
    <td><strong>Current</strong></td>
    <td>Remove Water counter on opponent for 3x damage</td>
    <td><span class="aspect-water">Water</span></td>
    <td>Water</td>
    <td data-value="35">35</td>
    <td>Water</td>
  </tr>
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_37.png" alt="Whirlpool"></td>
    <td><strong>Whirlpool</strong></td>
    <td>Adds/Replaces Whirlpool Counter to Arena, which deals damage equal to the amount healed by Water counter</td>
    <td><span class="aspect-water">Water</span></td>
    <td>Water</td>
    <td data-value="40">40</td>
    <td>Water, DOT</td>
  </tr>
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_38.png" alt="Tidal Wave"></td>
    <td><strong>Tidal Wave</strong></td>
    <td>Does 4x damage if Whirlpool counter is active</td>
    <td><span class="aspect-water">Water</span></td>
    <td>Water</td>
    <td data-value="25">25</td>
    <td>Water</td>
  </tr>
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_35.png" alt="Aqua Shield"></td>
    <td><strong>Aqua Shield</strong></td>
    <td>Removes Water Counter to heal for 2x healing</td>
    <td><span class="aspect-water">Water</span></td>
    <td>Water</td>
    <td data-value="0">0</td>
    <td>Water, Heal</td>
  </tr>
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_39.png" alt="Tsunami"></td>
    <td><strong>Tsunami</strong></td>
    <td>Adds a Water counter to both players and a Whirlpool counter to arena. Both Players with Water Counters heal 2x healing</td>
    <td><span class="aspect-water">Water</span></td>
    <td>Water</td>
    <td data-value="50">50</td>
    <td>Water, Heal</td>
  </tr>

  <!-- WATER/STEAM CARDS -->
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_13.png" alt="Gust"></td>
    <td><strong>Gust</strong></td>
    <td>Adds or increases Steam counter, which decreases damage done by percentage of steam counter stacks</td>
    <td><span class="aspect-water">Water</span></td>
    <td><span class="aspect-steam">Steam</span></td>
    <td data-value="50">50</td>
    <td>Water, Steam, Defense</td>
  </tr>
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_40.png" alt="Haze"></td>
    <td><strong>Haze</strong></td>
    <td><b>Priority move.</b> Adds/Replaces Haze Counter to arena that decreases damage from opponent by percentage of steam counters on user</td>
    <td><span class="aspect-water">Water</span></td>
    <td><span class="aspect-steam">Steam</span></td>
    <td data-value="10">10</td>
    <td>Water, Steam, Defense</td>
  </tr>
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_25.png" alt="Ice Wall"></td>
    <td><strong>Ice Wall</strong></td>
    <td>Places a Wall counter on the Arena that nullifies any damage from any attack from any player once</td>
    <td><span class="aspect-water">Water</span></td>
    <td>Water</td>
    <td data-value="0">0</td>
    <td>Water, Defense</td>
  </tr>

  <!-- WIND CARDS -->
  <tr data-aspect="Wind">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_03.png" alt="Tornado"></td>
    <td><strong>Tornado</strong></td>
    <td>Shuffles your deck and grants a Wind counter that gives the next move priority</td>
    <td><span class="aspect-wind">Wind</span></td>
    <td>Wind</td>
    <td data-value="50">50</td>
    <td>Wind, Utility</td>
  </tr>
  <tr data-aspect="Wind">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_42.png" alt="Windstorm"></td>
    <td><strong>Windstorm</strong></td>
    <td>Adds/Replaces a Gust counter to the arena. Removes counters on players at the end of turn</td>
    <td><span class="aspect-wind">Wind</span></td>
    <td>Wind</td>
    <td data-value="30">30</td>
    <td>Wind</td>
  </tr>
  <tr data-aspect="Wind">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_05.png" alt="Air Slash"></td>
    <td><strong>Air Slash</strong></td>
    <td><b>Priority move.</b> Remove Wind counter on self for 3x damage</td>
    <td><span class="aspect-wind">Wind</span></td>
    <td>Wind</td>
    <td data-value="30">30</td>
    <td>Wind, Priority</td>
  </tr>
  <tr data-aspect="Wind">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_41.png" alt="Lucky Star"></td>
    <td><strong>Lucky Star</strong></td>
    <td>Remove Wind Counter for Luck Counter that increases crit chance by 25%</td>
    <td><span class="aspect-wind">Wind</span></td>
    <td>Wind</td>
    <td data-value="40">40</td>
    <td>Wind, Buff</td>
  </tr>
  <tr data-aspect="Wind">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_45.png" alt="Sonicboom"></td>
    <td><strong>Sonicboom</strong></td>
    <td>25% chance to crit</td>
    <td><span class="aspect-wind">Wind</span></td>
    <td>Wind</td>
    <td data-value="60">60</td>
    <td>Wind, Crit</td>
  </tr>
  <tr data-aspect="Wind">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_43.png" alt="Gale Force"></td>
    <td><strong>Gale Force</strong></td>
    <td>Removes counters from each player and arena. Multiplies damage per counter removed</td>
    <td><span class="aspect-wind">Wind</span></td>
    <td>Wind</td>
    <td data-value="35">35</td>
    <td>Wind</td>
  </tr>
  <tr data-aspect="Wind">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_44.png" alt="Force Rush"></td>
    <td><strong>Force Rush</strong></td>
    <td>Damage is divided by number of turns that passed and grants a Wind counter</td>
    <td><span class="aspect-wind">Wind</span></td>
    <td>Wind</td>
    <td data-value="90">90</td>
    <td>Wind</td>
  </tr>
  <tr data-aspect="Wind">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_46.png" alt="Pressure"></td>
    <td><strong>Pressure</strong></td>
    <td>Damage is multiplied by the number of times this ability was used this game</td>
    <td><span class="aspect-wind">Wind</span></td>
    <td>Wind</td>
    <td data-value="35">35</td>
    <td>Wind</td>
  </tr>

  <!-- NEUTRAL/NULL CARDS -->
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_07.png" alt="Pierce"></td>
    <td><strong>Pierce</strong></td>
    <td>Ignore the effects of counters from both players</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td data-value="30">30</td>
    <td>Neutral, Ignore Counter</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_09.png" alt="Lunge"></td>
    <td><strong>Lunge</strong></td>
    <td><b>Priority move.</b></td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td data-value="40">40</td>
    <td>Neutral, Priority</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_12.png" alt="Block"></td>
    <td><strong>Block</strong></td>
    <td><b>Priority move.</b> Prevent all damage from opponent's move if Block was not used last turn</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td data-value="0">0</td>
    <td>Neutral, Defense, Priority</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_36.png" alt="Payout"></td>
    <td><strong>Payout</strong></td>
    <td>Grants extra money if this is the killing move</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td data-value="15">15</td>
    <td>Neutral, Reward</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_48.png" alt="Juxtapose"></td>
    <td><strong>Juxtapose</strong></td>
    <td>Switch counters with opponent</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td data-value="0">0</td>
    <td>Neutral, Swap</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_28.png" alt="Time Flip"></td>
    <td><strong>Time Flip</strong></td>
    <td>Place a Reversal arena counter that allows the slower Pico to attack first</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td data-value="0">0</td>
    <td>Neutral, Utility</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_47.png" alt="Insult"></td>
    <td><strong>Insult</strong></td>
    <td>Lower the opponent's defense stage by 1</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td data-value="0">0</td>
    <td>Neutral, Debuff</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_24.png" alt="Curse"></td>
    <td><strong>Curse</strong></td>
    <td>Places a curse counter on opponent that deals 30% of their moves back to them</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td data-value="0">0</td>
    <td>Neutral, Reflect</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_23.png" alt="Aegis"></td>
    <td><strong>Aegis</strong></td>
    <td>Place an aegis counter that prevents all damage under 40 base damage</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td data-value="0">0</td>
    <td>Neutral, Defense</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_49.png" alt="Blood Moon"></td>
    <td><strong>Blood Moon</strong></td>
    <td>Place a Blood Moon arena counter that turns healing into damage</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td data-value="0">0</td>
    <td>Neutral, Arena</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_18.png" alt="Recover"></td>
    <td><strong>Recover</strong></td>
    <td>Heal 35% of max HP</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td data-value="0">0</td>
    <td>Neutral, Heal</td>
  </tr>
</tbody>
</table>

<style>
/* Filter Controls */
.filter-controls {
  margin: 2rem 0;
}

.card-search {
  width: 100%;
  padding: 0.75rem 1rem;
  font-size: 1rem;
  border: 2px solid #FF7573;
  border-radius: 25px;
  background-color: rgba(0, 31, 37, 0.5);
  color: ##1f2a2b;
  margin-bottom: 1rem;
  font-family: 'Comfortaa', sans-serif;
}

.card-search::placeholder {
  color: rgba(230, 235, 197, 0.5);
}

.filter-buttons {
  display: flex;
  gap: 0.5rem;
  flex-wrap: wrap;
}

.filter-btn {
  padding: 0.5rem 1rem;
  border: 2px solid #FF7573;
  background-color: rgba(0, 31, 37, 0.5);
  color: #E6EBC5;
  border-radius: 25px;
  cursor: pointer;
  font-family: 'Comfortaa', sans-serif;
  font-weight: 500;
  transition: all 0.3s ease;
}

.filter-btn:hover {
  background-color: #FF7573;
  color: #00313A;
  transform: translateY(-2px);
}

.filter-btn.active {
  background-color: #FF7573;
  color: #00313A;
}

/* Stats Box */
.stats-box {
  background: linear-gradient(135deg, rgba(0, 31, 37, 0.8) 0%, rgba(0, 49, 58, 0.8) 100%);
  border: 2px solid #FF7573;
  border-radius: 25px;
  padding: 1rem;
  margin: 1rem 0;
  text-align: center;
  color: ##1f2a2b;
}

/* Table Styles */
.card-table {
  width: 100%;
  max-width: 100%;
  border-collapse: collapse;
  margin: 2rem 0;
  font-size: 0.75rem;
  table-layout: fixed;
}

.card-table thead th {
  background-color: #FF7573;
  color: #00313A;
  padding: 0.5rem 0.35rem;
  text-align: left;
  font-weight: 600;
  position: sticky;
  top: 0;
  z-index: 10;
  font-size: 0.75rem;
  line-height: 1.2;
}

/* Column widths - optimized to fit everything */
.card-table thead th:nth-child(1) { width: 45px; }   /* Icon - smaller */
.card-table thead th:nth-child(2) { width: 10%; }    /* Name */
.card-table thead th:nth-child(3) { width: 38%; }    /* Description - largest */
.card-table thead th:nth-child(4) { width: 10%; }     /* Aspect */
.card-table thead th:nth-child(5) { width: 12%; }    /* Series */
.card-table thead th:nth-child(6) { width: 6%; }     /* Damage - very small */
.card-table thead th:nth-child(7) { width: 17%; }    /* Tags */

.card-table thead th.sortable {
  cursor: pointer;
  user-select: none;
}

.card-table thead th.sortable:hover {
  background-color: #E6EBC5;
}

.sort-arrow {
  float: right;
  opacity: 0.5;
  font-size: 0.7rem;
}

.card-table thead th.sorted .sort-arrow {
  opacity: 1;
}

.card-table tbody tr {
  border-bottom: 1px solid rgba(255, 117, 115, 0.2);
  transition: all 0.2s ease;
}

.card-table tbody tr:hover {
  background-color: rgba(255, 117, 115, 0.1);
}

.card-table tbody td {
  padding: 0.5rem 0.35rem;
  color: #E6EBC5;
  vertical-align: middle;
  font-size: 0.75rem;
  line-height: 1.3;
  word-wrap: break-word;
}

.card-icon {
  width: 50px;
  text-align: center;
  padding: 0.3rem !important;
}

.card-icon img {
  width: 35px;
  height: 35px;
  object-fit: contain;
  display: block;
  margin: 0 auto;
}

/* Make aspect badges smaller */
.aspect-fire {
  background-color: #ff4444;
  color: white;
  padding: 0.2rem 0.5rem;
  border-radius: 12px;
  font-size: 0.7rem;
  font-weight: 600;
  display: inline-block;
  white-space: nowrap;
}

.aspect-earth {
  background-color: #8b4513;
  color: white;
  padding: 0.2rem 0.5rem;
  border-radius: 12px;
  font-size: 0.7rem;
  font-weight: 600;
  display: inline-block;
  white-space: nowrap;
}

.aspect-water {
  background-color: #4169e1;
  color: white;
  padding: 0.2rem 0.5rem;
  border-radius: 12px;
  font-size: 0.7rem;
  font-weight: 600;
  display: inline-block;
  white-space: nowrap;
}

.aspect-wind {
  background-color: #87ceeb;
  color: #00313A;
  padding: 0.2rem 0.5rem;
  border-radius: 12px;
  font-size: 0.7rem;
  font-weight: 600;
  display: inline-block;
  white-space: nowrap;
}

.aspect-null {
  background-color: #808080;
  color: white;
  padding: 0.2rem 0.5rem;
  border-radius: 12px;
  font-size: 0.7rem;
  font-weight: 600;
  display: inline-block;
  white-space: nowrap;
}

.aspect-lightning {
  background-color: #ffd700;
  color: #00313A;
  padding: 0.2rem 0.5rem;
  border-radius: 12px;
  font-size: 0.7rem;
  font-weight: 600;
  display: inline-block;
  white-space: nowrap;
}

.aspect-nature {
  background-color: #228b22;
  color: white;
  padding: 0.2rem 0.5rem;
  border-radius: 12px;
  font-size: 0.7rem;
  font-weight: 600;
  display: inline-block;
  white-space: nowrap;
}

.aspect-steam {
  background-color: #e0e0e0;
  color: #00313A;
  padding: 0.2rem 0.5rem;
  border-radius: 12px;
  font-size: 0.7rem;
  font-weight: 600;
  display: inline-block;
  white-space: nowrap;
}

/* Hidden row class */
.hidden-row {
  display: none !important;
}
</style>

<script src="/assets/js/cardlist.js"></script>

## Want to Contribute?

Found an error or have balance suggestions? 

- [Edit this page on GitHub](https://github.com/VincentVeak/PicoNav/edit/main/docs/CardList.md)
- [Open an issue](https://github.com/VincentVeak/PicoNav/issues)
- [Discuss in community](https://github.com/VincentVeak/PicoNav/discussions)

---

**Note:** Card stats and mechanics are subject to change with balance updates. Check the [Updates page](/docs/updates) for the latest patch notes.
