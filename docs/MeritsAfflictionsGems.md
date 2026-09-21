---
layout: default
title: Merits, Afflictions & Gems
nav_order: 4
---

# Merits, Afflictions & Gems

Deep Dungeon merits, afflictions, and gems. Pick a version to see descriptions as of that patch, or search across all three.

<div class="filter-controls">
  <input type="text" class="card-search" id="mag-search" placeholder="🔍 Search by name or description...">
  <div class="version-control">
    <label for="mag-version"><strong>Version:</strong></label>
    <select id="mag-version" class="version-select"></select>
  </div>
</div>

---

## Gems

<table class="card-table" id="gems-table">
<thead><tr><th style="width:64px">Icon</th><th style="width:22%">Name</th><th>Description</th></tr></thead>
<tbody id="gems-body"></tbody>
</table>

## Merits

<table class="card-table" id="merits-table">
<thead><tr><th id="merit-sort-name" class="sortable" style="width:20%">Name <span class="sort-arrow"></span></th><th id="merit-sort-rarity" class="sortable" style="width:110px">Rarity <span class="sort-arrow"></span></th><th>Description</th></tr></thead>
<tbody id="merits-body"></tbody>
</table>

## Afflictions

<table class="card-table" id="affl-table">
<thead><tr><th style="width:22%">Name</th><th>Description</th></tr></thead>
<tbody id="affl-body"></tbody>
</table>

