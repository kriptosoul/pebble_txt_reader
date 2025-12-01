
#include <pebble.h>

#define NUM_TEXTS 2
#define KEY_SCROLL_0 1000
#define KEY_SCROLL_1 1001

static Window *s_main_window;
static MenuLayer *s_menu_layer;
static Window *s_reader_window;
static ScrollLayer *s_scroll_layer;
static TextLayer *s_text_layer;
static GFont s_font;

static char *s_text_buffer = NULL;
static int s_current_index = 0;
static int s_font_size_index = 1;

static const char* s_text_names[NUM_TEXTS] = {
  "Book.txt", "Another.txt"
};
static const uint32_t s_text_resource_ids[NUM_TEXTS] = {
  RESOURCE_ID_BOOK_TXT,
  RESOURCE_ID_ANOTHER_TXT
};

static void load_text(int idx) {
  ResHandle h = resource_get_handle(s_text_resource_ids[idx]);
  size_t size = resource_size(h);
  if(s_text_buffer) free(s_text_buffer);
  s_text_buffer = malloc(size+1);
  resource_load(h, s_text_buffer, size);
  s_text_buffer[size]=0;
}

static void update_layout() {
  GRect bounds = layer_get_bounds(window_get_root_layer(s_reader_window));

  switch(s_font_size_index){
    case 0: s_font = fonts_get_system_font(FONT_KEY_GOTHIC_18); break;
    case 2: s_font = fonts_get_system_font(FONT_KEY_GOTHIC_28); break;
    default: s_font = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  }

  if(s_text_layer) text_layer_destroy(s_text_layer);
  s_text_layer = text_layer_create(GRect(3,3,bounds.size.w-6,2000));
  text_layer_set_font(s_text_layer, s_font);
  text_layer_set_text(s_text_layer, s_text_buffer);
  text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);

  GSize content = graphics_text_layout_get_content_size(
    s_text_buffer, s_font,
    GRect(0,0,bounds.size.w-6,2000),
    GTextOverflowModeWordWrap, GTextAlignmentLeft
  );
  layer_set_frame(text_layer_get_layer(s_text_layer),
                  GRect(3,3,bounds.size.w-6, content.h));

  if(s_scroll_layer) scroll_layer_destroy(s_scroll_layer);
  s_scroll_layer = scroll_layer_create(bounds);
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_text_layer));
  scroll_layer_set_content_size(s_scroll_layer, GSize(bounds.size.w, content.h+6));

  layer_add_child(window_get_root_layer(s_reader_window),
                  scroll_layer_get_layer(s_scroll_layer));

  // restore scroll offset
  int key = (s_current_index==0)?KEY_SCROLL_0:KEY_SCROLL_1;
  if(persist_exists(key)){
    int y = persist_read_int(key);
    scroll_layer_set_content_offset(s_scroll_layer, GPoint(0,y), false);
  }
}

static void up_click(ClickRecognizerRef r, void *ctx){
  GPoint o = scroll_layer_get_content_offset(s_scroll_layer);
  o.y -= 40;
  if(o.y<0) o.y=0;
  scroll_layer_set_content_offset(s_scroll_layer,o,true);
}
static void down_click(ClickRecognizerRef r, void *ctx){
  GPoint o = scroll_layer_get_content_offset(s_scroll_layer);
  o.y += 40;
  scroll_layer_set_content_offset(s_scroll_layer,o,true);
}
static void select_click(ClickRecognizerRef r, void *ctx){
  s_font_size_index=(s_font_size_index+1)%3;
  update_layout();
}

static void click_config(void *ctx){
  window_single_click_subscribe(BUTTON_ID_UP, up_click);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click);
}

static void reader_load(Window *w){
  update_layout();
}
static void reader_unload(Window *w){
  // save scroll position
  int key = (s_current_index==0)?KEY_SCROLL_0:KEY_SCROLL_1;
  GPoint o = scroll_layer_get_content_offset(s_scroll_layer);
  persist_write_int(key, o.y);

  if(s_text_layer) text_layer_destroy(s_text_layer);
  if(s_scroll_layer) scroll_layer_destroy(s_scroll_layer);
}

static void open_reader(int idx){
  s_current_index = idx;
  load_text(idx);

  if(!s_reader_window){
    s_reader_window = window_create();
    window_set_window_handlers(s_reader_window, (WindowHandlers){
      .load = reader_load,
      .unload = reader_unload
    });
    window_set_click_config_provider(s_reader_window, click_config);
  }
  window_stack_push(s_reader_window,true);
}

static uint16_t menu_rows(MenuLayer *m, uint16_t sec, void *ctx){
  return NUM_TEXTS;
}
static void menu_draw(GContext* ctx, const Layer *cell, MenuIndex *idx, void *ctx2){
  menu_cell_basic_draw(ctx, cell, s_text_names[idx->row], NULL, NULL);
}
static void menu_select(MenuLayer *m, MenuIndex *idx, void *ctx){
  open_reader(idx->row);
}

static void main_load(Window *w){
  Layer *root = window_get_root_layer(w);
  GRect b = layer_get_bounds(root);
  s_menu_layer = menu_layer_create(b);
  menu_layer_set_click_config_onto_window(s_menu_layer, w);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_rows = menu_rows,
    .draw_row = menu_draw,
    .select_click = menu_select
  });
  layer_add_child(root, menu_layer_get_layer(s_menu_layer));
}
static void main_unload(Window *w){
  menu_layer_destroy(s_menu_layer);
  if(s_reader_window) window_destroy(s_reader_window);
  if(s_text_buffer) free(s_text_buffer);
}

static void init(){
  s_main_window = window_create();
  window_set_window_handlers(s_main_window,(WindowHandlers){
    .load=main_load,
    .unload=main_unload
  });
  window_stack_push(s_main_window,true);
}
static void deinit(){
  window_destroy(s_main_window);
}
int main(){ init(); app_event_loop(); deinit(); }
