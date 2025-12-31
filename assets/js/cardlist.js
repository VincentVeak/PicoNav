(function() {
  'use strict';
  
  let currentAspectFilter = 'all';
  let sortDirection = {};

  // Initialize on page load
  function init() {
    console.log('CardList JavaScript initializing...');
    
    updateCardCount();
    
    // Attach event listeners to filter buttons
    const filterButtons = document.querySelectorAll('.filter-btn');
    console.log('Found', filterButtons.length, 'filter buttons');
    
    filterButtons.forEach(btn => {
      btn.addEventListener('click', function(e) {
        e.preventDefault();
        const aspect = this.textContent.trim();
        console.log('Filter button clicked:', aspect);
        filterByAspect(aspect);
      });
    });
    
    // Attach event listener to search input
    const searchInput = document.getElementById('card-search');
    if (searchInput) {
      console.log('Search input found');
      searchInput.addEventListener('input', function() {
        console.log('Search input changed:', this.value);
        filterCards();
      });
      searchInput.addEventListener('keyup', filterCards);
    } else {
      console.error('Search input not found');
    }
    
    // Attach event listeners to sortable headers
    const sortableHeaders = document.querySelectorAll('.sortable');
    console.log('Found', sortableHeaders.length, 'sortable headers');
    
    sortableHeaders.forEach((header, index) => {
      header.addEventListener('click', function(e) {
        e.preventDefault();
        console.log('Sorting column:', index);
        sortTable(index);
      });
    });
    
    console.log('CardList JavaScript initialized successfully');
  }

  // Filter by aspect
  function filterByAspect(aspect) {
    console.log('filterByAspect called with:', aspect);
    
    // Map button text to aspect values
    const aspectMap = {
      'All': 'all',
      'Fire': 'Fire',
      'Earth': 'Earth',
      'Water': 'Water',
      'Wind': 'Wind',
      'Neutral': 'NULL'
    };
    
    currentAspectFilter = aspectMap[aspect] || 'all';
    console.log('Current aspect filter set to:', currentAspectFilter);
    
    // Update button states
    document.querySelectorAll('.filter-btn').forEach(btn => {
      btn.classList.remove('active');
      if (btn.textContent.trim() === aspect) {
        btn.classList.add('active');
      }
    });
    
    // Apply filter
    filterCards();
  }

  // Combined filter function (search + aspect)
  function filterCards() {
    console.log('filterCards called');
    
    const searchInput = document.getElementById('card-search');
    const searchTerm = searchInput ? searchInput.value.toLowerCase() : '';
    const table = document.getElementById('card-table');
    
    if (!table) {
      console.error('Table not found!');
      return;
    }
    
    const tbody = table.getElementsByTagName('tbody')[0];
    if (!tbody) {
      console.error('Table body not found!');
      return;
    }
    
    const rows = tbody.getElementsByTagName('tr');
    let visibleCount = 0;
    
    console.log('Filtering', rows.length, 'rows with search:', searchTerm, 'aspect:', currentAspectFilter);
    
    for (let i = 0; i < rows.length; i++) {
      const row = rows[i];
      const aspect = row.getAttribute('data-aspect');
      const text = row.textContent.toLowerCase();
      
      // Check aspect filter
      const aspectMatch = currentAspectFilter === 'all' || aspect === currentAspectFilter;
      
      // Check search term
      const searchMatch = searchTerm === '' || text.includes(searchTerm);
      
      // Show/hide row
      if (aspectMatch && searchMatch) {
        row.style.display = '';
        row.classList.remove('hidden-row');
        visibleCount++;
      } else {
        row.style.display = 'none';
        row.classList.add('hidden-row');
      }
    }
    
    console.log('Visible rows:', visibleCount);
    
    // Update count
    const showingElement = document.getElementById('showing-cards');
    if (showingElement) {
      showingElement.textContent = visibleCount;
    }
  }

  // Sort table
  function sortTable(columnIndex) {
    console.log('sortTable called for column:', columnIndex);
    
    const table = document.getElementById('card-table');
    if (!table) {
      console.error('Table not found!');
      return;
    }
    
    const tbody = table.getElementsByTagName('tbody')[0];
    const rows = Array.from(tbody.getElementsByTagName('tr'));
    
    // Determine sort direction
    if (!sortDirection[columnIndex]) {
      sortDirection[columnIndex] = 'asc';
    } else {
      sortDirection[columnIndex] = sortDirection[columnIndex] === 'asc' ? 'desc' : 'asc';
    }
    
    const direction = sortDirection[columnIndex];
    console.log('Sorting direction:', direction);
    
    // Sort rows
    rows.sort((a, b) => {
      const aCells = a.getElementsByTagName('td');
      const bCells = b.getElementsByTagName('td');
      
      let aValue = aCells[columnIndex].textContent.trim();
      let bValue = bCells[columnIndex].textContent.trim();
      
      // Special handling for damage column (numeric sort)
      if (columnIndex === 5) {
        const aData = aCells[columnIndex].getAttribute('data-value');
        const bData = bCells[columnIndex].getAttribute('data-value');
        aValue = parseInt(aData) || 0;
        bValue = parseInt(bData) || 0;
      }
      
      // Numeric comparison
      if (!isNaN(aValue) && !isNaN(bValue)) {
        return direction === 'asc' ? aValue - bValue : bValue - aValue;
      }
      
      // String comparison
      if (direction === 'asc') {
        return aValue.localeCompare(bValue);
      } else {
        return bValue.localeCompare(aValue);
      }
    });
    
    // Reorder rows in table
    rows.forEach(row => tbody.appendChild(row));
    
    // Update visual indicators
    updateSortIndicators(columnIndex, direction);
  }

  // Update sort arrow indicators
  function updateSortIndicators(activeColumn, direction) {
    const headers = document.querySelectorAll('.card-table thead th');
    headers.forEach((header, index) => {
      const arrow = header.querySelector('.sort-arrow');
      if (arrow) {
        if (index === activeColumn) {
          header.classList.add('sorted');
          arrow.textContent = direction === 'asc' ? '↑' : '↓';
        } else {
          header.classList.remove('sorted');
          arrow.textContent = '⇅';
        }
      }
    });
  }

  // Update card count
  function updateCardCount() {
    const table = document.getElementById('card-table');
    if (!table) {
      console.error('Table not found for count update');
      return;
    }
    
    const tbody = table.getElementsByTagName('tbody')[0];
    const totalRows = tbody.getElementsByTagName('tr').length;
    
    console.log('Total cards:', totalRows);
    
    const totalElement = document.getElementById('total-cards');
    const showingElement = document.getElementById('showing-cards');
    
    if (totalElement) totalElement.textContent = totalRows;
    if (showingElement) showingElement.textContent = totalRows;
  }

  // Run initialization when DOM is ready
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
  } else {
    // DOM already loaded
    init();
  }
})();
