#include "../../lv_examples.h"
#if LV_USE_CHART && LV_BUILD_EXAMPLES

static lv_obj_t * chart;
static lv_chart_series_t * ser;
static lv_chart_cursor_t * cursor;

static void event_cb(lv_obj_t * obj, lv_event_t e)
{
    static int32_t last_id = -1;
    if(e == LV_EVENT_VALUE_CHANGED) {
        last_id = lv_chart_get_pressed_point(obj);
        if(last_id != LV_CHART_POINT_NONE) {
            lv_point_t p;
            lv_chart_get_point_pos_by_id(obj, ser, last_id, &p);
            lv_chart_set_cursor_point(obj, cursor, &p);
        }
    }
    else if(e == LV_EVENT_DRAW_PART_END) {
        lv_obj_draw_dsc_t * dsc = lv_event_get_param();
        if(dsc->part == LV_PART_CURSOR && dsc->p1 && dsc->p2 && dsc->p1->y == dsc->p2->y && last_id >= 0) {
            lv_coord_t * data_array = lv_chart_get_array(chart, ser);
            lv_coord_t v = data_array[last_id];
            char buf[16];
            lv_snprintf(buf, sizeof(buf), "%d", v);

            lv_point_t size;
            lv_txt_get_size(&size, buf, LV_FONT_DEFAULT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

            lv_area_t a;
            a.y2 = dsc->p1->y - 5;
            a.y1 = a.y2 - size.y - 10;
            a.x1 = dsc->p1->x + 10;
            a.x2 = a.x1 + size.x + 10;

            lv_draw_rect_dsc_t draw_rect_dsc;
            lv_draw_rect_dsc_init(&draw_rect_dsc);
            draw_rect_dsc.bg_color = lv_color_blue();
            draw_rect_dsc.radius = 3;

            lv_draw_rect(&a, dsc->clip_area, &draw_rect_dsc);

            lv_draw_label_dsc_t draw_label_dsc;
            lv_draw_label_dsc_init(&draw_label_dsc);
            draw_label_dsc.color = lv_color_white();
            a.x1 += 5;
            a.x2 -= 5;
            a.y1 += 5;
            a.y2 -= 5;
            lv_draw_label(&a, dsc->clip_area, &draw_label_dsc, buf, NULL);
        }
    }
}

/**
 * Show cursor on the clicked point
 */
void lv_example_chart_6(void)
{
    chart = lv_chart_create(lv_scr_act());
    lv_obj_set_size(chart, 200, 150);
    lv_obj_align(chart, LV_ALIGN_CENTER, 0, -10);

    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 10, 5, 6, 5, true, 40);
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_X, 10, 5, 10, 1, true, 30);

    lv_obj_add_event_cb(chart, event_cb, NULL);
    lv_obj_refresh_ext_draw_size(chart);

    cursor = lv_chart_add_cursor(chart, lv_color_blue(), LV_DIR_LEFT | LV_DIR_BOTTOM);

    ser = lv_chart_add_series(chart, lv_color_red(), LV_CHART_AXIS_PRIMARY_Y);
    uint32_t i;
    for(i = 0; i < 10; i++) {
        lv_chart_set_next_value(chart, ser, lv_rand(10,90));
    }

    lv_chart_set_zoom_x(chart, 500);

    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Click on a point");
    lv_obj_align_to(label, chart, LV_ALIGN_OUT_TOP_MID, 0, -5);

}

#endif
