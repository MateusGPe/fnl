// Tree.hpp
#pragma once
#include "Theme.hpp"
#include "Policies.hpp"
#include <FL/Fl_Tree.H>

namespace mui
{
    class Tree : public policy::HoverTracker<policy::CallbackRouter<Fl_Tree>>
    {
    public:
        Tree(int x, int y, int w, int h, const char *l = nullptr) 
            : policy::HoverTracker<policy::CallbackRouter<Fl_Tree>>(x, y, w, h, l)
        {
            const auto& palette = ThemeManager::get_palette();
            
            // Flatten the box and apply your dark theme inputs
            box(FL_FLAT_BOX);
            color(palette.input_bg);
            selection_color(palette.selection);
            
            // Clean up the archaic rendering
            connectorstyle(FL_TREE_CONNECTOR_NONE); // Removes dotted lines
            marginleft(10);
            margintop(10);
            linespacing(6);
            
            // Override item colors to prevent default white backgrounds
            item_labelbgcolor(palette.input_bg);
            item_labelfgcolor(palette.fg_main);
            
            // Optional: You can load modern SVG chevrons here using your Image.hpp wrapper
            // openicon(...);
            // closeicon(...);
        }
    };
}