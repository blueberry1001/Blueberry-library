"use strict";

(function () {
  const normalize = (text) => String(text || "").normalize("NFKC").toLowerCase();
  const dimensions = ["queries", "updates", "conditions"];

  function matches(entry, filters = {}) {
    if (!dimensions.every(name => !filters[name] || entry[name].includes(filters[name]))) return false;
    const searchText = entry.searchText === undefined
      ? [entry.title, entry.prerequisites, entry.notes,
          ...(entry.headers || []),
          ...(entry.calls || []).map(call => call.signature + " " + call.complexity)].join(" ")
      : entry.searchText;
    const words = normalize(filters.search).trim().split(/\s+/).filter(Boolean);
    const text = normalize(searchText);
    return words.every(word => text.includes(word));
  }

  function setupFinder(root) {
    const controls = Object.fromEntries(
      [...dimensions, "search"].map(name => [name, root.querySelector('[data-operation-filter="' + name + '"]')])
    );
    const cards = Array.from(root.querySelectorAll("[data-operation-entry]"));
    const entries = cards.map(card => ({
      queries: card.dataset.queries.split(" "),
      updates: card.dataset.updates.split(" "),
      conditions: card.dataset.conditions.split(" "),
      searchText: card.textContent + " " + (card.dataset.keywords || "")
    }));
    function apply() {
      const filters = Object.fromEntries(Object.entries(controls).map(([name, control]) => [name, control.value]));
      let count = 0;
      cards.forEach((card, index) => {
        card.hidden = !matches(entries[index], filters);
        if (!card.hidden) ++count;
      });
      root.querySelector("[data-operation-count]").textContent = count + " / " + cards.length + " 候補";
      root.querySelector("[data-operation-empty]").hidden = count !== 0;
    }
    for (const name of dimensions) controls[name].addEventListener("change", apply);
    controls.search.addEventListener("input", apply);
    root.querySelector("[data-operation-reset]").addEventListener("click", () => {
      Object.values(controls).forEach(control => { control.value = ""; });
      apply();
      controls.queries.focus();
    });
    root.querySelector("[data-operation-controls]").hidden = false;
    apply();
  }

  if (typeof module !== "undefined" && module.exports) module.exports = { matches, setupFinder };
  if (typeof document !== "undefined") {
    document.addEventListener("DOMContentLoaded", () => {
      document.querySelectorAll("[data-operation-finder]").forEach(setupFinder);
    });
  }
})();
