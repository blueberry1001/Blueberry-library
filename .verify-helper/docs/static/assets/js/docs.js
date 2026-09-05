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
  document.querySelectorAll('a[href^="#"]').forEach((link) => {
    link.addEventListener("click", () => {
      const target = document.getElementById(link.getAttribute("href").slice(1));
      if (target instanceof HTMLDetailsElement) target.open = true;
    });
  });
});
