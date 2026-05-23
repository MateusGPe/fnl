#pragma once

#include <thread>
#include <mutex>
#include <sstream>
#include "UILock.hpp"
#include "Dispatcher.hpp"
#include "ImageViewer.hpp"
#include "NativeFileChooser.hpp"

#include "ImageViewer/Transform.hpp"
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

            perform_heavy_undoable_action(
                [this]()
                {
                    std::vector<int> to_delete;
                    for (int id : state_->selection_ids())
                        if (int idx = state_->document()->get_layer_index(id); idx >= 0)
                            to_delete.push_back(idx);

                    std::sort(to_delete.rbegin(), to_delete.rend());

                    for (int idx : to_delete)
                        state_->document()->remove_layer(idx);
                    state_->clear_selection();
                });
            return *this;
        }

        AdvancedImageViewer &remove_layer(int index)
        {
            if (index >= 0 && index < (int)state_->document()->layer_count())
            {
                perform_heavy_undoable_action(
                    [this, index]()
                    {
                        // The heavy action memento will save the selection state.
                        // We just need to define the 'after' state.
                        state_->document()->remove_layer(index);
                        // After removing, it's safest to clear the selection.
                        state_->clear_selection();
                    });
            }
            return *this;
        }

        AdvancedImageViewer &toggle_layer_visibility(int index)
        {
            if (auto l = get_image_layer(index))
                perform_light_undoable_action(
                    {index},
                    [l]()
                    {
                        l->visible = !l->visible;
                    });
            return *this;
        }

        AdvancedImageViewer &toggle_layer_lock(int index)
        {
            if (auto l = get_image_layer(index))
                perform_light_undoable_action(
                    {index},
                    [l]()
                    {
                        l->locked = !l->locked;
                    });
            return *this;
        }

        AdvancedImageViewer &move_layer_up(int index)
        {
            if (index >= 0 && index < (int)state_->document()->layer_count() - 1)
                perform_heavy_undoable_action(
                    [this, index]()
                    {
                        state_->document()->swap_layers(index, index + 1);
                    });
            return *this;
        }

        AdvancedImageViewer &move_layer_down(int index)
        {
            if (index > 0 && index < (int)state_->document()->layer_count())
                perform_heavy_undoable_action(
                    [this, index]()
                    {
                        state_->document()->swap_layers(index, index - 1);
                    });
            return *this;
        }

        AdvancedImageViewer &move_layer_to(int index, double new_x, double new_y)
        {
            if (auto l = get_image_layer(index))
            {
                perform_light_undoable_action(
                    {index},
                    [l, new_x, new_y]()
                    {
                        l->x = new_x;
                        l->y = new_y;
                    });
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

            perform_light_undoable_action(
                indices,
                [this, dx, dy]()
                {
                    for (int id : state_->selection_ids())
                        if (auto l = get_image_layer(state_->document()->get_layer_index(id)))
                        {
                            l->x += dx;
                            l->y += dy;
                        }
                });
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
                perform_light_undoable_action(
                    {index},
                    [l]()
                    {
                        l->flip_h = !l->flip_h;
                    });
            return *this;
        }

        AdvancedImageViewer &flip_layer_vertical(int index)
        {
            if (auto l = get_image_layer(index))
                perform_light_undoable_action(
                    {index},
                    [l]()
                    {
                        l->flip_v = !l->flip_v;
                    });
            return *this;
        }

        AdvancedImageViewer &rotate_layer(int index, double angle)
        {
            if (auto l = get_image_layer(index))
                perform_light_undoable_action(
                    {index},
                    [l, angle]()
                    {
                        l->rotation_angle = angle;
                    });
            return *this;
        }

        AdvancedImageViewer &layer_opacity(int index, double alpha)
        {
            if (auto l = get_image_layer(index))
                perform_light_undoable_action(
                    {index},
                    [l, alpha]()
                    {
                        l->alpha = std::clamp(alpha, 0.0, 1.0);
                    });
            return *this;
        }

        AdvancedImageViewer &layer_blend_mode(int index, BlendMode mode)
        {
            if (auto l = get_image_layer(index))
                perform_light_undoable_action(
                    {index},
                    [l, mode]()
                    {
                        l->blend_mode = mode;
                    });
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

            perform_heavy_undoable_action(
                [=]()
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
                    l->y += world_off.y;
                });
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
                perform_light_undoable_action(
                    {index},
                    [lyr, parent_id]()
                    {
                        lyr->parent_id = parent_id;
                    });
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

        AdvancedImageViewer &duplicate_layer(int index)
        {
            if (index < 0 || index >= (int)state_->document()->layer_count())
                return *this;

            perform_heavy_undoable_action(
                [this, index]()
                {
                    auto original = get_image_layer(index);
                    if (!original)
                        return;

                    auto duplicate = std::static_pointer_cast<ImageLayer>(original->clone());

                    duplicate->id = -1; // So a new one is assigned
                    duplicate->name += " (Copy)";
                    duplicate->x += 20;
                    duplicate->y += 20;

                    // Insert above the original
                    state_->document()->insert_layer(index + 1, duplicate);

                    // Select the new layer
                    state_->clear_selection();
                    state_->set_primary_selection(duplicate->id);
                });

            return *this;
        }

        AdvancedImageViewer &rename_layer(int index, const std::string &new_name)
        {
            if (auto l = get_image_layer(index))
            {
                perform_light_undoable_action({index}, [l, new_name]()
                                              { l->name = new_name; });
            }
            return *this;
        }

        AdvancedImageViewer &bring_to_front(int index)
        {
            if (index < 0 || index >= (int)state_->document()->layer_count())
                return *this;
            if (index == (int)state_->document()->layer_count() - 1)
                return *this; // Already at front

            perform_heavy_undoable_action(
                [this, index]()
                {
                    auto layer = state_->document()->get_layer(index);
                    state_->document()->remove_layer(index);
                    state_->document()->add_layer(layer); // add_layer puts it at the end (top)
                });
            return *this;
        }

        AdvancedImageViewer &send_to_back(int index)
        {
            if (index <= 0 || index >= (int)state_->document()->layer_count())
                return *this; // Already at back or invalid

            perform_heavy_undoable_action(
                [this, index]()
                {
                    auto layer = state_->document()->get_layer(index);
                    state_->document()->remove_layer(index);
                    state_->document()->insert_layer(0, layer); // insert at the beginning (bottom)
                });
            return *this;
        }

        AdvancedImageViewer &select_all()
        {
            if (state_->document()->layer_count() == 0)
                return *this;
            perform_heavy_undoable_action(
                [this]()
                {
                    state_->clear_selection();
                    for (size_t i = 0; i < state_->document()->layer_count(); ++i)
                    {
                        int id = state_->document()->get_layer(i)->id;
                        state_->set_primary_selection(id);
                    }
                });
            return *this;
        }

        AdvancedImageViewer &clear_selection()
        {
            if (state_->selection_ids().empty())
                return *this;
            perform_heavy_undoable_action([this]()
                                          { state_->clear_selection(); });
            return *this;
        }

        AdvancedImageViewer &invert_selection()
        {
            if (state_->document()->layer_count() == 0)
                return *this;

            perform_heavy_undoable_action(
                [this]()
                {
                    std::unordered_set<int> all_ids;
                    for (size_t i = 0; i < state_->document()->layer_count(); ++i)
                        all_ids.insert(state_->document()->get_layer(i)->id);

                    const auto &current_selection = state_->selection_ids();
                    std::unordered_set<int> new_selection;
                    int new_primary = -1;

                    for (int id : all_ids)
                    {
                        if (current_selection.find(id) == current_selection.end())
                        {
                            new_selection.insert(id);
                            new_primary = id;
                        }
                    }

                    state_->clear_selection();
                    for (int id : new_selection)
                        state_->set_primary_selection(id);
                    if (new_primary != -1)
                        state_->set_primary_selection(new_primary);
                });
            return *this;
        }

        AdvancedImageViewer &align_selection_top() { return align_selection<0>(); }
        AdvancedImageViewer &align_selection_bottom() { return align_selection<1>(); }
        AdvancedImageViewer &align_selection_left() { return align_selection<2>(); }
        AdvancedImageViewer &align_selection_right() { return align_selection<3>(); }
        AdvancedImageViewer &align_selection_center_v() { return align_selection<4>(); }
        AdvancedImageViewer &align_selection_center_h() { return align_selection<5>(); }

        AdvancedImageViewer &distribute_selection_horizontal() { return distribute_selection<0>(); }
        AdvancedImageViewer &distribute_selection_vertical() { return distribute_selection<1>(); }

    private:
        template <int AlignMode>
        AdvancedImageViewer &align_selection()
        {
            auto indices = get_selected_indices();
            if (indices.size() < 2)
                return *this;

            auto primary_layer = get_selected_image_layer();
            if (!primary_layer)
                return *this;

            const auto primary_bounds = get_layer_world_bounds(*primary_layer);

            perform_light_undoable_action(
                indices,
                [&]()
                {
                    for (int id : state_->selection_ids())
                    {
                        if (auto l = get_image_layer(state_->document()->get_layer_index(id)))
                        {
                            if (l->id == primary_layer->id)
                                continue;
                            const auto current_bounds = get_layer_world_bounds(*l);
                            if constexpr (AlignMode == 0)
                                l->y += (primary_bounds.y - current_bounds.y); // Top
                            else if constexpr (AlignMode == 1)
                                l->y += ((primary_bounds.y + primary_bounds.h) - (current_bounds.y + current_bounds.h)); // Bottom
                            else if constexpr (AlignMode == 2)
                                l->x += (primary_bounds.x - current_bounds.x); // Left
                            else if constexpr (AlignMode == 3)
                                l->x += ((primary_bounds.x + primary_bounds.w) - (current_bounds.x + current_bounds.w)); // Right
                            else if constexpr (AlignMode == 4)
                                l->y += ((primary_bounds.y + primary_bounds.h * 0.5) - (current_bounds.y + current_bounds.h * 0.5)); // Center V
                            else if constexpr (AlignMode == 5)
                                l->x += ((primary_bounds.x + primary_bounds.w * 0.5) - (current_bounds.x + current_bounds.w * 0.5)); // Center H
                        }
                    }
                });
            return *this;
        }

        template <int DistributeMode>
        AdvancedImageViewer &distribute_selection()
        {
            auto indices = get_selected_indices();
            if (indices.size() < 3) // Distribution needs at least 3 items
                return *this;

            perform_light_undoable_action(
                indices,
                [&]()
                {
                    std::vector<std::shared_ptr<ImageLayer>> selected_layers;
                    selected_layers.reserve(indices.size());
                    for (int id : state_->selection_ids())
                    {
                        if (auto l = get_image_layer(state_->document()->get_layer_index(id)))
                        {
                            selected_layers.push_back(l);
                        }
                    }

                    // Sort layers by position
                    if constexpr (DistributeMode == 0) // Horizontal
                        std::sort(selected_layers.begin(), selected_layers.end(),
                                  [this](const auto &a, const auto &b)
                                  { return get_layer_world_bounds(*a).x < get_layer_world_bounds(*b).x; });
                    else // Vertical
                        std::sort(selected_layers.begin(), selected_layers.end(),
                                  [this](const auto &a, const auto &b)
                                  { return get_layer_world_bounds(*a).y < get_layer_world_bounds(*b).y; });

                    const auto first_bounds = get_layer_world_bounds(*selected_layers.front());
                    const auto last_bounds = get_layer_world_bounds(*selected_layers.back());

                    double total_size = 0;
                    for (const auto &l : selected_layers)
                    {
                        const auto bounds = get_layer_world_bounds(*l);
                        if constexpr (DistributeMode == 0) total_size += bounds.w;
                        else total_size += bounds.h;
                    }

                    double span = (DistributeMode == 0) ? ((last_bounds.x + last_bounds.w) - first_bounds.x) : ((last_bounds.y + last_bounds.h) - first_bounds.y);
                    if (span < total_size) return; // Don't distribute if layers overlap, gap would be negative

                    double gap = (span - total_size) / (selected_layers.size() - 1);
                    double current_pos = (DistributeMode == 0) ? (first_bounds.x + first_bounds.w + gap) : (first_bounds.y + first_bounds.h + gap);

                    // Reposition layers between the first and the last
                    for (size_t i = 1; i < selected_layers.size() - 1; ++i)
                    {
                        auto l = selected_layers[i];
                        const auto current_bounds = get_layer_world_bounds(*l);
                        if constexpr (DistributeMode == 0) l->x += (current_pos - current_bounds.x);
                        else l->y += (current_pos - current_bounds.y);
                        current_pos += ((DistributeMode == 0) ? current_bounds.w : current_bounds.h) + gap;
                    }
                });
            return *this;
        }

        // Helper to get indices of all selected layers
        std::vector<int> get_selected_indices() const
        {
            std::vector<int> indices;
            if (state_->selection_ids().empty())
                return indices;
            indices.reserve(state_->selection_ids().size());
            for (int id : state_->selection_ids())
            {
                if (int idx = state_->document()->get_layer_index(id); idx >= 0)
                    indices.push_back(idx);
            }
            return indices;
        }
    };
}