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
        class ToggleDraw : public FlBase
        {
        protected:
            void draw() override
            {
                fl_push_clip(this->x(), this->y(), this->w(), this->h());
                engine::WidgetState state{this->active_r() != 0, false, Fl::focus() == this, 0.0};
                if constexpr (requires { this->is_hovered; })
                {
                    state.hovered = this->is_hovered;
                }
                if constexpr (requires { this->value(); })
                {
                    if constexpr (std::is_convertible_v<decltype(this->value()), double>)
                    {
                        state.value = static_cast<double>(this->value());
                    }
                    else if constexpr (std::is_pointer_v<decltype(this->value())>)
                    {
                        state.value = this->value() ? 1.0 : 0.0;
                    }
                }
                engine::dispatch_toggle(this->x(), this->y(), this->w(), this->h(), state);
                fl_color(this->active_r() ? this->labelcolor() : fl_inactive(this->labelcolor()));
                this->draw_label();
                fl_pop_clip();
            }

        public:
            template <typename... Args>
            ToggleDraw(Args &&...args) : FlBase(std::forward<Args>(args)...) {}
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

        template <typename FlBase>
        class CheckButtonDraw : public FlBase
        {
        protected:
            void draw() override
            {
                if (this->damage())
                    this->draw_box();

                fl_push_clip(this->x(), this->y(), this->w(), this->h());
                engine::WidgetState state{this->active_r() != 0, false, Fl::focus() == this, 0.0};
                if constexpr (requires { this->is_hovered; })
                    state.hovered = this->is_hovered;

                if constexpr (requires { this->value(); })
                {
                    if constexpr (std::is_convertible_v<decltype(this->value()), double>)
                        state.value = static_cast<double>(this->value());
                    else if constexpr (std::is_pointer_v<decltype(this->value())>)
                        state.value = this->value() ? 1.0 : 0.0;
                }

                engine::dispatch_checkbox(this->x(), this->y(), this->w(), this->h(), state);
                if (this->label())
                {
                    int bx = this->x() + mui::ThemeManager::get_palette().metrics.checkbox_focus_padding + (mui::ThemeManager::get_palette().metrics.checkbox_focus_width / 2);
                    int label_x = bx + mui::ThemeManager::get_palette().metrics.checkbox_size + mui::ThemeManager::get_palette().metrics.checkbox_label_padding;
                    fl_color(this->active_r() ? this->labelcolor() : fl_inactive(this->labelcolor()));
                    fl_font(this->labelfont(), this->labelsize());
                    fl_draw(this->label(), label_x, this->y(), this->x() + this->w() - label_x, this->h(), FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
                }
                fl_pop_clip();
            }

        public:
            template <typename... Args>
            CheckButtonDraw(Args &&...args) : FlBase(std::forward<Args>(args)...) {}
        };

        template <typename FlBase>
        class RadioButtonDraw : public FlBase
        {
        protected:
            void draw() override
            {
                if (this->damage())
                    this->draw_box();

                fl_push_clip(this->x(), this->y(), this->w(), this->h());
                engine::WidgetState state{this->active_r() != 0, false, Fl::focus() == this, 0.0};
                if constexpr (requires { this->is_hovered; })
                    state.hovered = this->is_hovered;

                if constexpr (requires { this->value(); })
                {
                    if constexpr (std::is_convertible_v<decltype(this->value()), double>)
                        state.value = static_cast<double>(this->value());
                    else if constexpr (std::is_pointer_v<decltype(this->value())>)
                        state.value = this->value() ? 1.0 : 0.0;
                }

                engine::dispatch_radio_button(this->x(), this->y(), this->w(), this->h(), state);
                if (this->label())
                {
                    int radio_x = this->x() + mui::ThemeManager::get_palette().metrics.radio_focus_padding;
                    int label_x = radio_x + mui::ThemeManager::get_palette().metrics.radio_size + mui::ThemeManager::get_palette().metrics.radio_label_padding;
                    fl_color(this->active_r() ? this->labelcolor() : fl_inactive(this->labelcolor()));
                    fl_font(this->labelfont(), this->labelsize());
                    fl_draw(this->label(), label_x, this->y(), this->x() + this->w() - label_x, this->h(), FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
                }
                fl_pop_clip();
            }

        public:
            template <typename... Args>
            RadioButtonDraw(Args &&...args) : FlBase(std::forward<Args>(args)...) {}
        };

        template <typename FlBase>
        class SliderDraw : public FlBase
        {
        protected:
            void draw() override
            {
                if (this->damage())
                    this->draw_box();

                fl_push_clip(this->x(), this->y(), this->w(), this->h());
                engine::WidgetState state{this->active_r() != 0, false, Fl::focus() == this, 0.0};
                if constexpr (requires { this->is_hovered; })
                    state.hovered = this->is_hovered;

                if constexpr (requires { this->value(); this->minimum(); this->maximum(); })
                {
                    double val = static_cast<double>(this->value());
                    double min_v = static_cast<double>(this->minimum());
                    double max_v = static_cast<double>(this->maximum());
                    if (max_v > min_v)
                        state.value = std::max(0.0, std::min(1.0, (val - min_v) / (max_v - min_v)));
                    else
                        state.value = 0.0;
                }

                engine::dispatch_slider(this->x(), this->y(), this->w(), this->h(), state);
                fl_pop_clip();
            }

        public:
            template <typename... Args>
            SliderDraw(Args &&...args) : FlBase(std::forward<Args>(args)...) {}
        };

        template <typename FlBase>
        class ChoiceDraw : public FlBase
        {
        protected:
            void draw() override
            {
                fl_push_clip(this->x(), this->y(), this->w(), this->h());
                engine::WidgetState state{this->active_r() != 0, false, Fl::focus() == this, 0.0};
                if constexpr (requires { this->is_hovered; })
                    state.hovered = this->is_hovered;

                engine::dispatch_choice(this->x(), this->y(), this->w(), this->h(), state, false);

                const auto *v = this->mvalue();
                const char *txt = v ? v->text : "";
                if (txt && *txt)
                {
                    fl_color(this->active_r() ? this->textcolor() : fl_inactive(this->textcolor()));
                    fl_font(this->textfont(), this->textsize());
                    int text_x = this->x() + mui::ThemeManager::get_palette().metrics.choice_text_padding;
                    int text_w = this->w() - (mui::ThemeManager::get_palette().metrics.choice_text_padding * 2 + mui::ThemeManager::get_palette().metrics.choice_arrow_padding + mui::ThemeManager::get_palette().metrics.choice_arrow_size);

                    int old_shortcut = fl_draw_shortcut;
                    fl_draw_shortcut = 2;
                    fl_draw(txt, text_x, this->y(), text_w, this->h(), this->align() | FL_ALIGN_INSIDE);
                    fl_draw_shortcut = old_shortcut;
                }

                fl_pop_clip();
            }

        public:
            template <typename... Args>
            ChoiceDraw(Args &&...args) : FlBase(std::forward<Args>(args)...) {}
        };

        template <typename FlBase>
        class TabsDraw : public FlBase
        {
        protected:
            void draw() override
            {
                if (this->damage())
                    this->draw_box();
                fl_push_clip(this->x(), this->y(), this->w(), this->h());
                engine::WidgetState state{this->active_r() != 0, false, Fl::focus() == this, 0.0};
                if constexpr (requires { this->is_hovered; })
                    state.hovered = this->is_hovered;

                FlBase::draw();
                engine::dispatch_tabs(this->x(), this->y(), this->w(), this->h(), state);
                fl_pop_clip();
            }

        public:
            template <typename... Args>
            TabsDraw(Args &&...args) : FlBase(std::forward<Args>(args)...) {}
        };

        template <typename FlBase>
        class ProgressDraw : public FlBase
        {
        protected:
            void draw() override
            {
                if (this->damage())
                    this->draw_box();
                fl_push_clip(this->x(), this->y(), this->w(), this->h());
                engine::WidgetState state{this->active_r() != 0, false, Fl::focus() == this, 0.0};
                if constexpr (requires { this->is_hovered; })
                    state.hovered = this->is_hovered;

                if constexpr (requires { this->value(); this->minimum(); this->maximum(); })
                {
                    double val = static_cast<double>(this->value());
                    double min_v = static_cast<double>(this->minimum());
                    double max_v = static_cast<double>(this->maximum());
                    if (max_v > min_v)
                        state.value = std::max(0.0, std::min(1.0, (val - min_v) / (max_v - min_v)));
                    else
                        state.value = 0.0;
                }

                engine::dispatch_progress(this->x(), this->y(), this->w(), this->h(), state);
                fl_color(this->active_r() ? this->labelcolor() : fl_inactive(this->labelcolor()));
                this->draw_label();
                fl_pop_clip();
            }

        public:
            template <typename... Args>
            ProgressDraw(Args &&...args) : FlBase(std::forward<Args>(args)...) {}
        };

        template <typename FlBase>
        class OutputDraw : public FlBase
        {
        protected:
            void draw() override
            {
                if (this->damage())
                    this->damage(FL_DAMAGE_ALL);
                fl_push_clip(this->x(), this->y(), this->w(), this->h());
                engine::WidgetState state{this->active_r() != 0, false, Fl::focus() == this, 0.0};
                if constexpr (requires { this->is_hovered; })
                    state.hovered = this->is_hovered;

                engine::dispatch_input(this->x(), this->y(), this->w(), this->h(), state);

                FlBase::draw();
                fl_pop_clip();
            }

        public:
            template <typename... Args>
            OutputDraw(Args &&...args) : FlBase(std::forward<Args>(args)...) {}
        };

        template <typename FlBase>
        class MultilineOutputDraw : public FlBase
        {
        protected:
            void draw() override
            {
                if (this->damage())
                    this->damage(FL_DAMAGE_ALL);
                fl_push_clip(this->x(), this->y(), this->w(), this->h());
                engine::WidgetState state{this->active_r() != 0, false, Fl::focus() == this, 0.0};
                if constexpr (requires { this->is_hovered; })
                    state.hovered = this->is_hovered;

                engine::dispatch_input(this->x(), this->y(), this->w(), this->h(), state);

                FlBase::draw();
                fl_pop_clip();
            }

        public:
            template <typename... Args>
            MultilineOutputDraw(Args &&...args) : FlBase(std::forward<Args>(args)...) {}
        };

        template <typename FlBase>
        class MenuBarDraw : public FlBase
        {
        protected:
            void draw() override
            {
                if (this->damage())
                    this->damage(FL_DAMAGE_ALL);
                fl_push_clip(this->x(), this->y(), this->w(), this->h());
                engine::WidgetState state{this->active_r() != 0, false, Fl::focus() == this, 0.0};
                if constexpr (requires { this->is_hovered; })
                    state.hovered = this->is_hovered;

                FlBase::draw();
                fl_pop_clip();
            }

        public:
            template <typename... Args>
            MenuBarDraw(Args &&...args) : FlBase(std::forward<Args>(args)...) {}
        };

        template <typename FlBase>
        class ValueSliderDraw : public FlBase
        {
        protected:
            void draw() override
            {
                if (this->damage())
                    this->draw_box();
                
                fl_push_clip(this->x(), this->y(), this->w(), this->h());
                engine::WidgetState state{this->active_r() != 0, false, Fl::focus() == this, 0.0};
                if constexpr (requires { this->is_hovered; })
                    state.hovered = this->is_hovered;

                if constexpr (requires { this->value(); this->minimum(); this->maximum(); })
                {
                    double val = static_cast<double>(this->value());
                    double min_v = static_cast<double>(this->minimum());
                    double max_v = static_cast<double>(this->maximum());
                    if (max_v > min_v)
                        state.value = std::max(0.0, std::min(1.0, (val - min_v) / (max_v - min_v)));
                    else
                        state.value = 0.0;
                }

                bool is_horiz = (this->w() > this->h());
                int gap = mui::ThemeManager::get_palette().metrics.valueslider_gap;
                int input_size = mui::ThemeManager::get_palette().metrics.valueslider_input_width;
                if (!is_horiz) input_size = mui::ThemeManager::get_palette().metrics.valueslider_input_height;

                int text_x, text_y, text_w, text_h;
                int sl_x, sl_y, sl_w, sl_h;

                if (is_horiz)
                {
                    text_x = this->x();
                    text_y = this->y();
                    text_w = input_size;
                    text_h = this->h();
                    
                    sl_x = this->x() + input_size + gap;
                    sl_y = this->y();
                    sl_w = this->w() - input_size - gap;
                    sl_h = this->h();
                }
                else
                {
                    text_x = this->x();
                    text_y = this->y() + this->h() - input_size;
                    text_w = this->w();
                    text_h = input_size;
                    
                    sl_x = this->x();
                    sl_y = this->y();
                    sl_w = this->w();
                    sl_h = this->h() - input_size - gap;
                }

                // Isolate formatting input backing
                engine::dispatch_input(text_x, text_y, text_w, text_h, state);

                char buffer[128] = {0};
                this->format(buffer);

                fl_color(this->active_r() ? this->textcolor() : fl_inactive(this->textcolor()));
                fl_font(this->textfont(), this->textsize());
                fl_draw(buffer, text_x, text_y, text_w, text_h, FL_ALIGN_CENTER);

                // Render specific isolated slider track
                engine::dispatch_slider(sl_x, sl_y, sl_w, sl_h, state);

                fl_pop_clip();
            }

        public:
            template <typename... Args>
            ValueSliderDraw(Args &&...args) : FlBase(std::forward<Args>(args)...) {}
        };
    }
}