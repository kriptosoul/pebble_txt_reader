\
#include <pebble.h>
#define PKEY_OFFSET 1
#define PKEY_FONT 2
#define PKEY_THEME 3
#define PAGE_SIZE 500
static Window *s_window;
static TextLayer *s_text;
static char *s_book = NULL;
static int s_book_len = 0;
static int s_offset = 0;
static int s_font_idx = 1;
static bool s_dark = false;
static GFont fonts[3];
static void load_book() {
  ResHandle rh = resource_get_handle(RESOURCE_ID_BOOK);
  if (!rh) return;
  s_book_len = (int)resource_size(rh);
  s_book = malloc(s_book_len + 1);
  if (s_book) {
    resource_load(rh, s_book, s_book_len);
    s_book[s_book_len] = '\\0';
  }
}
static void save_state() {
  persist_write_int(PKEY_OFFSET, s_offset);
  persist_write_int(PKEY_FONT, s_font_idx);
  persist_write_bool(PKEY_THEME, s_dark);
}
static void load_state() {
  if (persist_exists(PKEY_OFFSET)) s_offset = persist_read_int(PKEY_OFFSET);
  if (persist_exists(PKEY_FONT)) s_font_idx = persist_read_int(PKEY_FONT);
  if (persist_exists(PKEY_THEME)) s_dark = persist_read_bool(PKEY_THEME);
}
static GFont select_font(int idx) {
  if (idx==0) return fonts[0];
  if (idx==2) return fonts[2];
  return fonts[1];
}
static void apply_theme() {
  window_set_background_color(s_window, s_dark ? GColorBlack : GColorWhite);
  text_layer_set_text_color(s_text, s_dark ? GColorWhite : GColorBlack);
}
static void update_page_text() {
  if (!s_book) {
    text_layer_set_text(s_text, "No book resource found.");
    return;
  }
  int start = s_offset;
  if (start < 0) start = 0;
  if (start > s_book_len) start = s_book_len;
  int len = PAGE_SIZE;
  if (start + len > s_book_len) len = s_book_len - start;
  static char buf[PAGE_SIZE+1];
  memcpy(buf, s_book + start, len);
  buf[len] = '\\0';
  text_layer_set_text(s_text, buf);
}
static void next_page(ClickRecognizerRef ref, void *ctx) {
  if (s_offset + PAGE_SIZE < s_book_len) s_offset += PAGE_SIZE;
  update_page_text();
  save_state();
}
static void prev_page(ClickRecognizerRef ref, void *ctx) {
  if (s_offset >= PAGE_SIZE) s_offset -= PAGE_SIZE;
  else s_offset = 0;
  update_page_text();
  save_state();
}
static void increase_font(ClickRecognizerRef ref, void *ctx) {
  if (s_font_idx < 2) s_font_idx++;
  text_layer_set_font(s_text, select_font(s_font_idx));
  save_state();
}
static void decrease_font(ClickRecognizerRef ref, void *ctx) {
  if (s_font_idx > 0) s_font_idx--;
  text_layer_set_font(s_text, select_font(s_font_idx));
  save_state();
}
static void toggle_theme_long(ClickRecognizerRef ref, void *ctx) {
  s_dark = !s_dark;
  apply_theme();
  save_state();
}
static void click_config(void *ctx) {
  window_single_click_subscribe(BUTTON_ID_SELECT, next_page);
  window_single_click_subscribe(BUTTON_ID_DOWN, prev_page);
  window_single_click_subscribe(BUTTON_ID_UP, decrease_font);
  window_long_click_subscribe(BUTTON_ID_UP, 700, increase_font, NULL);
  window_long_click_subscribe(BUTTON_ID_SELECT, 700, toggle_theme_long, NULL);
}
static void window_load(Window *window) {
  s_window = window;
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  s_text = text_layer_create(GRect(4,0,bounds.size.w-8,bounds.size.h));
  text_layer_set_overflow_mode(s_text, GTextOverflowModeWordWrap);
  text_layer_set_text_alignment(s_text, GTextAlignmentLeft);
  layer_add_child(root, text_layer_get_layer(s_text));
  fonts[0] = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  fonts[1] = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  fonts[2] = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  load_state();
  load_book();
  text_layer_set_font(s_text, select_font(s_font_idx));
  apply_theme();
  update_page_text();
}
static void window_unload(Window *window) {
  save_state();
  text_layer_destroy(s_text);
  if (s_book) free(s_book);
}
static void init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers){ .load = window_load, .unload = window_unload });
  window_set_click_config_provider(s_window, click_config);
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
