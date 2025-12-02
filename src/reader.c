#include <pebble.h>
#include "reader.h"

// This is a skeleton implementation with placeholders.

static TextLayer *s_text_layer;
static char *s_text;
static int s_offset = 0; // saved position

void load_saved_position() {
  if (persist_exists(1)) s_offset = persist_read_int(1);
}

void save_position() {
  persist_write_int(1, s_offset);
}

void load_text() {
  // Placeholder text; real implementation would read a resource
  s_text = "Ejemplo de texto. Implementar lectura real de TXT aquí...";
}

void update_page() {
  // Placeholder pagination logic
  text_layer_set_text(s_text_layer, s_text + s_offset);
}

void reader_init(Window *window) {
  load_text();
  load_saved_position();

  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);

  s_text_layer = text_layer_create(bounds);
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentLeft);
  layer_add_child(root, text_layer_get_layer(s_text_layer));

  update_page();
}

void reader_deinit() {
  save_position();
  text_layer_destroy(s_text_layer);
}
