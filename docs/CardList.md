---
layout: default
title: Card Reference Database
nav_order: 3
---

# Card Reference Database

Complete reference for all cards in PicoNav. Pick a game version to see stats as of that patch. Click column headers to sort, or use search and the aspect filters.

<div class="stats-box">
  <strong>Total Available Cards:</strong> <span id="total-cards">0</span> |
  <strong>Showing:</strong> <span id="showing-cards">0</span>
</div>

<div class="filter-controls">
  <input type="text" class="card-search" id="card-search" placeholder="🔍 Search cards by name, description, or tags...">

  <div class="version-control">
    <label for="version-select"><strong>Version:</strong></label>
    <select id="version-select" class="version-select"></select>
  </div>

  <div class="filter-buttons">
    <button class="filter-btn active" data-aspect="all">All</button>
    <button class="filter-btn aspect-fire" data-aspect="Fire">Fire</button>
    <button class="filter-btn aspect-earth" data-aspect="Earth">Earth</button>
    <button class="filter-btn aspect-water" data-aspect="Water">Water</button>
    <button class="filter-btn aspect-wind" data-aspect="Wind">Wind</button>
    <button class="filter-btn aspect-neutral" data-aspect="Neutral">Neutral</button>
  </div>
</div>

---

<table class="card-table" id="card-table">
<thead>
  <tr>
    <th data-col="icon">Icon</th>
    <th class="sortable" data-col="name">Name <span class="sort-arrow">⇅</span></th>
    <th class="sortable" data-col="desc">Description</th>
    <th class="sortable" data-col="aspect">Aspect <span class="sort-arrow">⇅</span></th>
    <th class="sortable" data-col="series">Series <span class="sort-arrow">⇅</span></th>
    <th class="sortable" data-col="power">Base Power <span class="sort-arrow">⇅</span></th>
    <th class="sortable" data-col="tags">Tags <span class="sort-arrow">⇅</span></th>
  </tr>
</thead>
<tbody id="card-body"></tbody>
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
  background-color: #1f2a2b;
  color: #E6EBC5;
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
  padding: 0.25rem 1rem;
  border: 2px solid #FF7573;
  background-color: rgba(0, 31, 37, 0.5);
  color: #E6EBC5;
  border-radius: 45px;
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
  background-color: #1f2a2b;
  color: #E6EBC5;
  border: 2px solid #FF7573;
  border-radius: 45px;
  padding: 1rem;
  margin: 1rem 0;
  text-align: center;
}

/* Table Styles */
.card-table {
  width: 100%;
  max-width: 100%;
  border-collapse: separate;
  border-spacing: 0;
  border: 1px solid rgba(255, 117, 115, 0.3);
  border-radius: 12px;
  margin: 2rem 0;
  font-size: 0.75rem;
  table-layout: fixed;
}

.card-table thead th:first-child { border-top-left-radius: 11px; }
.card-table thead th:last-child { border-top-right-radius: 11px; }
.card-table tbody tr:last-child td:first-child { border-bottom-left-radius: 11px; }
.card-table tbody tr:last-child td:last-child { border-bottom-right-radius: 11px; }

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
.card-table thead th:nth-child(2) { width: 15%; }    /* Name */
.card-table thead th:nth-child(3) { width: 30%; }    /* Description - largest */
.card-table thead th:nth-child(4) { width: 10%; }     /* Aspect */
.card-table thead th:nth-child(5) { width: 12%; }    /* Series */
.card-table thead th:nth-child(6) { width: 8%; }     /* Damage - very small */
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
  transition: all 0.2s ease;
}

.card-table tbody td {
  border-bottom: 1px solid rgba(255, 117, 115, 0.2);
}

