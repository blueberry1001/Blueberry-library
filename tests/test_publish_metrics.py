"""Current-run measurements and persisted AC provenance publish independently."""
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from scripts import publish_metrics as publication
ACTUAL_PUBLICATION_STATE = publication.publication_state


class PublicationStateTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        (self.root / 'verify').mkdir()
        (self.root / 'verify/example.test.cpp').write_text('// verifier')
        (self.root / '.verification').mkdir()
        self.state = {'schema': 1, 'revision': 'head', 'records': {
            'verify/example.test.cpp': {
                'fingerprint': 'fingerprint', 'verified_revision': 'prior-head',
                'verified_at': '2026-10-14T00:00:00Z', 'run_url': 'https://example.invalid/run/1',
                'measurement': {'solution_seconds': 12.5},
            }}, 'failures': {}}
        self.addCleanup(patch.stopall)
        patch.object(publication.subprocess, 'check_output', return_value='head\n').start()
        patch.object(publication, 'publication_state', return_value=self.state).start()

    def read_asset(self, name):
        return json.loads((self.root / '.verify-helper/markdown/assets' / name).read_text())

    def write_report(self, **changes):
        plan = {'schema_version': 1, 'mode': 'affected', 'base_sha': 'base', 'head_sha': 'head',
                'reason': 'docs only', 'repeats': 1,
                'all_verify': ['verify/example.test.cpp'], 'selected_verify': []}
        report = {'schema': 1, 'revision': 'head', 'succeeded': True, 'selection': plan,
                  'results': [{'path': 'verify/example.test.cpp', 'environment_index': 0, 'status': 'not_selected'}]}
        report.update(changes)
        (self.root / '.verification/current.json').write_text(json.dumps(report))
        return report

    def test_docs_only_publication_preserves_ac_without_copying_old_measurement(self):
        report = self.write_report()
        with patch.object(publication, 'validate_plan_against_repository'):
            publication.publish_metrics(self.root)
        self.assertEqual(self.read_asset('verification-metrics.json'), report)
        self.assertEqual(self.read_asset('verification-state.json'), self.state)
        row = self.read_asset('verification-metrics.json')['results'][0]
        self.assertNotIn('solution_seconds', row)
        self.assertEqual(self.read_asset('verification-state.json')['records']['verify/example.test.cpp']['verified_revision'], 'prior-head')

    def test_no_report_removes_stale_generated_metrics_but_retains_ac(self):
        output = self.root / '.verify-helper/markdown'
        for destination in (output / '_data/verification_metrics.json', output / 'assets/verification-metrics.json'):
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_text('{"revision":"stale"}')
        publication.publish_metrics(self.root)
        self.assertFalse((output / '_data/verification_metrics.json').exists())
        self.assertFalse((output / 'assets/verification-metrics.json').exists())
        self.assertEqual(self.read_asset('verification-state.json'), self.state)

    def test_stale_current_report_is_rejected_not_relabelled(self):
        self.write_report(revision='prior-head')
        with self.assertRaises(ValueError):
            publication.publish_metrics(self.root)
        self.assertFalse((self.root / '.verify-helper/markdown/assets').exists())

    def test_unselected_rows_cannot_borrow_historical_timings(self):
        report = self.write_report()
        report['results'][0]['solution_seconds'] = 12.5
        (self.root / '.verification/current.json').write_text(json.dumps(report))
        with self.assertRaises(ValueError):
            publication.publish_metrics(self.root)

    def test_docs_only_run_rejects_missing_or_corrupt_saved_ac_before_writing(self):
        self.write_report()
        state_file = self.root / '.verification/state.json'
        for value in (None, '{broken json', '{"schema":1,"records":[]}'):
            with self.subTest(state=value):
                if value is None:
                    state_file.unlink(missing_ok=True)
                else:
                    state_file.write_text(value)
                with patch.object(publication, 'publication_state', side_effect=ACTUAL_PUBLICATION_STATE), \
                        patch.object(publication, 'validate_plan_against_repository'), \
                        self.assertRaisesRegex(ValueError, 'every verifier'):
                    publication.publish_metrics(self.root)
                self.assertFalse((self.root / '.verify-helper/markdown/assets').exists())

    def test_fresh_success_requires_its_persisted_evidence_too(self):
        report = self.write_report()
        report['selection']['selected_verify'] = ['verify/example.test.cpp']
        report['results'] = [{'path': 'verify/example.test.cpp', 'status': 'passed', 'environment_index': 0,
                              'repeat_count': 1, 'runs': [[{'elapsed': .2}]],
                              'compile_seconds': .1, 'solution_seconds': .2}]
        (self.root / '.verification/current.json').write_text(json.dumps(report))
        with patch.object(publication, 'publication_state', return_value={'schema': 1, 'records': {}}), \
                patch.object(publication, 'validate_plan_against_repository'), \
                self.assertRaisesRegex(ValueError, 'every verifier'):
            publication.publish_metrics(self.root)

    def test_saved_ac_ui_names_actual_revision_and_no_current_measurement(self):
        template = (Path(__file__).resolve().parents[1] / '.verify-helper/docs/static/_includes/verification_metrics.html').read_text(encoding='utf-8')
        self.assertIn('saved_verification.verified_revision', template)
        self.assertIn('saved_verification.verified_at', template)
        self.assertIn('再計測を意味しません', template)
        self.assertIn('今回の計測値はありません', template)

