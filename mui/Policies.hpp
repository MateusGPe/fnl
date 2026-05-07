#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include "Theme.hpp"
#include "Theme/engine.hpp"
#include <functional>

namespace mui
{
    namespace policy
    {

        template <typename FlBase>
        class CallbackRouter : public FlBase
        {
        protected:
            std::function<void()> m_handler;

            static void thunk(Fl_Widget *w, void *data)
            {
                CallbackRouter *self = static_cast<CallbackRouter *>(data);
                if (self && self->m_handler)
                {
                    self->m_handler();
                }
            }

        public:
            template <typename... Args>
            CallbackRouter(Args &&...args) : FlBase(std::forward<Args>(args)...)
            {
                this->callback(thunk, this);
            }

            template <typename T, void (T::*Method)()>
            void bind_callback(T *instance)
            {
                m_handler = [instance]()
                { (instance->*Method)(); };
            }

            void bind_callback(std::function<void()> handler)
            {
                m_handler = std::move(handler);
            }
        };

        template <typename FlBase>
        class AutoThemed : public FlBase
        {
        public:
            template <typename... Args>
            AutoThemed(Args &&...args) : FlBase(std::forward<Args>(args)...)
            {
                this->box(FL_GTK_UP_BOX);
                if constexpr (requires { this->down_box(FL_NO_BOX); })
                {
                    this->down_box(Theme::schemes::BUTTON_DOWN_BOX);
                }
                this->color(mui::ThemeManager::get_palette().bg_main);
                this->selection_color(mui::ThemeManager::get_palette().bg_sec);
            }
        };

        template <typename FlBase>
        class HoverTracker : public FlBase
        {
        protected:
            bool is_hovered = false;

            static inline Fl_Boxtype resolve_hover_box(Fl_Boxtype base)
            {
                switch (base)
                {
                case FL_GTK_UP_BOX:
                    return FL_PLASTIC_UP_BOX;
                case Theme::schemes::ROUNDED_BUTTON_UP_BOX:
                    return Theme::schemes::ROUNDED_HOVERED_UP_BOX;
                case FL_DIAMOND_UP_BOX:
                    return FL_PLASTIC_THIN_UP_BOX;
                case FL_GTK_THIN_UP_BOX:
                    return FL_PLASTIC_UP_BOX;
                default:
                    return base;
                }
            }

        public:
            template <typename... Args>
            HoverTracker(Args &&...args) : FlBase(std::forward<Args>(args)...) {}

            int handle(int event) override
            {
                int res = FlBase::handle(event);
                switch (event)
                {
                case FL_ENTER:
                    is_hovered = true;
                    this->redraw();
                    break;
                case FL_LEAVE:
                    is_hovered = false;
                    this->redraw();
                    break;
                case FL_FOCUS:
                case FL_UNFOCUS:
                    this->redraw();
                    break;
                }
                return res;
            }
        };

        template <typename FlBase>
        class StandardButtonDraw : public FlBase
        {
        protected:
            void draw() override
            {
                fl_push_clip(this->x(), this->y(), this->w(), this->h());

                Fl_Boxtype draw_b = this->value() ? this->down_box() : this->box();
                Fl_Color draw_c = this->value() ? this->selection_color() : this->color();

                if constexpr (requires { this->is_hovered; })
                {
                    if (!this->value() && this->is_hovered && this->active_r())
                    {
                        Fl_Boxtype hover_b = this->resolve_hover_box(this->box());
                        if (hover_b != this->box())
                        {
                            draw_b = hover_b;
                        }
                        else
                        {
                            draw_c = mui::ThemeManager::get_palette().bg_sec;
                        }
                    }
                }

                fl_draw_box(draw_b, this->x(), this->y(), this->w(), this->h(), draw_c);

                fl_color(this->active_r() ? this->labelcolor() : fl_inactive(this->labelcolor()));
                this->draw_label();

                fl_pop_clip();
            }

        public:
            template <typename... Args>
            StandardButtonDraw(Args &&...args) : FlBase(std::forward<Args>(args)...) {}
        };


        template <typename FlBase>
        class Blinkable : public FlBase
        {
        protected:
            bool cursor_visible = false;
            static void blink_cb(void *data)
            {
                Blinkable *self = static_cast<Blinkable *>(data);
                if (Fl::focus() == self)
                {
                    self->cursor_visible = !self->cursor_visible;
                    if constexpr (requires { self->cursor_color(FL_BLACK); })
                    {
                        self->cursor_color(self->cursor_visible ? mui::ThemeManager::get_palette().selection : mui::ThemeManager::get_palette().input_bg);
                        self->damage(FL_DAMAGE_USER1);
                    }
                    else
                    {
                        self->redraw();
                    }
                    Fl::repeat_timeout(0.5, blink_cb, data);
                }
                else
                {
                    self->cursor_visible = false;
                }
            }

            void restart_blink()
            {
                Fl::remove_timeout(blink_cb, this);
                cursor_visible = true;
                if constexpr (requires { this->cursor_color(FL_BLACK); })
                {
                    this->cursor_color(mui::ThemeManager::get_palette().selection);
                    this->damage(FL_DAMAGE_USER1);
                }
                else
                {
                    this->redraw();
                }
                Fl::add_timeout(0.5, blink_cb, this);
            }

        public:
            template <typename... Args>
            Blinkable(Args &&...args) : FlBase(std::forward<Args>(args)...) {}

            ~Blinkable()
            {
                Fl::remove_timeout(blink_cb, this);
            }

            int handle(int event) override
            {
                int res = FlBase::handle(event);
                switch (event)
                {
                case FL_FOCUS:
                case FL_KEYBOARD:
                case FL_PUSH:
                case FL_DRAG:
                    restart_blink();
                    break;
                case FL_UNFOCUS:
                    Fl::remove_timeout(blink_cb, this);
                    cursor_visible = false;
                    if constexpr (requires { this->cursor_color(FL_BLACK); })
                    {
                        this->cursor_color(mui::ThemeManager::get_palette().input_bg);
                        this->damage(FL_DAMAGE_USER1);
                    }
                    else
                    {
                        this->redraw();
                    }
                    break;
                }
                return res;
            }
        };
    }
}