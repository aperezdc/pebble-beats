#include <pebble.h>

#define LENGTH_OF(v)  ((sizeof (v) / sizeof(v[0])))

enum PersistKey {
    PERSIST_KEY_FONT  = 0,
};
enum FontId {
    FONT_ACE_FUTURISM   = 0,
    FONT_FALSE_POSITIVE = 1,
    FONT_AKRON_SANS     = 2,
};


static Window    *s_main_window    = NULL;
static TextLayer *s_beats_layer    = NULL;
static TextLayer *s_time_layer     = NULL;
static Layer     *s_battery_layer  = NULL;
static char       s_beats_buffer[] = "@000";
static char       s_time_buffer[]  = "HH:MM NN";
static uint8_t    s_battery_level  = 100;


static void
update_time (struct tm *localtm,
             TimeUnits  units_changed)
{
    /* We'll do all calculations starting from the UTC time. */
    time_t utctime = mktime (localtm);

    /* The normal HH:MM NN text is straightforward. */
    clock_copy_time_string (s_time_buffer, LENGTH_OF (s_time_buffer));
    layer_mark_dirty (text_layer_get_layer (s_time_layer));

    /*
     * Convert the "struct tm" back to a time_t (seconds from the Epoch), so
     * it is possible to manipulate artifically the timezone, because we cannot
     * set TZ in the environment we have to do our own timezone adjustment :-/
     */
    utctime += (60 * 60);   /* TZ=UTC+1 */
    struct tm *T = gmtime (&utctime);

    /*
     * Internet Time is calculated from UTC+1 as follows:
     *   (UTC+1seconds + (UTC+1minutes * 60) + (UTC+1hours * 3600)) / 86.4
     */
    snprintf (s_beats_buffer, LENGTH_OF (s_beats_buffer), "@%03u",
              ((T->tm_sec) + (T->tm_min * 60) + (T->tm_hour * 3600)) * 10 / 864);
    layer_mark_dirty (text_layer_get_layer (s_beats_layer));
}


static void
update_bluetooth (bool connected)
{
    if (!connected) {
        vibes_short_pulse ();
    }
}


static void
update_battery (BatteryChargeState state)
{
    if (s_battery_level != state.charge_percent) {
        s_battery_level = state.charge_percent;
        layer_mark_dirty (s_battery_layer);
    }
}


static void
draw_battery_line (Layer *layer, GContext *ctx)
{
    GRect bounds = layer_get_bounds (layer);
    unsigned width = bounds.size.w * s_battery_level / 100;

    graphics_context_set_fill_color (ctx, GColorBlack);
    graphics_fill_rect (ctx, bounds, 0, GCornerNone);

    graphics_context_set_fill_color (ctx, COLOR_FALLBACK ((s_battery_level <= 20
                                                            ? GColorRed
                                                            : GColorLightGray),
                                                          GColorWhite));
    graphics_fill_rect (ctx,
                        GRect ((bounds.size.w - width) / 2, 0, width, bounds.size.h),
                        1, GCornersAll);
}


static GFont
config_get_font (void)
{
    switch (persist_read_int (PERSIST_KEY_FONT)) {
        case FONT_FALSE_POSITIVE:
            return fonts_load_custom_font (resource_get_handle (RESOURCE_ID_FONT_FALSE_POSITIVE_70));
        case FONT_AKRON_SANS:
            return fonts_load_custom_font (resource_get_handle (RESOURCE_ID_FONT_AKRON_SANS_76));
        case FONT_ACE_FUTURISM:
        default:
            return fonts_load_custom_font (resource_get_handle (RESOURCE_ID_FONT_BEATS_60));
    }
}

static void
main_window_load (Window *window)
{
    window_set_background_color (s_main_window, GColorBlack);

    /* Beats */
    s_beats_layer = text_layer_create (GRect (0, 32, 144, 78));
    text_layer_set_background_color (s_beats_layer, GColorClear);
    text_layer_set_text_color (s_beats_layer, GColorWhite);
    text_layer_set_text (s_beats_layer, s_beats_buffer);
    text_layer_set_font (s_beats_layer, config_get_font ());
    text_layer_set_text_alignment (s_beats_layer, GTextAlignmentCenter);
    layer_add_child (window_get_root_layer (s_main_window),
                     text_layer_get_layer (s_beats_layer));

    /* Normal HH:MM display */
    s_time_layer = text_layer_create (GRect (0, 120, 144, 24));
    text_layer_set_background_color (s_time_layer, GColorClear);
    text_layer_set_text_color (s_time_layer,
                               COLOR_FALLBACK (GColorLightGray,
                                               GColorWhite));
    text_layer_set_text (s_time_layer, s_time_buffer);
    text_layer_set_font (s_time_layer,
                         fonts_get_system_font (FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment (s_time_layer, GTextAlignmentCenter);
    layer_add_child (window_get_root_layer (s_main_window),
                     text_layer_get_layer (s_time_layer));

    /* Battery meter */
    s_battery_layer = layer_create (GRect (22, 25, 100, 4));
    layer_set_update_proc (s_battery_layer, draw_battery_line);
    layer_add_child (window_get_root_layer (s_main_window), s_battery_layer);
}

static void
main_window_unload (Window *window)
{
    text_layer_destroy (s_beats_layer);
    text_layer_destroy (s_time_layer);
    layer_destroy (s_battery_layer);
}


static void
config_init (void)
{
    if (!persist_exists (PERSIST_KEY_FONT)) {
        persist_write_int (PERSIST_KEY_FONT, FONT_ACE_FUTURISM);
    }
}


static void
config_apply (void)
{
    text_layer_set_font (s_beats_layer, config_get_font ());
}


static void
message_received (DictionaryIterator *iter, void *context)
{
    for (Tuple *t = dict_read_first (iter); t; t = dict_read_next (iter)) {
        if (t->key == PERSIST_KEY_FONT) {
            if (!strcmp (t->value->cstring, "ace-futurism")) {
                persist_write_int (t->key, FONT_ACE_FUTURISM);
            } else if (!strcmp (t->value->cstring, "false-positive")) {
                persist_write_int (t->key, FONT_FALSE_POSITIVE);
            } else if (!strcmp (t->value->cstring, "akron-sans")) {
                persist_write_int (t->key, FONT_AKRON_SANS);
            }
        }
    }

    config_apply ();
}


int main (int argc, char *argv[])
{
    s_main_window = window_create ();
    window_set_window_handlers (s_main_window, (WindowHandlers) {
        .load   = main_window_load,
        .unload = main_window_unload,
    });
    window_stack_push (s_main_window, true);

    bluetooth_connection_service_subscribe (update_bluetooth);

    battery_state_service_subscribe (update_battery);
    update_battery (battery_state_service_peek ());

    tick_timer_service_subscribe (MINUTE_UNIT, update_time);
    time_t curtime = time (NULL);
    update_time (localtime (&curtime), 0);

    config_init ();
    app_message_register_inbox_received (message_received);
    app_message_open(app_message_inbox_size_maximum(),
                     app_message_outbox_size_maximum());

    app_event_loop ();

    tick_timer_service_unsubscribe ();
    battery_state_service_unsubscribe ();
    bluetooth_connection_service_unsubscribe ();
    window_destroy (s_main_window);
    return 0;
}