class SavedAcDocumentationIntegrationTest(unittest.TestCase):
    def test_docs_only_zero_run_keeps_source_valid_ac_but_never_old_timings(self):
        import hashlib
        import subprocess
        from scripts.generate_docs import documentation_marker_statuses
        from scripts.plan_verification import build_cached_plan
        from scripts.verification_state import empty_state, update_state

        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            def git(*args):
                return subprocess.check_output(['git', '-C', str(root), *args], text=True).strip()
            def write(name, text):
                destination = root / name
                destination.parent.mkdir(parents=True, exist_ok=True)
                destination.write_text(text)
            git('init', '-q')
            write('.github/workflows/verify.yml', 'repository: atcoder/ac-library\nref: ' + '1' * 40 + '\n')
            write('requirements-dev.txt', 'online-judge-verify-helper==5.6.0\n')
            write('.verify-helper/config.toml', '# verifier configuration\n')
            write('blueberry/example.hpp', '#pragma once\ninline int value() { return 1; }\n')
            for name in ('one', 'two'):
                write(f'verify/{name}.test.cpp', f'#include "blueberry/example.hpp"\n// {name}\n')
            write('README.md', 'original docs\n')
            git('add', '.')
            git('-c', 'user.name=Test', '-c', 'user.email=test@example.invalid', 'commit', '-qm', 'verified sources')
            verified_revision = git('rev-parse', 'HEAD')
            paths = ['verify/one.test.cpp', 'verify/two.test.cpp']
            rows = []
            for name in paths:
                rows.append({'path': name, 'status': 'passed', 'environment_index': 0,
                             'compiler': 'g++ fixture', 'flags': ['-std=gnu++20'],
                             'verifier_sha256': hashlib.sha256((root / name).read_bytes()).hexdigest(),
                             'dataset_sha256': '2' * 64, 'repeat_count': 1, 'case_count': 1,
                             'compile_seconds': .1, 'solution_seconds': .2, 'worst_case_seconds': .2,
                             'min_solution_seconds': .2, 'max_solution_seconds': .2, 'runs': [[{'elapsed': .2}]]})
            verified_report = {'schema': 1, 'revision': verified_revision, 'generated_at': '2026-10-14T00:00:00Z',
                               'succeeded': True, 'results': rows}
            state = update_state(empty_state(), verified_report, root, 'https://example.invalid/run/1')
            self.assertEqual(set(state['records']), set(paths))
            write('.verification/state.json', json.dumps(state))
            write('.verification/previous-state.json', json.dumps(state))
            write('README.md', 'shorter docs; identical sources\n')
            git('add', 'README.md')
            git('-c', 'user.name=Test', '-c', 'user.email=test@example.invalid', 'commit', '-qm', 'docs only')
            current_revision = git('rev-parse', 'HEAD')
            plan = build_cached_plan(root, verified_revision, current_revision, state)
            self.assertEqual(plan['selected_verify'], [])
            write('.verification/current.json', json.dumps({
                'schema': 1, 'revision': current_revision, 'generated_at': '2026-10-15T00:00:00Z',
                'succeeded': True, 'selection': plan,
                'results': [{'path': name, 'status': 'not_selected', 'environment_index': 0} for name in paths],
            }))
            published = publication.publish_metrics(root)
            class Marker:
                def is_verified(self, path): return False
                def is_failed(self, path): return True
            with documentation_marker_statuses(Marker, set(), root, published['records']):
                self.assertTrue(all(Marker().is_verified(Path(name)) for name in paths))
                self.assertFalse(any(Marker().is_failed(Path(name)) for name in paths))
            for record in published['records'].values():
                self.assertEqual(record['verified_revision'], verified_revision)
                self.assertEqual(record['verified_at'], '2026-10-14T00:00:00Z')
            current = json.loads((root / '.verify-helper/markdown/assets/verification-metrics.json').read_text())
            self.assertTrue(all(row['status'] == 'not_selected' and 'solution_seconds' not in row for row in current['results']))
            # The actual file being documented changed; a matching old Git timestamp cannot authorize AC.
            write('blueberry/example.hpp', '#pragma once\ninline int value() { return 2; }\n')
            self.assertEqual(publication.publication_state(root, current_revision)['records'], {})
