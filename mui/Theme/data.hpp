#pragma once
#include "primitives.hpp"

#define rgba(r, g, b, a) \
    (((r) & 0xff) << 24 | ((g) & 0xff) << 16 | ((b) & 0xff) << 8 | (unsigned)(a))
#define frgb fl_rgb_color

namespace mui
{
    struct Gradient4
    {
        Fl_Color t_start;
        Fl_Color t_end;
        Fl_Color b_start;
        Fl_Color b_end;
    };

    struct ThemeMetrics
    {
        int radius = 2;
        int input_focused_border_width = 4;
        int input_default_border_width = 1;
        int input_focused_inset = 1;
        float state_hover_opacity = 0.08f;
        float shadow_outer_opacity = 0.12f;
        float shadow_inner_opacity = 0.08f;
        float border_opacity = 0.2f;
        float downbox_shadow_opacity = 0.15f;
        int secondary_radius = 6;
        float secondary_shadow_base = 0.02f;
        float secondary_shadow_hover = 0.05f;
        float focus_ring_opacity = 0.2f;
        int focus_ring_width = 3;
        int secondary_border_width = 1;
        float secondary_downbox_shadow = 0.10f;
        double cursor_blink_rate = 0.53;
        int input_text_padding = 4;
        int toggle_track_height = 18;
        int toggle_knob_size = 20;
        int toggle_focus_padding = 4;
        int toggle_shadow_offset = 1;
        int toggle_label_padding = 8;
        float toggle_track_hover_factor = 0.9f;
        float toggle_outline_hover_factor = 0.7f;
        float toggle_thumb_hover_factor = 0.95f;
        float toggle_focus_factor = 0.5f;
        float toggle_shadow_factor = 0.15f;
        int button_focus_padding = 1;
        int button_focus_width = 2;
        int checkbox_size = 18;
        int checkbox_corner_radius = 3;
        float checkbox_hover_factor = 0.85f;
        int checkbox_border_width = 1;
        int checkbox_focus_padding = 2;
        int checkbox_focus_width = 2;
        int checkbox_tick_width = 2;
        int checkbox_tick_padding = 4;
        int checkbox_label_padding = 8;
        int choice_arrow_padding = 14;
        int choice_arrow_thickness = 2;
        int choice_arrow_height = 4;
        int choice_text_padding = 8;
        int choice_arrow_size = 8;
        int radio_size = 18;
        int radio_dot_size = 8;
        float radio_hover_factor = 0.85f;
        int radio_border_width = 2;
        int radio_focus_padding = 2;
        int radio_label_padding = 8;
        int slider_track_height = 4;
        int slider_thumb_size = 16;
        int slider_thumb_focus_halo_size = 24;
        float slider_thumb_hover_halo_opacity = 0.15f;
        int valueslider_gap = 4;
        int valueslider_input_width = 40;
        int valueslider_input_height = 30;
        int valueslider_min_slider_size = 20;
        int spinner_button_width = 28;
        int spinner_arrow_size = 8;
        int spinner_arrow_thickness = 2;
        int progress_height = 6;
        int progress_corner_radius = 3;
        int imageviewer_grid_size = 20;
        int imageviewer_handle_size = 8;
        int imageviewer_handle_hit_padding = 2;
        int imageviewer_selection_dash_width = 1;
        float state_focus_opacity = 0.12f;
        float state_pressed_opacity = 0.12f;
        float widget_disabled_label_factor = 0.5f;
        int svg_stroke_width = 1;
        int roller_line_spacing = 10;
        float roller_fade_power = 2.5f;

        int tab_height = 30;
        int tab_active_line_thickness = 4;
        int tab_close_icon_size = 10;
        int tab_close_padding = 4;
    };

    struct ThemePalette
    {
        Fl_Color bg_main, bg_sec, fg_main, inactive, selection, error_main, success_main, free_bg, tt_bg, tt_fg;
        Gradient4 btn_grad;
        core::FrameColors btn_frame;
        Gradient4 hover_grad;
        core::FrameColors hover_frame;
        Gradient4 down_grad;
        core::FrameColors down_frame;
        Gradient4 def_btn_grad;
        core::FrameColors def_btn_frame;
        core::FrameColors thin_up_frame;
        core::FrameColors thin_down_frame;
        Fl_Color input_bg;
        core::FrameColors input_frame;
        Fl_Color focus_ring;
        Fl_Color toggle_tick;
        Fl_Color toggle_border;
        Fl_Color slider_track;
        Fl_Color spinner_arrow;
        ThemeMetrics metrics;
    };

