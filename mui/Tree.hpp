#pragma once
#include "Theme.hpp"
#include "Policies.hpp"
#include <FL/Fl_Tree.H>
#include <FL/Fl_Browser.H>

namespace mui
{

    class Tree : public policy::HoverTracker<policy::CallbackRouter<Fl_Tree>>
    {
    protected:
        void draw() override
        {
            const auto &palette = ThemeManager::get_palette();
            color(palette.input_bg);
            selection_color(palette.selection);
            item_labelbgcolor(palette.input_bg);
            item_labelfgcolor(palette.fg_main);
            Fl_Tree::draw();
        }

    public:
        Tree(int x=0, int y=0, int w=0, int h=0, const char *l = nullptr)
            : policy::HoverTracker<policy::CallbackRouter<Fl_Tree>>(x, y, w, h, l)
        {
            box(Theme::schemes::ROUNDED_INPUT_THIN_DOWN_BOX);
            connectorstyle(FL_TREE_CONNECTOR_NONE);
            marginleft(10);
            margintop(10);
            linespacing(6);
        }
    };

    class Browser : public policy::HoverTracker<policy::CallbackRouter<Fl_Browser>>
    {
    protected:
        void draw() override
        {
            const auto &palette = ThemeManager::get_palette();
            color(palette.input_bg);
            selection_color(palette.selection);
            textcolor(palette.fg_main);
            Fl_Browser::draw();
        }

    public:
        Browser(int x, int y, int w, int h, const char *l = nullptr)
            : policy::HoverTracker<policy::CallbackRouter<Fl_Browser>>(x, y, w, h, l)
        {
            box(Theme::schemes::ROUNDED_INPUT_THIN_DOWN_BOX);
        }
    };
}
