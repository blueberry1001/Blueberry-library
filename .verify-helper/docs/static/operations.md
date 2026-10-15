---
layout: page
title: 操作から探す
---

<link rel="stylesheet" href="{{ '/assets/css/operations.css' | relative_url }}">
<script src="{{ '/assets/js/operations.js' | relative_url }}" defer></script>

<div class="operation-finder" data-operation-finder>
  <div class="operation-intro">
    <h1>操作から探す</h1>
  </div>

  <fieldset class="operation-controls" data-operation-controls hidden>
    <legend class="visually-hidden">候補の条件</legend>
    <div class="operation-fields">
      <div class="operation-target">
        <label for="operation-targets">1. 対象のデータ</label>
        <select id="operation-targets" data-operation-filter="targets">
          <option value="">すべての対象</option>
          {% for option in site.data.operations.facets.targets %}
          <option value="{{ option.id | escape }}">{{ option.label | escape }}</option>
          {% endfor %}
        </select>
      </div>
      <div>
        <label for="operation-queries">2. 知りたい値・探索</label>
        <select id="operation-queries" data-operation-filter="queries">
          <option value="">すべて</option>
          {% for option in site.data.operations.facets.queries %}
          <option value="{{ option.id | escape }}">{{ option.label | escape }}</option>
          {% endfor %}
        </select>
      </div>
      <div>
        <label for="operation-updates">3. 必要な更新</label>
        <select id="operation-updates" data-operation-filter="updates">
          <option value="">すべて</option>
          {% for option in site.data.operations.facets.updates %}
          <option value="{{ option.id | escape }}">{{ option.label | escape }}</option>
          {% endfor %}
        </select>
      </div>
      <div>
        <label for="operation-conditions">入力条件・必要な性質</label>
        <select id="operation-conditions" data-operation-filter="conditions">
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
      <button class="operation-reset" data-operation-reset type="button">条件をクリア</button>
    </div>
  </fieldset>

  <noscript><p class="operation-notice">JavaScriptが無効のため全候補を表示しています。各候補の文書リンクから使い方を確認できます。</p></noscript>


  <div class="operation-results-heading">
    <h2>候補</h2>
    <p role="status" aria-live="polite" aria-atomic="true" data-operation-count>{{ site.data.operations.entries.size }} / {{ site.data.operations.entries.size }} 候補</p>
  </div>

  <div class="operation-empty" data-operation-empty hidden>
    <h3>この組み合わせの掲載候補はありません</h3>
    <p>条件を減らすか、「条件をクリア」で探し直してください。未掲載の解法もあります。</p>
  </div>

  <div class="operation-results">
    {% for entry in site.data.operations.entries %}
    <article class="operation-card" data-operation-entry data-targets="{{ entry.targets | join: ' ' | escape }}" data-queries="{{ entry.queries | join: ' ' | escape }}" data-updates="{{ entry.updates | join: ' ' | escape }}" data-conditions="{{ entry.conditions | join: ' ' | escape }}" data-keywords="{{ entry.headers | join: ' ' | escape }}">
      <div class="operation-card-heading">
        <span class="operation-source operation-source--{{ entry.source | escape }}">{% if entry.source == 'acl' %}ACL · 外部{% elsif entry.source == 'combination' %}Blueberry + ACL{% else %}Blueberry{% endif %}</span>
        <span class="operation-caption">{% if entry.updates contains 'static' %}静的専用{% else %}更新対応 · 静的にも使用可{% endif %}</span>
        <h3><a href="{% if entry.source == 'acl' %}{{ entry.url | escape }}{% else %}{{ entry.url | relative_url }}{% endif %}">{{ entry.title | escape }}</a></h3>
      </div>
      <details class="operation-detail">
      <summary>操作・計算量・条件</summary>
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
      {% if entry.notes %}<p class="operation-notes">{{ entry.notes | escape }}</p>{% endif %}
      </details>
      <p class="operation-prerequisites"><strong>前提</strong> {{ entry.prerequisites | escape }}</p>
      {% for family in site.data.comparisons %}
      {% assign hint = family.hints[entry.id] %}
      {% if hint %}<p class="operation-choice"><strong>使い分け</strong> {{ hint | escape }} <a href="{{ '/comparisons.html#comparison-' | append: family.id | relative_url }}">比較を開く<span class="visually-hidden">：{{ family.title | escape }}</span></a></p>{% endif %}
      {% endfor %}
      {% if entry.external_url %}<p class="operation-external"><a href="{{ entry.external_url | escape }}">組み合わせる ACL の公式文書（外部）</a></p>{% endif %}
    </article>
    {% endfor %}
  </div>
  <p class="operation-caption">計算量は記載の前提と O(1) の演算を仮定します。</p>
</div>