<style>
.filter-controls { margin: 2rem 0; }
.card-search { width: 100%; padding: 0.75rem 1rem; font-size: 1rem; border: 2px solid #FF7573; border-radius: 25px; background-color: #1f2a2b; color: #E6EBC5; margin-bottom: 1rem; font-family: 'Comfortaa', sans-serif; }
.card-search::placeholder { color: rgba(230, 235, 197, 0.5); }
.version-control { margin-bottom: 1rem; display: flex; align-items: center; gap: 0.5rem; }
.version-select { padding: 0.4rem 0.75rem; border: 2px solid #FF7573; border-radius: 25px; background-color: #1f2a2b; color: #E6EBC5; font-family: 'Comfortaa', sans-serif; font-weight: 600; cursor: pointer; }
.card-table { width: 100%; max-width: 100%; border-collapse: separate; border-spacing: 0; border: 1px solid rgba(255, 117, 115, 0.3); border-radius: 12px; margin: 1rem 0 2.5rem; font-size: 0.8rem; table-layout: fixed; }
.card-table thead th { background-color: #FF7573; color: #00313A; padding: 0.6rem 0.5rem; text-align: left; font-weight: 600; position: sticky; top: 0; z-index: 10; }
.card-table thead th:first-child { border-top-left-radius: 11px; }
.card-table thead th:last-child { border-top-right-radius: 11px; }
.card-table tbody tr:last-child td:first-child { border-bottom-left-radius: 11px; }
.card-table tbody tr:last-child td:last-child { border-bottom-right-radius: 11px; }
.card-table tbody td { padding: 0.5rem; color: #E6EBC5; vertical-align: middle; word-wrap: break-word; border-bottom: 1px solid rgba(255, 117, 115, 0.2); }
.card-table tbody tr:last-child td { border-bottom: none; }
.card-table tbody tr:hover { background-color: rgba(255, 117, 115, 0.1); }
.gem-icon { text-align: center; padding: 0.3rem !important; }
.gem-icon img { width: 40px; height: 40px; object-fit: contain; display: block; margin: 0 auto; image-rendering: pixelated; }
.rarity { font-weight: 600; white-space: nowrap; }
.rarity-common { color: #C9D1D9; }
.rarity-uncommon { color: #57D18C; }
.rarity-rare { color: #5AA9F0; }
.rarity-epic { color: #B98BFF; }
.rarity-legendary { color: #F5B84A; }
.card-table th.sortable { cursor: pointer; user-select: none; }
.card-table th.sortable:hover { background-color: #ff8f8d; }
.sort-arrow { font-size: 0.7em; margin-left: 0.25rem; }
</style>

<script>
(function(){
  var RAW = 'https://raw.githubusercontent.com/VincentVeak/PicoNav/main/gamedata.json';
  var GEMBASE = '/assets/images/gems/';
  var DATA = null, VERSION = null;

  function cmpVer(a, b){ var pa = String(a).split('.').map(Number), pb = String(b).split('.').map(Number); for (var i = 0; i < 3; i++){ var x = pa[i]||0, y = pb[i]||0; if (x !== y) return x - y; } return 0; }
  function resolve(item, ver){ var best = null; for (var i = 0; i < item.history.length; i++){ var h = item.history[i]; if (cmpVer(h.version, ver) <= 0 && (!best || cmpVer(h.version, best.version) > 0)) best = h; } return best || item.history[item.history.length - 1]; }
  function esc(s){ return (s==null?'':String(s)).replace(/[&<>"]/g, function(c){ return {'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]; }); }
  function q(){ return (document.getElementById('mag-search').value || '').toLowerCase(); }
  function match(name, desc){ var s = q(); return !s || (name + ' ' + desc).toLowerCase().indexOf(s) >= 0; }

  var RARITY_RANK = { common: 1, uncommon: 2, rare: 3, epic: 4, legendary: 5 };
  var meritSort = { key: 'name', dir: 1 };
  function meritCmp(a, b){
    if (meritSort.key === 'rarity'){
      var ra = RARITY_RANK[(a.rarity||'').toLowerCase()] || 99;
      var rb = RARITY_RANK[(b.rarity||'').toLowerCase()] || 99;
      if (ra !== rb) return (ra - rb) * meritSort.dir;
      return a.name.localeCompare(b.name);
    }
    return a.name.localeCompare(b.name) * meritSort.dir;
  }
  function updateSortArrows(){
    var n = document.querySelector('#merit-sort-name .sort-arrow');
    var r = document.querySelector('#merit-sort-rarity .sort-arrow');
    if (n) n.textContent = meritSort.key === 'name' ? (meritSort.dir > 0 ? '▲' : '▼') : '';
    if (r) r.textContent = meritSort.key === 'rarity' ? (meritSort.dir > 0 ? '▲' : '▼') : '';
  }
  function renderList(list, bodyId, withIcon, cols, sortByName, withRarity, cmp){
    var body = document.getElementById(bodyId); if (!body) return;
    var arr = list.slice();
    if (cmp) arr.sort(cmp);
    else if (sortByName) arr.sort(function(a, b){ return a.name.localeCompare(b.name); });
    var html = arr.map(function(it){
      var h = resolve(it, VERSION); var desc = h.description || '';
      if (!match(it.name, desc)) return '';
      var row = '<tr>';
      if (withIcon) row += '<td class="gem-icon"><img src="' + GEMBASE + encodeURI(it.icon || '') + '" alt="" onerror="this.style.display=\'none\'"></td>';
      row += '<td><strong>' + esc(it.name) + '</strong></td>';
      if (withRarity) row += '<td class="rarity rarity-' + (it.rarity||'').toLowerCase() + '">' + esc(it.rarity || '') + '</td>';
      row += '<td>' + esc(desc) + '</td></tr>';
      return row;
    }).join('');
    if (!html) html = '<tr><td colspan="' + cols + '">No matches.</td></tr>';
    body.innerHTML = html;
  }
  function render(){
    renderList(DATA.gems || [], 'gems-body', true, 3, false);
    renderList(DATA.merits || [], 'merits-body', false, 3, false, true, meritCmp);
    renderList(DATA.afflictions || [], 'affl-body', false, 2, true);
    updateSortArrows();
  }

  fetch(RAW).then(function(r){ return r.json(); }).then(function(d){
    DATA = d;
    if (d.iconPaths && d.iconPaths.gems) GEMBASE = '/' + d.iconPaths.gems;
    var vers = {};
    ['cards', 'merits', 'afflictions', 'gems'].forEach(function(k){ (d[k] || []).forEach(function(it){ (it.history || []).forEach(function(h){ vers[h.version] = 1; }); }); });
    var sorted = Object.keys(vers).sort(cmpVer);
    VERSION = sorted[sorted.length - 1];
    var sel = document.getElementById('mag-version');
    sorted.slice().reverse().forEach(function(v){ var o = document.createElement('option'); o.value = v; o.textContent = 'v' + v; sel.appendChild(o); });
    sel.value = VERSION;
    sel.addEventListener('change', function(){ VERSION = sel.value; render(); });
    document.getElementById('mag-search').addEventListener('input', render);
    var sortName = document.getElementById('merit-sort-name');
    var sortRarity = document.getElementById('merit-sort-rarity');
    if (sortName) sortName.addEventListener('click', function(){ if (meritSort.key === 'name') meritSort.dir *= -1; else { meritSort.key = 'name'; meritSort.dir = 1; } render(); });
    if (sortRarity) sortRarity.addEventListener('click', function(){ if (meritSort.key === 'rarity') meritSort.dir *= -1; else { meritSort.key = 'rarity'; meritSort.dir = 1; } render(); });
    render();
  }).catch(function(){ document.getElementById('gems-body').innerHTML = '<tr><td colspan="3">Could not load data from GitHub.</td></tr>'; });
})();
</script>

## Want to Contribute?

Found an error or have balance suggestions?

- [Open an issue](https://github.com/VincentVeak/PicoNav/issues)
- [Discuss in the community Discord](https://discord.gg/79x5fdQMWX)

---
**Note:** Descriptions are subject to change with balance updates. Check the [Updates page](/docs/updates) for the latest patch notes.
