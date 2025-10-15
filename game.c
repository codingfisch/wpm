#include "./game.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#define BACKGROUND_COLOR 0xFF181818
#define TEXT_COLOR 0xFFFFFFFF
#define GRAY_COLOR 0xFF606060
#define RED_COLOR 0xFF0000FF
#define YELLOW_COLOR 0xFF34AEEB

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
    "the", "be", "of", "and", "a", "to", "in", "he", "have", "it", "that", "for", "they", "with", "as", "not", "on",
    "she", "at", "by", "this", "we", "you", "do", "but", "from", "or", "which", "one", "would", "all", "will", "there",
    "say", "who", "make", "when", "can", "more", "if", "no", "man", "out", "other", "so", "what", "time", "up", "go",
    "about", "than", "into", "could", "state", "only", "new", "year", "some", "take", "come", "these", "know", "see",
    "use", "get", "like", "then", "first", "any", "work", "now", "may", "such", "give", "over", "think", "most", "even",
    "find", "day", "also", "after", "way", "many", "must", "look", "before", "great", "back", "through", "long",
    "where", "much", "should", "well", "people", "down", "own", "just", "because", "good", "each", "those", "feel",
    "seem", "how", "high", "too", "place", "little", "world", "very", "still", "nation", "hand", "old", "life", "tell",
    "write", "become", "here", "show", "house", "both", "between", "need", "mean", "call", "develop", "under", "last",
    "right", "move", "thing", "general", "school", "never", "same", "another", "begin", "while", "number", "part",
    "turn", "real", "leave", "might", "want", "point", "form", "off", "child", "few", "small", "since", "against",
    "ask", "late", "home", "interest", "large", "person", "end", "open", "public", "follow", "during", "present",
    "without", "again", "hold", "govern", "around", "possible", "head", "consider", "word", "program", "problem",
    "however", "lead", "system", "set", "order", "eye", "plan", "run", "keep", "face", "fact", "group", "play", "stand",
    "increase", "early", "course", "change", "help", "line"
};
static const size_t NUM_WORDS = sizeof(WORDS) / sizeof(WORDS[0]);
static const int NUM_TEST_WORDS = 50;

typedef struct {
    u32 width;
    u32 height;
    State state;
    char target[512];
    char input[512];
    size_t input_len;
    f32 time;
    f32 start_time;
    f32 end_time;
    f32 time_taken;
    int wpm;
    f32 accuracy;
} Game;

static Game game = {0};

static size_t get_char_index_after_n_words(size_t start, int n) {
    size_t pos = start;
    if (pos < strlen(game.target) && game.target[pos] == ' ') pos++;
    int words = 0;
    while (pos < strlen(game.target) && words < n) {
        while (pos < strlen(game.target) && game.target[pos] != ' ') pos++;
        words++;
        if (words < n) {
            if (pos < strlen(game.target) && game.target[pos] == ' ') pos++;
        }
    }
    return pos;
}

void game_init(u32 width, u32 height)
{
    game.width = width;
    game.height = height;
    game.target[0] = 0;
    rand_state = (u64)platform_get_time();
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
    game.accuracy = 0;
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
    if (game.input_len == tlen) {
        game.end_time = game.time;
        game.time_taken = game.end_time - game.start_time;
        if (game.time_taken <= 0) game.time_taken = 0.001f;
        game.wpm = (int)((f32)tlen / 5.0f * 60.0f / game.time_taken);
        int correct = 0;
        for (size_t i = 0; i < tlen; i++) {
            if (game.input[i] == game.target[i]) correct++;
        }
        game.accuracy = (f32)correct / (f32)tlen * 100.0f;
        game.state = STATE_WIN;
    }
}

void game_render(void)
{
    platform_fill_rect(0, 0, game.width, game.height, BACKGROUND_COLOR);

    if (game.state == STATE_PLAY) {
        fill_text_aligned(game.width/2, game.height * 0.2, "WPM - Type faster!", 32, TEXT_COLOR, ALIGN_CENTER);

        const u32 size = 32;
        const char *target = game.target;
        size_t tlen = strlen(target);
        const int words_per_line[] = {12, 13, 13, 12};
        size_t line_starts[4];
        size_t line_ends[4];
        size_t current = 0;
        for (int ln = 0; ln < 4; ln++) {
            line_starts[ln] = current;
            current = get_char_index_after_n_words(current, words_per_line[ln]);
            line_ends[ln] = current;
            if (ln < 3) {
                if (current < tlen && target[current] == ' ') current++;
                line_ends[ln] = current;
            }
        }

        f32 start_y = game.height * 0.35f;
        const i32 line_height = 40;
        int blink = ((int)(game.time * 2.0f)) % 2;

        for (int ln = 0; ln < 4; ln++) {
            u32 line_width = 0;
            for (size_t j = line_starts[ln]; j < line_ends[ln]; j++) {
                char text[2] = {target[j], 0};
                line_width += platform_text_width(text, size);
            }

            i32 x = game.width / 2 - line_width / 2;
            i32 y = (i32)(start_y + ln * line_height);
            i32 cur_x = x;

            for (size_t j = line_starts[ln]; j < line_ends[ln]; j++) {
                size_t i = j;
                char text[2] = {target[i], 0};
                u32 color = (i < game.input_len) ? ((game.input[i] == target[i]) ? TEXT_COLOR : RED_COLOR) : GRAY_COLOR;
                platform_fill_text(cur_x, y, text, size, color);
                cur_x += platform_text_width(text, size);
            }

            char cursor_on_this_line = (game.input_len >= line_starts[ln] && game.input_len < line_ends[ln]);
            if (cursor_on_this_line && blink == 0) {
                i32 cursor_cur = x;
                size_t chars_before_cursor = game.input_len - line_starts[ln];
                for (size_t k = 0; k < chars_before_cursor; k++) {
                    size_t jj = line_starts[ln] + k;
                    char cc[2] = {target[jj], 0};
                    cursor_cur += platform_text_width(cc, size);
                }
                platform_fill_text(cursor_cur - 3, y, "|", size, YELLOW_COLOR);
            }
        }
    } else { // STATE_WIN
        char buf[64];
        stbsp_snprintf(buf, sizeof(buf), "WPM: %d", game.wpm);
        fill_text_aligned(game.width/2, game.height * 0.3, buf, 48, TEXT_COLOR, ALIGN_CENTER);
        stbsp_snprintf(buf, sizeof(buf), "Accuracy: %.1f%%", game.accuracy);
        fill_text_aligned(game.width/2, game.height * 0.4, buf, 32, TEXT_COLOR, ALIGN_CENTER);
        stbsp_snprintf(buf, sizeof(buf), "Time taken: %.1f seconds", game.time_taken);
        fill_text_aligned(game.width/2, game.height * 0.5, buf, 32, TEXT_COLOR, ALIGN_CENTER);
        fill_text_aligned(game.width/2, game.height * 0.7, "Press R to play again", 32, TEXT_COLOR, ALIGN_CENTER);
    }
}
