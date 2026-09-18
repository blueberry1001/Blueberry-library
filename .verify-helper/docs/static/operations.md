---
layout: page
title: 操作から探す
---

<link rel="stylesheet" href="{{ '/assets/css/operations.css' | relative_url }}">
<script src="{{ '/assets/js/operations.js' | relative_url }}" defer></script>

<div class="operation-finder" data-operation-finder>
  <div class="operation-intro">
    <p class="operation-eyebrow">操作から探す</p>
    <h1>どのデータに、何をしたい？</h1>
    <p>まず配列・木・二次元などの対象を選び、知りたい値と更新方法で絞り込みます。候補の前提・構築コスト・操作の計算量を比べて選んでください。</p>
    <p class="operation-caption">データ構造と木の操作を中心に掲載しています。<a href="{{ '/' | relative_url }}#library-catalog">全ライブラリ一覧</a>も利用できます。</p>
  </div>

  <fieldset class="operation-controls" data-operation-controls hidden>
    <legend>目的に合う候補を絞り込む</legend>
    <div class="operation-fields">
      <div class="operation-target">
        <label for="operation-targets">1. 対象のデータ</label>
        <select id="operation-targets" data-operation-filter="targets" aria-describedby="operation-filter-help">
          <option value="">すべての対象</option>
          {% for option in site.data.operations.facets.targets %}
          <option value="{{ option.id | escape }}">{{ option.label | escape }}</option>
          {% endfor %}
        </select>
      </div>
      <div>
        <label for="operation-queries">2. 知りたい値・探索</label>
        <select id="operation-queries" data-operation-filter="queries" aria-describedby="operation-filter-help">
          <option value="">すべて</option>
          {% for option in site.data.operations.facets.queries %}
          <option value="{{ option.id | escape }}">{{ option.label | escape }}</option>
          {% endfor %}
        </select>
      </div>
      <div>
        <label for="operation-updates">3. 必要な更新</label>
        <select id="operation-updates" data-operation-filter="updates" aria-describedby="operation-filter-help">
          <option value="">すべて</option>
          {% for option in site.data.operations.facets.updates %}
          <option value="{{ option.id | escape }}">{{ option.label | escape }}</option>
          {% endfor %}
        </select>
      </div>
      <div>
        <label for="operation-conditions">入力条件・必要な性質</label>
        <select id="operation-conditions" data-operation-filter="conditions" aria-describedby="operation-filter-help">
          <option value="">指定しない</option>
          {% for option in site.data.operations.facets.conditions %}
          <option value="{{ option.id | escape }}">{{ option.label | escape }}</option>
          {% endfor %}
        </select>
      </div>
      <div class="operation-search">
        <label for="operation-search">名前・関数・前提で検索</label>
        <input id="operation-search" data-operation-filter="search" type="search" placeholder="例: prod、長方形、ACL" autocomplete="off">
      </div>
      <button class="operation-reset" data-operation-reset type="button">条件をすべて解除</button>
    </div>
    <p id="operation-filter-help" class="operation-caption">選んだ項目をすべて満たす候補を表示します。検索語も AND 条件です。「値を更新しない」では静的専用を先に、動的構造も候補に表示します。速さの順位ではないため、構築・メモリ・クエリのコストを比較してください。「全クエリを先に集められる」には逐次処理できる構造も含みます。</p>
  </fieldset>

  <noscript><p class="operation-notice">JavaScriptが無効のため全候補を表示しています。各候補の文書リンクから使い方を確認できます。</p></noscript>

  <div class="operation-results-heading">
    <h2>候補</h2>
    <p role="status" aria-live="polite" aria-atomic="true" data-operation-count>{{ site.data.operations.entries.size }} / {{ site.data.operations.entries.size }} 候補</p>
  </div>
  <p class="operation-caption">ACL は外部ライブラリです。組み合わせ例は、それぞれの構造を解答側で接続します。計算量は前提を満たし、利用する演算が O(1) の場合です。</p>

  <div class="operation-empty" data-operation-empty hidden>
    <h3>この組み合わせの掲載候補はありません</h3>
    <p>条件を1つずつ外すか、「条件をすべて解除」を使ってください。未掲載の解法もあるため、解けないことを意味する表示ではありません。</p>
  </div>

  <div class="operation-results">
    {% for entry in site.data.operations.entries %}
    <article class="operation-card" data-operation-entry data-targets="{{ entry.targets | join: ' ' | escape }}" data-queries="{{ entry.queries | join: ' ' | escape }}" data-updates="{{ entry.updates | join: ' ' | escape }}" data-conditions="{{ entry.conditions | join: ' ' | escape }}" data-keywords="{{ entry.headers | join: ' ' | escape }}">
      <div class="operation-card-heading">
        <span class="operation-source operation-source--{{ entry.source | escape }}">{% if entry.source == 'acl' %}ACL · 外部{% elsif entry.source == 'combination' %}Blueberry + ACL{% else %}Blueberry{% endif %}</span>
        <span class="operation-caption">{% if entry.updates contains 'static' %}静的専用{% else %}更新対応 · 静的にも使用可{% endif %}</span>
        <h3><a href="{% if entry.source == 'acl' %}{{ entry.url | escape }}{% else %}{{ entry.url | relative_url }}{% endif %}">{{ entry.title | escape }}</a></h3>
      </div>
      <dl class="operation-summary">
        <div><dt>対象</dt><dd>{% for option in site.data.operations.facets.targets %}{% if entry.targets contains option.id %}<span>{{ option.label | escape }}</span>{% endif %}{% endfor %}</dd></div>
        <div><dt>知りたい値</dt><dd>{% for option in site.data.operations.facets.queries %}{% if entry.queries contains option.id %}<span>{{ option.label | escape }}</span>{% endif %}{% endfor %}</dd></div>
        <div><dt>更新方法</dt><dd>{% for option in site.data.operations.facets.updates %}{% if entry.updates contains option.id %}<span>{{ option.label | escape }}</span>{% endif %}{% endfor %}</dd></div>
        <div><dt>条件・性質</dt><dd>{% for option in site.data.operations.facets.conditions %}{% if entry.conditions contains option.id %}<span>{{ option.label | escape }}</span>{% endif %}{% endfor %}</dd></div>
      </dl>
      <ul class="operation-calls">
        {% for call in entry.calls %}
        <li><code>{{ call.signature | escape }}</code><span>{{ call.complexity | escape }}</span></li>
        {% endfor %}
      </ul>
      <p class="operation-prerequisites"><strong>前提</strong> {{ entry.prerequisites | escape }}</p>
      {% if entry.notes %}<p class="operation-notes">{{ entry.notes | escape }}</p>{% endif %}
      {% if entry.external_url %}<p class="operation-external"><a href="{{ entry.external_url | escape }}">組み合わせる ACL の公式文書（外部）</a></p>{% endif %}
    </article>
    {% endfor %}
  </div>
</div>
