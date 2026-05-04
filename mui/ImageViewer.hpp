#pragma once

#include <thread>
#include <mutex>
#include <sstream>
#include "UILock.hpp"
#include "Dispatcher.hpp"
#include "ImageViewer/Internal.hpp"
#include "NativeFileChooser.hpp"

namespace mui
{
    class ImageViewer : public InternalImageViewer
    {
    public:
        explicit ImageViewer(int x = 0, int y = 0, int w = 0, int h = 0) : InternalImageViewer(x, y, w, h)
        {
        }
        template <typename T, void (T::*Method)()>
        ImageViewer &on_view_change(T *instance)
        {
            user_data_ = instance;
            view_changed_thunk_ = [](void *data)
            { (static_cast<T *>(data)->*Method)(); };
            return *this;
        }

        template <typename T, void (T::*Method)()>
        ImageViewer &on_layer_select(T *instance)
        {
            user_data_ = instance;
            layer_selected_thunk_ = [](void *data)
            { (static_cast<T *>(data)->*Method)(); };
            return *this;
        }

        template <typename T, void (T::*Method)()>
        ImageViewer &on_right_click(T *instance)
        {
            user_data_ = instance;
            right_click_thunk_ = [](void *data)
            { (static_cast<T *>(data)->*Method)(); };
            return *this;
        }

        template <typename T, void (T::*Method)(unsigned char, unsigned char, unsigned char, unsigned char)>
        ImageViewer &on_color_picked(T *instance)
        {
            user_data_ = instance;
            color_picked_thunk_ = [](uchar r, uchar g, uchar b, uchar a, void *data)
            { (static_cast<T *>(data)->*Method)(r, g, b, a); };
            return *this;
        }

        ImageViewer &export_image()
        {
            NativeFileChooser chooser(NativeFileChooser::SaveFile);
            chooser.title("Export Flat Image")
                .filter("PPM Image (Raw RGB)\t*.ppm")
                .options(NativeFileChooser::SaveAsConfirm);
            if (chooser.show())
            {
                InternalImageViewer::export_image(chooser.filename());
            }
            return *this;
        }
        template <typename T, void (T::*Method)()>
        ImageViewer &load_layer_async(const std::string &filepath, const std::string &name, double lx, double ly, T *instance)
        {
            std::thread(
                [this, filepath, name, lx, ly, instance]()
                {
                    try
                    {
                        auto img = std::make_shared<Image>(filepath.c_str());
                        Fl_Image *raw_thumb = img->handle()->copy(32, 32);
                        auto thumb = std::make_shared<Image>(raw_thumb);

                        UILock lock;
                        auto new_layer = std::make_shared<ImageLayer>(img, name, lx, ly, int(img->width()), int(img->height()), 1.0, 1.0, 0.0, 1.0, BlendMode::Normal, true, false, thumb);
                        document_->add_layer(new_layer);
                        selected_layer_index_ = document_->layer_count() - 1;
                        invalidate();
                        Dispatcher::awake<T, Method>(instance);
                    }
                    catch (...)
                    {
                    }
                })
                .detach();
            return *this;
        }

        ImageViewer &add_layer(std::shared_ptr<Image> img, const std::string &name = "Layer", double lx = 0, double ly = 0)
        {
            Fl_Image *raw_thumb = img->handle()->copy(32, 32);
            auto thumb = std::make_shared<Image>(raw_thumb);
            auto new_layer = std::make_shared<ImageLayer>(img, name, lx, ly, int(img->width()), int(img->height()), 1.0, 1.0, 0.0, 1.0, BlendMode::Normal, true, false, thumb);
            document_->add_layer(new_layer);
            selected_layer_index_ = document_->layer_count() - 1;
            invalidate();
            return *this;
        }

        ImageViewer &select_layer(int index)
        {
            selected_layer_index_ = std::clamp(index, -1, (int)document_->layer_count() - 1);
            redraw();
            return *this;
        }

        ImageViewer &remove_layer(int index)
        {
            if (index >= 0 && index < (int)document_->layer_count())
            {
                document_->remove_layer(index);
                selected_layer_index_ = -1;
                invalidate();
            }
            return *this;
        }

        ImageViewer &toggle_layer_visibility(int index)
        {
            if (index >= 0 && index < (int)document_->layer_count())
            {
                auto l = std::static_pointer_cast<ImageLayer>(document_->get_layer(index));
                l->visible = !l->visible;
                invalidate();
            }
            return *this;
        }

        ImageViewer &toggle_layer_lock(int index)
        {
            if (index >= 0 && index < (int)document_->layer_count())
            {
                auto l = std::static_pointer_cast<ImageLayer>(document_->get_layer(index));
                l->locked = !l->locked;
                invalidate();
            }
            return *this;
        }

