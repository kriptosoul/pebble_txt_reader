#include <pebble.h>

// AppMessage keys
#define KEY_CHUNK 2
#define KEY_CHUNK_INDEX 3
#define KEY_FINISHED 4

// Persist keys
#define PERSIST_KEY_PAGE 100
#define PERSIST_KEY_FONT 101
#define PERSIST_KEY_TEXT 200

static Window *s_main_window;
static TextLayer *s_text_layer;
static char *s_text = NULL;
static size_t s_text_len = 0;
static int s_page = 0;
static int s_font_index = 1; // 0 small,1 medium,2 large

// characters per page per font index (approximate)
static const int chars_per_page_by_font[3] = {360, 220, 140};

static void save_state() {
  persist_write_int(PERSIST_KEY_PAGE, s_page);
  persist_write_int(PERSIST_KEY_FONT, s_font_index);
  if (s_text && s_text_len > 0) {
    // store as nul-terminated blob
    persist_write_data(PERSIST_KEY_TEXT, s_text, s_text_len + 1);
  }
}

static void load_state() {
  if (persist_exists(PERSIST_KEY_PAGE)) {
    s_page = persist_read_int(PERSIST_KEY_PAGE);
  } else s_page = 0;
  if (persist_exists(PERSIST_KEY_FONT)) {
    s_font_index = persist_read_int(PERSIST_KEY_FONT);
    if (s_font_index < 0 || s_font_index > 2) s_font_index = 1;
  } else s_font_index = 1;
  if (persist_exists(PERSIST_KEY_TEXT)) {
    size_t size = persist_read_size(PERSIST_KEY_TEXT);
    s_text = malloc(size);
    if (s_text) {
      persist_read_data(PERSIST_KEY_TEXT, s_text, size);
      s_text_len = strlen(s_text);
    } else {
      s_text_len = 0;
    }
  }
}

static void free_text() {
  if (s_text) {
    free(s_text);
    s_text = NULL;
    s_text_len = 0;
  }
}

static GFont get_font_for_index(int idx) {
  switch(idx) {
    case 0: return fonts_get_system_font(FONT_KEY_GOTHIC_18);
    case 2: return fonts_get_system_font(FONT_KEY_GOTHIC_28);
    case 1:
    default: return fonts_get_system_font(FONT_KEY_GOTHIC_24);
  }
}

static void update_display() {
  if (!s_text || s_text_len == 0) {
    text_layer_set_text(s_text_layer, "No text loaded. Open configuration on the phone and send a .txt file.");
    return;
  }
  int chars_per_page = chars_per_page_by_font[s_font_index];
  int start = s_page * chars_per_page;
  if (start >= (int)s_text_len) {
    // clamp to last page
    int max_pages = (s_text_len + chars_per_page - 1) / chars_per_page;
    if (max_pages > 0) s_page = max_pages - 1;
    else s_page = 0;
    start = s_page * chars_per_page;
  }
  int remaining = s_text_len - start;
  int take = chars_per_page;
  if (remaining < take) take = remaining;
  // temporary buffer for page content
  char *buf = malloc(take + 1);
  if (!buf) return;
  memcpy(buf, s_text + start, take);
  buf[take] = '\0';
  text_layer_set_text(s_text_layer, buf);
  free(buf);
  // Save current page whenever updated
  persist_write_int(PERSIST_KEY_PAGE, s_page);
  persist_write_int(PERSIST_KEY_FONT, s_font_index);
}

static void next_page() {
  if (!s_text) return;
  int chars_per_page = chars_per_page_by_font[s_font_index];
  int max_pages = (s_text_len + chars_per_page - 1) / chars_per_page;
  if (s_page < max_pages - 1) s_page++;
  update_display();
  save_state();
}

static void prev_page() {
  if (!s_text) return;
  if (s_page > 0) s_page--;
  update_display();
  save_state();
}

static void cycle_font() {
  s_font_index = (s_font_index + 1) % 3;
  text_layer_set_font(s_text_layer, get_font_for_index(s_font_index));
  update_display();
  save_state();
}

static void reset_to_start() {
  s_page = 0;
  update_display();
  save_state();
}

// Button handlers
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  prev_page();
}
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  next_page();
}
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  // short press cycle font size
  cycle_font();
}
static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
  // long press reset to start
  reset_to_start();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT, 700, select_long_click_handler, NULL);
}

// AppMessage inbox handler: receive chunked text from companion
static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *t_chunk = dict_find(iter, KEY_CHUNK);
  if (t_chunk) {
    Tuple *t_index = dict_find(iter, KEY_CHUNK_INDEX);
    Tuple *t_finished = dict_find(iter, KEY_FINISHED);
    int chunk_index = t_index ? t_index->value->int32 : 0;
    const char *chunk = t_chunk->value->cstring;
    if (chunk_index == 0) {
      // first chunk: free previous and start new
      free_text();
      s_text_len = strlen(chunk);
      s_text = malloc(s_text_len + 1);
      if (s_text) strcpy(s_text, chunk);
    } else {
      if (!s_text) {
        // unexpected - allocate new
        s_text_len = strlen(chunk);
        s_text = malloc(s_text_len + 1);
        if (s_text) strcpy(s_text, chunk);
      } else {
        size_t addlen = strlen(chunk);
        char *newbuf = realloc(s_text, s_text_len + addlen + 1);
        if (newbuf) {
          s_text = newbuf;
          memcpy(s_text + s_text_len, chunk, addlen);
          s_text_len += addlen;
          s_text[s_text_len] = '\\0';
        }
      }
    }
    // If finished flag present, save persist and reset page to 0
    if (t_finished && t_finished->value->int32 == 1) {
      persist_write_data(PERSIST_KEY_TEXT, s_text, s_text_len + 1);
      s_page = 0;
      save_state();
    }
    update_display();
  }
}

static void inbox_dropped_handler(AppMessageResult reason, void *context) {
  // optional: handle dropped messages
}

static void init_appmessage() {
  app_message_register_inbox_received(inbox_received_handler);
  app_message_register_inbox_dropped(inbox_dropped_handler);
  // allocate larger buffers for large text transfers
  const uint32_t inbound_size = 64 * 1024;
  const uint32_t outbound_size = 256;
  app_message_open(inbound_size, outbound_size);
}

// Window handlers
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(2, 2, bounds.size.w - 4, bounds.size.h - 4));
  text_layer_set_font(s_text_layer, get_font_for_index(s_font_index));
  text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));

  update_display();
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_set_click_config_provider(s_main_window, click_config_provider);
  init_appmessage();
  load_state();
  window_stack_push(s_main_window, true);
  update_display();
}

static void deinit() {
  save_state();
  free_text();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
