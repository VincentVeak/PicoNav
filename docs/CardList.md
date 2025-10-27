---
layout: default
title: Card Reference
nav_order: 8
description: "Complete card database for PicoNav card game"
permalink: /docs/cards
---

<style>
/* Card reference table styling */
.card-table {
  width: 100%;
  border-collapse: collapse;
  margin: 20px 0;
  font-size: 14px;
}

.card-table th {
  background-color: #2a2a2a;
  color: #E6EBC5;
  padding: 12px;
  text-align: left;
  font-weight: bold;
  border-bottom: 2px solid #00313A;
  cursor: pointer;
  user-select: none;
}

.card-table th:hover {
  background-color: #333;
}

.card-table th::after {
  content: ' ⇅';
  opacity: 0.5;
  font-size: 12px;
}

.card-table td {
  padding: 10px 12px;
  border-bottom: 1px solid #333;
}

.card-table tr:hover {
  background-color: #252525;
}

/* Aspect type badges */
.aspect-fire { color: #ff6b35; font-weight: bold; }
.aspect-earth { color: #8b4513; font-weight: bold; }
.aspect-water { color: #4a9eff; font-weight: bold; }
.aspect-lightning { color: #ffeb3b; font-weight: bold; }
.aspect-nature { color: #4caf50; font-weight: bold; }

/* Search box */
.card-search {
  width: 100%;
  max-width: 400px;
  padding: 10px;
  margin: 20px 0;
  border: 2px solid #00313A;
  border-radius: 5px;
  font-size: 16px;
  background-color: #1e1e1e;
  color: #E6EBC5;
}

/* Filter buttons */
.filter-buttons {
  margin: 20px 0;
  display: flex;
  gap: 10px;
  flex-wrap: wrap;
}

.filter-btn {
  padding: 8px 16px;
  border: 2px solid #00313A;
  background-color: #2a2a2a;
  color: #E6EBC5;
  border-radius: 5px;
  cursor: pointer;
  font-weight: bold;
}

.filter-btn:hover {
  background-color: #00313A;
}

.filter-btn.active {
  background-color: #00313A;
  border-color: #E6EBC5;
}

.stats-box {
  background-color: #2a2a2a;
  padding: 15px;
  border-radius: 5px;
  margin: 20px 0;
}
</style>

# Card Reference Database

Complete reference for all cards in PicoNav. Click column headers to sort, or use filters and search below.

<div class="stats-box">
  <strong>Total Cards:</strong> <span id="total-cards">32</span> | 
  <strong>Showing:</strong> <span id="showing-cards">32</span>
</div>

## Filters

<div class="filter-buttons">
  <button class="filter-btn active" onclick="filterByAspect('all')">All Cards</button>
  <button class="filter-btn" onclick="filterByAspect('Fire')">🔥 Fire</button>
  <button class="filter-btn" onclick="filterByAspect('Earth')">🪨 Earth</button>
  <button class="filter-btn" onclick="filterByAspect('Water')">💧 Water</button>
  <button class="filter-btn" onclick="filterByAspect('Lightning')">⚡ Lightning</button>
  <button class="filter-btn" onclick="filterByAspect('Nature')">🌿 Nature</button>
</div>

<input type="text" class="card-search" id="card-search" placeholder="Search cards by name, description, or tags..." onkeyup="searchCards()">

---

<table class="card-table" id="card-table">
<thead>
  <tr>
    <th onclick="sortTable(0)">Name</th>
    <th onclick="sortTable(1)">Description</th>
    <th onclick="sortTable(2)">Aspect</th>
    <th onclick="sortTable(3)">Series</th>
    <th onclick="sortTable(4)">Damage</th>
    <th onclick="sortTable(5)">Tags</th>
  </tr>
</thead>
<tbody>
  <!-- Fire Cards -->
  <tr data-aspect="Fire">
    <td><strong>Fireball</strong></td>
    <td>Adds a Fire counter that deals 12.5% damage per turn</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td>70</td>
    <td>Fire, Burst Damage, DOT</td>
  </tr>
  <tr data-aspect="Fire">
    <td><strong>Flare</strong></td>
    <td>Removes Fire counter on opponent for 3x damage</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td>35</td>
    <td>Fire, Burst Damage</td>
  </tr>
  <tr data-aspect="Fire">
    <td><strong>Intensify</strong></td>
    <td>Triggers ALL Fire counter counters an additional time</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td>30</td>
    <td>Fire</td>
  </tr>
  <tr data-aspect="Fire">
    <td><strong>Inferno</strong></td>
    <td>Adds a Fire Counter to both players and a Blaze counter to the arena. All players with Fire counters take 2x damage</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td>50</td>
    <td>Fire</td>
  </tr>
  <tr data-aspect="Fire">
    <td><strong>Combustion</strong></td>
    <td>Does 4x damage if Blaze counter is active</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td>25</td>
    <td>Fire</td>
  </tr>
  <tr data-aspect="Fire">
    <td><strong>Explosion</strong></td>
    <td>Removes Fire counter on self to deal 5x damage</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td>30</td>
    <td>Fire</td>
  </tr>
  <tr data-aspect="Fire">
    <td><strong>Overheat</strong></td>
    <td>Replace user's counter with a DEF DOWN counter</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td>100</td>
    <td>Fire</td>
  </tr>
  <tr data-aspect="Fire">
    <td><strong>Incinerate</strong></td>
    <td>Removes a counter on the opponent</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td>Fire</td>
    <td>55</td>
    <td>Fire</td>
  </tr>

  <!-- Fire/Lightning Cards -->
  <tr data-aspect="Fire">
    <td><strong>Plasma</strong></td>
    <td>Swaps Fire and Lightning counters on opponent</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td><span class="aspect-lightning">Lightning</span></td>
    <td>45</td>
    <td>Fire, Lightning</td>
  </tr>
  <tr data-aspect="Fire">
    <td><strong>Lightning</strong></td>
    <td>Adds or increases Lightning counter. Damage is multiplied by number of counters</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td><span class="aspect-lightning">Lightning</span></td>
    <td>30</td>
    <td>Fire, Lightning</td>
  </tr>
  <tr data-aspect="Fire">
    <td><strong>Discharge</strong></td>
    <td>Removes Lightning counters from opponent for 3x damage</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td><span class="aspect-lightning">Lightning</span></td>
    <td>40</td>
    <td>Fire, Lightning</td>
  </tr>
  <tr data-aspect="Fire">
    <td><strong>Storm</strong></td>
    <td>Adds a Storm counter to the arena that Adds or increases Lightning on both players</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td><span class="aspect-lightning">Lightning</span></td>
    <td>25</td>
    <td>Fire, Lightning</td>
  </tr>
  <tr data-aspect="Fire">
    <td><strong>Thunder</strong></td>
    <td>Does 4x damage if Storm counter is active</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td><span class="aspect-lightning">Lightning</span></td>
    <td>25</td>
    <td>Fire, Lightning</td>
  </tr>
  <tr data-aspect="Fire">
    <td><strong>Recharge</strong></td>
    <td>Removes Lightning counters from opponent to deal 3x damage and heal damage dealt</td>
    <td><span class="aspect-fire">Fire</span></td>
    <td><span class="aspect-lightning">Lightning</span></td>
    <td>20</td>
    <td>Fire, Lightning, Heal</td>
  </tr>

  <!-- Earth Cards -->
  <tr data-aspect="Earth">
    <td><strong>Stonewall</strong></td>
    <td>Adds an Earth counter that reduces damage by 25%. Maximum of 3 stacks</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td>Earth</td>
    <td>70</td>
    <td>Earth, Defense</td>
  </tr>
  <tr data-aspect="Earth">
    <td><strong>Fortify</strong></td>
    <td>Instantly maxes out Earth counter stacks to 3</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td>Earth</td>
    <td>15 Priority +1</td>
    <td>Earth, Defense</td>
  </tr>
  <tr data-aspect="Earth">
    <td><strong>Aftershock</strong></td>
    <td>Add a Slow counter to opponent if you have an Earth counter</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td>Earth</td>
    <td>70</td>
    <td>Earth</td>
  </tr>
  <tr data-aspect="Earth">
    <td><strong>Stone Rain</strong></td>
    <td>Add a Sandstorm counter to the arena. Does not damage Earth counter. Sandstorm deals 12.5% per turn</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td>Earth</td>
    <td>40</td>
    <td>Earth, DOT</td>
  </tr>

  <!-- Earth/Nature Cards -->
  <tr data-aspect="Earth">
    <td><strong>Terraform</strong></td>
    <td>Upgrades Earth Counter to Nature Counter, which heals 12.5% each turn</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td><span class="aspect-nature">Nature</span></td>
    <td>15 Priority +1</td>
    <td>Earth, Nature, Heal</td>
  </tr>
  <tr data-aspect="Earth">
    <td><strong>Naturalize</strong></td>
    <td>Remove opponent counter and heal 12.5%. Doubles damage if counter is removed</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td><span class="aspect-nature">Nature</span></td>
    <td>50</td>
    <td>Earth, Nature, Heal</td>
  </tr>
  <tr data-aspect="Earth">
    <td><strong>Life Leech</strong></td>
    <td>Deals damage and heals 50% of damage dealt. Deals 2x damage if user has Nature counter</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td><span class="aspect-nature">Nature</span></td>
    <td>45</td>
    <td>Earth, Nature, Heal</td>
  </tr>
  <tr data-aspect="Earth">
    <td><strong>Undercut</strong></td>
    <td>Deals 2x bonus damage if an Earth card is at the bottom of the deck</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td>Earth</td>
    <td>40</td>
    <td>Earth</td>
  </tr>
  <tr data-aspect="Earth">
    <td><strong>Erosion</strong></td>
    <td>Damage is multiplied by the number of turns that have passed if Earth counter is stacked</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td>Earth</td>
    <td>15</td>
    <td>Earth</td>
  </tr>
  <tr data-aspect="Earth">
    <td><strong>Earthquake</strong></td>
    <td>Deals damage, dealing extra damage while consuming earth counters</td>
    <td><span class="aspect-earth">Earth</span></td>
    <td>Earth</td>
    <td>25 (50,75,100)</td>
    <td>Earth</td>
  </tr>

  <!-- Add more cards here as you provide them -->

</tbody>
</table>

<script>
// Table sorting functionality
let sortDirection = {};

function sortTable(columnIndex) {
  const table = document.getElementById('card-table');
  const tbody = table.getElementsByTagName('tbody')[0];
  const rows = Array.from(tbody.getElementsByTagName('tr'));
  
  // Determine sort direction
  if (!sortDirection[columnIndex]) {
    sortDirection[columnIndex] = 'asc';
  } else {
    sortDirection[columnIndex] = sortDirection[columnIndex] === 'asc' ? 'desc' : 'asc';
  }
  
  // Sort rows
  rows.sort((a, b) => {
    const aValue = a.getElementsByTagName('td')[columnIndex].textContent.trim();
    const bValue = b.getElementsByTagName('td')[columnIndex].textContent.trim();
    
    // Try to parse as number for damage column
    const aNum = parseFloat(aValue);
    const bNum = parseFloat(bValue);
    
    if (!isNaN(aNum) && !isNaN(bNum)) {
      return sortDirection[columnIndex] === 'asc' ? aNum - bNum : bNum - aNum;
    }
    
    // String comparison
    if (sortDirection[columnIndex] === 'asc') {
      return aValue.localeCompare(bValue);
    } else {
      return bValue.localeCompare(aValue);
    }
  });
  
  // Re-append sorted rows
  rows.forEach(row => tbody.appendChild(row));
}

// Search functionality
function searchCards() {
  const input = document.getElementById('card-search');
  const filter = input.value.toLowerCase();
  const table = document.getElementById('card-table');
  const rows = table.getElementsByTagName('tr');
  let visibleCount = 0;
  
  for (let i = 1; i < rows.length; i++) {
    const row = rows[i];
    const text = row.textContent.toLowerCase();
    
    if (text.includes(filter)) {
      row.style.display = '';
      visibleCount++;
    } else {
      row.style.display = 'none';
    }
  }
  
  document.getElementById('showing-cards').textContent = visibleCount;
}

// Filter by aspect
function filterByAspect(aspect) {
  const table = document.getElementById('card-table');
  const rows = table.getElementsByTagName('tr');
  let visibleCount = 0;
  
  // Update button states
  const buttons = document.querySelectorAll('.filter-btn');
  buttons.forEach(btn => btn.classList.remove('active'));
  event.target.classList.add('active');
  
  for (let i = 1; i < rows.length; i++) {
    const row = rows[i];
    const rowAspect = row.getAttribute('data-aspect');
    
    if (aspect === 'all' || rowAspect === aspect) {
      row.style.display = '';
      visibleCount++;
    } else {
      row.style.display = 'none';
    }
  }
  
  document.getElementById('showing-cards').textContent = visibleCount;
  document.getElementById('card-search').value = ''; // Clear search
}
</script>

---

## Card Mechanics Guide

### Counters

Counters are status effects that persist on players or the arena:

- **Fire Counter** - Deals 12.5% damage per turn (DOT)
- **Lightning Counter** - Amplifies lightning damage based on stack count
- **Earth Counter** - Reduces incoming damage by 25% (max 3 stacks)
- **Nature Counter** - Heals 12.5% per turn
- **Slow Counter** - Reduces priority/speed
- **Blaze Counter** - Arena effect that amplifies fire damage
- **Storm Counter** - Arena effect that adds lightning to both players
- **Sandstorm Counter** - Arena effect that deals 12.5% DOT (doesn't affect Earth counters)

### Damage Types

- **Direct Damage** - Immediate damage dealt
- **DOT (Damage Over Time)** - Damage dealt each turn via counters
- **Burst Damage** - High damage by consuming counters
- **Priority** - Cards with +1 priority resolve before normal cards

### Strategy Tips

- **Fire builds** excel at DOT (damage over time) with Fire counters
- **Earth builds** provide strong defense and sustain
- **Lightning builds** ramp up damage with counter stacks
- **Hybrid builds** (Fire/Lightning, Earth/Nature) offer versatility
- Watch for arena counters (Blaze, Storm, Sandstorm) that affect both players

---

## Deck Building Tips

- **Synergize elements** - Cards that work together are stronger
- **Balance offense and defense** - Pure aggro or pure defense can be exploited
- **Include counter removal** - Cards like Incinerate and Naturalize
- **Consider priority** - Fortify and Terraform can set up combos
- **Watch your curve** - Mix of high and low damage cards

---

## Want to Contribute?

Found an error or have suggestions for card balance? [Edit this page on GitHub](https://github.com/VincentVeak/PicoNav/edit/main/docs/cards.md) or [open an issue](https://github.com/VincentVeak/PicoNav/issues).

---

**Note:** This card database is updated with each new release. Check the [Updates page](/docs/updates) for balance changes and new card additions.
