"""Contracts for operation recommendations and the real browser filter."""
import json
from pathlib import Path
import re
import shutil
import subprocess
import unittest

import yaml

ROOT = Path(__file__).resolve().parents[1]
STATIC = ROOT / ".verify-helper/docs/static"


class OperationFinderTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.data = yaml.safe_load((STATIC / "_data/operations.yml").read_text(encoding="utf-8"))
        cls.entries = cls.data["entries"]

    def test_metadata_references_real_documented_apis(self):
        self.assertEqual(self.data["version"], 1)
        ids = [entry["id"] for entry in self.entries]
        self.assertEqual(len(ids), len(set(ids)))
        catalog = yaml.safe_load((STATIC / "_data/libraries.yml").read_text(encoding="utf-8"))
        headers = {row["path"] for row in catalog}
        for facet, values in self.data["facets"].items():
            identifiers = [item["id"] for item in values]
            self.assertEqual(len(identifiers), len(set(identifiers)), facet)
        for entry in self.entries:
            with self.subTest(entry=entry["id"]):
                self.assertRegex(entry["id"], r"^[a-z0-9-]+$")
                self.assertIn(entry["source"], ("blueberry", "acl", "combination"))
                self.assertTrue(entry["title"])
                self.assertTrue(entry["prerequisites"])
                self.assertTrue(entry["calls"])
                for call in entry["calls"]:
                    self.assertTrue(call["signature"])
                    self.assertIn("O(", call["complexity"])
                for facet in ("targets", "queries", "updates", "conditions"):
                    allowed = {item["id"] for item in self.data["facets"][facet]}
                    self.assertTrue(entry[facet])
                    self.assertEqual(len(entry[facet]), len(set(entry[facet])))
                    self.assertLessEqual(set(entry[facet]), allowed)
                if entry["source"] == "acl":
                    self.assertEqual(entry["headers"], [])
                    self.assertTrue(entry["url"].startswith("https://atcoder.github.io/ac-library/"))
                else:
                    self.assertTrue(entry["headers"])
                    for header in entry["headers"]:
                        self.assertIn(header, headers)
                        self.assertTrue((ROOT / header).is_file())
                        doc = ROOT / ("docs/" + header.removeprefix("blueberry/").removesuffix(".hpp") + ".md")
                        self.assertIn("documentation_of: //" + header, doc.read_text(encoding="utf-8"))
                    self.assertEqual(entry["url"], "/" + entry["headers"][0] + ".html")
                if entry["source"] == "combination":
                    self.assertTrue(entry["external_url"].startswith("https://atcoder.github.io/ac-library/"))
                else:
                    self.assertNotIn("external_url", entry)

    def run_node(self, script, payload=None):
        node = shutil.which("node")
        if node is None:
            self.skipTest("Node.js is needed for the production JavaScript filter tests")
        result = subprocess.run(
            [node, "-e", script], cwd=ROOT, input=json.dumps(payload),
            text=True, capture_output=True, check=True)
        return json.loads(result.stdout)

    def test_query_update_and_condition_are_all_required(self):
        states = [
            {},
            {"queries": "line-min", "updates": "line-add", "conditions": "unregistered"},
            {"queries": "line-min", "updates": "line-add", "conditions": "known-queries"},
            {"queries": "rectangle-sum", "updates": "point-add", "conditions": "known-updates"},
            {"queries": "range-kth", "updates": "range-action"},
            {"queries": "range-sum", "updates": "point-set", "conditions": "history"},
            {"search": "ＡＣＬ　ＦＥＮＷＩＣＫ"},
        ]
        script = """
const fs = require('node:fs');
const { matches } = require('./.verify-helper/docs/static/assets/js/operations.js');
const input = JSON.parse(fs.readFileSync(0, 'utf8'));
console.log(JSON.stringify(input.states.map(state =>
  input.entries.filter(entry => matches(entry, state)).map(entry => entry.id))));
"""
        result = self.run_node(script, {"entries": self.entries, "states": states})
        self.assertEqual(len(result[0]), len(self.entries))
        self.assertEqual(result[1], ["dynamic-li-chao"])
        self.assertEqual(result[2], ["compressed-li-chao"])
        self.assertEqual(result[3], ["offline-fenwick-2d", "weighted-wavelet-matrix"])
        self.assertEqual(result[4], [])
        self.assertIn("persistent-segment-tree", result[5])
        self.assertEqual(result[6], ["acl-fenwick"])

    def test_unknown_filter_never_silently_shows_unrelated_results(self):
        script = """
const { matches } = require('./.verify-helper/docs/static/assets/js/operations.js');
const entry = {queries:['a'], updates:['b'], conditions:['c'], title:'test', calls:[]};
console.log(JSON.stringify([
  matches(entry, {queries:'missing'}),
  matches(entry, {queries:'a', updates:'wrong'}),
  matches(entry, {queries:'a', updates:'b', conditions:'c'}),
  matches(entry, {})
]));
"""
        self.assertEqual(self.run_node(script), [False, False, True, True])

    def test_workload_capabilities_and_target_separation(self):
        states = [
            {"targets": "array", "queries": "range-sum", "updates": "static"},
            {"targets": "array", "queries": "range-kth"},
            {"targets": "multiset", "queries": "range-kth"},
            {"targets": "grid", "queries": "rectangle-sum", "updates": "static"},
            {"targets": "array", "queries": "aggregate", "updates": "point-set"},
            {"queries": "boundary-search", "updates": "point-set"},
            {"queries": "range-sum", "updates": "range-affine"},
            {"queries": "range-mode", "conditions": "online-queries"},
            {"queries": "range-sum", "conditions": "offline-queries"},
            {"targets": "tree", "queries": "path-aggregate", "updates": "static"},
            {"targets": "array", "queries": "range-sum", "updates": "point-add"},
        ]
        result = self.run_node("""
const fs = require('node:fs');
const { matches } = require('./.verify-helper/docs/static/assets/js/operations.js');
const {entries, states} = JSON.parse(fs.readFileSync(0, 'utf8'));
console.log(JSON.stringify(states.map(s => entries.filter(e => matches(e,s)).map(e=>e.id))));
""", {"entries": self.entries, "states": states})
        self.assertTrue({"acl-fenwick", "acl-segtree", "disjoint-sparse-table", "sqrt-tree", "dynamic-sqrt-tree"} <= set(result[0]))
        self.assertNotIn("ordered-multiset", result[0])
        self.assertIn("wavelet-matrix", result[1])
        self.assertNotIn("binary-trie", result[1])
        self.assertIn("binary-trie", result[2])
        self.assertNotIn("wavelet-matrix", result[2])
        self.assertTrue({"offline-fenwick-2d", "dynamic-fenwick-2d", "weighted-wavelet-matrix"} <= set(result[3]))
        self.assertNotIn("disjoint-sparse-table", result[4])
        self.assertNotIn("sqrt-tree", result[4])
        self.assertIn("dynamic-sqrt-tree", result[4])
        self.assertTrue({"acl-boundary-search", "acl-lazy-boundary-search"} <= set(result[5]))
        self.assertIn("acl-lazy-sum", result[6])
        self.assertNotIn("segment-tree-beats", result[6])
        self.assertEqual(result[7], [])
        self.assertIn("acl-segtree", result[8])
        self.assertIn("hld-point-update", result[9])
        self.assertIn("dynamic-sqrt-tree", result[10])
        self.assertNotIn("sqrt-tree", result[10])

    def test_specialized_structures_match_only_supported_operations(self):
        states = [
            {"queries": "range-sum", "updates": "range-clamp"},
            {"queries": "xor-min", "updates": "insert-erase", "conditions": "duplicates"},
            {"queries": "whole-aggregate", "updates": "queue-edit", "conditions": "noncommutative"},
            {"queries": "whole-aggregate", "updates": "deque-edit"},
            {"queries": "aggregate", "updates": "deque-edit"},
            {"queries": "range-kth", "updates": "range-clamp"},
        ]
        script = """
const fs = require('node:fs');
const { matches } = require('./.verify-helper/docs/static/assets/js/operations.js');
const input = JSON.parse(fs.readFileSync(0, 'utf8'));
console.log(JSON.stringify(input.states.map(state =>
  input.entries.filter(entry => matches(entry, state)).map(entry => entry.id))));
"""
        result = self.run_node(script, {"entries": self.entries, "states": states})
        self.assertEqual(result[0], ["segment-tree-beats"])
        self.assertIn("binary-trie", result[1])
        self.assertEqual(result[2], ["aggregate-queue", "aggregate-deque"])
        self.assertEqual(result[3], ["aggregate-deque"])
        self.assertEqual(result[4], [])  # Whole-fold APIs do not provide range prod(l,r).
        self.assertEqual(result[5], [])

    def test_generic_monoid_query_does_not_recommend_idempotent_only_table(self):
        script = """
const fs = require('node:fs');
const { matches } = require('./.verify-helper/docs/static/assets/js/operations.js');
const entries = JSON.parse(fs.readFileSync(0, 'utf8'));
console.log(JSON.stringify(entries.filter(entry => matches(entry, {queries:'aggregate'})).map(entry=>entry.id)));
"""
        result = self.run_node(script, self.entries)
        self.assertNotIn("sparse-table", result)
        self.assertIn("acl-segtree", result)

    def test_reset_restores_results_and_focus_without_reload(self):
        script = """
const { setupFinder } = require('./.verify-helper/docs/static/assets/js/operations.js');
function control(value='') {
  return {value, options:[{value:''}], replaceChildren(...options){this.options=options;},
    handlers:{}, addEventListener(type, f){this.handlers[type]=f;},
    focus(){this.focused=true;}};
}
const controls = {targets:control(), queries:control(), updates:control(), conditions:control(), search:control()};
const reset = control(), panel={hidden:true}, status={textContent:''}, empty={hidden:true};
const cards = [
 {dataset:{queries:'sum', updates:'add', conditions:'online'}, textContent:'A', hidden:false},
 {dataset:{queries:'min', updates:'static', conditions:'static'}, textContent:'B', hidden:false}
];
const order=[];
const container={appendChild(card){const i=order.indexOf(card); if(i>=0)order.splice(i,1); order.push(card);}};
const nodes = {'.operation-results':container, '[data-operation-controls]':panel, '[data-operation-reset]':reset,
 '[data-operation-count]':status, '[data-operation-empty]':empty};
for (const name of Object.keys(controls)) nodes['[data-operation-filter="'+name+'"]'] = controls[name];
const root={querySelector:s=>nodes[s],querySelectorAll:()=>cards};
setupFinder(root);
controls.updates.value='static';
controls.updates.handlers.change();
const prioritized=order[0]===cards[1] && cards.every(card=>!card.hidden);
controls.queries.value='missing';
controls.queries.handlers.change();
const unmatched=empty.hidden===false && cards.every(card=>card.hidden);
reset.handlers.click();
console.log(JSON.stringify({unmatched, prioritized, originalOrder:order[0]===cards[0], visible:cards.filter(card=>!card.hidden).length,
 reset:controls.queries.value==='' && controls.updates.value==='' && controls.conditions.value==='',
 focus:controls.targets.focused, emptyHidden:empty.hidden, enhanced:!panel.hidden}));
"""
        self.assertEqual(self.run_node(script), {
            "unmatched": True, "prioritized": True, "originalOrder": True, "visible": 2, "reset": True, "focus": True,
            "emptyHidden": True, "enhanced": True})

    def test_target_changes_remove_irrelevant_options_and_clear_stale_choices(self):
        script = """
const fs = require('node:fs');
const {setupFinder} = require('./.verify-helper/docs/static/assets/js/operations.js');
const data = JSON.parse(fs.readFileSync(0,'utf8'));
function control(options=[]) {
  return {value:'', options:options.map(value=>({value})), handlers:{},
    replaceChildren(...options){this.options=options;this.value=options[0]?.value||'';},
    addEventListener(type,fn){this.handlers[type]=fn;}, focus(){}};
}
const controls={search:control()};
for(const [key,options] of Object.entries(data.facets)) controls[key]=control(['',...options.map(o=>o.id)]);
const cards=data.entries.map(e=>({dataset:{targets:e.targets.join(' '),queries:e.queries.join(' '),
  updates:e.updates.join(' '),conditions:e.conditions.join(' ')},textContent:e.title}));
const status={textContent:''},reset=control();
const nodes={'.operation-results':{appendChild(){}},'[data-operation-count]':status,
  '[data-operation-reset]':reset,'[data-operation-controls]':{},'[data-operation-empty]':{}};
for(const [key,c] of Object.entries(controls)) nodes['[data-operation-filter="'+key+'"]']=c;
setupFinder({querySelector:s=>nodes[s],querySelectorAll:()=>cards});
const values=key=>controls[key].options.map(o=>o.value);
const chooseTarget=value=>{controls.targets.value=value;controls.targets.handlers.change();};
chooseTarget('tree');
const tree=values('queries');
controls.queries.value='kth-ancestor';controls.updates.value='static';controls.conditions.value='static-tree';
chooseTarget('array');
const array={queries:values('queries'),updates:values('updates'),conditions:values('conditions'),
  selected:[controls.queries.value,controls.updates.value,controls.conditions.value],notice:status.textContent};
chooseTarget('grid');
const grid={queries:values('queries'),updates:values('updates'),conditions:values('conditions')};
chooseTarget('multiset');
const multiset={queries:values('queries'),updates:values('updates')};
reset.handlers.click();
console.log(JSON.stringify({tree,array,grid,multiset,restored:values('queries').length===data.facets.queries.length+1}));
"""
        result = self.run_node(script, self.data)
        self.assertIn("kth-ancestor", result["tree"])
        self.assertNotIn("kth-ancestor", result["array"]["queries"])
        self.assertNotIn("merge", result["array"]["updates"])
        self.assertNotIn("static-tree", result["array"]["conditions"])
        self.assertEqual(result["array"]["selected"], ["", "static", ""])
        self.assertIn("対象外の条件を解除", result["array"]["notice"])
        self.assertIn("rectangle-sum", result["grid"]["queries"])
        self.assertNotIn("ancestor", result["grid"]["queries"])
        self.assertIn("static", result["grid"]["updates"])
        self.assertIn("offline-queries", result["grid"]["conditions"])
        self.assertIn("xor-min", result["multiset"]["queries"])
        self.assertIn("insert-erase", result["multiset"]["updates"])
        self.assertTrue(result["restored"])

    def test_progressive_enhancement_and_accessibility_markup(self):
        page = (STATIC / "operations.md").read_text(encoding="utf-8")
        self.assertIn("layout: page", page)
        self.assertIn("data-operation-controls hidden", page)
        self.assertIn('aria-live="polite"', page)
        self.assertIn("<noscript>", page)
        self.assertIn("site.data.operations.entries", page)
        self.assertIn("entry.prerequisites", page)
        self.assertIn("call.complexity", page)
        self.assertNotRegex(page, r"data-operation-entry[^>]*\bhidden\b")
        for facet in ("targets", "queries", "updates", "conditions", "search"):
            self.assertIn(f'for="operation-{facet}"', page)
            self.assertIn(f'id="operation-{facet}"', page)


if __name__ == "__main__":
    unittest.main()
