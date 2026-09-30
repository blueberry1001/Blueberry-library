// Dependency-free behavior checks for the documentation's progressive enhancement.
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const vm = require("node:vm");

class Element {
  constructor(text = "", parent = null) {
    this.textContent = text;
    this.parentElement = parent;
    this.dataset = {};
    this.hidden = false;
    this.attributes = {};
    this.listeners = {};
  }
  setAttribute(name, value) { this.attributes[name] = value; }
  addEventListener(name, listener) { this.listeners[name] = listener; }
  closest() { return this.viewer || null; }
}
class Details extends Element { open = false; }
const outer = new Details();
const inner = new Details("", outer);
const target = new Element("", inner);
const nodes = new Map([["nested example", target], ["operation", inner]]);
let copied, selected;
const context = vm.createContext({
  document: {
    getElementById: (id) => nodes.get(id),
    addEventListener() {},
    createRange: () => ({ selectNodeContents(node) { selected = node; } }),
  },
  window: {
    location: { hash: "#nested%20example" },
    addEventListener() {},
    getSelection: () => ({ removeAllRanges() {}, addRange() {} }),
  },
  navigator: { clipboard: { async writeText(text) { copied = text; } } },
  HTMLDetailsElement: Details,
});
vm.runInContext(fs.readFileSync(path.join(__dirname, "../.verify-helper/docs/static/assets/js/docs.js"), "utf8"), context);
context.openLinkedOperation();
assert.equal(inner.open, true, "deep links reveal the nearest closed operation");
assert.equal(outer.open, true, "deep links reveal all closed ancestor details");
context.window.location.hash = "#%broken";
assert.doesNotThrow(() => context.openLinkedOperation());

const sourceNodes = Object.fromEntries([
  "#unbundled", "#bundled-source", "[data-source-toggle]", "[data-source-actions]",
  "[data-source-copy]", "[data-source-status]", "#unbundled pre > code", "#bundled-source pre > code",
].map(selector => [selector, new Element(selector)]));
const viewer = { dataset: {}, querySelector: selector => sourceNodes[selector] };
for (const node of Object.values(sourceNodes)) node.viewer = viewer;
nodes.set("bundled", sourceNodes["#bundled-source"]);
nodes.set("bundled-source", sourceNodes["#bundled-source"]);
nodes.set("unbundled", sourceNodes["#unbundled"]);
context.initializeSourceViewer(viewer);
const toggle = sourceNodes["[data-source-toggle]"];
assert.equal(toggle.textContent, "Bundle");
assert.equal(sourceNodes["#bundled-source"].hidden, true);
toggle.listeners.click();
assert.equal(toggle.textContent, "Unbundle");
assert.equal(toggle.attributes["aria-pressed"], "true");
assert.equal(sourceNodes["#unbundled"].hidden, true);
assert.equal(sourceNodes["#bundled-source"].hidden, false);
sourceNodes["[data-source-copy]"].listeners.click();
assert.equal(copied, "#bundled-source pre > code", "copy uses the visible bundled source");
toggle.listeners.click();
sourceNodes["[data-source-copy]"].listeners.click();
assert.equal(copied, "#unbundled pre > code", "copy returns to the original source");
for (const hash of ["#bundled", "#bundled-source"]) {
  context.openLinkedOperation(hash);
  assert.equal(sourceNodes["#bundled-source"].hidden, false);
}
context.openLinkedOperation("#unbundled");
assert.equal(sourceNodes["#unbundled"].hidden, false);
assert.equal(toggle.textContent, "Bundle");
assert.equal(toggle.attributes["aria-pressed"], "false");
assert.equal(sourceNodes["[data-source-status]"].textContent, "");
assert.doesNotThrow(() => context.initializeSourceViewer({ querySelector: () => null }));

const rows = [
  ["Wavelet Matrix 区間 頻度", "data-structure", "range kth smallest"],
  ["Lowest Common Ancestor 共通祖先", "graph", "lca 木 距離"],
  ["Formal Power Series 冪級数", "math", "fps"],
].map(([text, category, keywords]) => {
  const row = new Element(text);
  row.dataset = { category, keywords };
  return row;
});
const count = new Element();
const empty = new Element();
const search = { value: "  ＬＣＡ  木 " };
const category = { value: "" };
const catalog = {
  querySelectorAll: () => rows,
  querySelector: (selector) => ({
    "[data-catalog-search]": search,
    "[data-catalog-category]": category,
    "[data-catalog-count]": count,
    "[data-catalog-empty]": empty,
  })[selector],
};
context.filterCatalog(catalog);
assert.deepEqual(rows.map(row => row.hidden), [true, false, true], "all words match normalized aliases");
assert.equal(count.textContent, "1 / 3 件");
category.value = "math";
context.filterCatalog(catalog);
assert.equal(empty.hidden, false, "a category/query intersection can be empty");
search.value = "";
context.filterCatalog(catalog);
assert.deepEqual(rows.map(row => row.hidden), [true, true, false]);
category.value = "";
context.filterCatalog(catalog);
assert.deepEqual(rows.map(row => row.hidden), [false, false, false]);
assert.equal(empty.hidden, true);
search.value = "<script>";
context.filterCatalog(catalog);
assert.equal(empty.hidden, false, "queries are plain text, not HTML or regex");

// The monoid finder deliberately shares catalog behavior; test the published data.
const recipes = JSON.parse(fs.readFileSync(path.join(__dirname, "../.verify-helper/docs/static/_data/monoids.yml"), "utf8"));
const recipeRows = recipes.map(recipe => {
  const row = new Element(Object.values(recipe).join(" "));
  row.dataset = { category: recipe.kind, keywords: recipe.keywords };
  return row;
});
const recipeCatalog = { ...catalog, querySelectorAll: () => recipeRows };
for (const [query, kind, expected] of [
  ["等差数列", "lazy", ["ap"]],
  ["区間等差数列加算", "lazy", ["ap"]],
  ["最大部分和", "segtree", ["maxsub"]],
  ["ＦＬＩＰ 転倒数", "lazy", ["inv"]],
  ["DP", "segtree", ["matrix"]],
  ["第二最大", "segtree", ["top2"]],
  ["等差数列", "beats", []],
]) {
  search.value = query;
  category.value = kind;
  context.filterCatalog(recipeCatalog);
  assert.deepEqual(recipes.filter((_, i) => !recipeRows[i].hidden).map(recipe => recipe.id), expected);
}
search.value = category.value = "";
context.filterCatalog(recipeCatalog);
assert.equal(recipeRows.every(row => !row.hidden), true, "clearing filters restores every recipe");
assert.equal(new Set(recipes.map(recipe => recipe.id)).size, recipes.length, "recipe anchors are unique");

(async () => {
  const code = new Element("int main() {\n  assert(1 < 2);\n}\n");
  const status = new Element();
  await context.copyCode(code, status);
  assert.equal(copied, code.textContent, "copy preserves whitespace and C++ operators");
  assert.match(status.textContent, /コピーしました/);
  context.navigator.clipboard.writeText = async () => { throw Error("denied"); };
  await context.copyCode(code, status);
  assert.equal(selected, code, "clipboard denial selects the exact source for manual copy");
  assert.match(status.textContent, /選択/);
  console.log("PASS: catalog filtering, source toggle/copy/anchors, nested anchors, clipboard success and fallback");
})().catch(error => { console.error(error); process.exitCode = 1; });
