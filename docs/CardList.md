# Card Reference Database

Complete reference for all cards in PicoNav. Click column headers to sort, or use filters and search below.


<div class="stats-box">
  <strong>Total Available Cards:</strong> <span id="total-cards">50</span> | 
  <strong>Future Planned Cards:</strong> <span id="showing-cards">33</span>
</div>

<input type="text" class="card-search" id="card-search" placeholder="Search cards by name, description, or tags..." onkeyup="searchCards()">

---

<table class="card-table" id="card-table">
<thead>
  <tr>
    <th onclick="sortTable(0)">Icon</th>
    <th onclick="sortTable(1)">Name</th>
    <th onclick="sortTable(2)">Description</th>
    <th onclick="sortTable(3)">Aspect</th>
    <th onclick="sortTable(4)">Series</th>
    <th onclick="sortTable(5)">Damage</th>
    <th onclick="sortTable(6)">Tags</th>
  </tr>
</thead>
<tbody>
  <!-- FIRE CARDS -->
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_00.png"></td>
    <td><strong>Fireball</strong></td>
    <td>Adds a Fire counter that deals 12.5% damage per turn</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td>70</td>
    <td>Fire, Burst Damage, DOT</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_08.png"></td>
    <td><strong>Flare</strong></td>
    <td>Removes Fire counter on opponent for 3x damage</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td>35</td>
    <td>Fire, Burst Damage</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_21.png"></td>
    <td><strong>Inferno</strong></td>
    <td>Adds a Fire Counter to both players and a Blaze counter to the arena. All players with Fire counters take 2x damage</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td>50</td>
    <td>Fire</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_22.png"></td>
    <td><strong>Combustion</strong></td>
    <td>Does 4x damage if Blaze counter is active</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td>25</td>
    <td>Fire</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_10.png"></td>
    <td><strong>Explosion</strong></td>
    <td>Removes Fire counter on self to deal 5x damage</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td>30</td>
    <td>Fire</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_20.png"></td>
    <td><strong>Overheat</strong></td>
    <td>Lowers the user's defense stat by 2 stages</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td>100</td>
    <td>Fire</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_19.png"></td>
    <td><strong>Incinerate</strong></td>
    <td>Removes a counter on the opponent</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td>55</td>
    <td>Fire</td>
  </tr>

  <!-- FIRE/LIGHTNING CARDS -->
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_11.png"></td>
    <td><strong>Lightning</strong></td>
    <td>Adds or increases Lightning counter. Damage is multiplied by number of counters</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td><span class="aspect-lightning">Lightning</span></td>
    <td>40</td>
    <td>Fire, Lightning</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_16.png"></td>
    <td><strong>Discharge</strong></td>
    <td>Removes Lightning counters from opponent for 3x damage</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td><span class="aspect-lightning">Lightning</span></td>
    <td>40</td>
    <td>Fire, Lightning</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_17.png"></td>
    <td><strong>Storm</strong></td>
    <td>Adds a Storm counter to the arena that adds or increases Lightning on both players</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td><span class="aspect-lightning">Lightning</span></td>
    <td>25</td>
    <td>Fire, Lightning</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_14.png"></td>
    <td><strong>Thunder</strong></td>
    <td>Does 4x damage if Storm counter is active</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td><span class="aspect-lightning">Lightning</span></td>
    <td>25</td>
    <td>Fire, Lightning</td>
  </tr>
  <tr data-aspect="Fire">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_15.png"></td>
    <td><strong>Recharge</strong></td>
    <td>Removes Lightning counters from opponent to deal 3x damage and heal damage dealt</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td><span class="aspect-lightning">Lightning</span></td>
    <td>20</td>
    <td>Fire, Lightning, Heal</td>
  </tr>

  <!-- EARTH CARDS -->
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_01.png"></td>
    <td><strong>Stonewall</strong></td>
    <td>Adds an Earth counter that reduces damage by 25%. Maximum of 3 stacks</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td>Earth</td>
    <td>70</td>
    <td>Earth, Defense</td>
  </tr>
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_04.png"></td>
    <td><strong>Fortify</strong></td>
    <td><b>Priority move.</b> Upgrades Earth to Metal counter, which on removal grants an Earth counter. Instantly maxes out Earth counter stacks to 3</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td>Earth</td>
    <td>15</td>
    <td>Earth, Defense, Priority</td>
  </tr>
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_26.png"></td>
    <td><strong>Aftershock</strong></td>
    <td>Add a Slow counter to opponent if you have an Earth counter</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td>Earth</td>
    <td>70</td>
    <td>Earth</td>
  </tr>
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_27.png"></td>
    <td><strong>Stone Rain</strong></td>
    <td>Add a Sandstorm counter to the arena. Does not damage Earth counter. Sandstorm deals 12.5% per turn</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td>Earth</td>
    <td>40</td>
    <td>Earth, DOT</td>
  </tr>

  <!-- EARTH/NATURE CARDS -->
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_29.png"></td>
    <td><strong>Terraform</td>
    <td><b>Priority move.</b>Upgrades Earth Counter to Nature Counter, which heals 12.5% each turn</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td><span class="aspect-nature">Nature</span></td>
    <td>15</td>
    <td>Earth, Nature, Heal, Priority</td>
  </tr>
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_30.png"></td>
    <td><strong>Naturalize</strong></td>
    <td>Remove opponent counter and heal 12.5%. Doubles damage if counter is removed</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td><span class="aspect-nature">Nature</span></td>
    <td>50</td>
    <td>Earth, Nature, Heal</td>
  </tr>
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_31.png"></td>
    <td><strong>Life Leech</strong></td>
    <td>Deals damage and heals 50% of damage dealt. Deals 2x damage if user has Nature counter</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td><span class="aspect-nature">Nature</span></td>
    <td>45</td>
    <td>Earth, Nature, Heal</td>
  </tr>
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_32.png"></td>
    <td><strong>Erosion</strong></td>
    <td>Damage is multiplied by the number of turns that have passed if Earth counter is spent</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td>Earth</td>
    <td>15</td>
    <td>Earth</td>
  </tr>
  <tr data-aspect="Earth">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_13.png"></td>
    <td><strong>Earthquake</strong></td>
    <td>Deals damage, dealing extra damage while consuming earth counters</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td>Earth</td>
    <td>25 (50,75,100)</td>
    <td>Earth</td>
  </tr>

  <!-- WATER CARDS -->
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_02.png"></td>
    <td><strong>Tidal Wave</strong></td>
    <td>Adds a Water counter to the opponent that reduces speed by 25% and removes the next counter placed on opponent</td>
    <td><span class="aspect-water">Water</span></td>
    <td>Water</td>
    <td>40</td>
    <td>Water</td>
  </tr>
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_35.png"></td>
    <td><strong>Freeze</strong></td>
    <td>Turns Water counter into Ice counter, that prevents opponent's next move</td>
    <td><span class="aspect-water">Water</span></td>
    <td>Water</td>
    <td>40</td>
    <td>Water, Ice</td>
  </tr>
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_06.png"></td>
    <td><strong>Torrent</strong></td>
    <td>Places a Water counter on both players and Rain on arena. No other counters outside of Water and Ice can be added while raining</td>
    <td><span class="aspect-water">Water</span></td>
    <td>Water</td>
    <td>25</td>
    <td>Water</td>
  </tr>
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_33.png"></td>
    <td><strong>Hydration</strong></td>
    <td>Places a Water counter on self and heals 12.5% Max HP</td>
    <td><span class="aspect-water">Water</span></td>
    <td>Water</td>
    <td>20</td>
    <td>Water, Heal</td>
  </tr>
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_34.png"></td>
    <td><strong>Icicle</strong></td>
    <td>Removes Water counter on self to deal 2x damage</td>
    <td><span class="aspect-water">Water</span></td>
    <td>Water</td>
    <td>50</td>
    <td>Water</td>
  </tr>
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_37.png"></td>
    <td><strong>Undertow</strong></td>
    <td>Replace opponent counter to Water counter if Water card is at the bottom of the deck</td>
    <td><span class="aspect-water">Water</span></td>
    <td>Water</td>
    <td>30</td>
    <td>Water</td>
  </tr>
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_38.png"></td>
    <td><strong>Life Steal</strong></td>
    <td>Replace Water counter with a life steal counter that heals the amount of damage dealt while active</td>
    <td><span class="aspect-water">Water</span></td>
    <td><span class="aspect-drain">Drain</span></td>
    <td>40</td>
    <td>Water, Drain, Heal</td>
  </tr>
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_39.png"></td>
    <td><strong>Boil</strong></td>
    <td>Replace all Water counters with Fire counters on both players</td>
    <td><span class="aspect-water">Water</span></td>
    <td><span class="aspect-steam">Steam</span></td>
    <td>60</td>
    <td>Water, Steam, Fire</td>
  </tr>
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_40.png"></td>
    <td><strong>Mistwalk</strong></td>
    <td>Convert a Water or Fire counter into a Mist counter, which nullifies the next hit done to the user</td>
    <td><span class="aspect-water">Water</span></td>
    <td><span class="aspect-steam">Steam</span></td>
    <td>10</td>
    <td>Water, Steam, Defense</td>
  </tr>
  <tr data-aspect="Water">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_25.png"></td>
    <td><strong>Ice Wall</strong></td>
    <td>Places a Wall counter on the Arena that nullifies any damage from any attack from any player once</td>
    <td><span class="aspect-water">Water</span></td>
    <td>Water</td>
    <td>0</td>
    <td>Water, Defense</td>
  </tr>

  <!-- WIND CARDS -->
  <tr data-aspect="Wind">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_03.png"></td>
    <td><strong>Tornado</strong></td>
    <td>Shuffles your deck and grants a Wind counter that gives the next move priority</td>
    <td><span class="aspect-wind">Wind</span></td>
    <td>Wind</td>
    <td>50</td>
    <td>Wind, Utility</td>
  </tr>
  <tr data-aspect="Wind">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_42.png"></td>
    <td><strong>Windstorm</strong></td>
    <td>Adds/Replaces a Gust counter to the arena. Removes counters on players at the end of turn</td>
    <td><span class="aspect-wind">Wind</span></td>
    <td>Wind</td>
    <td>30</td>
    <td>Wind</td>
  </tr>
  <tr data-aspect="Wind">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_05.png"></td>
    <td><strong>Air Slash</td>
    <td><b>Priority move.</b> Remove Wind counter on self for 3x damage</td>
    <td><span class="aspect-wind">Wind</span></td>
    <td>Wind</td>
    <td>30</td>
    <td>Wind, Priority</td>
  </tr>
  <tr data-aspect="Wind">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_41.png"></td>
    <td><strong>Lucky Star</strong></td>
    <td>Remove Wind Counter for Luck Counter that increases crit chance by 25%</td>
    <td><span class="aspect-wind">Wind</span></td>
    <td>Wind</td>
    <td>40</td>
    <td>Wind, Buff</td>
  </tr>
  <tr data-aspect="Wind">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_45.png"></td>
    <td><strong>Sonicboom</strong></td>
    <td>25% chance to crit</td>
    <td><span class="aspect-wind">Wind</span></td>
    <td>Wind</td>
    <td>60</td>
    <td>Wind, Crit</td>
  </tr>
  <tr data-aspect="Wind">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_43.png"></td>
    <td><strong>Gale Force</strong></td>
    <td>Removes counters from each player and arena. Multiplies damage per counter removed</td>
    <td><span class="aspect-wind">Wind</span></td>
    <td>Wind</td>
    <td>35</td>
    <td>Wind</td>
  </tr>
  <tr data-aspect="Wind">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_44.png"></td>
    <td><strong>Force Rush</strong></td>
    <td>Damage is divided by number of turns that passed and grants a Wind counter</td>
    <td><span class="aspect-wind">Wind</span></td>
    <td>Wind</td>
    <td>90</td>
    <td>Wind</td>
  </tr>
  <tr data-aspect="Wind">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_46.png"></td>
    <td><strong>Pressure</strong></td>
    <td>Damage is multiplied by the number of times this ability was used this game</td>
    <td><span class="aspect-wind">Wind</span></td>
    <td>Wind</td>
    <td>35</td>
    <td>Wind</td>
  </tr>

  <!-- NEUTRAL/NULL CARDS -->
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_07.png"></td>
    <td><strong>Pierce</strong></td>
    <td>Ignore the effects of counters from both players</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td>30</td>
    <td>Neutral, Ignore Counter</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_09.png"></td>
    <td><strong>Lunge</td>
    <td><b>Priority move.</b></td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td>40</td>
    <td>Neutral, Priority</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_12.png"></td>
    <td><strong>Block</td>
    <td><b>Priority move.</b> Prevent all damage from opponent's move if Block was not used last turn</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td>0</td>
    <td>Neutral, Defense, Priority</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_35.png"></td>
    <td><strong>Payout</strong></td>
    <td>Grants extra money if this is the killing move</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td>15</td>
    <td>Neutral, Reward</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_48.png"></td>
    <td><strong>Juxtapose</strong></td>
    <td>Switch counters with opponent</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td>0</td>
    <td>Neutral, Swap</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_28.png"></td>
    <td><strong>Time Flip</strong></td>
    <td>Place a Reversal arena counter that allows the slower Pico to attack first</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td>0</td>
    <td>Neutral, Utility</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_47.png"></td>
    <td><strong></strong></td>
    <td>Lower the opponent's defense stage by 1</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td>0</td>
    <td>Neutral, Debuff</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_24.png"></td>
    <td><strong>Curse</strong></td>
    <td>Places a curse counter on opponent that deals 30% of their moves back to them</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td>0</td>
    <td>Neutral, Reflect</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_23.png"></td>
    <td><strong>Aegis</strong></td>
    <td>Place an aegis counter that prevents all damage under 40 base damage</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td>0</td>
    <td>Neutral, Defense</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_49.png"></td>
    <td><strong>Blood Moon</strong></td>
    <td>Place a Blood Moon arena counter that turns healing into damage</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td>0</td>
    <td>Neutral, Arena</td>
  </tr>
  <tr data-aspect="NULL">
    <td class="card-icon"><img src="/assets/images/cards/Card Icons_All_18.png"></td>
    <td><strong>Recover</strong></td>
    <td>Heal 35% of max HP</td>
    <td><span class="aspect-null">Neutral</span></td>
    <td>Neutral</td>
    <td>0</td>
    <td>Neutral, Heal</td>
  </tr>
