#pragma once
#include "Theme.hpp"
#include "Policies.hpp"
#include <FL/Fl_Tree.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Value_Output.H>

namespace mui
{
    class Tree : public policy::HoverTracker<policy::CallbackRouter<Fl_Tree>>
    {
    public:
        Tree(int x, int y, int w, int h, const char *l = nullptr)
            : policy::HoverTracker<policy::CallbackRouter<Fl_Tree>>(x, y, w, h, l)
        {
            const auto &palette = ThemeManager::get_palette();

            connectorstyle(FL_TREE_CONNECTOR_NONE);
            marginleft(10);
            margintop(10);
            linespacing(6);

            item_labelbgcolor(palette.input_bg);
            item_labelfgcolor(palette.fg_main);
        }
    };

    class Browser : public policy::HoverTracker<policy::CallbackRouter<Fl_Browser>>
    {
    public:
        Browser(int x, int y, int w, int h, const char *l = nullptr)
            : policy::HoverTracker<policy::CallbackRouter<Fl_Browser>>(x, y, w, h, l)
        {
            const auto &palette = ThemeManager::get_palette();

            box(Theme::schemes::ROUNDED_INPUT_THIN_DOWN_BOX);
            color(palette.input_bg);
            selection_color(palette.selection);
            textcolor(palette.fg_main);
        }
    };
}