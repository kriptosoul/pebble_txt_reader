#include <pebble.h>
#include "reader.h"

static Window *s_main_window;
static TextLayer *s_text_layer;

static char *book_text = NULL;
static int page = 0;
static int page_size = 500;

static void load_book() {
  ResHandle rh = resource_get_handle(RESOURCE_ID_BOOK_TXT);
  size_t size = resource_size(rh);
  book_text = malloc(size + 1);
  resource_load(rh, book_text, size);
  book_text[size] = 0;
}

static void update_page() {
  int start = page * page_size;
  if(start >= strlen(book_text)) start = strlen(book_text) - 1;

  static char buffer[512];
  strncpy(buffer, book_text + start, page_size);
  buffer[page_size] = 0;

  text_layer_set_text(s_text_layer, buffer);
}

static void next_page(ClickRecognizerRef recognizer, void *context) {
  page++;
  update_page();
}

static void prev_page(ClickRecognizerRef recognizer, void *context) {
  if(page > 0) page--;
  update_page();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_DOWN, next_page);
  window_single_click_subscribe(BUTTON_ID_UP, prev_page);
}

void reader_init(void) {
  load_book();
  update_page();
}

void reader_deinit(void) {
  if(book_text) free(book_text);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(bounds);
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_text_layer, "Cargando libro...");
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_set_window_handlers(s_main_window, (WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(s_main_window, true);

  reader_init();
}

static void deinit() {
  reader_deinit();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
