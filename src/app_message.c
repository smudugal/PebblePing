#include <pebble.h>

static Window *s_window;	
static TextLayer *s_text;
	
// Keys for AppMessage Dictionary
// These should correspond to the values you defined in appinfo.json/Settings
enum {
	STATUS_KEY = 0,	
	MESSAGE_KEY = 1,
  PING_KEY = 2
};

const int PING_VALUE = 2;

// Write message to buffer & send
static void send_message(void){
	DictionaryIterator *iter;
	
	app_message_outbox_begin(&iter);
	dict_write_int(iter, PING_KEY, &PING_VALUE, sizeof(int), true);
	
	dict_write_end(iter);
  app_message_outbox_send();
}

// Called when a message is received from PebbleKitJS
static void in_received_handler(DictionaryIterator *received, void *context) {
}

// Called when an incoming message from PebbleKitJS is dropped
static void in_dropped_handler(AppMessageResult reason, void *context) {	
}

// Called when PebbleKitJS does not acknowledge receipt of a message
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  text_layer_set_text(s_text, "Ping failed!");
  APP_LOG(APP_LOG_LEVEL_ERROR, "Fail reason: %d", (int)reason);
}

static void out_sent_handler(DictionaryIterator *iterator, void *context){
  text_layer_set_text(s_text, "Ping Phone");
  APP_LOG(APP_LOG_LEVEL_INFO, "Ping succeeded");
}

static void main_window_load(Window *window){
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  const GEdgeInsets text_insets = GEdgeInsets((bounds.size.h - 20)/2 , 0);
  
  s_text = text_layer_create(grect_inset(bounds, text_insets));
  text_layer_set_text(s_text, "Ping Phone");
  text_layer_set_text_alignment(s_text, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(s_text));  
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_text);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *ctx) {
  // Add a msg to outbox to ping phone.
  send_message();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void init(void) {
	s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_set_click_config_provider(s_window, click_config_provider);
	window_stack_push(s_window, true);
	
	// Register AppMessage handlers
	app_message_register_inbox_received(in_received_handler); 
	app_message_register_inbox_dropped(in_dropped_handler); 
	app_message_register_outbox_failed(out_failed_handler);
  app_message_register_outbox_sent(out_sent_handler);

  // Initialize AppMessage inbox and outbox buffers with a suitable size
  const int inbox_size = 128;
  const int outbox_size = 128;
	app_message_open(inbox_size, outbox_size);
}

static void deinit(void) {
	app_message_deregister_callbacks();
	window_destroy(s_window);
}

int main( void ) {
	init();
	app_event_loop();
	deinit();
}