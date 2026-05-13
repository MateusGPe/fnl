#pragma once
#include "primitives.hpp"

#include <FL/Enumerations.H>
#include <string>
#include <utility>
#include <vector>

namespace mui
{
    constexpr Fl_Color rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0) noexcept
    {
        return static_cast<Fl_Color>(
            (static_cast<uint32_t>(r) << 24) |
            (static_cast<uint32_t>(g) << 16) |
            (static_cast<uint32_t>(b) << 8) |
            static_cast<uint32_t>(a));
    }

    constexpr Fl_Color frgb(uint8_t r, uint8_t g, uint8_t b) noexcept
    {
        return rgba(r, g, b, 0);
    }

    struct Gradient4
    {
        Fl_Color t_start, t_end;
        Fl_Color b_start, b_end;
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
        int slider_thumb_focus_halo_size = 20;
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
        Fl_Color bg_main, bg_sec, fg_main;
        Fl_Color inactive, selection;
        Fl_Color error_main, success_main;
        Fl_Color free_bg, tt_bg, tt_fg;
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

    //

    inline ThemePalette BLUE_LIGHT_PALETTE = {
        frgb(240, 240, 240),
        frgb(255, 255, 255),
        frgb(30, 30, 30),
        frgb(150, 150, 150),
        frgb(0, 120, 215),
        frgb(220, 50, 47),
        frgb(42, 157, 143),
        frgb(255, 255, 255),
        frgb(255, 255, 220),
        frgb(0, 0, 0),
        {frgb(252, 252, 252), frgb(245, 245, 245), frgb(235, 235, 235), frgb(225, 225, 225)},
        {frgb(170, 170, 170), frgb(170, 170, 170), frgb(170, 170, 170),
         frgb(255, 255, 255), frgb(255, 255, 255), frgb(255, 255, 255), 0, 0, nullptr},
        {frgb(240, 248, 255), frgb(220, 238, 255), frgb(200, 228, 255), frgb(180, 218, 255)},
        {frgb(0, 120, 215), frgb(0, 120, 215), frgb(0, 120, 215),
         frgb(255, 255, 255), frgb(255, 255, 255), frgb(255, 255, 255), 0, 0, nullptr},
        {frgb(210, 210, 210), frgb(200, 200, 200), frgb(190, 190, 190), frgb(180, 180, 180)},
        {frgb(150, 150, 150), frgb(150, 150, 150), frgb(150, 150, 150), 0, 0, 0, 0, 0, nullptr},
        {frgb(0, 140, 235), frgb(0, 120, 215), frgb(0, 100, 195), frgb(0, 80, 175)},
        {frgb(0, 60, 155), frgb(0, 60, 155), frgb(0, 60, 155),
         frgb(100, 180, 255), frgb(100, 180, 255), frgb(100, 180, 255), 0, 0, nullptr},
        {frgb(180, 180, 180), frgb(180, 180, 180), frgb(180, 180, 180), 0, 0, 0, 0, 0, nullptr},
        {frgb(150, 150, 150), frgb(150, 150, 150), frgb(150, 150, 150), 0, 0, 0, 0, 0, nullptr},
        frgb(255, 255, 255),
        {frgb(180, 180, 180), frgb(180, 180, 180), frgb(180, 180, 180), 0, 0, 0, 0, 0, nullptr},
        frgb(100, 180, 255),
        frgb(30, 30, 30),
        frgb(150, 150, 150),
        frgb(0, 120, 215),
        frgb(30, 30, 30),
    };

    inline ThemePalette FUTURISTIC_DARK_PALETTE = {
        frgb(20, 20, 35),
        frgb(30, 30, 45),
        frgb(230, 230, 240),
        frgb(80, 80, 100),
        frgb(0, 255, 255),
        frgb(255, 0, 128),
        frgb(0, 255, 128),
        frgb(30, 30, 45),
        frgb(40, 40, 55),
        frgb(230, 230, 240),
        {frgb(50, 50, 70), frgb(40, 40, 60), frgb(35, 35, 55), frgb(30, 30, 50)},
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr},
        {frgb(90, 90, 130), frgb(70, 70, 110), frgb(65, 65, 105), frgb(55, 55, 95)},
        {frgb(0, 255, 255), frgb(0, 255, 255), frgb(0, 255, 255), 0, 0, 0, 0, 0, nullptr},
        {frgb(15, 15, 30), frgb(25, 25, 40), frgb(25, 25, 40), frgb(35, 35, 55)},
        {frgb(0, 255, 255), frgb(0, 255, 255), frgb(0, 255, 255), 0, 0, 0, 0, 0, nullptr},
        {frgb(255, 0, 128), frgb(128, 128, 255), frgb(128, 128, 255), frgb(0, 255, 255)},
        {frgb(0, 200, 200), frgb(0, 200, 200), frgb(0, 200, 200), 0, 0, 0, 0, 0, nullptr},
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr},
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr},
        frgb(20, 20, 35),
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr},
        frgb(255, 0, 255),
        frgb(230, 230, 240),
        frgb(80, 80, 100),
        frgb(80, 80, 100),
        frgb(230, 230, 240),
    };

    inline ThemePalette FUTURISTIC_DARK_ORANGE_PALETTE = {
        frgb(20, 20, 35),    // bg_main
        frgb(30, 30, 45),    // bg_sec
        frgb(230, 230, 240), // fg_main
        frgb(80, 80, 100),   // inactive
        frgb(255, 150, 50),  // selection
        frgb(255, 0, 128),   // error_main
        frgb(0, 255, 128),   // success_main
        frgb(30, 30, 45),    // free_bg
        frgb(40, 40, 55),    // tt_bg
        frgb(230, 230, 240), // tt_fg
        {frgb(50, 50, 70), frgb(40, 40, 60), frgb(35, 35, 55), frgb(30, 30, 50)},
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr},
        {frgb(90, 90, 130), frgb(70, 70, 110), frgb(65, 65, 105), frgb(55, 55, 95)},
        {frgb(255, 150, 50), frgb(255, 150, 50), frgb(255, 150, 50), 0, 0, 0, 0, 0, nullptr},
        {frgb(15, 15, 30), frgb(25, 25, 40), frgb(25, 25, 40), frgb(35, 35, 55)},
        {frgb(255, 150, 50), frgb(255, 150, 50), frgb(255, 150, 50), 0, 0, 0, 0, 0, nullptr},
        {frgb(255, 0, 128), frgb(255, 75, 90), frgb(255, 75, 90), frgb(255, 150, 50)},
        {frgb(220, 120, 40), frgb(220, 120, 40), frgb(220, 120, 40), 0, 0, 0, 0, 0, nullptr},
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr},
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr},
        frgb(20, 20, 35),
        {frgb(80, 80, 100), frgb(80, 80, 100), frgb(80, 80, 100), 0, 0, 0, 0, 0, nullptr},
        frgb(50, 50, 255),
        frgb(230, 230, 240),
        frgb(80, 80, 100),
        frgb(80, 80, 100),
        frgb(230, 230, 240),
    };

    inline ThemePalette SLATE_TEAL_PALETTE = {
        frgb(49, 63, 64),
        frgb(44, 54, 56),
        frgb(160, 174, 173),
        frgb(80, 95, 96),
        frgb(107, 178, 160),
        frgb(224, 108, 117),
        frgb(152, 195, 121),
        frgb(44, 54, 56),
        frgb(35, 45, 46),
        frgb(230, 240, 240),
        {frgb(60, 75, 76), frgb(55, 70, 71), frgb(49, 63, 64), frgb(45, 59, 60)},
        {frgb(80, 95, 96), frgb(80, 95, 96), frgb(80, 95, 96), 0, 0, 0, 0, 0, nullptr},
        {frgb(70, 85, 86), frgb(65, 80, 81), frgb(59, 73, 74), frgb(55, 69, 70)},
        {frgb(107, 178, 160), frgb(107, 178, 160), frgb(107, 178, 160), 0, 0, 0, 0, 0, nullptr},
        {frgb(40, 50, 51), frgb(45, 55, 56), frgb(45, 55, 56), frgb(49, 63, 64)},
        {frgb(107, 178, 160), frgb(107, 178, 160), frgb(107, 178, 160), 0, 0, 0, 0, 0, nullptr},
        {frgb(127, 198, 180), frgb(107, 178, 160), frgb(97, 168, 150), frgb(87, 158, 140)},
        {frgb(77, 148, 130), frgb(77, 148, 130), frgb(77, 148, 130), 0, 0, 0, 0, 0, nullptr},
        {frgb(80, 95, 96), frgb(80, 95, 96), frgb(80, 95, 96), 0, 0, 0, 0, 0, nullptr},
        {frgb(80, 95, 96), frgb(80, 95, 96), frgb(80, 95, 96), 0, 0, 0, 0, 0, nullptr},
        frgb(38, 49, 50),
        {frgb(80, 95, 96), frgb(80, 95, 96), frgb(80, 95, 96), 0, 0, 0, 0, 0, nullptr},
        frgb(255, 123, 0),
        frgb(160, 174, 173),
        frgb(80, 95, 96),
        frgb(107, 178, 160),
        frgb(160, 174, 173),
    };
    inline ThemePalette PRO_STUDIO_PALETTE = {
        frgb(30, 30, 30),    // bg_main
        frgb(37, 37, 38),    // bg_sec
        frgb(204, 204, 204), // fg_main
        frgb(104, 104, 104), // inactive
        frgb(0, 122, 204),   // selection
        frgb(244, 71, 71),   // error_main
        frgb(75, 181, 67),   // success_main
        frgb(30, 30, 30),    // free_bg
        frgb(37, 37, 38),    // tt_bg
        frgb(204, 204, 204), // tt_fg
        {frgb(60, 60, 60), frgb(60, 60, 60), frgb(50, 50, 50), frgb(50, 50, 50)},
        {frgb(45, 45, 45), frgb(45, 45, 45), frgb(45, 45, 45), 0, 0, 0, 0, 0, nullptr},
        {frgb(75, 75, 75), frgb(75, 75, 75), frgb(65, 65, 65), frgb(65, 65, 65)},
        {frgb(45, 45, 45), frgb(45, 45, 45), frgb(45, 45, 45), 0, 0, 0, 0, 0, nullptr},
        {frgb(40, 40, 40), frgb(40, 40, 40), frgb(40, 40, 40), frgb(40, 40, 40)},
        {frgb(0, 122, 204), frgb(0, 122, 204), frgb(0, 122, 204), 0, 0, 0, 0, 0, nullptr},
        {frgb(0, 122, 204), frgb(0, 122, 204), frgb(0, 102, 184), frgb(0, 102, 184)},
        {frgb(0, 90, 158), frgb(0, 90, 158), frgb(0, 90, 158), 0, 0, 0, 0, 0, nullptr},
        {frgb(60, 60, 60), frgb(60, 60, 60), frgb(60, 60, 60), 0, 0, 0, 0, 0, nullptr},
        {frgb(60, 60, 60), frgb(60, 60, 60), frgb(60, 60, 60), 0, 0, 0, 0, 0, nullptr},
        frgb(25, 25, 25), // input_bg
        {frgb(60, 60, 60), frgb(60, 60, 60), frgb(60, 60, 60), 0, 0, 0, 0, 0, nullptr},
        frgb(100, 100, 140),   // focus_ring
        frgb(204, 204, 204), // toggle_tick
        frgb(104, 104, 104), // toggle_border
        frgb(37, 37, 38),    // slider_track
        frgb(204, 204, 204)  // spinner_arrow
    };
    inline ThemePalette NORD_MINIMAL_PALETTE = {
        frgb(46, 52, 64),    // bg_main
        frgb(59, 66, 82),    // bg_sec
        frgb(216, 222, 233), // fg_main
        frgb(76, 86, 106),   // inactive
        frgb(136, 192, 208), // selection
        frgb(191, 97, 106),  // error_main
        frgb(163, 190, 140), // success_main
        frgb(46, 52, 64),    // free_bg
        frgb(59, 66, 82),    // tt_bg
        frgb(216, 222, 233), // tt_fg
        {frgb(67, 76, 94), frgb(67, 76, 94), frgb(59, 66, 82), frgb(59, 66, 82)},
        {frgb(46, 52, 64), frgb(46, 52, 64), frgb(46, 52, 64), 0, 0, 0, 0, 0, nullptr},
        {frgb(76, 86, 106), frgb(76, 86, 106), frgb(67, 76, 94), frgb(67, 76, 94)},
        {frgb(46, 52, 64), frgb(46, 52, 64), frgb(46, 52, 64), 0, 0, 0, 0, 0, nullptr},
        {frgb(46, 52, 64), frgb(46, 52, 64), frgb(46, 52, 64), frgb(46, 52, 64)},
        {frgb(136, 192, 208), frgb(136, 192, 208), frgb(136, 192, 208), 0, 0, 0, 0, 0, nullptr},
        {frgb(129, 161, 193), frgb(129, 161, 193), frgb(129, 161, 193), frgb(129, 161, 193)},
        {frgb(94, 129, 172), frgb(94, 129, 172), frgb(94, 129, 172), 0, 0, 0, 0, 0, nullptr},
        {frgb(67, 76, 94), frgb(67, 76, 94), frgb(67, 76, 94), 0, 0, 0, 0, 0, nullptr},
        {frgb(67, 76, 94), frgb(67, 76, 94), frgb(67, 76, 94), 0, 0, 0, 0, 0, nullptr},
        frgb(46, 52, 64), // input_bg
        {frgb(67, 76, 94), frgb(67, 76, 94), frgb(67, 76, 94), 0, 0, 0, 0, 0, nullptr},
        frgb(94, 129, 172),  // focus_ring
        frgb(216, 222, 233), // toggle_tick
        frgb(76, 86, 106),   // toggle_border
        frgb(59, 66, 82),    // slider_track
        frgb(216, 222, 233)  // spinner_arrow
    };
    inline ThemePalette CLEAN_LIGHT_PALETTE = {
        frgb(249, 250, 251), // bg_main
        frgb(255, 255, 255), // bg_sec
        frgb(17, 24, 39),    // fg_main
        frgb(156, 163, 175), // inactive
        frgb(76, 126, 236),  // selection
        frgb(220, 38, 38),   // error_main
        frgb(22, 163, 74),   // success_main
        frgb(249, 250, 251), // free_bg
        frgb(255, 255, 255), // tt_bg
        frgb(17, 24, 39),    // tt_fg
        {frgb(255, 255, 255), frgb(255, 255, 255), frgb(243, 244, 246), frgb(243, 244, 246)},
        {frgb(209, 213, 219), frgb(209, 213, 219), frgb(209, 213, 219), 0, 0, 0, 0, 0, nullptr},
        {frgb(249, 250, 251), frgb(249, 250, 251), frgb(229, 231, 235), frgb(229, 231, 235)},
        {frgb(156, 163, 175), frgb(156, 163, 175), frgb(156, 163, 175), 0, 0, 0, 0, 0, nullptr},
        {frgb(229, 231, 235), frgb(229, 231, 235), frgb(229, 231, 235), frgb(229, 231, 235)},
        {frgb(37, 99, 235), frgb(37, 99, 235), frgb(37, 99, 235), 0, 0, 0, 0, 0, nullptr},
        {frgb(59, 130, 246), frgb(59, 130, 246), frgb(37, 99, 235), frgb(37, 99, 235)},
        {frgb(29, 78, 216), frgb(29, 78, 216), frgb(29, 78, 216), 0, 0, 0, 0, 0, nullptr},
        {frgb(229, 231, 235), frgb(229, 231, 235), frgb(229, 231, 235), 0, 0, 0, 0, 0, nullptr},
        {frgb(209, 213, 219), frgb(209, 213, 219), frgb(209, 213, 219), 0, 0, 0, 0, 0, nullptr},
        frgb(255, 255, 255), // input_bg
        {frgb(209, 213, 219), frgb(209, 213, 219), frgb(209, 213, 219), 0, 0, 0, 0, 0, nullptr},
        frgb(0, 0, 0),  // focus_ring
        frgb(17, 24, 39),    // toggle_tick
        frgb(156, 163, 175), // toggle_border
        frgb(229, 231, 235), // slider_track
        frgb(17, 24, 39)     // spinner_arrow
    };
    struct NamedPalette
    {
        std::string name;
        ThemePalette palette;
    };

    inline std::vector<NamedPalette> &get_theme_palettes()
    {
        static std::vector<NamedPalette> palettes = {
            {"Blue Light", BLUE_LIGHT_PALETTE},
            {"Futuristic Dark", FUTURISTIC_DARK_PALETTE},
            {"Futuristic Dark Orange", FUTURISTIC_DARK_ORANGE_PALETTE},
            {"Slate Teal", SLATE_TEAL_PALETTE},
            {"Pro Studio", PRO_STUDIO_PALETTE},
            {"Nord Minimal", NORD_MINIMAL_PALETTE},
            {"Clean Light", CLEAN_LIGHT_PALETTE},
        };
        return palettes;
    }

    inline void add_theme_palette(std::string name, const ThemePalette &palette)
    {
        get_theme_palettes().push_back({std::move(name), palette});
    }
}
