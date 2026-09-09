"use strict";

function openLinkedOperation() {
  if (!window.location.hash) return;
  let id;
  try {
    id = decodeURIComponent(window.location.hash.slice(1));
  } catch {
    return;
  }
  const target = document.getElementById(id);
  if (target instanceof HTMLDetailsElement) target.open = true;
}

window.addEventListener("hashchange", openLinkedOperation);
document.addEventListener("DOMContentLoaded", () => {
  openLinkedOperation();
  const copy = document.getElementById("copy-include");
  if (copy) copy.addEventListener("click", async () => {
    const code = document.getElementById("include-directive-code");
    const status = document.getElementById("copy-include-status");
    try {
      await navigator.clipboard.writeText(code.textContent);
      status.textContent = " コピーしました";
    } catch {
      status.textContent = " コードを選択してコピーしてください";
    }
  });
  document.querySelectorAll('a[href^="#"]').forEach((link) => {
    link.addEventListener("click", () => {
      const target = document.getElementById(link.getAttribute("href").slice(1));
      if (target instanceof HTMLDetailsElement) target.open = true;
    });
  });
});
