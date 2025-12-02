
#include <pebble.h>

#define STORAGE_OFFSET_KEY 1
#define STORAGE_FONT_KEY 2
#define STORAGE_THEME_KEY 3
#define PAGE_SIZE 500

static Window *s_main;
static TextLayer *s_text;
static char *book;
static int offset = 0;
static int font_size = 24;
static bool dark_mode = false;

static void save_progress() { persist_write_int(STORAGE_OFFSET_KEY, offset); }
static void save_font() { persist_write_int(STORAGE_FONT_KEY, font_size); }
static void save_theme() { persist_write_bool(STORAGE_THEME_KEY, dark_mode); }

static void load_saved() {
  if (persist_exists(STORAGE_OFFSET_KEY)) offset = persist_read_int(STORAGE_OFFSET_KEY);
  if (persist_exists(STORAGE_FONT_KEY)) font_size = persist_read_int(STORAGE_FONT_KEY);
  if (persist_exists(STORAGE_THEME_KEY)) dark_mode = persist_read_bool(STORAGE_THEME_KEY);
}

static GFont get_font() {
  if(font_size <= 18) return fonts_get_system_font(FONT_KEY_GOTHIC_18);
  if(font_size <= 24) return fonts_get_system_font(FONT_KEY_GOTHIC_24);
  return fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
}

static void apply_theme() {
  window_set_background_color(s_main, dark_mode ? GColorBlack : GColorWhite);
  text_layer_set_text_color(s_text, dark_mode ? GColorWhite : GColorBlack);
}

static void load_book() {
  ResHandle rh = resource_get_handle(RESOURCE_ID_BOOK);
  size_t size = resource_size(rh);
  book = malloc(size + 1);
  resource_load(rh, book, size);
  book[size] = 0;
}

static void update_page() {
  static char buf[PAGE_SIZE + 1];
  snprintf(buf, sizeof(buf), "%.*s", PAGE_SIZE, book + offset);
  text_layer_set_text(s_text, buf);
}

static void next_page(ClickRecognizerRef ref, void *ctx) {
  offset += PAGE_SIZE;
  save_progress();
  update_page();
}

static void prev_page(ClickRecognizerRef ref, void *ctx) {
  if(offset >= PAGE_SIZE) offset -= PAGE_SIZE;
  save_progress();
  update_page();
}

static void increase_font(ClickRecognizerRef ref, void *ctx) {
  if(font_size < 28) font_size += 2;
  text_layer_set_font(s_text, get_font());
  save_font();
}

static void decrease_font(ClickRecognizerRef ref, void *ctx) {
  if(font_size > 14) font_size -= 2;
  text_layer_set_font(s_text, get_font());
  save_font();
}

static void toggle_theme(ClickRecognizerRef ref, void *ctx) {
  dark_mode = !dark_mode;
  apply_theme();
  save_theme();
}

static void click_cfg(void *ctx) {
  window_single_click_subscribe(BUTTON_ID_SELECT, next_page);
  window_single_click_subscribe(BUTTON_ID_DOWN, prev_page);
  window_single_click_subscribe(BUTTON_ID_UP, increase_font);
  window_long_click_subscribe(BUTTON_ID_SELECT, 700, toggle_theme, NULL);
}

static void main_load(Window *w) {
  s_text = text_layer_create(GRect(0,0,144,168));
  text_layer_set_overflow_mode(s_text, GTextOverflowModeWordWrap);

  load_saved();
  load_book();

  text_layer_set_font(s_text, get_font());
  layer_add_child(window_get_root_layer(w), text_layer_get_layer(s_text));
  apply_theme();
  update_page();
}

static void main_unload(Window *w) {
  text_layer_destroy(s_text);
  free(book);
}

static void init() {
  s_main = window_create();
  window_set_click_config_provider(s_main, click_cfg);
  window_set_window_handlers(s_main, (WindowHandlers){ .load = main_load, .unload = main_unload });
  window_stack_push(s_main, true);
}

static void deinit() { window_destroy(s_main); }

int main() {
  init();
  app_event_loop();
  deinit();
}
