"use strict";

function setSourceBundled(viewer, bundled) {
  viewer.querySelector("#unbundled").hidden = bundled;
  viewer.querySelector("#bundled-source").hidden = !bundled;
  const toggle = viewer.querySelector("[data-source-toggle]");
  toggle.textContent = bundled ? "Unbundle" : "Bundle";
  toggle.setAttribute("aria-pressed", String(bundled));
  viewer.querySelector("[data-source-status]").textContent = "";
}

function initializeSourceViewer(viewer) {
  const toggle = viewer.querySelector("[data-source-toggle]");
  if (!toggle) return;
  viewer.dataset.sourceEnhanced = "true";
  setSourceBundled(viewer, false);
  viewer.querySelector("[data-source-actions]").hidden = false;
  toggle.addEventListener("click", () => {
    setSourceBundled(viewer, !viewer.querySelector("#unbundled").hidden);
  });
  viewer.querySelector("[data-source-copy]").addEventListener("click", () => {
    const panel = viewer.querySelector("#unbundled").hidden ? "#bundled-source" : "#unbundled";
    copyCode(viewer.querySelector(`${panel} pre > code`), viewer.querySelector("[data-source-status]"));
  });
}

function openLinkedOperation(hash = window.location.hash) {
  if (!hash) return;
  let id;
  try {
    id = decodeURIComponent(hash.slice(1));
  } catch {
    return;
  }
  let target = document.getElementById(id);
  if (["bundled", "bundled-source", "unbundled"].includes(id)) {
    const viewer = target?.closest("[data-source-viewer]");
    if (viewer?.querySelector("[data-source-toggle]")) setSourceBundled(viewer, id !== "unbundled");
  }
  // A recipe's public anchor stays on its article for existing links.
  const linkedDetails = target?.querySelector?.("details[data-linked-details]");
  if (linkedDetails) linkedDetails.open = true;
  while (target) {
    if (target instanceof HTMLDetailsElement) target.open = true;
    target = target.parentElement;
  }
}

function filterCatalog(catalog) {
  const normalize = (text) => text.normalize("NFKC").toLowerCase();
  const words = normalize(catalog.querySelector("[data-catalog-search]").value).trim().split(/\s+/).filter(Boolean);
  const category = catalog.querySelector("[data-catalog-category]").value;
  const rows = catalog.querySelectorAll("[data-library]");
  let visible = 0;
  rows.forEach((row) => {
    const text = normalize(row.textContent + " " + (row.dataset.keywords || ""));
    row.hidden = (category !== "" && row.dataset.category !== category) || !words.every(word => text.includes(word));
    if (!row.hidden) ++visible;
  });
  catalog.querySelector("[data-catalog-count]").textContent = `${visible} / ${rows.length} 件`;
  catalog.querySelector("[data-catalog-empty]").hidden = visible !== 0;
  const reset = catalog.querySelector("[data-catalog-reset]");
  if (reset) reset.hidden = words.length === 0 && category === "";
  const choices = catalog.querySelectorAll("[data-catalog-choice]");
  choices.forEach((choice) => {
    if (choice.dataset.catalogChoice === category) choice.setAttribute("aria-current", "true");
    else choice.removeAttribute("aria-current");
  });
  const guide = catalog.querySelector("[data-catalog-category-guide]");
  if (guide) {
    const selected = Array.from(choices).find(choice => choice.dataset.catalogChoice === category);
    guide.hidden = !category || !selected;
    if (!guide.hidden) catalog.querySelector("[data-catalog-guide-link]").setAttribute("href", selected.getAttribute("href"));
  }
}

function initializeCatalog(catalog) {
  const search = catalog.querySelector("[data-catalog-search]");
  const category = catalog.querySelector("[data-catalog-category]");
  catalog.querySelector("[data-catalog-controls]").hidden = false;
  const choices = catalog.querySelectorAll("[data-catalog-choice]");
  if (choices.length) category.parentElement.hidden = true;
  choices.forEach(choice => choice.addEventListener("click", (event) => {
    // Leave modified clicks available for opening the category guide in another tab.
    if (event.ctrlKey || event.metaKey || event.shiftKey || event.altKey) return;
    event.preventDefault();
    category.value = choice.dataset.catalogChoice;
    filterCatalog(catalog);
  }));
  search.addEventListener("input", () => filterCatalog(catalog));
  category.addEventListener("change", () => filterCatalog(catalog));
  catalog.querySelector("[data-catalog-reset]").addEventListener("click", () => {
    search.value = "";
    category.value = "";
    filterCatalog(catalog);
    search.focus();
  });
  filterCatalog(catalog);
}

async function copyCode(code, status) {
  try {
    await navigator.clipboard.writeText(code.textContent);
    status.textContent = "コピーしました";
  } catch {
    const range = document.createRange();
    range.selectNodeContents(code);
    const selection = window.getSelection();
    selection.removeAllRanges();
    selection.addRange(range);
    status.textContent = "コードを選択しました。Ctrl+C / ⌘C でコピーしてください";
  }
}

window.addEventListener("hashchange", () => openLinkedOperation());
document.addEventListener("DOMContentLoaded", () => {
  document.querySelectorAll("[data-source-viewer]").forEach(initializeSourceViewer);
  openLinkedOperation();
  const copy = document.getElementById("copy-include");
  if (copy) copy.addEventListener("click", () => copyCode(
    document.getElementById("include-directive-code"), document.getElementById("copy-include-status")
  ));
  document.querySelectorAll('a[href^="#"]').forEach((link) => {
    link.addEventListener("click", () => openLinkedOperation(link.getAttribute("href")));
  });
  document.querySelectorAll("pre > code").forEach((code) => {
    if (code.closest("[data-source-viewer]")?.querySelector("[data-source-toggle]")) return;
    const controls = document.createElement("div");
    controls.className = "code-actions";
    const button = document.createElement("button");
    button.type = "button";
    button.textContent = "コードをコピー";
    const status = document.createElement("span");
    status.setAttribute("role", "status");
    button.addEventListener("click", () => copyCode(code, status));
    controls.append(button, status);
    code.parentElement.before(controls);
  });
  document.querySelectorAll("[data-catalog]").forEach(initializeCatalog);
  document.addEventListener("keydown", (event) => {
    if (event.key !== "/" || event.ctrlKey || event.altKey || event.metaKey || event.isComposing) return;
    if (event.target.closest("input, textarea, select, [contenteditable]:not([contenteditable='false'])")) return;
    const search = document.querySelector("[data-catalog-search]");
    if (search) {
      event.preventDefault();
      search.focus();
    }
  });
});
