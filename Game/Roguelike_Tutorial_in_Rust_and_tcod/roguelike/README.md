[Roguelike Tutorial in Rust + tcod](https://tomassedovic.github.io/roguelike-tutorial/)

---
### 備考

tcod::input::check_for_event関数を使うとpanicがでる。
`.cargo\registry\src\github.com-1ecc6299db9ec823\tcod-0.15.0\src\input.rs　`
を[これ](https://github.com/tomassedovic/tcod-rs/commit/d27df89a213c33871b6f704678be9ca6cd40a50e)
を参考にして書き換える。その後、cargo cleanして再コンパイルすれば動く。