</tbody>
</table>

---

## Card Mechanics Guide

### Counter System

**Status Counters** persist on players or arena and modify gameplay:

**Offensive Counters:**
- **Fire Counter** - 12.5% DOT per turn
- **Lightning Counter** - Amplifies lightning damage by stack count
- **Curse Counter** - Reflects 30% damage back
- **Blaze Counter** (Arena) - Doubles fire damage for all players
- **Storm Counter** (Arena) - Adds lightning to both players each turn

**Defensive Counters:**
- **Earth Counter** - Reduces damage by 25% (max 3 stacks)
- **Nature Counter** - Heals 12.5% per turn
- **Mist Counter** - Nullifies next hit
- **Invincible Counter** - Blocks any damage instance
- **Aegis Counter** - Prevents damage under 40

**Utility Counters:**
- **Wind Counter** - Grants priority to next move
- **Water/Ice Counter** - Reduces speed by 25%, Ice prevents next move
- **Slow Counter** - Reduces speed
- **Vision Counter** - Doubles next ability damage

### Priority System

Cards with <span class="priority-badge">PRIORITY</span> resolve before normal cards, allowing for:
- Pre-emptive strikes (Lunge, Air Slash)
- Defensive setup (Fortify, Block)
- Counter manipulation (Terraform)

