#include <pebble.h>

static Window *window;
static TextLayer *text_layer;

static AppTimer *timer;
static bool timer_running;
static int seconds_left;
static int interval;

static void decrease_timer() {
  interval = interval / 2;
  if (interval < 4) {
    interval = 4;
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Decreased timer to %d", interval);
  seconds_left = interval;
}

static void increase_timer() {
  interval = interval * 2;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Increased timer to %d", interval);
  seconds_left = interval;
}

static void update_countdown() {
  static char text_buffer[] = "100000000 seconds left";
  snprintf(text_buffer, sizeof(text_buffer), "%d seconds left", seconds_left);
  text_layer_set_text(text_layer, text_buffer);
}

static void tick_down(void* data) {
  seconds_left = seconds_left - 1;
  update_countdown();
  if (seconds_left == 0) {
    vibes_short_pulse();
    vibes_long_pulse();
    vibes_double_pulse();
  }
  if (seconds_left < -10) {
    decrease_timer();
  }
  timer = app_timer_register(1000, tick_down, NULL);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (timer_running) {
    if (seconds_left <= 0) {
      decrease_timer();
    }
  } else {
    timer_running = true;
    interval = 5;
    seconds_left = interval;
    update_countdown();
    timer = app_timer_register(1000, tick_down, NULL);
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (timer_running && seconds_left <= 0) {
    increase_timer();
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Click select to start");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
