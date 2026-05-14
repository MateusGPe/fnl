#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Input_.H>
#include <FL/Fl_Text_Display.H>
#include "Theme.hpp"
#include "Theme/engine.hpp"
#include <functional>
#include <type_traits>

namespace mui
{
    namespace policy
    {
        template <typename FlBase>
        class CallbackRouter : public FlBase
        {
        protected:
            std::function<void()> m_handler;

        private:
            static void thunk(Fl_Widget *, void *data)
            {
                auto *self = static_cast<CallbackRouter *>(data);
                if (self && self->m_handler)
                    self->m_handler();
            }

        public:
            template <typename... Args>
            explicit CallbackRouter(Args &&...args) : FlBase(std::forward<Args>(args)...)
            {
                this->callback(thunk, this);
            }

            template <typename T, void (T::*Method)()>
            void bind_callback(T *instance)
            {
                m_handler = [instance]
                { (instance->*Method)(); };
            }

            void bind_callback(std::function<void()> handler)
            {
                m_handler = std::move(handler);
            }

            template <typename T, void (T::*Method)()>
            auto &on_change(T *instance)
            {
                bind_callback<T, Method>(instance);
                return *this;
            }

            auto &on_change(std::function<void()> handler)
            {
                bind_callback(std::move(handler));
                return *this;
            }
        };

        template <typename FlBase>
        class Focusable : public FlBase
        {
        public:
            template <typename... Args>
            explicit Focusable(Args &&...args) : FlBase(std::forward<Args>(args)...) {}
            static inline void draw_box_focus(Fl_Boxtype bt, int x, int y, int w, int h)
            {
                switch (bt)
                {
                case FL_DOWN_BOX:
                case FL_DOWN_FRAME:
                case FL_THIN_DOWN_BOX:
                case FL_THIN_DOWN_FRAME:
                    x++;
                    y++;
                    w--;
                    h--;
                    break;
                default:
                    break;
                }
                x += Fl::box_dx(bt);
                y += Fl::box_dy(bt);
                w -= Fl::box_dw(bt);
                h -= Fl::box_dh(bt);

                const auto &palette = ThemeManager::get_palette();
                engine::draw_focus_ring(x, y, w, h, palette.focus_ring, palette.metrics.focus_ring_opacity, palette.metrics.focus_ring_width, palette.metrics.radius);
            }

        protected:
            void draw() override
            {
                const bool is_focused = (Fl::focus() == this && FlBase::visible_focus() && Fl::visible_focus());
                if constexpr (std::is_base_of_v<Fl_Input_, FlBase> || std::is_base_of_v<Fl_Text_Display, FlBase>)
                {
                    FlBase::draw();
                    if (is_focused)
                        draw_box_focus(FL_NO_BOX, FlBase::x(), FlBase::y(), FlBase::w(), FlBase::h());
                }
                else
                {
                    FlBase::visible_focus(false);
                    FlBase::draw();
                    FlBase::visible_focus(is_focused);
                    if (is_focused)
                        draw_box_focus(FlBase::box(), FlBase::x(), FlBase::y(), FlBase::w(), FlBase::h());
                }
            }
        };

        template <typename FlBase>
        class AutoThemed : public FlBase
        {
        public:
            template <typename... Args>
            explicit AutoThemed(Args &&...args) : FlBase(std::forward<Args>(args)...)
            {
                FlBase::box(FL_GTK_UP_BOX);
                if constexpr (requires { FlBase::down_box(FL_NO_BOX); })
                    FlBase::down_box(Theme::schemes::BUTTON_DOWN_BOX);
            }
        };

        template <typename FlBase>
        class HoverTracker : public FlBase
        {
        protected:
            bool is_hovered = false;

            [[nodiscard]] static Fl_Boxtype resolve_hover_box(Fl_Boxtype base) noexcept
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
            explicit HoverTracker(Args &&...args) : FlBase(std::forward<Args>(args)...) {}

            int handle(int event) override
            {
                const int res = FlBase::handle(event);
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
                default:
                    break;
                }
                return res;
            }
        };

        template <typename FlBase>
        class Blinkable : public FlBase
        {
        protected:
            bool cursor_visible = false;

        private:
            void apply_cursor_visible()
            {
                cursor_visible = true;
                if constexpr (requires { this->cursor_color(FL_BLACK); })
                {
                    this->cursor_color(ThemeManager::get_palette().selection);
                    this->damage(FL_DAMAGE_USER1);
                }
                else
                {
                    this->redraw();
                }
            }

            void apply_cursor_hidden()
            {
                cursor_visible = false;
                if constexpr (requires { this->cursor_color(FL_BLACK); })
                {
                    this->cursor_color(ThemeManager::get_palette().input_bg);
                    this->damage(FL_DAMAGE_USER1);
                }
                else
                {
                    this->redraw();
                }
            }

            static void blink_cb(void *data)
            {
                auto *self = static_cast<Blinkable *>(data);
                if (Fl::focus() != self)
                {
                    self->cursor_visible = false;
                    return;
                }
                self->cursor_visible = !self->cursor_visible;
                if (self->cursor_visible)
                    self->apply_cursor_visible();
                else
                    self->apply_cursor_hidden();

                Fl::repeat_timeout(0.5, blink_cb, data);
            }

        protected:
            void restart_blink()
            {
                Fl::remove_timeout(blink_cb, this);
                apply_cursor_visible();
                Fl::add_timeout(0.5, blink_cb, this);
            }

        public:
            template <typename... Args>
            explicit Blinkable(Args &&...args) : FlBase(std::forward<Args>(args)...) {}

            ~Blinkable()
            {
                Fl::remove_timeout(blink_cb, this);
            }

            int handle(int event) override
            {
                const int res = FlBase::handle(event);
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
                    apply_cursor_hidden();
                    break;
                default:
                    break;
                }
                return res;
            }
        };
    }
}
