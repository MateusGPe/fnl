#pragma once
#include "primitives.hpp"

#include <FL/Enumerations.H>

namespace mui
{
    constexpr Fl_Color rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0)
    {
        return ((r & 0xff) << 24) | ((g & 0xff) << 16) | ((b & 0xff) << 8) | (a & 0xff);
    }

    constexpr Fl_Color frgb(uint8_t r, uint8_t g, uint8_t b)
    {
        return ((r & 0xff) << 24) | ((g & 0xff) << 16) | ((b & 0xff) << 8);
    }
    struct Gradient4
    {
        Fl_Color t_start;
        Fl_Color t_end;
        Fl_Color b_start;
        Fl_Color b_end;
    };

    struct ThemeMetrics
    {
        int radius = 5;
        float focus_ring_opacity = 0.2f;
        int focus_ring_width = 1;
        int choice_arrow_padding = 14;
        int choice_arrow_thickness = 2;
        int choice_arrow_height = 4;
        int choice_arrow_size = 8;
        int slider_track_height = 4;
        int slider_thumb_size = 16;
        int slider_thumb_focus_halo_size = 24;
        float slider_thumb_hover_halo_opacity = 0.15f;
        int valueslider_gap = 4;
        int valueslider_input_width = 40;
        int valueslider_input_height = 30;
        int valueslider_min_slider_size = 20;
        int spinner_arrow_size = 8;
        int progress_height = 6;
        int imageviewer_grid_size = 20;
        int imageviewer_handle_size = 8;
        int imageviewer_handle_hit_padding = 2;
        int imageviewer_selection_dash_width = 1;
        float widget_disabled_label_factor = 0.5f;

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
    // The default palette, with gradients enabled.
    constexpr ThemePalette DARK_PALETTE = {
        frgb(240, 240, 240),  // bg_main: Surface/Base
        frgb(255, 255, 255),  // bg_sec: Canvas/Shade
        frgb(30, 30, 30),     // fg_main: Tint
        frgb(150, 150, 150),  // inactive: Inactive/Border
        frgb(0, 120, 215),    // selection: Primary Main
        frgb(220, 50, 47),    // error_main: Error Main
        frgb(42, 157, 143),   // success_main: Success Main
        frgb(255, 255, 255),  // free_bg
        frgb(255, 255, 220),  // tt_bg
        frgb(0, 0, 0),        // tt_fg

        {frgb(252, 252, 252), frgb(245, 245, 245), frgb(235, 235, 235), frgb(225, 225, 225)}, // btn_grad
        {frgb(170, 170, 170), frgb(170, 170, 170), frgb(170, 170, 170), frgb(255, 255, 255), frgb(255, 255, 255), frgb(255, 255, 255), 0, 0, nullptr}, // btn_frame

        {frgb(240, 248, 255), frgb(220, 238, 255), frgb(200, 228, 255), frgb(180, 218, 255)}, // hover_grad
        {frgb(0, 120, 215), frgb(0, 120, 215), frgb(0, 120, 215), frgb(255, 255, 255), frgb(255, 255, 255), frgb(255, 255, 255), 0, 0, nullptr}, // hover_frame

        {frgb(210, 210, 210), frgb(200, 200, 200), frgb(190, 190, 190), frgb(180, 180, 180)}, // down_grad
        {frgb(150, 150, 150), frgb(150, 150, 150), frgb(150, 150, 150), 0, 0, 0, 0, 0, nullptr}, // down_frame

        {frgb(0, 140, 235), frgb(0, 120, 215), frgb(0, 100, 195), frgb(0, 80, 175)}, // def_btn_grad
        {frgb(0, 60, 155), frgb(0, 60, 155), frgb(0, 60, 155), frgb(100, 180, 255), frgb(100, 180, 255), frgb(100, 180, 255), 0, 0, nullptr}, // def_btn_frame

        {frgb(180, 180, 180), frgb(180, 180, 180), frgb(180, 180, 180), 0, 0, 0, 0, 0, nullptr}, // thin_up_frame
        {frgb(150, 150, 150), frgb(150, 150, 150), frgb(150, 150, 150), 0, 0, 0, 0, 0, nullptr}, // thin_down_frame

        frgb(255, 255, 255),     // input_bg
        {frgb(180, 180, 180), frgb(180, 180, 180), frgb(180, 180, 180), 0, 0, 0, 0, 0, nullptr}, // input_frame

        frgb(0, 120, 215),    // focus_ring
        frgb(30, 30, 30),     // toggle_tick
        frgb(150, 150, 150),  // toggle_border
        frgb(0, 120, 215),    // slider_track
        frgb(30, 30, 30)      // spinner_arrow
    };
    // FUTURISTIC_DARK_PALETTE
    /* constexpr ThemePalette DARK_PALETTE = {
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
