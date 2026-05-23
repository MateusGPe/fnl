#pragma once

#include <thread>
#include <mutex>
#include <sstream>
#include "UILock.hpp"
#include "Dispatcher.hpp"
#include "ImageViewer.hpp"
#include "NativeFileChooser.hpp"

namespace mui
{
    class AdvancedImageViewer : public ImageViewer
    {
    public:
        explicit AdvancedImageViewer(int x = 0, int y = 0, int w = 0, int h = 0,
                                     const char *label = nullptr)
            : ImageViewer(x, y, w, h, label) {}

        AdvancedImageViewer &export_image()
        {
            NativeFileChooser chooser(NativeFileChooser::SaveFile);
            chooser.title("Export Flat Image")
                .filter("PPM Image (Raw RGB)\t*.ppm")
                .options(NativeFileChooser::SaveAsConfirm);
            if (chooser.show())
                InternalImageViewer::export_image(chooser.filename());
            return *this;
        }

        template <typename T, void (T::*Method)()>
        AdvancedImageViewer &load_layer_async(const std::string &filepath, const std::string &name,
                                              double lx, double ly, T *instance)
        {
            std::thread(
                [this, filepath, name, lx, ly, instance]()
                {
                    try
                    {
                        auto img = std::make_shared<Image>(filepath.c_str());
                        {
                            UILock lock; // Protect FLTK image operations AND UI state
                            auto thumb = std::make_shared<Image>(img->handle()->copy(32, 32));
                            auto layer = std::make_shared<ImageLayer>(
                                img, name, lx, ly,
                                int(img->width()), int(img->height()),
                                1.0, 1.0, 0.0, 1.0, BlendMode::Normal, true, false, thumb);

                            state_->document()->add_layer(layer);
                            state_->clear_selection();
                            state_->set_primary_selection(layer->id);
                            state_->notify_changed();
                        }
                        Dispatcher::awake<T, Method>(instance);
                    }
                    catch (const std::exception &e)
                    {
                        fprintf(stderr, "Async image load failed: %s\n", e.what());
                    }
                    catch (...)
                    {
                        fprintf(stderr, "Async image load failed: unknown error\n");
                    }
                })
                .detach();
            return *this;
        }

        AdvancedImageViewer &add_to_selection(int index)
        {
            if (index >= 0 && index < (int)state_->document()->layer_count())
            {
                int id = state_->document()->get_layer(index)->id;
                state_->set_primary_selection(id);
            }
            return *this;
        }

        AdvancedImageViewer &remove_from_selection(int index)
        {
            if (index >= 0 && index < (int)state_->document()->layer_count())
            {
                int id = state_->document()->get_layer(index)->id;
                state_->toggle_selection(id);
            }
            return *this;
        }

        bool is_selected(int index) const
        {
            if (index < 0 || index >= (int)state_->document()->layer_count())
                return false;
            return state_->is_in_selection(state_->document()->get_layer(index)->id);
        }

        int selection_count() const { return static_cast<int>(state_->selection_ids().size()); }

        AdvancedImageViewer &delete_selection()
        {
            if (state_->selection_ids().empty())
                return *this;

            perform_heavy_undoable_action([this]()
                                          {
                std::vector<int> to_delete;
                for (int id : state_->selection_ids())
                    if (int idx = state_->document()->get_layer_index(id); idx >= 0)
                        to_delete.push_back(idx);
                std::sort(to_delete.rbegin(), to_delete.rend());
                for (int idx : to_delete)
                    state_->document()->remove_layer(idx);
                state_->clear_selection(); });
            return *this;
        }

        AdvancedImageViewer &remove_layer(int index)
        {
            if (index >= 0 && index < (int)state_->document()->layer_count())
            {
                perform_heavy_undoable_action([this, index]()
                                              {
                    int id_to_remove = state_->document()->get_layer(index)->id;
                    state_->document()->remove_layer(index);
                    if (state_->is_in_selection(id_to_remove))
                    {
                        state_->toggle_selection(id_to_remove);
                    } });
            }
            return *this;
        }

        AdvancedImageViewer &toggle_layer_visibility(int index)
        {
            if (auto l = get_image_layer(index))
                perform_light_undoable_action({index}, [l]()
                                              { l->visible = !l->visible; });
            return *this;
        }

        AdvancedImageViewer &toggle_layer_lock(int index)
        {
            if (auto l = get_image_layer(index))
                perform_light_undoable_action({index}, [l]()
                                              { l->locked = !l->locked; });
            return *this;
        }

        AdvancedImageViewer &move_layer_up(int index)
        {
            if (index >= 0 && index < (int)state_->document()->layer_count() - 1)
                perform_heavy_undoable_action([this, index]()
                                              { state_->document()->swap_layers(index, index + 1); });
            return *this;
        }

        AdvancedImageViewer &move_layer_down(int index)
        {
            if (index > 0 && index < (int)state_->document()->layer_count())
                perform_heavy_undoable_action([this, index]()
                                              { state_->document()->swap_layers(index, index - 1); });
            return *this;
        }

