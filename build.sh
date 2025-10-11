#!/bin/sh

set -xe

clang -Os -fno-builtin -Wall -Wextra -Wswitch-enum --target=wasm32 --no-standard-libraries -Wl,--export=game_init -Wl,--export=game_render -Wl,--export=game_update -Wl,--export=game_keydown -Wl,--no-entry -Wl,--allow-undefined  -o game.wasm game.c
