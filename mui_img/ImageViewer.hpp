#pragma once

#include "ImageViewer/Internal.hpp"

namespace mui
{
    class ImageViewer : public InternalImageViewer
    {
    protected:
        void frame_region(double region_x, double region_y,
                          double region_w, double region_h, double padding = 0.9)
        {
            if (region_w <= 0 || region_h <= 0 || w() <= 0 || h() <= 0)
                return;
            double scale_x = (w() * padding) / region_w;
            double scale_y = (h() * padding) / region_h;
            scale_ = std::clamp(std::min(scale_x, scale_y), 0.01, 100.0);
            view_x_ = region_x + region_w * 0.5 - w() / scale_ * 0.5;
            view_y_ = region_y + region_h * 0.5 - h() / scale_ * 0.5;
        }

    public:
        explicit ImageViewer(int x = 0, int y = 0, int w = 0, int h = 0,
                             const char *label = nullptr)
            : InternalImageViewer(x, y, w, h) {}

        template <typename T, void (T::*Method)()>
        ImageViewer &on_view_change(T *instance)
        {
            user_data_ = instance;
            view_changed_thunk_ = [](void *d)
            { (static_cast<T *>(d)->*Method)(); };
            return *this;
        }

        template <typename T, void (T::*Method)()>
        ImageViewer &on_layer_select(T *instance)
        {
            user_data_ = instance;
            layer_selected_thunk_ = [](void *d)
            { (static_cast<T *>(d)->*Method)(); };
            return *this;
        }

        template <typename T, void (T::*Method)()>
        ImageViewer &on_right_click(T *instance)
        {
            user_data_ = instance;
            right_click_thunk_ = [](void *d)
            { (static_cast<T *>(d)->*Method)(); };
            return *this;
        }

        template <typename T,
                  void (T::*Method)(unsigned char, unsigned char, unsigned char, unsigned char)>
        ImageViewer &on_color_picked(T *instance)
        {
            user_data_ = instance;
            color_picked_thunk_ = [](uchar r, uchar g, uchar b, uchar a, void *d)
            {
                (static_cast<T *>(d)->*Method)(r, g, b, a);
            };
            return *this;
        }

        ImageViewer &add_layer(std::shared_ptr<Image> img, const std::string &name = "Layer",
                               double lx = 0, double ly = 0)
        {
            auto thumb = std::make_shared<Image>(img->handle()->copy(32, 32));
            auto layer = std::make_shared<ImageLayer>(
                img, name, lx, ly,
                int(img->width()), int(img->height()),
                1.0, 1.0, 0.0, 1.0, BlendMode::Normal, true, false, thumb);
            state_->document()->add_layer(layer);
            state_->clear_selection();
            state_->set_primary_selection(layer->id);
            state_->notify_changed();
            return *this;
        }

        ImageViewer &select_layer(int index)
        {
            state_->clear_selection();
            if (index >= 0 && index < (int)state_->document()->layer_count())
                state_->set_primary_selection(state_->document()->get_layer(index)->id);
            return *this;
        }

        ImageViewer &clear_layers()
        {
            state_->document()->clear_layers();
            state_->clear_selection();
            state_->notify_changed();
            return *this;
        }

        ImageViewer &reset_view()
        {
            fit_all();
            scale_ = 1.0;
            clamp_view();
            invalidate();
            notify_view_changed();
            return *this;
        }

        ImageViewer &fit_all()
        {
            if (state_->document()->layer_count() == 0 &&
                state_->document()->mode() == DocumentMode::InfiniteCanvas)
            {
                view_x_ = 0;
                view_y_ = 0;
                scale_ = 1.0;
            }
            else
            {
                double min_x, min_y, max_x, max_y;
                get_world_bounds(min_x, min_y, max_x, max_y);
                frame_region(min_x, min_y, max_x - min_x, max_y - min_y);
            }
            clamp_view();
            invalidate();
            notify_view_changed();
            return *this;
        }

        ImageViewer &fit_to_canvas()
        {
            if (state_->document()->mode() == DocumentMode::InfiniteCanvas)
                return fit_all();
            frame_region(0, 0, state_->document()->canvas_width(), state_->document()->canvas_height());
            clamp_view();
            invalidate();
            notify_view_changed();
            return *this;
        }

        ImageViewer &center_all()
        {
            double cx, cy;
            if (state_->document()->layer_count() == 0)
            {
                cx = (state_->document()->mode() == DocumentMode::InfiniteCanvas) ? 0
                                                                                  : state_->document()->canvas_width() * 0.5;
                cy = (state_->document()->mode() == DocumentMode::InfiniteCanvas) ? 0
                                                                                  : state_->document()->canvas_height() * 0.5;
            }
            else
            {
                double min_x, min_y, max_x, max_y;
                get_world_bounds(min_x, min_y, max_x, max_y);
                cx = min_x + (max_x - min_x) * 0.5;
                cy = min_y + (max_y - min_y) * 0.5;
            }
            view_x_ = cx - w() / scale_ * 0.5;
            view_y_ = cy - h() / scale_ * 0.5;
            clamp_view();
            invalidate();
            notify_view_changed();
            return *this;
        }

        ImageViewer &show_minimap(bool show)
        {
            show_minimap_ = show;
            redraw();
            return *this;
        }
        bool show_minimap() const { return show_minimap_; }

        ImageViewer &active_tool(ViewerTool tool)
        {
            active_tool_ = tool;
            return *this;
        }
        ViewerTool active_tool() const { return active_tool_; }

        ImageViewer &document_mode(DocumentMode mode)
        {
            state_->document()->mode(mode);
            clamp_view();
            state_->notify_changed();
            return *this;
        }

        ImageViewer &canvas_size(int w, int h)
        {
            state_->document()->canvas_size(w, h);
            if (state_->document()->mode() == DocumentMode::FixedCanvas)
            {
                clamp_view();
                state_->notify_changed();
            }
            return *this;
        }

        ImageViewer &grid(bool use_solid, Fl_Color solid_c = FL_WHITE, int grid_size = 20)
        {
            use_solid_bg_ = use_solid;
            solid_bg_color_ = solid_c;
            grid_size_ = grid_size;
            invalidate();
            return *this;
        }

        ImageViewer &snap_to_canvas(bool snap)
        {
            snap_to_canvas_ = snap;
            return *this;
        }
        bool snap_to_canvas() const { return snap_to_canvas_; }

        ImageViewer &view_position(double x, double y)
        {
            view_x_ = x;
            view_y_ = y;
            clamp_view();
            invalidate();
            return *this;
        }

        double scale() const { return scale_; }
        double view_x() const { return view_x_; }
        double view_y() const { return view_y_; }
        int selected_layer() const { return get_selected_layer_index(); }

        std::shared_ptr<ImageDocument> document() const { return state_->document(); }
        InternalImageViewer *internal_ptr() { return this; }

        void get_world_bounds(double &min_x, double &min_y, double &max_x, double &max_y)
        {
            InternalImageViewer::get_world_bounds(min_x, min_y, max_x, max_y);
        }
    };
}