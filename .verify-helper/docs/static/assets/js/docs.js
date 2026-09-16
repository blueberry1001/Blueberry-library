"use strict";

function openLinkedOperation(hash = window.location.hash) {
  if (!hash) return;
  let id;
  try {
    id = decodeURIComponent(hash.slice(1));
  } catch {
    return;
  }
  let target = document.getElementById(id);
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
  openLinkedOperation();
  const copy = document.getElementById("copy-include");
  if (copy) copy.addEventListener("click", () => copyCode(
    document.getElementById("include-directive-code"), document.getElementById("copy-include-status")
  ));
  document.querySelectorAll('a[href^="#"]').forEach((link) => {
    link.addEventListener("click", () => openLinkedOperation(link.getAttribute("href")));
  });
  document.querySelectorAll("pre > code").forEach((code) => {
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
  document.querySelectorAll("[data-catalog]").forEach((catalog) => {
    const search = catalog.querySelector("[data-catalog-search]");
    const category = catalog.querySelector("[data-catalog-category]");
    catalog.querySelector("[data-catalog-controls]").hidden = false;
    search.addEventListener("input", () => filterCatalog(catalog));
    category.addEventListener("change", () => filterCatalog(catalog));
    catalog.querySelector("[data-catalog-reset]").addEventListener("click", () => {
      search.value = "";
      category.value = "";
      filterCatalog(catalog);
      search.focus();
    });
    filterCatalog(catalog);
  });
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
