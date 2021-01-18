# 手順
1. csvファイルを作成
 * フォーマットは，sample.csvの形式
2. 実行ファイルはrun，run\_simdを使用
 * 他の正規表現マッチングを利用するときは，内部のアルゴリズムも併記
3. 実行結果をcsvファイルに記入（自動化したい）

# 使用ファイル
## 実行ファイル
* 実行するコマンド

```sh
gtime -f "Memory: %M KB, Time: %E" "それぞれの実行コマンド" > "execute_pattern_text".output 2> "execute_pattern_text".result
```
* run（SIMD処理を行なっていない自前の正規表現マッチング）

```
./run "pattern file" "text file"
```

* run\_simd（SIMD処理を行なっている自前の正規表現マッチング）

```
./run_simd "pattern file" "text file"
```

* python（pythonの正規表現ライブラリ）

```
python3 reg.py "pattern file" "text file"
```

## パターンファイル
ディレクトリはpattern/
* repeat.txt

 バックトラックが不利

* include.txt
 
 \#include <文字列>でヒット
 

## テキストファイル
* dblp.xml.100MB
* howto\_sanitized
* bible.txt

# 注意
* バックグラウンドで実行しているアプリなどに注意する
 * 特に同じデータセットで異なる状況にならないようにする
* SIMDの恩恵があるか確認するためにパターン長が長いものも試す
* パターンは少なくともunionが多いもの，繰り返しが多いもの，ランダムなものを用意する
* backtrackの正規表現マッチングに勝てているのか確認（繰り返しが多いようなbacktrackでは不利なパターンを用意）
