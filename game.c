#include "./game.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#define BACKGROUND_COLOR 0xFF181818
#define TEXT_COLOR 0xFFFFFFFF
#define GRAY_COLOR 0xFFC0C0C0
#define RED_COLOR 0xFF0000FF

typedef enum {
    STATE_PLAY = 0,
    STATE_WIN,
} State;

static char logf_buf[4096] = {0};
#define LOGF(...) \
    do { \
        stbsp_snprintf(logf_buf, sizeof(logf_buf), __VA_ARGS__); \
        platform_log(logf_buf); \
    } while(0)

static u64 rand_state = 0;
static u32 rand(void)
{
    rand_state = rand_state * 6364136223846793005ULL + 1442695040888963407ULL;
    return (rand_state >> 32) & 0xFFFFFFFF;
}

static size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len]) ++len;
    return len;
}

static void strcpy(char *dst, const char *src) {
    size_t i = 0;
    while (src[i]) {
        dst[i] = src[i];
        ++i;
    }
    dst[i] = 0;
}

static int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static void strcat_c(char *dst, const char *src) {
    size_t dlen = strlen(dst);
    strcpy(dst + dlen, src);
}

typedef enum {
    ALIGN_LEFT,
    ALIGN_RIGHT,
    ALIGN_CENTER,
} Align;

static void fill_text_aligned(i32 x, i32 y, const char *text, u32 size, u32 color, Align align)
{
    u32 width = platform_text_width(text, size);
    switch (align) {
    case ALIGN_LEFT:                 break;
    case ALIGN_CENTER: x -= width/2; break;
    case ALIGN_RIGHT:  x -= width;   break;
    }
    platform_fill_text(x, y, text, size, color);
}

static const char *WORDS[] = {
    "the", "of", "and", "a", "to", "in", "is", "you", "that", "it"
};
static const size_t NUM_WORDS = sizeof(WORDS) / sizeof(WORDS[0]);
static const int NUM_TEST_WORDS = 20;

typedef struct {
    u32 width;
    u32 height;
    State state;
    char target[256];
    char input[256];
    size_t input_len;
    f32 time;
    f32 start_time;
    f32 end_time;
    f32 time_taken;
    int wpm;
} Game;

static Game game = {0};

void game_init(u32 width, u32 height)
{
    game.width = width;
    game.height = height;
    game.target[0] = 0;
    for (int i = 0; i < NUM_TEST_WORDS; i++) {
        size_t idx = rand() % NUM_WORDS;
        strcat_c(game.target, WORDS[idx]);
        if (i < NUM_TEST_WORDS - 1) strcat_c(game.target, " ");
    }
    game.input_len = 0;
    game.input[0] = 0;
    game.state = STATE_PLAY;
    game.time = 0;
    game.start_time = -1;
    game.end_time = 0;
    game.time_taken = 0;
    game.wpm = 0;
    LOGF("Game initialized");
}

void game_resize(u32 width, u32 height)
{
    game.width = width;
    game.height = height;
}

void game_update(f32 dt)
{
    if (game.state == STATE_PLAY) {
        game.time += dt;
    }
}

void game_keydown(int key)
{
    int ch = key;

    if (game.state == STATE_WIN) {
        if (ch == 'r') game_init(game.width, game.height);
        return;
    }

    // STATE_PLAY
    if (game.start_time < 0 && key != 66) game.start_time = game.time;

    if (key == 66 && game.input_len > 0) {
        game.input_len--;
        game.input[game.input_len] = 0;
    } else if (((ch >= 'a' && ch <= 'z') || ch == ' ') && game.input_len < sizeof(game.input) - 1) {
        game.input[game.input_len] = ch;
        game.input_len++;
        game.input[game.input_len] = 0;
    }

    size_t tlen = strlen(game.target);
    if (game.input_len == tlen && strcmp(game.input, game.target) == 0) {
        game.end_time = game.time;
        game.time_taken = game.end_time - game.start_time;
        if (game.time_taken <= 0) game.time_taken = 0.001f;
        game.wpm = (int)((f32)tlen / 5.0f * 60.0f / game.time_taken);
        game.state = STATE_WIN;
    }
}

void game_render(void)
{
    platform_fill_rect(0, 0, game.width, game.height, BACKGROUND_COLOR);

    if (game.state == STATE_PLAY) {
        fill_text_aligned(game.width/2, game.height * 0.2, "Typing Speed Test", 32, TEXT_COLOR, ALIGN_CENTER);

        const u32 size = 32;
        const char *target = game.target;
        size_t tlen = strlen(target);
        u32 total_width = platform_text_width(target, size);
        i32 x = game.width / 2 - total_width / 2;
        i32 y = game.height / 2;
        for (size_t i = 0; i < tlen; i++) {
            u32 color;
            color = i < game.input_len ? ((game.input[i] == target[i]) ? TEXT_COLOR : RED_COLOR) : GRAY_COLOR;
            char text[2] = {target[i], 0};
            platform_fill_text(x, y, text, size, color);
            x += platform_text_width(text, size);
        }
    } else { // STATE_WIN
        char buf[64];
        stbsp_snprintf(buf, sizeof(buf), "WPM: %d", game.wpm);
        fill_text_aligned(game.width/2, game.height * 0.3, buf, 48, TEXT_COLOR, ALIGN_CENTER);
        stbsp_snprintf(buf, sizeof(buf), "Time taken: %.1f seconds", game.time_taken);
        fill_text_aligned(game.width/2, game.height * 0.45, buf, 32, TEXT_COLOR, ALIGN_CENTER);
        fill_text_aligned(game.width/2, game.height * 0.7, "Press R to play again", 32, TEXT_COLOR, ALIGN_CENTER);
    }
}