### Combo Mechanics

**Alpha → Beta → Gamma:** 
Build up through the sequence for massive 999 damage finisher

**Trap Cards:**
Set hidden counters that activate when opponent performs specific actions

### Arena Counters

Arena-wide effects that impact both players:
- **Rain** (Torrent) - Restricts to Water/Ice counters only
- **Blood Moon** - Converts healing to damage
- **Reversal** (Time Flip) - Slower Pico attacks first
- **Gust** (Windstorm) - Removes all counters end of turn

---

## Deck Building Strategies

### Elemental Archetypes

**🔥 Fire/Lightning Aggro**
- Focus: High burst damage and DOT
- Key Cards: Fireball, Lightning, Discharge, Thunder
- Strategy: Stack counters then cash out for massive damage

**🪨 Earth/Nature Control**
- Focus: Defense and sustain
- Key Cards: Stonewall, Terraform, Life Leech, Naturalize
- Strategy: Outlast opponent while chipping away with healing

**💧 Water/Ice Tempo**
- Focus: Speed manipulation and disruption
- Key Cards: Freeze, Torrent, Tidal Wave, Ice Wall
- Strategy: Control opponent's actions and counter timing

**💨 Wind Combo**
- Focus: Priority manipulation and burst
- Key Cards: Tornado, Air Slash, Lucky Star, Gale Force
- Strategy: Control turn order and build critical strikes

