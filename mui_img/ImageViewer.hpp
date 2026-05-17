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
    protected:
        void frame_region(double region_x, double region_y, double region_w, double region_h, double padding = 0.9)
        {
            if (region_w <= 0 || region_h <= 0 || w() <= 0 || h() <= 0)
                return;

            double scale_x = (w() * padding) / region_w;
            double scale_y = (h() * padding) / region_h;

            scale_ = std::clamp(std::min(scale_x, scale_y), 0.01, 100.0);
            view_x_ = region_x + (region_w * 0.5) - (w() / scale_ * 0.5);
            view_y_ = region_y + (region_h * 0.5) - (h() / scale_ * 0.5);
        }

    public:
        explicit ImageViewer(int x = 0, int y = 0, int w = 0, int h = 0, const char *label = nullptr) : InternalImageViewer(x, y, w, h)
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
                        selected_layer_id_ = new_layer->id;
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
            selected_layer_id_ = new_layer->id;
            invalidate();
            return *this;
        }

        ImageViewer &select_layer(int index)
        {
            if (index >= 0 && index < (int)document_->layer_count())
            {
                selected_layer_id_ = document_->get_layer(index)->id;
            }
            else
            {
                selected_layer_id_ = -1;
            }
            redraw();
            return *this;
        }

        ImageViewer &remove_layer(int index)
        {
            if (index >= 0 && index < (int)document_->layer_count())
            {
                if (get_selected_layer_index() == index)
                    selected_layer_id_ = -1;
                document_->remove_layer(index);
                invalidate();
            }
            return *this;
        }

        ImageViewer &toggle_layer_visibility(int index)
        {
            if (index >= 0 && index < (int)document_->layer_count())
            {
                if (auto l = get_image_layer(index))
                {
                    push_command(std::make_shared<CommandVisibility>(l->id, l->visible, !l->visible));
                }
            }
            return *this;
        }

        ImageViewer &toggle_layer_lock(int index)
        {
            if (auto l = get_image_layer(index))
            {
                push_command(std::make_shared<CommandLock>(l->id, l->locked, !l->locked));
            }
            return *this;
        }
        ImageViewer &move_layer_up(int index)
        {
            if (index >= 0 && index < (int)document_->layer_count() - 1)
            {
                push_command(std::make_shared<CommandMoveLayer>(index, index + 1));
            }
            return *this;
        }
        ImageViewer &move_layer_down(int index)
        {
            if (index > 0 && index < (int)document_->layer_count())
            {
                push_command(std::make_shared<CommandMoveLayer>(index, index - 1));
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
            if (auto l = get_image_layer(index))
            {
                auto cmd = std::make_shared<CommandFlip>(l->id, l->flip_h, l->flip_v, !l->flip_h, l->flip_v);
                push_command(cmd);
            }
            return *this;
        }
        ImageViewer &flip_layer_vertical(int index)
        {
            if (auto l = get_image_layer(index))
            {
                auto cmd = std::make_shared<CommandFlip>(l->id, l->flip_h, l->flip_v, l->flip_h, !l->flip_v);
                push_command(cmd);
            }
            return *this;
        }
        ImageViewer &rotate_layer(int index, double angle)
        {
            if (auto l = get_image_layer(index))
            {
                auto cmd = std::make_shared<CommandRotate>(l->id, l->rotation_angle, angle);
                push_command(cmd);
            }
            return *this;
        }
        ImageViewer &layer_opacity(int index, double alpha)
        {
            if (auto l = get_image_layer(index))
            {
                push_command(std::make_shared<CommandOpacity>(l->id, l->alpha, std::clamp(alpha, 0.0, 1.0)));
            }
            return *this;
        }
        ImageViewer &layer_blend_mode(int index, BlendMode mode)
        {
            if (auto l = get_image_layer(index))
            {
                push_command(std::make_shared<CommandBlendMode>(l->id, l->blend_mode, mode));
            }
            return *this;
        }

        ImageViewer &crop_layer(int index, double crop_x, double crop_y, double crop_w, double crop_h)
        {
            if (auto l = get_image_layer(index))
            {
                if (!l->image || !l->image->handle())
                    return *this;

                const int img_w = l->image->data_w();
                const int img_h = l->image->data_h();

                double nx = std::clamp(crop_x, 0.0, (double)img_w);
                double ny = std::clamp(crop_y, 0.0, (double)img_h);
                double nw = std::clamp(crop_w, 0.0, (double)img_w - nx);
                double nh = std::clamp(crop_h, 0.0, (double)img_h - ny);

                if (nw < 1.0 || nh < 1.0)
                    return *this;

                double ox = (l->crop_w >= 0) ? l->crop_x : 0.0;
                double oy = (l->crop_h >= 0) ? l->crop_y : 0.0;

                double dx = (ox - nx) * l->scale_x;
                double dy = (oy - ny) * l->scale_y;

                Point2D world_offset = Transform::local_to_world(dx, dy, 0, 0, l->rotation_angle, l->flip_h, l->flip_v);
                auto cmd = std::make_shared<CommandCrop>(l->id, l->crop_x, l->crop_y, l->crop_w, l->crop_h, nx, ny, nw, nh, l->x, l->y, l->x + world_offset.x, l->y + world_offset.y);
                push_command(cmd);
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
                push_command(std::make_shared<CommandParent>(document_->get_layer(index)->id, document_->get_layer(index)->parent_id, parent_id));
            }
            return *this;
        }

        ImageViewer &clear_layers()
        {
            document_->clear_layers();
            selected_layer_id_ = -1;
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
            if (document_->mode() == DocumentMode::InfiniteCanvas)
            {
                return fit_all();
            }

            frame_region(0, 0, document_->canvas_width(), document_->canvas_height());

            clamp_view();
            invalidate();
            notify_view_changed();
            return *this;
        }

        ImageViewer &center_all()
        {
            double target_cx, target_cy;

            if (document_->layer_count() == 0)
            {
                if (document_->mode() == DocumentMode::InfiniteCanvas)
                {
                    target_cx = 0.0;
                    target_cy = 0.0;
                }
                else
                {
                    target_cx = document_->canvas_width() * 0.5;
                    target_cy = document_->canvas_height() * 0.5;
                }
            }
            else
            {
                double min_x, min_y, max_x, max_y;
                get_world_bounds(min_x, min_y, max_x, max_y);
                target_cx = min_x + (max_x - min_x) * 0.5;
                target_cy = min_y + (max_y - min_y) * 0.5;
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
                if (auto l = get_image_layer(i))
                {
                    ss << "  {\"name\":\"" << l->name << "\", \"x\":" << l->x << ", \"y\":" << l->y
                       << ", \"sx\":" << l->scale_x << ", \"sy\":" << l->scale_y << "}";
                }
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
        int selected_layer() const { return get_selected_layer_index(); }
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