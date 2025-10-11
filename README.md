# Typing Test

[![thumbnail](thumbnail.png)](http://tsoding.org/snake-c-wasm/)


## Build

```console
clang -Os -fno-builtin -Wall -Wextra -Wswitch-enum --target=wasm32 --no-standard-libraries -Wl,--export=game_init -Wl,--export=game_render -Wl,--export=game_update -Wl,--export=game_keydown -Wl,--no-entry -Wl,--allow-undefined  -o wpm.wasm wpm.c
```

## Running WASM version

```console
python3 -m http.server 6969
```

## Font

[Anek Latin Light](https://github.com/EkType/Anek)
