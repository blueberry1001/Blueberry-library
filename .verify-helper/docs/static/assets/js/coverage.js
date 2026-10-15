"use strict";
(function () {
  const normalize = value => String(value || "").normalize("NFKC").toLowerCase();
  function matches(row, filters) {
    return (!filters.category || row.category === filters.category) &&
      (!filters.status || row.status === filters.status) &&
      normalize(filters.search).trim().split(/\s+/).filter(Boolean).every(word => normalize(row.text).includes(word));
  }
  function readFilters(url, categories, statuses) {
    const params = new URL(url).searchParams;
    const category = params.get("category") || "", status = params.get("status") || "";
    return {search: params.get("query") || "", category: categories.includes(category) ? category : "",
      status: statuses.includes(status) ? status : ""};
  }
  function filterURL(url, filters) {
    const next = new URL(url);
    for (const [name, value] of [["query", filters.search], ["category", filters.category], ["status", filters.status]]) {
      if (value) next.searchParams.set(name, value);
      else next.searchParams.delete(name);
    }
    return next;
  }
  function setup(root) {
    const browser = root.ownerDocument.defaultView;
    const search = root.querySelector("[data-coverage-search]");
    const category = root.querySelector("[data-coverage-category]");
    const status = root.querySelector("[data-coverage-status]");
    const rows = Array.from(root.querySelectorAll("[data-coverage-row]"));
    const records = rows.map(row => ({category: row.dataset.category, status: row.dataset.status, text: row.textContent}));
    const quickLinks = Array.from(root.querySelectorAll("[data-coverage-quick]"));
    function apply(updateURL = true) {
      let count = 0;
      const filters = {search: search.value, category: category.value, status: status.value};
      rows.forEach((row, i) => { row.hidden = !matches(records[i], filters); if (!row.hidden) ++count; });
      const label = status.options[status.selectedIndex].textContent;
      root.querySelector("[data-coverage-count]").textContent = label + " · " + count + " / " + rows.length + " 問題";
      root.querySelector("[data-coverage-empty]").hidden = count !== 0;
      root.querySelector("[data-coverage-reset]").hidden = !filters.search && !filters.category && !filters.status;
      quickLinks.forEach(link => {
        if (link.dataset.coverageQuick === status.value) link.setAttribute("aria-current", "true");
        else link.removeAttribute("aria-current");
      });
      if (updateURL) {
        const next = filterURL(browser.location.href, filters);
        if (next.href !== browser.location.href) browser.history.replaceState(browser.history.state, "", next.href);
      }
    }
    function restore() {
      const filters = readFilters(browser.location.href, Array.from(category.options, option => option.value),
        Array.from(status.options, option => option.value));
      search.value = filters.search; category.value = filters.category; status.value = filters.status;
      apply(false);
    }
    search.addEventListener("input", () => apply());
    category.addEventListener("change", () => apply());
    status.addEventListener("change", () => apply());
    quickLinks.forEach(link => link.addEventListener("click", event => {
      if (event.button !== 0 || event.ctrlKey || event.metaKey || event.altKey || event.shiftKey) return;
      event.preventDefault(); status.value = link.dataset.coverageQuick; apply();
    }));
    root.querySelector("[data-coverage-reset]").addEventListener("click", () => {
      search.value = category.value = status.value = "";
      apply(); search.focus();
    });
    browser.addEventListener("popstate", restore);
    root.ownerDocument.addEventListener("keydown", event => {
      const target = event.target;
      if (event.key !== "/" || event.ctrlKey || event.metaKey || event.altKey || event.shiftKey || event.isComposing ||
          target.closest("input, textarea, select, [contenteditable]:not([contenteditable='false'])")) return;
      event.preventDefault(); search.focus();
    });
    root.querySelector("[data-coverage-controls]").hidden = false;
    restore();
  }
  if (typeof module !== "undefined" && module.exports) module.exports = {matches, readFilters, filterURL, setup};
  if (typeof document !== "undefined") document.addEventListener("DOMContentLoaded", () => document.querySelectorAll("[data-coverage]").forEach(setup));
})();
