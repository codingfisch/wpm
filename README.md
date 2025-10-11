# WPM - Type faster!

How many words per minute (WPM) can you type? 

Try it at: https://codingfisch.github.io/wpm/

<img width="1587" height="731" alt="image" src="https://github.com/user-attachments/assets/05925c12-b4d1-4139-bec7-db6349c9d51a" />


## Build

```bash
clang -Os -fno-builtin -Wall -Wextra -Wswitch-enum --target=wasm32 --no-standard-libraries -Wl,--export=game_init -Wl,--export=game_render -Wl,--export=game_update -Wl,--export=game_keydown -Wl,--no-entry -Wl,--allow-undefined  -o wpm.wasm wpm.c
```

## Run locally via WASM

```bash
python3 -m http.server 6969
```
Then visit the following URL in a browser: [http://localhost:6969/](http://localhost:6969/)
## Font

[Anek Latin Light](https://github.com/EkType/Anek)
