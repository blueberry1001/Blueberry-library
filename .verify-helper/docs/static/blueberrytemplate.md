---
layout: page
title: blueberrytemplate.h（原文）
---

# blueberrytemplate.h（原文）

WSLの個人用テンプレートを、マクロ名・本文・整形・コメントを変えずに掲載しています。
下のコードをコピーして、末尾に `void _main()` の実装を追加してください。

<p><a href="{{ '/assets/code/blueberrytemplate.h' | relative_url }}" download="blueberrytemplate.h">blueberrytemplate.hをダウンロード</a></p>

<pre><code class="language-cpp" id="original-template-code">{{ site.data.original_template.code | escape }}</code></pre>

<details markdown="1">
<summary>最小使用例・原本の情報</summary>

`blueberrytemplate.h` と同じディレクトリに、次の提出コードを置きます。
テンプレート内に `main()` があるため、利用側では `_main()` を定義します。

{% raw %}
```cpp
#include "blueberrytemplate.h"
void _main() {
    LL(a, b);
    cout << a + b << Endl;
}
```
{% endraw %}

保存元: `/home/blueberry/lib/blueberrytemplate.h`。
`{{ site.data.original_template.bytes }}` バイト、SHA-256:
`{{ site.data.original_template.sha256 }}`。
原本に含まれる出典コメントもそのまま残しています。

前回作成した [Contest Template（整理版）]({{ '/blueberry/utility/contest-template.hpp.html' | relative_url }}) は、
一部の名前や使い方が異なる別のファイルです。

</details>