        ImageViewer &move_layer_up(int index)
        {
            if (index >= 0 && index < (int)document_->layer_count() - 1)
            {
                document_->swap_layers(index, index + 1);
                if (selected_layer_index_ == index)
                    selected_layer_index_++;
                else if (selected_layer_index_ == index + 1)
                    selected_layer_index_--;
                invalidate();
            }
            return *this;
        }

        ImageViewer &move_layer_down(int index)
        {
            if (index > 0 && index < (int)document_->layer_count())
            {
                document_->swap_layers(index, index - 1);
                if (selected_layer_index_ == index)
                    selected_layer_index_--;
                else if (selected_layer_index_ == index - 1)
                    selected_layer_index_++;
                invalidate();
            }
            return *this;
        }
        ImageViewer &undo()
        {
            InternalImageViewer::undo();
            return *this;
        }

        ImageViewer &redo()
        {
            InternalImageViewer::redo();
            return *this;
        }
        ImageViewer &flip_layer_horizontal(int index)
        {
            if (index >= 0 && index < (int)document_->layer_count())
            {
                auto l = std::static_pointer_cast<ImageLayer>(document_->get_layer(index));
                auto cmd = std::make_shared<CommandFlip>(index, l->flip_h, l->flip_v, !l->flip_h, l->flip_v);
                push_command(cmd);
            }
            return *this;
        }

        ImageViewer &flip_layer_vertical(int index)
        {
            if (index >= 0 && index < (int)document_->layer_count())
            {
                auto l = std::static_pointer_cast<ImageLayer>(document_->get_layer(index));
                auto cmd = std::make_shared<CommandFlip>(index, l->flip_h, l->flip_v, l->flip_h, !l->flip_v);
                push_command(cmd);
            }
            return *this;
        }

        ImageViewer &rotate_layer(int index, double angle)
        {
            if (index >= 0 && index < (int)document_->layer_count())
            {
                auto l = std::static_pointer_cast<ImageLayer>(document_->get_layer(index));
                auto cmd = std::make_shared<CommandRotate>(index, l->rotation_angle, angle);
                push_command(cmd);
            }
            return *this;
        }
        ImageViewer &layer_opacity(int index, double alpha)
        {
            if (index >= 0 && index < (int)document_->layer_count())
            {
                auto l = std::static_pointer_cast<ImageLayer>(document_->get_layer(index));
                l->alpha = std::clamp(alpha, 0.0, 1.0);
                invalidate();
            }
            return *this;
        }

        ImageViewer &layer_blend_mode(int index, BlendMode mode)
        {
            if (index >= 0 && index < (int)document_->layer_count())
            {
                auto l = std::static_pointer_cast<ImageLayer>(document_->get_layer(index));
                l->blend_mode = mode;
                invalidate();
            }
            return *this;
        }

        ImageViewer &layer_id(int index, int id)
        {
            if (index >= 0 && index < (int)document_->layer_count())
                document_->get_layer(index)->id = id;
            return *this;
        }

        ImageViewer &layer_parent(int index, int parent_id)
        {
            if (index >= 0 && index < (int)document_->layer_count())
            {
                document_->get_layer(index)->parent_id = parent_id;
                invalidate();
            }
            return *this;
        }

        ImageViewer &clear_layers()
        {
            document_->clear_layers();
            selected_layer_index_ = -1;
            invalidate();
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
            if (document_->layer_count() == 0 && document_->mode() == DocumentMode::InfiniteCanvas)
            {
                view_x_ = 0;
                view_y_ = 0;
                scale_ = 1.0;
                redraw();
                notify_view_changed();
                return *this;
            }

            double min_x, min_y, max_x, max_y;
            get_world_bounds(min_x, min_y, max_x, max_y);

            double world_w = max_x - min_x;
            double world_h = max_y - min_y;

            if (world_w > 0 && world_h > 0)
            {
                const double padding = 0.9;
                double scale_x = (w() * padding) / world_w;
                double scale_y = (h() * padding) / world_h;

                scale_ = std::clamp(std::min(scale_x, scale_y), 0.01, 100.0);
                view_x_ = min_x + (world_w * 0.5) - (w() / scale_ * 0.5);
                view_y_ = min_y + (world_h * 0.5) - (h() / scale_ * 0.5);
            }

            clamp_view();
            invalidate();
            notify_view_changed();
            return *this;
        }