 const ThemePalette DARK_PALETTE = {
            frgb(39, 45, 61),     // bg_main: Surface/Base
            frgb(27, 32, 45),     // bg_sec: Canvas/Shade
            frgb(255, 255, 255),  // fg_main: Tint
            frgb(60, 68, 85),     // inactive: Inactive/Border
            frgb(255, 107, 0),    // selection: Primary Main
            frgb(230, 57, 70),    // error_main: Error Main
            frgb(42, 157, 143),   // success_main: Success Main
            frgb(27, 32, 45),     // free_bg
            frgb(39, 45, 61),     // tt_bg
            frgb(255, 255, 255),  // tt_fg

            {frgb(39, 45, 61), frgb(39, 45, 61), frgb(39, 45, 61), frgb(39, 45, 61)}, // btn_grad (solid surface)
            {frgb(60, 68, 85), frgb(60, 68, 85), frgb(60, 68, 85), 0, 0, 0, 0, 0, nullptr}, // btn_frame

            {frgb(60, 68, 85), frgb(60, 68, 85), frgb(60, 68, 85), frgb(60, 68, 85)}, // hover_grad
            {frgb(255, 107, 0), frgb(255, 107, 0), frgb(255, 107, 0), 0, 0, 0, 0, 0, nullptr}, // hover_frame

            {frgb(27, 32, 45), frgb(27, 32, 45), frgb(27, 32, 45), frgb(27, 32, 45)}, // down_grad
            {frgb(255, 107, 0), frgb(255, 107, 0), frgb(255, 107, 0), 0, 0, 0, 0, 0, nullptr}, // down_frame

            {frgb(255, 107, 0), frgb(255, 107, 0), frgb(255, 107, 0), frgb(255, 107, 0)}, // def_btn_grad (primary)
            {frgb(255, 107, 0), frgb(255, 107, 0), frgb(255, 107, 0), 0, 0, 0, 0, 0, nullptr}, // def_btn_frame

            {frgb(60, 68, 85), frgb(60, 68, 85), frgb(60, 68, 85), 0, 0, 0, 0, 0, nullptr}, // thin_up_frame
            {frgb(60, 68, 85), frgb(60, 68, 85), frgb(60, 68, 85), 0, 0, 0, 0, 0, nullptr}, // thin_down_frame

            frgb(27, 32, 45),     // input_bg
            {frgb(60, 68, 85), frgb(60, 68, 85), frgb(60, 68, 85), 0, 0, 0, 0, 0, nullptr}, // input_frame

            frgb(255, 107, 0),    // focus_ring
            frgb(255, 255, 255),  // toggle_tick
            frgb(60, 68, 85),     // toggle_border
            frgb(255, 107, 0),    // slider_track
            frgb(255, 255, 255)   // spinner_arrow
        }; 
    // GRADIENT_PALETTE
    // const ThemePalette DARK_PALETTE = {
    //     frgb(240, 240, 240),  // bg_main: Surface/Base
    //     frgb(255, 255, 255),  // bg_sec: Canvas/Shade
    //     frgb(30, 30, 30),     // fg_main: Tint
    //     frgb(150, 150, 150),  // inactive: Inactive/Border
    //     frgb(0, 120, 215),    // selection: Primary Main
    //     frgb(220, 50, 47),    // error_main: Error Main
    //     frgb(42, 157, 143),   // success_main: Success Main
    //     frgb(255, 255, 255),  // free_bg
    //     frgb(255, 255, 220),  // tt_bg
    //     frgb(0, 0, 0),        // tt_fg

    //     {frgb(252, 252, 252), frgb(245, 245, 245), frgb(235, 235, 235), frgb(225, 225, 225)}, // btn_grad
    //     {frgb(170, 170, 170), frgb(170, 170, 170), frgb(170, 170, 170), frgb(255, 255, 255), frgb(255, 255, 255), frgb(255, 255, 255), 0, 0, nullptr}, // btn_frame

