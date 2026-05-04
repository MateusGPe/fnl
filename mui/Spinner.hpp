// Spinner.hpp
#pragma once
#include "Policies.hpp"
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <utility>

namespace mui
{
    class Spinner : public policy::HoverTracker<policy::CallbackRouter<Fl_Spinner>>
    {
    protected:
        void draw() override
        {
            if (damage()) 
                draw_box();
            
            fl_push_clip(x(), y(), w(), h());

            // Determine if the group or its children have focus
            bool is_focused = (Fl::focus() == this || contains(Fl::focus()));
            engine::WidgetState state{active_r() != 0, is_hovered, is_focused, 0.0};
            
            // Dispatch the background and outer styling
            engine::dispatch_spinner(x(), y(), w(), h(), state);

            // Supress default FLTK child drawing artifacts
            if (children() >= 3)
            {
                Fl_Widget* inp = child(0);
                inp->box(FL_NO_BOX);
                inp->color(FL_FREE_COLOR); // Prevent input background overdraw
                
                Fl_Widget* up_btn = child(1);
                up_btn->box(FL_NO_BOX);
                up_btn->clear_visible_focus();
                
                Fl_Widget* dn_btn = child(2);
                dn_btn->box(FL_NO_BOX);
                dn_btn->clear_visible_focus();
            }

            // Draw the text and invisible interaction targets
            draw_children();
            
            fl_pop_clip();
        }

    public:
        template <typename... Args>
        Spinner(int x, int y, int w, int h, const char *l = nullptr, Args &&...args)
            : policy::HoverTracker<policy::CallbackRouter<Fl_Spinner>>(x, y, w, h, l, std::forward<Args>(args)...)
        {
            align(FL_ALIGN_CENTER);
            box(FL_NO_BOX);
            color(mui::ThemeManager::get_palette().bg_main);
            selection_color(mui::ThemeManager::get_palette().selection);
            labelcolor(mui::ThemeManager::get_palette().fg_main);
            
            if (children() >= 3)
            {
                // Synchronize child text styling
                Fl_Input *inp = static_cast<Fl_Input *>(child(0));
                inp->textcolor(mui::ThemeManager::get_palette().fg_main);
                inp->cursor_color(mui::ThemeManager::get_palette().selection);
                
                // Erase default FLTK arrows to allow dispatch_spinner to draw them
                child(1)->label("");
                child(2)->label("");
            }
        }
        
        // Ensure manual textcolor changes propagate to the wrapped Fl_Input
        void textcolor(Fl_Color c) 
        {
            Fl_Spinner::textcolor(c);
            if (children() >= 1) 
            {
                static_cast<Fl_Input *>(child(0))->textcolor(c);
            }
        }
        
        Fl_Color textcolor() const
        {
            return Fl_Spinner::textcolor();
        }
    };
}
