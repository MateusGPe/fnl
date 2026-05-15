
#pragma once
#include "Theme.hpp"
#include "Dispatcher.hpp"
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <algorithm>
#include <cmath>

namespace mui
{
    class Progress : public Fl_Widget
    {
    private:
        float m_display_value = 0.0f;
        bool m_is_animating = false;
        int m_indeterminate = 0;
        float m_indeterminate_pos = 0.0f;

        float value_ = 0.0f,
              minimum_ = 0.0,
              maximum_ = 100.0,
              step_ = 0.05f, // A value between 0 and 1 for smooth animation
            rate_ = 0.01f;

        void indeterminate_tick()
        {
            m_indeterminate_pos += 0.015f;
            if (m_indeterminate_pos > 1.3f)
                m_indeterminate_pos = -0.3f;
            redraw();
            m_indeterminate = (m_indeterminate + 1) | 1;
            Dispatcher::repeat_timeout<Progress, &Progress::indeterminate_tick>(rate_, this);
        }

        void tick()
        {
            const float target = value_;
            const float diff = target - m_display_value;

            if (std::abs(diff) < step_ || target == maximum_)
            {
                m_display_value = target;
                m_is_animating = false;
            }
            else
            {
                Dispatcher::repeat_timeout<Progress, &Progress::tick>(rate_, this);
                m_display_value += diff * step_;
            }
            redraw();
        }

    protected:
        void draw() override
        {
            const auto &palette = ThemeManager::get_palette();
            fl_push_clip(x(), y(), w(), h());

            if (damage() & ~FL_DAMAGE_CHILD)
            {
                fl_draw_box(FL_FLAT_BOX, x(), y(), w(), h(), color());
            }

            const int track_h = palette.metrics.progress_height;
            const int track_y = y() + (h() - track_h) / 2;
            const int radius = track_h / 2;

            // Draw background track
            fl_color(policy::resolve_color_active(this, palette.bg_sec));
            fl_rounded_rectf(x(), track_y, w(), track_h, radius);

            if (m_indeterminate)
            {
                const float segment_width_ratio = 0.3f;
                const int segment_w = static_cast<int>(w() * segment_width_ratio);
                const int segment_x = x() + static_cast<int>(m_indeterminate_pos * w());

                int clip_x = std::max(x(), segment_x);
                int clip_w = std::min(x() + w(), segment_x + segment_w) - clip_x;

                if (clip_w > 0)
                {
                    const Fl_Color bar_color = lerp_rgb(palette.success_main, palette.selection, palette.success_main, (m_indeterminate % 100) * float(.01f));
                    fl_push_clip(clip_x, track_y, clip_w, track_h);
                    fl_color(policy::resolve_color_active(this, bar_color));
                    fl_rounded_rectf(x(), track_y, w(), track_h, radius);
                    fl_pop_clip();
                }
            }
            else
            {
                const float val = core::normalize_value(m_display_value, minimum(), maximum());
                const int fill_w = static_cast<int>(val * w());
                if (fill_w > 0)
                {
                    const Fl_Color bar_color = lerp_rgb(palette.error_main, palette.selection, palette.success_main, val);
                    fl_push_clip(x(), track_y, fill_w, track_h);
                    fl_color(policy::resolve_color_active(this, bar_color));
                    fl_rounded_rectf(x(), track_y, w(), track_h, radius);
                    fl_pop_clip();
                }
            }

            if (label() && label()[0])
            {
                fl_color(policy::resolve_color_active(this, labelcolor()));
                fl_font(labelfont(), labelsize());
                fl_draw(label(), x(), y(), w(), h(), align());
            }

            fl_pop_clip();
        }

    public:
        Progress(int x, int y, int w, int h, const char *l = nullptr)
            : Fl_Widget(x, y, w, h, l)
        {
            box(FL_FLAT_BOX);
            m_display_value = value_;
        }

        ~Progress()
        {
            if (m_is_animating)
                Dispatcher::remove_timeout<Progress, &Progress::tick>(this);
            if (m_indeterminate)
                Dispatcher::remove_timeout<Progress, &Progress::indeterminate_tick>(this);
        }

        void indeterminate(bool i)
        {
            if ((m_indeterminate != 0) == i)
                return;
            m_indeterminate = i;

            if (m_indeterminate)
            {
                if (m_is_animating)
                {
                    Dispatcher::remove_timeout<Progress, &Progress::tick>(this);
                    m_is_animating = false;
                }
                m_indeterminate_pos = -0.3f;
                Dispatcher::repeat_timeout<Progress, &Progress::indeterminate_tick>(rate_, this);
            }
            else
            {
                Dispatcher::remove_timeout<Progress, &Progress::indeterminate_tick>(this);
                if (value_ != m_display_value && !m_is_animating)
                {
                    m_is_animating = true;
                    Dispatcher::repeat_timeout<Progress, &Progress::tick>(rate_, this);
                }
            }
            redraw();
        }
        bool indeterminate() const { return m_indeterminate != 0; }

        void value(float v)
        {
            value_ = std::clamp(v, minimum_, maximum_);

            if (m_indeterminate)
                return;

            if (value_ != m_display_value && !m_is_animating)
            {
                m_is_animating = true;
                Dispatcher::repeat_timeout<Progress, &Progress::tick>(rate_, this);
            }
        }
        float value() const { return value_; }

        void minimum(float m) { minimum_ = m; }
        float minimum() const { return minimum_; }

        void maximum(float m) { maximum_ = m; }
        float maximum() const { return maximum_; }
        void step(float s) { step_ = s; }
        float step() const { return step_; }
        void rate(float r) { rate_ = r; }
        float rate() const { return rate_; }
    };

}
