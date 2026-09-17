"use strict";
(function () {
  const normalize = value => String(value || "").normalize("NFKC").toLowerCase();
  function matches(row, filters) {
    return (!filters.category || row.category === filters.category) &&
      (!filters.status || row.status === filters.status) &&
      normalize(filters.search).trim().split(/\s+/).filter(Boolean).every(word => normalize(row.text).includes(word));
  }
  function setup(root) {
    const search = root.querySelector("[data-coverage-search]");
    const category = root.querySelector("[data-coverage-category]");
    const status = root.querySelector("[data-coverage-status]");
    const rows = Array.from(root.querySelectorAll("[data-coverage-row]"));
    const records = rows.map(row => ({category: row.dataset.category, status: row.dataset.status, text: row.textContent}));
    function apply() {
      let count = 0;
      const filters = {search: search.value, category: category.value, status: status.value};
      rows.forEach((row, i) => { row.hidden = !matches(records[i], filters); if (!row.hidden) ++count; });
      root.querySelector("[data-coverage-count]").textContent = count + " / " + rows.length + " 問題";
      root.querySelector("[data-coverage-empty]").hidden = count !== 0;
    }
    search.addEventListener("input", apply);
    category.addEventListener("change", apply);
    status.addEventListener("change", apply);
    root.querySelector("[data-coverage-reset]").addEventListener("click", () => {
      search.value = category.value = status.value = "";
      apply(); search.focus();
    });
    root.querySelector("[data-coverage-controls]").hidden = false;
    apply();
  }
  if (typeof module !== "undefined" && module.exports) module.exports = {matches, setup};
  if (typeof document !== "undefined") document.addEventListener("DOMContentLoaded", () => document.querySelectorAll("[data-coverage]").forEach(setup));
})();
