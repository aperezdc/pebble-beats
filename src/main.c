#include <pebble.h>

#define LENGTH_OF(v)  ((sizeof (v) / sizeof(v[0])))
    
static Window *s_main_window = NULL;
static TextLayer *s_time_layer = NULL;


static void
update_time (struct tm *localtm,
             TimeUnits  units_changed)
{
    /*
     * Convert the "struct tm" back to a time_t (seconds from the Epoch), so
     * it is possible to manipulate artifically the timezone, because we cannot
     * set TZ in the environment we have to do our own timezone adjustment :-/
     */
    time_t utctime = mktime (localtm) + (60 * 60);   /* TZ=UTC-1 */
    struct tm *T = gmtime (&utctime);
    
    static char buffer[] = "@nnn";
    snprintf (buffer, LENGTH_OF (buffer), "@%03u",
              ((T->tm_sec) + (T->tm_min * 60) + (T->tm_hour * 3600)) * 1000 / 864);
    text_layer_set_text (s_time_layer, buffer);
}


static void
main_window_load (Window *window)
{
    s_time_layer = text_layer_create (GRect (0, 55, 144, 50));
    text_layer_set_background_color (s_time_layer, GColorClear);
    text_layer_set_text_color (s_time_layer, GColorBlack);
    text_layer_set_text (s_time_layer, "@000");
    text_layer_set_font (s_time_layer,
                         fonts_get_system_font (FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text_alignment (s_time_layer, GTextAlignmentCenter);
    layer_add_child (window_get_root_layer (s_main_window),
                     text_layer_get_layer (s_time_layer));
}

static void
main_window_unload (Window *window)
{
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
    
    tick_timer_service_subscribe (MINUTE_UNIT, update_time);
    time_t curtime = time (NULL);
    update_time (localtime (&curtime), 0);
    
    app_event_loop ();
    
    window_destroy (s_main_window);
}