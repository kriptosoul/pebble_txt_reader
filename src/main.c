#include <pebble.h>
#include "reader.h"

static Window *s_main_window;
static TextLayer *s_text_layer;

static void main_window_load(Window *window) {
  reader_init(window);
}

static void main_window_unload(Window *window) {
  reader_deinit();
}

int main(void) {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  app_event_loop();
  window_destroy(s_main_window);
}
