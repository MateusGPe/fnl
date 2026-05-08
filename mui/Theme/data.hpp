#pragma once
#include "primitives.hpp"

#include <FL/Enumerations.H>
#include <string>
#include <utility>
#include <vector>

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
        float slider_thumb_hover_halo_opacity = 0.9f;
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
        ThemeMetrics() = default;
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
        inline static const ThemeMetrics metrics{};
    };
    // The default palette, with gradients enabled.
    ThemePalette DARK_PALETTE = {
        frgb(240, 240, 240), // bg_main: Surface/Base
        frgb(255, 255, 255), // bg_sec: Canvas/Shade
        frgb(30, 30, 30),    // fg_main: Tint
        frgb(150, 150, 150), // inactive: Inactive/Border
        frgb(0, 120, 215),   // selection: Primary Main
        frgb(220, 50, 47),   // error_main: Error Main
        frgb(42, 157, 143),  // success_main: Success Main
        frgb(255, 255, 255), // free_bg
        frgb(255, 255, 220), // tt_bg
        frgb(0, 0, 0),       // tt_fg

        {frgb(252, 252, 252), frgb(245, 245, 245), frgb(235, 235, 235), frgb(225, 225, 225)},                                                          // btn_grad
        {frgb(170, 170, 170), frgb(170, 170, 170), frgb(170, 170, 170), frgb(255, 255, 255), frgb(255, 255, 255), frgb(255, 255, 255), 0, 0, nullptr}, // btn_frame

        {frgb(240, 248, 255), frgb(220, 238, 255), frgb(200, 228, 255), frgb(180, 218, 255)},                                                    // hover_grad
        {frgb(0, 120, 215), frgb(0, 120, 215), frgb(0, 120, 215), frgb(255, 255, 255), frgb(255, 255, 255), frgb(255, 255, 255), 0, 0, nullptr}, // hover_frame

        {frgb(210, 210, 210), frgb(200, 200, 200), frgb(190, 190, 190), frgb(180, 180, 180)},    // down_grad
        {frgb(150, 150, 150), frgb(150, 150, 150), frgb(150, 150, 150), 0, 0, 0, 0, 0, nullptr}, // down_frame

        {frgb(0, 140, 235), frgb(0, 120, 215), frgb(0, 100, 195), frgb(0, 80, 175)},                                                          // def_btn_grad
        {frgb(0, 60, 155), frgb(0, 60, 155), frgb(0, 60, 155), frgb(100, 180, 255), frgb(100, 180, 255), frgb(100, 180, 255), 0, 0, nullptr}, // def_btn_frame

        {frgb(180, 180, 180), frgb(180, 180, 180), frgb(180, 180, 180), 0, 0, 0, 0, 0, nullptr}, // thin_up_frame
        {frgb(150, 150, 150), frgb(150, 150, 150), frgb(150, 150, 150), 0, 0, 0, 0, 0, nullptr}, // thin_down_frame

        frgb(255, 255, 255),                                                                     // input_bg
        {frgb(180, 180, 180), frgb(180, 180, 180), frgb(180, 180, 180), 0, 0, 0, 0, 0, nullptr}, // input_frame

        frgb(0, 120, 215),   // focus_ring
        frgb(30, 30, 30),    // toggle_tick
        frgb(150, 150, 150), // toggle_border
        frgb(0, 120, 215),   // slider_track
        frgb(30, 30, 30)     // spinner_arrow
    };
    // FUTURISTIC_DARK_PALETTE
    ThemePalette FUTURISTIC_DARK_PALETTE = {
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

        {frgb(50, 50, 70), frgb(40, 40, 60), frgb(35, 35, 55), frgb(30, 30, 50)},          // btn_grad
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr}, // btn_frame

        {frgb(90, 90, 130), frgb(70, 70, 110), frgb(65, 65, 105), frgb(55, 55, 95)},       // hover_grad
        {frgb(0, 255, 255), frgb(0, 255, 255), frgb(0, 255, 255), 0, 0, 0, 0, 0, nullptr}, // hover_frame

        {frgb(15, 15, 30), frgb(25, 25, 40), frgb(25, 25, 40), frgb(35, 35, 55)},          // down_grad
        {frgb(0, 255, 255), frgb(0, 255, 255), frgb(0, 255, 255), 0, 0, 0, 0, 0, nullptr}, // down_frame

        {frgb(255, 0, 128), frgb(128, 128, 255), frgb(128, 128, 255), frgb(0, 255, 255)},  // def_btn_grad (Magenta -> Cyan)
        {frgb(0, 200, 200), frgb(0, 200, 200), frgb(0, 200, 200), 0, 0, 0, 0, 0, nullptr}, // def_btn_frame

        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr}, // thin_up_frame
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr}, // thin_down_frame

        frgb(20, 20, 35),                                                                  // input_bg
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr}, // input_frame

        frgb(255, 0, 255),   // focus_ring (Cyan)
        frgb(230, 230, 240), // toggle_tick
        frgb(80, 80, 100),   // toggle_border
        frgb(80, 80, 100),   // slider_track (Disabled/Inactive side)
        frgb(230, 230, 240)  // spinner_arrow
    };

    // FUTURISTIC_DARK_ORANGE_PALETTE
    ThemePalette FUTURISTIC_DARK_ORANGE_PALETTE = {
        frgb(20, 20, 35),    // bg_main: Surface/Base
        frgb(30, 30, 45),    // bg_sec: Canvas/Shade
        frgb(230, 230, 240), // fg_main: Tint
        frgb(80, 80, 100),   // inactive: Inactive/Border
        frgb(255, 150, 50),  // selection: Primary Main (Orange)
        frgb(255, 0, 128),   // error_main: Error Main (Magenta)
        frgb(0, 255, 128),   // success_main: Success Main (Spring Green)
        frgb(30, 30, 45),    // free_bg
        frgb(40, 40, 55),    // tt_bg
        frgb(230, 230, 240), // tt_fg

        {frgb(50, 50, 70), frgb(40, 40, 60), frgb(35, 35, 55), frgb(30, 30, 50)},          // btn_grad
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr}, // btn_frame

        {frgb(90, 90, 130), frgb(70, 70, 110), frgb(65, 65, 105), frgb(55, 55, 95)},          // hover_grad
        {frgb(255, 150, 50), frgb(255, 150, 50), frgb(255, 150, 50), 0, 0, 0, 0, 0, nullptr}, // hover_frame (Orange)

        {frgb(15, 15, 30), frgb(25, 25, 40), frgb(25, 25, 40), frgb(35, 35, 55)},             // down_grad
        {frgb(255, 150, 50), frgb(255, 150, 50), frgb(255, 150, 50), 0, 0, 0, 0, 0, nullptr}, // down_frame (Orange)

        {frgb(255, 0, 128), frgb(255, 75, 90), frgb(255, 75, 90), frgb(255, 150, 50)},        // def_btn_grad (Magenta -> Orange)
        {frgb(220, 120, 40), frgb(220, 120, 40), frgb(220, 120, 40), 0, 0, 0, 0, 0, nullptr}, // def_btn_frame (Dark Orange)

        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr}, // thin_up_frame
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr}, // thin_down_frame

        frgb(20, 20, 35),                                                                  // input_bg
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr}, // input_frame

        frgb(50, 50, 255),   // focus_ring (Orange)
        frgb(230, 230, 240), // toggle_tick
        frgb(80, 80, 100),   // toggle_border
        frgb(80, 80, 100),   // slider_track (Disabled/Inactive side)
        frgb(230, 230, 240)  // spinner_arrow
    };

    // SLATE_TEAL_PALETTE (Based on provided syntax theme)
    ThemePalette SLATE_TEAL_PALETTE = {
        frgb(49, 63, 64),    // bg_main: Editor background (Slate Teal)
        frgb(44, 54, 56),    // bg_sec: Header/Tab/Panel background
        frgb(160, 174, 173), // fg_main: Light muted teal-gray text
        frgb(80, 95, 96),    // inactive: Muted borders and inactive text
        frgb(107, 178, 160), // selection: Primary highlight (Mint/Teal Accent)
        frgb(224, 108, 117), // error_main: Error (Soft Red)
        frgb(152, 195, 121), // success_main: Success (Soft Green)
        frgb(44, 54, 56),    // free_bg
        frgb(35, 45, 46),    // tt_bg: Tooltip background
        frgb(230, 240, 240), // tt_fg: Tooltip text

        {frgb(60, 75, 76), frgb(55, 70, 71), frgb(49, 63, 64), frgb(45, 59, 60)},       // btn_grad
        {frgb(80, 95, 96), frgb(80, 95, 96), frgb(80, 95, 96), 0, 0, 0, 0, 0, nullptr}, // btn_frame

        {frgb(70, 85, 86), frgb(65, 80, 81), frgb(59, 73, 74), frgb(55, 69, 70)},                // hover_grad
        {frgb(107, 178, 160), frgb(107, 178, 160), frgb(107, 178, 160), 0, 0, 0, 0, 0, nullptr}, // hover_frame

        {frgb(40, 50, 51), frgb(45, 55, 56), frgb(45, 55, 56), frgb(49, 63, 64)},                // down_grad
        {frgb(107, 178, 160), frgb(107, 178, 160), frgb(107, 178, 160), 0, 0, 0, 0, 0, nullptr}, // down_frame

        {frgb(127, 198, 180), frgb(107, 178, 160), frgb(97, 168, 150), frgb(87, 158, 140)},   // def_btn_grad (Mint/Teal)
        {frgb(77, 148, 130), frgb(77, 148, 130), frgb(77, 148, 130), 0, 0, 0, 0, 0, nullptr}, // def_btn_frame

        {frgb(80, 95, 96), frgb(80, 95, 96), frgb(80, 95, 96), 0, 0, 0, 0, 0, nullptr}, // thin_up_frame
        {frgb(80, 95, 96), frgb(80, 95, 96), frgb(80, 95, 96), 0, 0, 0, 0, 0, nullptr}, // thin_down_frame

        frgb(38, 49, 50),                                                               // input_bg: Slightly darker than bg_main for depth
        {frgb(80, 95, 96), frgb(80, 95, 96), frgb(80, 95, 96), 0, 0, 0, 0, 0, nullptr}, // input_frame

        frgb(255, 123, 0),   // focus_ring
        frgb(160, 174, 173), // toggle_tick
        frgb(80, 95, 96),    // toggle_border
        frgb(107, 178, 160), // slider_track
        frgb(160, 174, 173)  // spinner_arrow
    };

    struct NamedPalette
    {
        std::string name;
        ThemePalette palette;
    };

    inline std::vector<NamedPalette>& get_theme_palettes()
    {
        static std::vector<NamedPalette> palettes = {
            {"Dark", DARK_PALETTE},
            {"Futuristic Dark", FUTURISTIC_DARK_PALETTE},
            {"Futuristic Dark Orange", FUTURISTIC_DARK_ORANGE_PALETTE},
            {"Slate Teal", SLATE_TEAL_PALETTE}};
        return palettes;
    }

    inline void add_theme_palette(std::string name, const ThemePalette& palette)
    {
        get_theme_palettes().push_back({std::move(name), palette});
    }
}
