'use strict';

let app = document.getElementById("app");
let ctx = app.getContext("2d");
let wasm = null;

function cstrlen(mem, ptr) {
    let len = 0;
    while (mem[ptr] != 0) {
        len++;
        ptr++;
    }
    return len;
}

function cstr_by_ptr(mem_buffer, ptr) {
    const mem = new Uint8Array(mem_buffer);
    const len = cstrlen(mem, ptr);
    const bytes = new Uint8Array(mem_buffer, ptr, len);
    return new TextDecoder().decode(bytes);
}

function color_hex(color) {
    // WebAssembly uses RGBA, but canvas expects #RRGGBBAA
    const r = ((color >> 0) & 0xFF).toString(16).padStart(2, '0');
    const g = ((color >> 8) & 0xFF).toString(16).padStart(2, '0');
    const b = ((color >> 16) & 0xFF).toString(16).padStart(2, '0');
    const a = ((color >> 24) & 0xFF).toString(16).padStart(2, '0');
    return "#" + r + g + b + a;
}

function platform_fill_rect(x, y, w, h, color) {
    ctx.fillStyle = color_hex(color);
    ctx.fillRect(x, y, w, h);
}

function platform_text_width(text_ptr, size) {
    const buffer = wasm.instance.exports.memory.buffer;
    const text = cstr_by_ptr(buffer, text_ptr);
    ctx.font = size + "px AnekLatin";
    return ctx.measureText(text).width;
}

function platform_fill_text(x, y, text_ptr, size, color) {
    const buffer = wasm.instance.exports.memory.buffer;
    const text = cstr_by_ptr(buffer, text_ptr);
    ctx.fillStyle = color_hex(color);
    ctx.font = size + "px AnekLatin";
    ctx.fillText(text, x, y);
}

function platform_panic(file_path_ptr, line, message_ptr) {
    const buffer = wasm.instance.exports.memory.buffer;
    const file_path = cstr_by_ptr(buffer, file_path_ptr);
    const message = cstr_by_ptr(buffer, message_ptr);
    console.error(file_path + ":" + line + ": " + message);
}

function platform_log(message_ptr) {
    const buffer = wasm.instance.exports.memory.buffer;
    const message = cstr_by_ptr(buffer, message_ptr);
    console.log(message);
}

function platform_get_time() {
    return performance.now();
}

let prev = null;
function loop(timestamp) {
    if (prev !== null) {
        wasm.instance.exports.game_update((timestamp - prev) * 0.001);
        wasm.instance.exports.game_render();
    }
    prev = timestamp;
    window.requestAnimationFrame(loop);
}

WebAssembly.instantiateStreaming(fetch('game.wasm'), {
    env: {
        platform_fill_rect,
        platform_fill_text,
        platform_panic,
        platform_log,
        platform_text_width,
        platform_get_time,
    }
}).then((w) => {
    wasm = w;
    wasm.instance.exports.game_init(app.width, app.height);

    document.addEventListener('keydown', (e) => {
        if (e.key.length === 1 || e.key === 'Backspace') {
            wasm.instance.exports.game_keydown(e.key.charCodeAt(0));
        }
    });

    window.requestAnimationFrame(loop);
});
