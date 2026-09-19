---
layout: page
title: ライブラリごとのverify状況
---

{% assign verification = site.data.library_verification %}
<h1>verify対象・検証が不足しているライブラリ</h1>
<p>ライブラリ側から、専用verifyドライバの不足と次に確認する操作をまとめています。問題側から探す場合は<a href="{{ '/library-checker.html' | relative_url }}">Library Checker対応表</a>を利用してください。</p>
<p><strong>「未登録」は対応問題が存在しないという意味ではありません。</strong>調査済みの範囲と今後の作業を注記します。専用ドライバありも、全APIの網羅や現在のACを保証しません。<a href="{{ '/benchmarks.html' | relative_url }}">実行結果</a>は別に確認できます。</p>
<p>専用verify未登録: <strong>{{ verification.counts.missing | default: 0 }}</strong> · 主機能の一部に公式verifyなし: <strong>{{ verification.counts.partial | default: 0 }}</strong> · 依存先としてのみ利用: <strong>{{ verification.counts.indirect | default: 0 }}</strong> · 専用ドライバあり: <strong>{{ verification.counts.direct | default: 0 }}</strong></p>
<p>全ヘッダをincludeするだけのA+Bは除外しています。「関連ローカルテスト」はinclude関係で集めた境界・ランダムテスト等で、全操作の網羅を意味しません。明示的に無効化されたverifyは成功扱いにしません。</p>

{% assign statuses = 'missing,partial,indirect,direct' | split: ',' %}
{% for status in statuses %}
{% assign libraries = verification.libraries | where: 'status', status %}
{% if status == 'direct' %}<details><summary>専用verifyドライバがあるライブラリ（{{ libraries.size }}件）の詳細も見る</summary>{% else %}<h2>{% if status == 'missing' %}公式verify未登録{% elsif status == 'partial' %}主機能の一部に公式verifyなし{% else %}依存先としてのみ利用{% endif %}（{{ libraries.size }}件）</h2>{% endif %}
{% if libraries.size == 0 %}<p>この分類のライブラリはありません。</p>{% endif %}
{% for library in libraries %}
<section class="verification-gap" id="{{ library.path | remove: 'blueberry/' | replace: '/', '-' | remove: '.hpp' }}">
  <h3><a href="{{ '/' | append: library.path | append: '.html' | relative_url }}">{{ library.name | escape }}</a></h3>
  {% if library.note.reason %}<p>{{ library.note.reason | escape }}</p>{% elsif status != 'direct' %}<p>専用の公式verifyは未登録です。対応問題とAPIの対応を確認する必要があります。</p>{% endif %}
  {% if library.note.limits %}<p><strong>未検証・注意:</strong> {{ library.note.limits | escape }}</p>{% endif %}
  {% if library.note.next %}<p><strong>次の作業:</strong> {{ library.note.next | escape }}</p>{% endif %}
  {% if library.note.issue %}<p><a href="{{ library.note.issue | escape }}">追跡Issue</a></p>{% endif %}
  <ul>
    {% for driver in library.direct %}<li>専用ドライバ: <a href="{{ '/' | append: driver.path | append: '.html' | relative_url }}">{{ driver.path | remove: 'verify/' | escape }}</a> / <a href="{{ driver.problem | escape }}">問題</a></li>{% endfor %}
    {% for driver in library.indirect %}<li>依存先として利用: <a href="{{ '/' | append: driver.path | append: '.html' | relative_url }}">{{ driver.path | remove: 'verify/' | escape }}</a> / <a href="{{ driver.problem | escape }}">問題</a></li>{% endfor %}
    {% for driver in library.ignored %}<li>無効化中: <a href="{{ site.github.repository_url }}/blob/main/{{ driver.path }}">{{ driver.path | escape }}</a></li>{% endfor %}
    {% for test in library.random_tests %}<li>関連ローカルテスト: <a href="{{ site.github.repository_url }}/blob/main/{{ test }}">{{ test | escape }}</a></li>{% endfor %}
  </ul>
</section>
{% endfor %}
{% if status == 'direct' %}</details>{% endif %}
{% endfor %}