        ImageViewer &fit_to_canvas()
        {
            if (document_->mode() == DocumentMode::InfiniteCanvas)
            {
                return fit_all();
            }

            double canvas_w = document_->canvas_width();
            double canvas_h = document_->canvas_height();

            if (canvas_w > 0 && canvas_h > 0)
            {
                if (document_->layer_count() > 0)
                {
                    double min_x = 1e99, min_y = 1e99, max_x = -1e99, max_y = -1e99;
                    for (size_t i = 0; i < document_->layer_count(); ++i)
                    {
                        auto layer = std::static_pointer_cast<ImageLayer>(document_->get_layer(i));
                        Rect2D b = layer->get_effective_bounds();
                        Rect2D rot_b = Transform::get_rotated_bounds(b.x, b.y, b.w, b.h, layer->rotation_angle);

                        min_x = std::min(min_x, rot_b.x);
                        max_x = std::max(max_x, rot_b.x + rot_b.w);
                        min_y = std::min(min_y, rot_b.y);
                        max_y = std::max(max_y, rot_b.y + rot_b.h);
                    }

                    if (min_x < max_x && min_y < max_y)
                    {
                        double current_w = max_x - min_x;
                        double current_h = max_y - min_y;
                        double current_cx = min_x + (current_w * 0.5);
                        double current_cy = min_y + (current_h * 0.5);

                        double target_cx = canvas_w * 0.5;
                        double target_cy = canvas_h * 0.5;

                        double scale_factor = std::min(canvas_w / current_w, canvas_h / current_h);
                        if (scale_factor > 1.0)
                            scale_factor = 1.0;
                        for (size_t i = 0; i < document_->layer_count(); ++i)
                        {
                            auto layer = std::static_pointer_cast<ImageLayer>(document_->get_layer(i));
                            layer->x = target_cx + (layer->x - current_cx) * scale_factor;
                            layer->y = target_cy + (layer->y - current_cy) * scale_factor;
                            layer->scale_x *= scale_factor;
                            layer->scale_y *= scale_factor;
                        }
                    }
                }
                const double padding = 0.9;
                double scale_x = (w() * padding) / canvas_w;
                double scale_y = (h() * padding) / canvas_h;

                scale_ = std::clamp(std::min(scale_x, scale_y), 0.01, 100.0);
                view_x_ = (canvas_w * 0.5) - (w() / scale_ * 0.5);
                view_y_ = (canvas_h * 0.5) - (h() / scale_ * 0.5);
            }

            clamp_view();
            invalidate();
            notify_view_changed();
            return *this;
        }

        ImageViewer &center_all()
        {
            if (document_->layer_count() == 0)
            {
                if (document_->mode() == DocumentMode::InfiniteCanvas)
                {
                    view_x_ = 0;
                    view_y_ = 0;
                }
                else
                {
                    view_x_ = (document_->canvas_width() * 0.5) - (w() / scale_ * 0.5);
                    view_y_ = (document_->canvas_height() * 0.5) - (h() / scale_ * 0.5);
                }
                clamp_view();
                invalidate();
                notify_view_changed();
                return *this;
            }

            double target_cx = 0.0;
            double target_cy = 0.0;

            if (document_->mode() == DocumentMode::FixedCanvas)
            {
                target_cx = document_->canvas_width() * 0.5;
                target_cy = document_->canvas_height() * 0.5;
            }

            for (size_t i = 0; i < document_->layer_count(); ++i)
            {
                auto layer = std::static_pointer_cast<ImageLayer>(document_->get_layer(i));
                Rect2D b = layer->get_effective_bounds();
                Rect2D rot_b = Transform::get_rotated_bounds(b.x, b.y, b.w, b.h, layer->rotation_angle);

                double layer_cx = rot_b.x + (rot_b.w * 0.5);
                double layer_cy = rot_b.y + (rot_b.h * 0.5);

                layer->x += (target_cx - layer_cx);
                layer->y += (target_cy - layer_cy);
            }

            view_x_ = target_cx - (w() / scale_ * 0.5);
            view_y_ = target_cy - (h() / scale_ * 0.5);

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
            document_->mode(mode);
            clamp_view();
            invalidate();
            return *this;
        }

        ImageViewer &canvas_size(int w, int h)
        {
            document_->canvas_size(w, h);
            if (document_->mode() == DocumentMode::FixedCanvas)
            {
                clamp_view();
                invalidate();
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

        std::string export_json() const
        {
            std::stringstream ss;
            ss << "[\n";
            for (size_t i = 0; i < document_->layer_count(); ++i)
            {
                auto l = std::static_pointer_cast<ImageLayer>(document_->get_layer(i));
                ss << "  {\"name\":\"" << l->name << "\", \"x\":" << l->x << ", \"y\":" << l->y
                   << ", \"sx\":" << l->scale_x << ", \"sy\":" << l->scale_y << "}";
                if (i < document_->layer_count() - 1)
                    ss << ",";
                ss << "\n";
            }
            ss << "]\n";
            return ss.str();
        }
        double scale() const { return scale_; }
        double view_x() const { return view_x_; }
        double view_y() const { return view_y_; }
        int selected_layer() const { return selected_layer_index_; }
        std::shared_ptr<ImageDocument> document() const { return document_; }
        InternalImageViewer *internal_ptr() { return this; }

        void get_world_bounds(double &min_x, double &min_y, double &max_x, double &max_y)
        {
            InternalImageViewer::get_world_bounds(min_x, min_y, max_x, max_y);
        }

        ImageViewer &view_position(double x, double y)
        {
            view_x_ = x;
            view_y_ = y;
            clamp_view();
            invalidate();
            return *this;
        }
    };
}