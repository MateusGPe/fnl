#pragma once

#include "Internal.hpp"
#include <FL/fl_ask.H>
#include <cstdio>

namespace mui
{
    inline void InternalImageViewer::draw()
    {
        if (!bg_buffer_ || off_w_ != w() || off_h_ != h())
        {
            if (bg_buffer_)
                fl_delete_offscreen(bg_buffer_);
            bg_buffer_ = fl_create_offscreen(w(), h());
            off_w_ = w();
            off_h_ = h();
            bg_dirty_ = true;
        }

        const auto &pal = ThemeManager::get_palette();
        if (bg_dirty_ || (!use_solid_bg_ && (last_checker_color1_ != pal.bg_main)))
        {
            fl_begin_offscreen(bg_buffer_);
            draw_background(0, 0, w(), h());
            fl_end_offscreen();
            bg_dirty_ = false;
            last_checker_color1_ = pal.bg_main;
        }

        fl_copy_offscreen(x(), y(), w(), h(), bg_buffer_, 0, 0);

        fl_push_clip(x(), y(), w(), h());
        for (size_t i = 0; i < document_->layer_count(); ++i)
        {
            if (auto l = get_image_layer(i))
            {
                render_layer_to_buffer(*l, i, w(), h(), view_x_, view_y_, scale_);
            }
        }

        draw_overlays(x(), y(), w(), h());

        if (show_minimap_)
        {
            draw_minimap();
        }
        fl_pop_clip();
    }

    inline void InternalImageViewer::clamp_view()
    {
        if (document_->mode() == DocumentMode::FixedCanvas)
        {
            double doc_w = document_->canvas_width();
            double doc_h = document_->canvas_height();
            double view_w = w() / scale_;
            double view_h = h() / scale_;

            view_x_ = (view_w > doc_w) ? (doc_w - view_w) / 2.0 : std::clamp(view_x_, 0.0, doc_w - view_w);
            view_y_ = (view_h > doc_h) ? (doc_h - view_h) / 2.0 : std::clamp(view_y_, 0.0, doc_h - view_h);
        }
    }

    inline bool InternalImageViewer::is_layer_visible(int index) const
    {
        if (index < 0 || index >= (int)document_->layer_count())
            return false;

        auto current_layer = document_->get_layer(index);
        while (current_layer)
        {
            if (!current_layer->visible)
                return false;
            if (current_layer->parent_id == -1)
                break;

            int parent_idx = document_->get_layer_index(current_layer->parent_id);
            if (parent_idx == -1)
                break;
            current_layer = document_->get_layer(parent_idx);
        }
        return true;
    }

    inline void InternalImageViewer::get_world_bounds(double &min_x, double &min_y, double &max_x, double &max_y)
    {
        min_x = 1e9;
        min_y = 1e9;
        max_x = -1e9;
        max_y = -1e9;
        if (document_->layer_count() == 0)
        {
            min_x = 0;
            min_y = 0;
            max_x = (document_->mode() == DocumentMode::FixedCanvas) ? document_->canvas_width() : 0;
            max_y = (document_->mode() == DocumentMode::FixedCanvas) ? document_->canvas_height() : 0;
            return;
        }

        for (size_t i = 0; i < document_->layer_count(); ++i)
        {
            double l_min_x, l_min_y, l_max_x, l_max_y;
            document_->get_layer(i)->get_bounds(l_min_x, l_min_y, l_max_x, l_max_y);
            min_x = std::min(min_x, l_min_x);
            min_y = std::min(min_y, l_min_y);
            max_x = std::max(max_x, l_max_x);
            max_y = std::max(max_y, l_max_y);
        }
    }

    inline int InternalImageViewer::hit_test(double world_x, double world_y)
    {
        for (int i = (int)document_->layer_count() - 1; i >= 0; --i)
        {
            if (is_layer_visible(i) && document_->get_layer(i)->hit_test(world_x, world_y))
            {
                return i;
            }
        }
        return -1;
    }

    inline void InternalImageViewer::sample_color(int mx, int my, double world_x, double world_y)
    {
        if (!Fl::pushed())
            Fl::first_window()->make_current();

        uchar c[4] = {0, 0, 0, 255};
        fl_read_image(c, mx, my, 1, 1, 0);

        if (color_picked_thunk_ && user_data_)
        {
            color_picked_thunk_(c[0], c[1], c[2], 255, user_data_);
        }
    }

    inline void InternalImageViewer::export_image(const char *filepath)
    {
        double min_x, min_y, max_x, max_y;
        get_world_bounds(min_x, min_y, max_x, max_y);
        int img_w = static_cast<int>(std::ceil(max_x - min_x));
        int img_h = static_cast<int>(std::ceil(max_y - min_y));

        if (img_w <= 0 || img_h <= 0)
            return;

        Fl_Offscreen off = fl_create_offscreen(img_w, img_h);
        fl_begin_offscreen(off);

        fl_color(FL_WHITE);
        fl_rectf(0, 0, img_w, img_h);

        for (size_t i = 0; i < document_->layer_count(); ++i)
        {
            if (auto l = get_image_layer(i))
            {
                render_layer_to_buffer(*l, i, img_w, img_h, min_x, min_y, 1.0);
            }
        }

        uchar *pixels = fl_read_image(nullptr, 0, 0, img_w, img_h, 0);
        fl_end_offscreen();
        fl_delete_offscreen(off);

        if (pixels)
        {
            FILE *f = fopen(filepath, "wb");
            if (f)
            {
                fprintf(f, "P6\n%d %d\n255\n", img_w, img_h);
                fwrite(pixels, 1, img_w * img_h * 3, f);
                fclose(f);
            }
            delete[] pixels;
        }
    }
}