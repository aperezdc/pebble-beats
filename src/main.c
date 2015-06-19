#include <pebble.h>

#define LENGTH_OF(v)  ((sizeof (v) / sizeof(v[0])))


static Window    *s_main_window    = NULL;
static TextLayer *s_beats_layer    = NULL;
static TextLayer *s_time_layer     = NULL;
static char       s_beats_buffer[] = "@000";
static char       s_time_buffer[]  = "HH:MM NN";


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
    utctime -= (60 * 60);   /* TZ=UTC-1 */
    struct tm *T = gmtime (&utctime);

    snprintf (s_beats_buffer, LENGTH_OF (s_beats_buffer), "@%03u",
              ((T->tm_sec) + (T->tm_min * 60) + (T->tm_hour * 3600)) * 1000 / 864);
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
main_window_load (Window *window)
{
    window_set_background_color (s_main_window, GColorBlack);

    /* Beats */
    s_beats_layer = text_layer_create (GRect (0, 32, 144, 60));
    text_layer_set_background_color (s_beats_layer, GColorClear);
    text_layer_set_text_color (s_beats_layer, GColorWhite);
    text_layer_set_text (s_beats_layer, s_beats_buffer);
    text_layer_set_font (s_beats_layer,
        fonts_load_custom_font (
            resource_get_handle (RESOURCE_ID_FONT_BEATS_60)));
    text_layer_set_text_alignment (s_beats_layer, GTextAlignmentCenter);
    layer_add_child (window_get_root_layer (s_main_window),
                     text_layer_get_layer (s_beats_layer));

    /* Normal HH:MM display */
    s_time_layer = text_layer_create (GRect (0, 120, 144, 120 + 24));
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
}

static void
main_window_unload (Window *window)
{
    text_layer_destroy (s_beats_layer);
    text_layer_destroy (s_time_layer);
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
    tick_timer_service_subscribe (MINUTE_UNIT, update_time);
    time_t curtime = time (NULL);
    update_time (localtime (&curtime), 0);

    app_event_loop ();

    tick_timer_service_unsubscribe ();
    bluetooth_connection_service_unsubscribe ();
    window_destroy (s_main_window);
    return 0;
}
