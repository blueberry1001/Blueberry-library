"use strict";

(function () {
  const normalize = (text) => String(text || "").normalize("NFKC").toLowerCase();
  const dimensions = ["targets", "queries", "updates", "conditions"];

  function matches(entry, filters = {}) {
    // No updates is a requirement on the workload, not a restriction on the
    // structure's capabilities. Build/load a dynamic structure once and query it.
    if (!dimensions.every(name => !filters[name] ||
      (name === "updates" && filters[name] === "static") ||
      (name === "conditions" && filters[name] === "offline-queries" &&
        (entry.conditions || []).includes("online-queries")) ||
      (entry[name] || []).includes(filters[name]))) return false;
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
      targets: (card.dataset.targets || "").split(" "),
      queries: card.dataset.queries.split(" "),
      updates: card.dataset.updates.split(" "),
      conditions: card.dataset.conditions.split(" "),
      searchText: card.textContent + " " + (card.dataset.keywords || "")
    }));
    const dependent = ["queries", "updates", "conditions"];
    const originalOptions = Object.fromEntries(dependent.map(name => [name, Array.from(controls[name].options)]));
    let notice = "";
    function refreshOptions() {
      let cleared = false;
      for (const name of dependent) {
        const selected = controls[name].value;
        const options = originalOptions[name].filter(option => !option.value || !controls.targets.value ||
          entries.some(entry => matches(entry, {targets: controls.targets.value, [name]: option.value})));
        // Detach unavailable options instead of relying on hidden <option>,
        // which native select popups do not consistently respect.
        controls[name].replaceChildren(...options);
        controls[name].value = options.some(option => option.value === selected) ? selected : "";
        cleared ||= Boolean(selected && !controls[name].value);
      }
      notice = cleared ? "。対象外の条件を解除しました" : "";
    }
    function apply() {
      const filters = Object.fromEntries(Object.entries(controls).map(([name, control]) => [name, control.value]));
      let count = 0;
      cards.forEach((card, index) => {
        card.hidden = !matches(entries[index], filters);
        if (!card.hidden) ++count;
      });
      // Move actual DOM nodes so visual and keyboard order stay the same.
      // This is a workload-specific preference, not a universal speed ranking.
      const priority = index => filters.updates === "static" && entries[index].updates.includes("static") ? 0 : 1;
      cards.map((_, index) => index).sort((a, b) => priority(a) - priority(b) || a - b)
        .forEach(index => root.querySelector(".operation-results").appendChild(cards[index]));
      root.querySelector("[data-operation-count]").textContent = count + " / " + cards.length + " 候補" + notice;
      root.querySelector("[data-operation-empty]").hidden = count !== 0;
    }
    controls.targets.addEventListener("change", () => { refreshOptions(); apply(); });
    for (const name of dependent) controls[name].addEventListener("change", () => { notice = ""; apply(); });
    controls.search.addEventListener("input", () => { notice = ""; apply(); });
    root.querySelector("[data-operation-reset]").addEventListener("click", () => {
      Object.values(controls).forEach(control => { control.value = ""; });
      refreshOptions();
      apply();
      controls.targets.focus();
    });
    root.querySelector("[data-operation-controls]").hidden = false;
    refreshOptions();
    apply();
  }

  if (typeof module !== "undefined" && module.exports) module.exports = { matches, setupFinder };
  if (typeof document !== "undefined") {
    document.addEventListener("DOMContentLoaded", () => {
      document.querySelectorAll("[data-operation-finder]").forEach(setupFinder);
    });
  }
})();
