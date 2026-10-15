---
layout: page
title: Library Checker 対応表
---

<link rel="stylesheet" href="{{ '/assets/css/coverage.css' | relative_url }}">
<script defer src="{{ '/assets/js/coverage.js' | relative_url }}"></script>

{% assign coverage = site.data.library_checker %}
<h1>Library Checker 対応表</h1>
<p class="coverage-note">「verifyあり」は検証コードがある問題です。ACの結果は<a href="{{ '/benchmarks.html' | relative_url }}">検証結果一覧</a>で確認できます。</p>

<div data-coverage>
  <fieldset class="coverage-controls" data-coverage-controls hidden>
    <legend>問題を絞り込む</legend>
    <div class="coverage-fields">
      <label>問題名・ライブラリ名<input data-coverage-search type="search" name="query" placeholder="例: convolution、Treap"></label>
      <label>分類<select data-coverage-category><option value="">すべて</option>{% for category in coverage.categories %}<option value="{{ category | escape }}">{{ category | escape }}</option>{% endfor %}</select></label>
      <label>対応状況<select data-coverage-status><option value="">すべて</option>{% for status in coverage.statuses %}<option value="{{ status.id | escape }}">{{ status.label | escape }}</option>{% endfor %}</select></label>
      <button data-coverage-reset type="button">条件をクリア</button>
    </div>
  </fieldset>
  <nav class="coverage-totals" aria-label="対応状況で絞り込む">
    <a data-coverage-quick="" href="?#coverage-problems">すべて <span>{{ coverage.total }}</span></a>
    {% assign quick_statuses = 'verify,acl,missing' | split: ',' %}
    {% for id in quick_statuses %}{% assign first_problem = coverage.problems | where: 'status', id | first %}{% for item in coverage.statuses %}{% if item.id == id %}<a data-coverage-quick="{{ id }}" href="?status={{ id }}#{% if first_problem %}problem-{{ first_problem.id | escape }}{% else %}coverage-problems{% endif %}">{{ item.label | escape }} <span>{{ coverage.counts[id] | default: 0 }}</span></a>{% endif %}{% endfor %}{% endfor %}
  </nav>
  <p role="status" aria-live="polite" data-coverage-count>すべて · {{ coverage.total }} / {{ coverage.total }} 問題</p>
  <p data-coverage-empty hidden>一致する問題がありません。条件を減らすか、クリアしてください。</p>
  <noscript><p>JavaScript無効時は全問題を表示します。ブラウザー内検索も利用できます。</p></noscript>
  <table class="coverage-table" id="coverage-problems">
    <thead><tr><th scope="col">問題・分類</th><th scope="col">対応状況</th><th scope="col">実装・検証コード</th></tr></thead>
    <tbody>
    {% for problem in coverage.problems %}
      <tr id="problem-{{ problem.id | escape }}" data-coverage-row data-category="{{ problem.category | escape }}" data-status="{{ problem.status | escape }}">
        <td><a href="{{ problem.url | escape }}">{{ problem.title | escape }}</a><br><code>{{ problem.id | escape }}</code><br><span class="coverage-category">{{ problem.category | escape }}</span></td>
        <td>{% if problem.status == 'verify' or problem.status == 'implemented' %}<span aria-label="ローカル実装あり">✓ </span>{% endif %}{{ problem.label | escape }}</td>
        <td>
          {% for header in problem.headers %}<a class="coverage-link" href="{{ '/' | append: header.path | append: '.html' | relative_url }}">{{ header.name | escape }}</a>{% endfor %}
          {% for verify in problem.verifies %}<a class="coverage-link" href="{{ '/' | append: verify | append: '.html' | relative_url }}">verify: {{ verify | remove: 'verify/' | escape }}</a>{% endfor %}
          {% if problem.external_url != '' %}<a class="coverage-link" href="{{ problem.external_url | escape }}">ACL公式ドキュメント ↗</a>{% endif %}
          {% if problem.note != '' %}<p>{{ problem.note | escape }}</p>{% endif %}
          {% if problem.status == 'missing' %}<span>追加候補</span>{% endif %}
          {% if problem.ignored_verifies.size > 0 %}<p>無効化されたverify {{ problem.ignored_verifies.size }}件は対応数に含めていません。</p>{% endif %}
        </td>
      </tr>
    {% endfor %}
    </tbody>
  </table>
</div>

<details class="category-guide"><summary>対応状況・データの出典</summary>
<p>「verifyあり」はACを保証しません。実行結果と測定条件は各verifyのリンク先と<a href="{{ '/benchmarks.html' | relative_url }}">検証結果一覧</a>で確認してください。専用verifyなしの項目は、対応するAPIと使い方を個別に確認したものです。ACLは外部ライブラリとして区別しています。</p>
<p>取得日時: <time>{{ coverage.fetched_at | escape }}</time> · <a href="{{ coverage.api_url | escape }}">公式公開問題一覧</a> · <a href="{{ coverage.upstream_url | escape }}/blob/{{ coverage.upstream_revision | escape }}/categories.toml">カテゴリの固定リビジョン</a></p>
<p>取得日付きのスナップショットを使い、閲覧時にはネットワーク取得しません。</p>
</details>

<details class="category-guide"><summary>開発時の更新方法</summary>
<p>ライブラリやverifyを追加した後は <code>python3 scripts/library_checker_coverage.py</code> で対応表とMarkdownチェックリストを再生成します。<code>make test</code> が更新漏れを検出します。</p>
<p>公開問題を更新する場合は、最新の公式問題リポジトリを用意し、<code>python3 scripts/library_checker_coverage.py --refresh --upstream /path/to/library-checker-problems</code> を実行します。取得できなかった場合に問題数をゼロに置き換えることはありません。</p>
<p><a href="https://github.com/blueberry1001/Blueberry-library/blob/main/docs/library-checker-checklist.md">GitHubでチェックリストを読む</a> · <a href="https://github.com/blueberry1001/Blueberry-library/blob/main/data/library-checker-mappings.json">個別に確認したAPI・ACL対応</a></p>
</details>