        AdvancedImageViewer &move_layer_to(int index, double new_x, double new_y)
        {
            if (auto l = get_image_layer(index))
            {
                perform_light_undoable_action({index}, [l, new_x, new_y]()
                                              { l->x = new_x; l->y = new_y; });
            }
            return *this;
        }

        AdvancedImageViewer &move_selection_by(double dx, double dy)
        {
            if (state_->selection_ids().empty())
                return *this;
            std::vector<int> indices;
            for (int id : state_->selection_ids())
                if (int idx = state_->document()->get_layer_index(id); idx >= 0)
                    indices.push_back(idx);

            perform_light_undoable_action(indices, [this, dx, dy]()
                                          {
                for (int id : state_->selection_ids())
                    if (auto l = get_image_layer(state_->document()->get_layer_index(id)))
                    {
                        l->x += dx;
                        l->y += dy;
                    } });
            return *this;
        }

        AdvancedImageViewer &undo()
        {
            InternalImageViewer::undo();
            return *this;
        }
        AdvancedImageViewer &redo()
        {
            InternalImageViewer::redo();
            return *this;
        }

        AdvancedImageViewer &flip_layer_horizontal(int index)
        {
            if (auto l = get_image_layer(index))
                perform_light_undoable_action({index}, [l]()
                                              { l->flip_h = !l->flip_h; });
            return *this;
        }

        AdvancedImageViewer &flip_layer_vertical(int index)
        {
            if (auto l = get_image_layer(index))
                perform_light_undoable_action({index}, [l]()
                                              { l->flip_v = !l->flip_v; });
            return *this;
        }

        AdvancedImageViewer &rotate_layer(int index, double angle)
        {
            if (auto l = get_image_layer(index))
                perform_light_undoable_action({index}, [l, angle]()
                                              { l->rotation_angle = angle; });
            return *this;
        }

        AdvancedImageViewer &layer_opacity(int index, double alpha)
        {
            if (auto l = get_image_layer(index))
                perform_light_undoable_action({index}, [l, alpha]()
                                              { l->alpha = std::clamp(alpha, 0.0, 1.0); });
            return *this;
        }

        AdvancedImageViewer &layer_blend_mode(int index, BlendMode mode)
        {
            if (auto l = get_image_layer(index))
                perform_light_undoable_action({index}, [l, mode]()
                                              { l->blend_mode = mode; });
            return *this;
        }

        AdvancedImageViewer &crop_layer(int index,
                                        double crop_x, double crop_y,
                                        double crop_w, double crop_h)
        {
            auto l = get_image_layer(index);
            if (!l || !l->image || !l->image->handle())
                return *this;

            const int iw = l->image->data_w();
            const int ih = l->image->data_h();
            double nx = std::clamp(crop_x, 0.0, (double)iw);
            double ny = std::clamp(crop_y, 0.0, (double)ih);
            double nw = std::clamp(crop_w, 0.0, (double)iw - nx);
            double nh = std::clamp(crop_h, 0.0, (double)ih - ny);
            if (nw < 1.0 || nh < 1.0)
                return *this;

            perform_heavy_undoable_action([=]()
                                          {
                double ox = (l->crop_w >= 0) ? l->crop_x : 0.0;
                double oy = (l->crop_h >= 0) ? l->crop_y : 0.0;
                Point2D world_off = Transform::local_to_world(
                    (ox - nx) * l->scale_x, (oy - ny) * l->scale_y,
                    0, 0, l->rotation_angle, l->flip_h, l->flip_v);
                l->crop_x = nx;
                l->crop_y = ny;
                l->crop_w = nw;
                l->crop_h = nh;
                l->x += world_off.x;
                l->y += world_off.y; });
            return *this;
        }

        AdvancedImageViewer &layer_id(int index, int id)
        {
            if (index >= 0 && index < (int)state_->document()->layer_count())
                state_->document()->get_layer(index)->id = id;
            return *this;
        }

        AdvancedImageViewer &layer_parent(int index, int parent_id)
        {
            if (index >= 0 && index < (int)state_->document()->layer_count())
            {
                auto lyr = state_->document()->get_layer(index);
                perform_light_undoable_action({index}, [lyr, parent_id]()
                                              { lyr->parent_id = parent_id; });
            }
            return *this;
        }

        AdvancedImageViewer &max_undo_records(size_t max_records)
        {
            undo_mgr_->set_max_records(max_records);
            return *this;
        }
        size_t max_undo_records() const { return undo_mgr_->max_records(); }

        std::string export_json() const
        {
            std::stringstream ss;
            ss << "[\n";
            for (size_t i = 0; i < state_->document()->layer_count(); ++i)
            {
                if (auto l = get_image_layer(i))
                    ss << "  {\"name\":\"" << l->name << "\","
                       << "\"x\":" << l->x << ",\"y\":" << l->y << ","
                       << "\"sx\":" << l->scale_x << ",\"sy\":" << l->scale_y << "}";
                if (i < state_->document()->layer_count() - 1)
                    ss << ",";
                ss << "\n";
            }
            ss << "]\n";
            return ss.str();
        }
    };
}