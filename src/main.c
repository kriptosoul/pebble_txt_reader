\
#include <pebble.h>
#include <ctype.h>

#define PKEY_PAGE 1
#define PKEY_FONT 2
#define PKEY_THEME 3

#define DEFAULT_FONT_IDX 1  // 0=small,1=medium,2=large

static Window *s_window;
static TextLayer *s_text_layer;
static char *s_book = NULL;
static size_t s_book_len = 0;

static int s_font_idx = DEFAULT_FONT_IDX;
static bool s_theme_dark = false;
static int s_current_page = 0;
static int s_chars_per_page = 600; // will be recalculated

static int s_total_pages = 0;

static GFont font_small;
static GFont font_medium;
static GFont font_large;
static GFont s_font;

static void load_book() {
  ResHandle rh = resource_get_handle(RESOURCE_ID_DATA_BOOK);
  if (!rh) return;
  s_book_len = (size_t)resource_size(rh);
  s_book = malloc(s_book_len + 1);
  if (s_book) {
    resource_load(rh, s_book, s_book_len);
    s_book[s_book_len] = '\0';
  }
}

static void free_book() {
  if (s_book) free(s_book);
  s_book = NULL;
  s_book_len = 0;
}

static void save_state() {
  persist_write_int(PKEY_PAGE, s_current_page);
  persist_write_int(PKEY_FONT, s_font_idx);
  persist_write_bool(PKEY_THEME, s_theme_dark);
}

static void load_state() {
  if (persist_exists(PKEY_PAGE)) s_current_page = persist_read_int(PKEY_PAGE);
  if (persist_exists(PKEY_FONT)) s_font_idx = persist_read_int(PKEY_FONT);
  if (persist_exists(PKEY_THEME)) s_theme_dark = persist_read_bool(PKEY_THEME);
}

static void select_font_by_index(int idx) {
  s_font_idx = idx;
  switch(s_font_idx) {
    case 0: s_font = font_small; s_chars_per_page = 400; break;
    case 1: s_font = font_medium; s_chars_per_page = 600; break;
    case 2: s_font = font_large; s_chars_per_page = 900; break;
    default: s_font = font_medium; s_chars_per_page = 600; break;
  }
  text_layer_set_font(s_text_layer, s_font);
  // recompute total pages
  if (s_book_len > 0) {
    s_total_pages = (int)ceil((double)s_book_len / (double)s_chars_per_page);
    if (s_current_page >= s_total_pages) s_current_page = s_total_pages - 1;
    if (s_current_page < 0) s_current_page = 0;
  }
}

static void update_page_text() {
  if (!s_book) {
    text_layer_set_text(s_text_layer, "No book found in resources.");
    return;
  }
  int start = s_current_page * s_chars_per_page;
  if (start < 0) start = 0;
  if (start > (int)s_book_len) start = (int)s_book_len;
  int len = s_chars_per_page;
  if (start + len > (int)s_book_len) len = (int)s_book_len - start;
  // create a temporary buffer trimmed to word boundary
  int end = start + len;
  while (end < (int)s_book_len && !isspace((unsigned char)s_book[end])) end++;
  int chunk_len = end - start;
  char *buf = malloc(chunk_len + 1);
  if (!buf) return;
  memcpy(buf, s_book + start, chunk_len);
  buf[chunk_len] = '\0';
  text_layer_set_text(s_text_layer, buf);
  free(buf);
}

static void apply_theme() {
  if (s_theme_dark) {
    window_set_background_color(s_window, GColorBlack);
    text_layer_set_text_color(s_text_layer, GColorWhite);
  } else {
    window_set_background_color(s_window, GColorWhite);
    text_layer_set_text_color(s_text_layer, GColorBlack);
  }
}

static void go_prev_page() {
  if (s_current_page > 0) {
    s_current_page--;
    update_page_text();
  }
}

static void go_next_page() {
  if (s_current_page < s_total_pages - 1) {
    s_current_page++;
    update_page_text();
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  go_prev_page();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  go_next_page();
}

// short select toggles theme; long select cycles font size
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  s_theme_dark = !s_theme_dark;
  apply_theme();
  save_state();
}
static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  s_font_idx = (s_font_idx + 1) % 3;
  select_font_by_index(s_font_idx);
  update_page_text();
  save_state();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 700, select_long_click_handler, NULL);
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);

  // fonts - use system fonts (no custom font selection)
  font_small = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  font_medium = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  font_large = fonts_get_system_font(FONT_KEY_GOTHIC_24);

  s_text_layer = text_layer_create(GRect(4, 0, bounds.size.w - 8, bounds.size.h));
  text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentLeft);
  layer_add_child(root, text_layer_get_layer(s_text_layer));

  load_state();
  load_book();
  select_font_by_index(s_font_idx);
  if (s_book_len > 0) {
    s_total_pages = (int)ceil((double)s_book_len / (double)s_chars_per_page);
    if (s_current_page >= s_total_pages) s_current_page = s_total_pages - 1;
  } else {
    s_total_pages = 1;
    s_current_page = 0;
  }
  update_page_text();
  apply_theme();
}

static void window_unload(Window *window) {
  save_state();
  text_layer_destroy(s_text_layer);
  free_book();
}

static void init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload
  });
  window_set_click_config_provider(s_window, click_config_provider);
  window_stack_push(s_window, true);
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
  return 0;
}
