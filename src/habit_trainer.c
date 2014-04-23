#include <pebble.h>

static Window *window;
static TextLayer *text_layer;

static AppTimer *timer;
static bool timer_running;
static int seconds_left;
static int interval;
static const int MIN_SECONDS = 15;
static const int STARTING_INTERVAL = 30;
static const float MILD_DECREASE_FACTOR = 0.7;
static const float DECREASE_FACTOR = 0.4;
static const float INCREASE_FACTOR = 1.4;

static void decrease_timer(float factor) {
  interval = (int)((float)(interval) * factor);
  if (interval < MIN_SECONDS) {
    interval = MIN_SECONDS;
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Decreased timer to %d", interval);
  seconds_left = interval;
}

static void maintain_timer() {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Maintained timer at %d", interval);
  seconds_left = interval;
}

static void increase_timer(float factor) {
  interval = (int)((float)(interval) * factor);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Increased timer to %d", interval);
  seconds_left = interval;
}

static void update_countdown() {
  static char text_buffer[] = "Press up to increase\nPress select to maintain\nPress down to decrease.";
  if (seconds_left > 0) {
    snprintf(text_buffer, sizeof(text_buffer), "%d seconds left", seconds_left);
  } else {
    snprintf(text_buffer, sizeof(text_buffer), "Press up to increase\nPress select to maintain\nPress down to decrease.");
  }
  text_layer_set_text(text_layer, text_buffer);
}

static void tick_down(void* data) {
  seconds_left = seconds_left - 1;
  update_countdown();
  if (seconds_left == 0) {
    vibes_double_pulse();
  }
  if (seconds_left < -10) {
    decrease_timer(MILD_DECREASE_FACTOR);
  }
  timer = app_timer_register(1000, tick_down, NULL);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (timer_running) {
    if (seconds_left <= 0) {
      maintain_timer();
    }
  } else {
    timer_running = true;
    interval = STARTING_INTERVAL;
    seconds_left = interval;
    update_countdown();
    timer = app_timer_register(1000, tick_down, NULL);
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (timer_running && seconds_left <= 0) {
    increase_timer(INCREASE_FACTOR);
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (timer_running && seconds_left <= 0) {
    decrease_timer(DECREASE_FACTOR);
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, bounds.size.h } });
  text_layer_set_text(text_layer, "Click select to start");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(text_layer, GTextOverflowModeTrailingEllipsis);
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
