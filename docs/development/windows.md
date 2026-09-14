# Windows (WSL2 Ubuntu) でのC++20開発

このリポジトリのMakefileはPOSIX shellを使用します。WindowsではWSL2 Ubuntu上で
GCC/Clangを実行します。WindowsネイティブのMSVC向け設定ではありません。

2026-09-14に既存のUbuntuへClangとpython3-venvを追加し、GCC 13.3.0、Clang 18.1.3、
Python 3.12.3で準備しました。Python依存はUbuntu側の
`~/.venvs/blueberry-library/`、ACLはリポジトリ内の`.deps/ac-library/`に置きます。
Windows共有フォルダ上のvenvは大量の小ファイル書き込みが遅いため、Linux側に配置します。

PowerShellからUbuntuへ入ります。

```powershell
wsl -d Ubuntu
```

初回のみUbuntu側で開発ツールと依存を準備します。

```bash
sudo apt-get update
sudo apt-get install -y build-essential clang python3-venv git
cd /mnt/c/Users/kubar/Documents/GitHub/Blueberry-library
python3 -m venv ~/.venvs/blueberry-library
source ~/.venvs/blueberry-library/bin/activate
make setup
```

以降、Ubuntuの新しいshellを開いたら仮想環境を有効にして検証します。

```bash
cd /mnt/c/Users/kubar/Documents/GitHub/Blueberry-library
source ~/.venvs/blueberry-library/bin/activate
make check
make compile-test random-test CXX=clang++
make verify
make docs
```

`make check`には`make test`（Pythonテストとドキュメント例）が含まれます。
公式テストの初回取得と`make setup`にはネットワーク接続が必要です。
`make verify`は全公式ケースを既定3回ずつ計測するため、compile/random testより時間がかかります。

初回のケース生成を含む実行が既定の25分上限に達する場合は、既存CLIで上限を延長できます。
未実行の結果を成功として扱わず、ログ内の全verifyの状態を確認してください。

```bash
CPLUS_INCLUDE_PATH="$PWD/.deps/ac-library" python3 scripts/verify_with_metrics.py --timeout 3600
```

再開時に対象を限定する場合は、同CLIに未実行の`verify/...test.cpp`を列挙できます。
`--output .verification/remaining`で既存の実測ログと分けて保存してください。

単独のC++プログラムは、リポジトリをinclude pathへ追加して実行できます。

```bash
mkdir -p .build
g++ -std=gnu++20 -O2 -Wall -Wextra -I. -I.deps/ac-library main.cpp -o .build/main
.build/main
```

この仮想環境はLinux用なので、WindowsのPythonから再利用しないでください。
別の場所へcloneした場合は上記の`cd`を置き換えてください。
