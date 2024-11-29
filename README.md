# リアクティブシステムの仕様の欠陥範囲の特定をするツール

## 必要ライブラリ

- [Spot](https://spot.lre.epita.fr/install.html)  
/thisProject/include/include/にspotをインストールする  

- [GOAL](https://goal.im.ntu.edu.tw/wiki/doku.php)  
実行環境のpcのローカルにアプリをインストールする  

- [Boost](https://www.boost.org/)  
/thisProject/include/include/にboostをインストールする  

- [BuDDy](https://buddy.sourceforge.net/manual/files.html)  
Spotのインストールと同時にインストールされるが、インストールできなかったら手動でインストールする  
/thisProject/include/include/にbddx.h, bvecx.h, fddx.hを置く  

## セットアップ

- .envに本プロジェクトのsrcフォルダのパスとGOALの実行ファイルgcまでのパスを書く

- config.cmakeにプロジェクトのincludeディレクトリまでのパスを書く

## 実行
buildフォルダに移動して、  
`cmake ..`  
`make`  
`./ffs`