**⚪ Neutral Tools**
- Focus: Adaptability and countering
- Key Cards: Pierce, Block, Mimicry, Juxtapose
- Strategy: Respond to opponent's strategy with answers

### Advanced Tips

1. **Include counter removal** - Incinerate, Naturalize, Gale Force
2. **Have a win condition** - Gamma Slash, Detonation, Berserk
3. **Consider trap cards** - Punish predictable opponents
4. **Protect key cards** - Use Stalwart to lock in important counters

---

## Synergy Examples

**Fire/Lightning Burst:**
1. Lightning (build stacks)
2. Lightning (build more stacks)
3. Discharge (3x damage per stack removed)

**Alpha Strike Combo:**
1. Alpha Slash
2. Beta Slash  
3. Gamma Slash (999 damage!)

**Defensive Stall:**
1. Fortify (max Earth counters)
2. Terraform (convert to Nature for healing)
3. Life Leech (damage + 50% heal, 2x with Nature)

**Arena Control:**
1. Torrent (Rain arena - only Water/Ice allowed)
2. Freeze (Ice prevents next move)
3. Icicle (remove Water for 2x damage)

---

## Want to Contribute?

Found an error or have balance suggestions? 

- [Edit this page on GitHub](https://github.com/VincentVeak/PicoNav/edit/main/docs/cards.md)
- [Open an issue](https://github.com/VincentVeak/PicoNav/issues)
- [Discuss in community](https://github.com/VincentVeak/PicoNav/discussions)

---

**Note:** Card stats and mechanics are subject to change with balance updates. Check the [Updates page](/docs/updates) for the latest patch notes.