.card-table tbody tr:last-child td {
  border-bottom: none;
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

.aspect-fire {
  background-color: #FF5A33;
  color: white;
  padding: 0.2rem 0.5rem;
  border-radius: 25px;
  font-size: 0.7rem;
  font-weight: 600;
  display: inline-block;
  white-space: nowrap;
}

.aspect-earth {
  background-color: #4CAF50;
  color: white;
  padding: 0.2rem 0.5rem;
  border-radius: 25px;
  font-size: 0.7rem;
  font-weight: 600;
  display: inline-block;
  white-space: nowrap;
}

.aspect-water {
  background-color: #29A8E0;
  color: white;
  padding: 0.2rem 0.5rem;
  border-radius: 25px;
  font-size: 0.7rem;
  font-weight: 600;
  display: inline-block;
  white-space: nowrap;
}

.aspect-wind {
  background-color: #9370DB;
  color: white;
  padding: 0.2rem 0.5rem;
  border-radius: 25px;
  font-size: 0.7rem;
  font-weight: 600;
  display: inline-block;
  white-space: nowrap;
}

.aspect-neutral {
  background-color: #808080;
  color: white;
  padding: 0.2rem 0.5rem;
  border-radius: 25px;
  font-size: 0.7rem;
  font-weight: 600;
  display: inline-block;
  white-space: nowrap;
}

.aspect-lightning {
  background-color: #ffd700;
  color: #00313A;
  padding: 0.2rem 0.5rem;
  border-radius: 25px;
  font-size: 0.7rem;
  font-weight: 600;
  display: inline-block;
  white-space: nowrap;
}

.aspect-nature {
  background-color: #228b22;
  color: white;
  padding: 0.2rem 0.5rem;
  border-radius: 25px;
  font-size: 0.7rem;
  font-weight: 600;
  display: inline-block;
  white-space: nowrap;
}

.aspect-steam {
  background-color: #e0e0e0;
  color: #00313A;
  padding: 0.2rem 0.5rem;
  border-radius: 25px;
  font-size: 0.7rem;
  font-weight: 600;
  display: inline-block;
  white-space: nowrap;
}

/* Hidden row class */
.hidden-row {
  display: none !important;
}
.version-control { margin-bottom: 1rem; display: flex; align-items: center; gap: 0.5rem; }
.version-select { padding: 0.4rem 0.75rem; border: 2px solid #FF7573; border-radius: 25px; background-color: #1f2a2b; color: #E6EBC5; font-family: 'Comfortaa', sans-serif; font-weight: 600; cursor: pointer; }
</style>

<script>
(function(){
  var RAW = 'https://raw.githubusercontent.com/VincentVeak/PicoNav/main/gamedata.json';
  var ICONBASE = '/assets/images/cards/';
  var DATA = null, VERSION = null, ASPECT = 'all', SORTCOL = null, SORTDIR = 1;

  function cmpVer(a, b){
    var pa = String(a).split('.').map(Number), pb = String(b).split('.').map(Number);
    for (var i = 0; i < 3; i++){ var x = pa[i]||0, y = pb[i]||0; if (x !== y) return x - y; }
    return 0;
  }
  function resolve(card, ver){
    var best = null;
    for (var i = 0; i < card.history.length; i++){
      var h = card.history[i];
      if (cmpVer(h.version, ver) <= 0 && (!best || cmpVer(h.version, best.version) > 0)) best = h;
    }
    return best || card.history[card.history.length - 1];
  }
  function esc(s){ return (s==null?'':String(s)).replace(/[&<>"]/g, function(c){ return {'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]; }); }

  function rows(){
    var q = (document.getElementById('card-search').value || '').toLowerCase();
    var out = [];
    DATA.cards.forEach(function(c){
      var h = resolve(c, VERSION);
      if (ASPECT !== 'all' && (h.aspect||'') !== ASPECT) return;
      var hay = (c.name + ' ' + (h.description||'') + ' ' + ((h.tags||[]).join(' '))).toLowerCase();
      if (q && hay.indexOf(q) < 0) return;
      out.push({ c: c, h: h });
    });
    if (SORTCOL){
      out.sort(function(a, b){
        if (SORTCOL === 'power') return ((a.h.power||0) - (b.h.power||0)) * SORTDIR;
        var va, vb;
        if (SORTCOL === 'name'){ va = a.c.name; vb = b.c.name; }
        else if (SORTCOL === 'aspect'){ va = a.h.aspect||''; vb = b.h.aspect||''; }
        else if (SORTCOL === 'series'){ va = a.h.series||''; vb = b.h.series||''; }
        else if (SORTCOL === 'tags'){ va = (a.h.tags||[]).join(','); vb = (b.h.tags||[]).join(','); }
        else if (SORTCOL === 'desc'){ va = a.h.description||''; vb = b.h.description||''; }
        else return 0;
        return String(va).localeCompare(String(vb)) * SORTDIR;
      });
    }
    return out;
  }
  function render(){
    var list = rows();
    document.getElementById('card-body').innerHTML = list.map(function(x){
      var c = x.c, h = x.h;
      return '<tr data-aspect="' + esc(h.aspect) + '">' +
        '<td class="card-icon"><img src="' + ICONBASE + encodeURI(c.icon) + '" alt="' + esc(c.name) + '"></td>' +
        '<td><strong>' + esc(c.name) + '</strong></td>' +
        '<td>' + esc(h.description) + '</td>' +
        '<td><span class="aspect-' + esc((h.aspect||'').toLowerCase()) + '">' + esc(h.aspect) + '</span></td>' +
        '<td>' + esc(h.series) + '</td>' +
        '<td data-value="' + esc(h.power) + '">' + esc(h.power) + '</td>' +
        '<td>' + esc((h.tags||[]).join(', ')) + '</td>' +
        '</tr>';
    }).join('');
    document.getElementById('total-cards').textContent = DATA.cards.length;
    document.getElementById('showing-cards').textContent = list.length;
  }

  fetch(RAW).then(function(r){ return r.json(); }).then(function(d){
    DATA = d;
    var vers = {};
    d.cards.forEach(function(c){ c.history.forEach(function(h){ vers[h.version] = 1; }); });
    var sorted = Object.keys(vers).sort(cmpVer);
    VERSION = sorted[sorted.length - 1];
    var sel = document.getElementById('version-select');
    sorted.slice().reverse().forEach(function(v){
      var o = document.createElement('option'); o.value = v; o.textContent = 'v' + v; sel.appendChild(o);
    });
    sel.value = VERSION;
    sel.addEventListener('change', function(){ VERSION = sel.value; render(); });
    document.getElementById('card-search').addEventListener('input', render);
    var btns = document.querySelectorAll('.filter-btn');
    btns.forEach(function(b){ b.addEventListener('click', function(){
      btns.forEach(function(x){ x.classList.remove('active'); }); b.classList.add('active');
      ASPECT = b.getAttribute('data-aspect'); render();
    }); });
    document.querySelectorAll('#card-table th.sortable').forEach(function(th){
      var col = th.getAttribute('data-col');
      th.style.cursor = 'pointer';
      th.addEventListener('click', function(){
        if (SORTCOL === col){ SORTDIR = -SORTDIR; } else { SORTCOL = col; SORTDIR = 1; }
        render();
      });
    });
    render();
  }).catch(function(){
    document.getElementById('card-body').innerHTML = '<tr><td colspan="7">Could not load card data from GitHub.</td></tr>';
  });
})();
</script>

## Want to Contribute?

Found an error or have balance suggestions?

- [Open an issue](https://github.com/VincentVeak/PicoNav/issues)
- [Discuss in the community Discord](https://discord.gg/79x5fdQMWX)

---
**Note:** Card stats and mechanics are subject to change with balance updates. Check the [Updates page](/docs/updates) for the latest patch notes.
