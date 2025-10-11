# WPM - Type faster!

How many words per minute (WPM) can you type? Try it at: https://codingfisch.github.io/wpm/

## Build

```bash
clang -Os -fno-builtin -Wall -Wextra -Wswitch-enum --target=wasm32 --no-standard-libraries -Wl,--export=game_init -Wl,--export=game_render -Wl,--export=game_update -Wl,--export=game_keydown -Wl,--no-entry -Wl,--allow-undefined  -o wpm.wasm wpm.c
```

## Run locally via WASM

```bash
python3 -m http.server 6969
```
Then 
## Font

[Anek Latin Light](https://github.com/EkType/Anek)