    //     {frgb(240, 248, 255), frgb(220, 238, 255), frgb(200, 228, 255), frgb(180, 218, 255)}, // hover_grad
    //     {frgb(0, 120, 215), frgb(0, 120, 215), frgb(0, 120, 215), frgb(255, 255, 255), frgb(255, 255, 255), frgb(255, 255, 255), 0, 0, nullptr}, // hover_frame

    //     {frgb(210, 210, 210), frgb(200, 200, 200), frgb(190, 190, 190), frgb(180, 180, 180)}, // down_grad
    //     {frgb(150, 150, 150), frgb(150, 150, 150), frgb(150, 150, 150), 0, 0, 0, 0, 0, nullptr}, // down_frame

    //     {frgb(0, 140, 235), frgb(0, 120, 215), frgb(0, 100, 195), frgb(0, 80, 175)}, // def_btn_grad
    //     {frgb(0, 60, 155), frgb(0, 60, 155), frgb(0, 60, 155), frgb(100, 180, 255), frgb(100, 180, 255), frgb(100, 180, 255), 0, 0, nullptr}, // def_btn_frame

    //     {frgb(180, 180, 180), frgb(180, 180, 180), frgb(180, 180, 180), 0, 0, 0, 0, 0, nullptr}, // thin_up_frame
    //     {frgb(150, 150, 150), frgb(150, 150, 150), frgb(150, 150, 150), 0, 0, 0, 0, 0, nullptr}, // thin_down_frame

    //     frgb(255, 255, 255),     // input_bg
    //     {frgb(180, 180, 180), frgb(180, 180, 180), frgb(180, 180, 180), 0, 0, 0, 0, 0, nullptr}, // input_frame

    //     frgb(0, 120, 215),    // focus_ring
    //     frgb(30, 30, 30),     // toggle_tick
    //     frgb(150, 150, 150),  // toggle_border
    //     frgb(0, 120, 215),    // slider_track
    //     frgb(30, 30, 30)      // spinner_arrow
    // };
    // FUTURISTIC_DARK_PALETTE
    /* const ThemePalette DARK_PALETTE = {
        frgb(20, 20, 35),    // bg_main: Surface/Base
        frgb(30, 30, 45),    // bg_sec: Canvas/Shade
        frgb(230, 230, 240), // fg_main: Tint
        frgb(80, 80, 100),   // inactive: Inactive/Border
        frgb(0, 255, 255),   // selection: Primary Main (Cyan)
        frgb(255, 0, 128),   // error_main: Error Main (Magenta)
        frgb(0, 255, 128),   // success_main: Success Main (Spring Green)
        frgb(30, 30, 45),    // free_bg
        frgb(40, 40, 55),    // tt_bg
        frgb(230, 230, 240), // tt_fg

        {frgb(45, 45, 65), frgb(40, 40, 60), frgb(40, 40, 60), frgb(35, 35, 55)},          // btn_grad
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr}, // btn_frame

        {frgb(80, 80, 120), frgb(60, 60, 100), frgb(60, 60, 100), frgb(50, 50, 90)},       // hover_grad
        {frgb(0, 255, 255), frgb(0, 255, 255), frgb(0, 255, 255), 0, 0, 0, 0, 0, nullptr}, // hover_frame

        {frgb(30, 30, 45), frgb(35, 35, 55), frgb(35, 35, 55), frgb(40, 40, 60)},          // down_grad
        {frgb(0, 255, 255), frgb(0, 255, 255), frgb(0, 255, 255), 0, 0, 0, 0, 0, nullptr}, // down_frame

        {frgb(255, 0, 128), frgb(128, 128, 255), frgb(128, 128, 255), frgb(0, 255, 255)},  // def_btn_grad (Magenta -> Cyan)
        {frgb(0, 200, 200), frgb(0, 200, 200), frgb(0, 200, 200), 0, 0, 0, 0, 0, nullptr}, // def_btn_frame

        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr}, // thin_up_frame
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr}, // thin_down_frame

        frgb(20, 20, 35),                                                                  // input_bg
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr}, // input_frame

        frgb(0, 255, 255),   // focus_ring (Cyan)
        frgb(230, 230, 240), // toggle_tick
        frgb(80, 80, 100),   // toggle_border
        frgb(80, 80, 100),   // slider_track (Disabled/Inactive side)
        frgb(230, 230, 240)  // spinner_arrow
    }; */
}
