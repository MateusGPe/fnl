#pragma once

#include "Types.hpp"
#include "Theme.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <vector>
#include <memory>
#include <unordered_set>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include "ImageDocument.hpp"
#include "Tools/ViewerToolState.hpp"
#include "Transform.hpp"

namespace mui
{
    class InternalImageViewer;

    struct StateMemento
    {
        std::shared_ptr<ImageDocument> document;
        int selected_layer_id;
        std::unordered_set<int> selection_ids;
    };

    // --- Hybrid Undo/Redo System ---

    struct UndoRecord
    {
        virtual ~UndoRecord() = default;
        virtual void apply(InternalImageViewer *viewer) = 0;
    };

    struct DocumentChangeRecord : public UndoRecord
    {
        std::unique_ptr<StateMemento> state;
        explicit DocumentChangeRecord(std::unique_ptr<StateMemento> s) : state(std::move(s)) {}
        void apply(InternalImageViewer *viewer) override;
    };

    struct LayerPropsChangeRecord : public UndoRecord
    {
        std::unordered_map<int, std::shared_ptr<Layer>> props;

        void apply(InternalImageViewer *viewer) override;
    };

    struct MinimapInfo
    {
        int x, y, w, h;
        double scale, offset_x, offset_y, min_x, min_y;
    };
    inline Rect2D layer_effective_rect(const ImageLayer &l) noexcept
    {
        return l.get_effective_bounds();
    }

    enum class DragMode
    {
        None,
        Pan,
        MoveLayer,
        ScaleBR,
        ScaleBL,
        ScaleTR,
        ScaleTL,
        Rotate,
        MinimapPan,
        Crop,
        Eyedropper
    };

    inline bool is_scale_mode(DragMode m) noexcept
    {
        return m == DragMode::ScaleBR || m == DragMode::ScaleBL ||
               m == DragMode::ScaleTR || m == DragMode::ScaleTL;
    }

    inline void draw_checker(int bx, int by, int bw, int bh, int checker_size)
    {
        const auto &pal = ThemeManager::get_palette();
        for (int j = 0; j < bh; j += checker_size)
            for (int i = 0; i < bw; i += checker_size)
            {
                fl_color((((i / checker_size) + (j / checker_size)) % 2 == 0)
                             ? pal.bg_main
                             : pal.bg_sec);
                fl_rectf(bx + i, by + j, checker_size, checker_size);
            }
    }

    class InternalImageViewer : public Fl_Widget
    {
        friend struct DocumentChangeRecord;
        friend struct LayerPropsChangeRecord;

    protected:
        std::vector<std::unique_ptr<UndoRecord>> undo_stack_;
        std::vector<std::unique_ptr<UndoRecord>> redo_stack_;
        std::shared_ptr<ImageDocument> document_;
        std::unique_ptr<ViewerToolState> active_tool_state_;

        double scale_ = 1.0;
        double view_x_ = 0.0;
        double view_y_ = 0.0;

        int last_mouse_x_ = 0;
        int last_mouse_y_ = 0;
        double drag_start_x_ = 0;
        double drag_start_y_ = 0;
        double orig_layer_x_ = 0, orig_layer_y_ = 0;
        double orig_layer_sx_ = 1, orig_layer_sy_ = 1;

        int selected_layer_id_ = -1;

        std::unordered_set<int> selection_ids_;

        std::vector<std::pair<int, std::pair<double, double>>> multi_drag_origins_;
        std::unique_ptr<LayerPropsChangeRecord> drag_undo_record_;
        DragMode drag_mode_ = DragMode::None;
        DragMode hover_mode_ = DragMode::None;
        bool drag_undo_state_pushed_ = false;

        bool bilinear_filtering_ = true;
        int grid_size_;
        bool use_solid_bg_ = false;
        Fl_Color solid_bg_color_ = 0;

        Fl_Offscreen bg_buffer_ = 0;
        int off_w_ = 0, off_h_ = 0;
        bool bg_dirty_ = true;
        Fl_Color last_checker_color1_ = 0;

        bool show_minimap_ = false;
        ViewerTool active_tool_ = ViewerTool::Select;
        int minimap_size_ = 150;
        int minimap_margin_ = 15;
        double crop_start_x_ = 0, crop_start_y_ = 0;
        double crop_end_x_ = 0, crop_end_y_ = 0;

        void *user_data_ = nullptr;
        void (*view_changed_thunk_)(void *) = nullptr;
        void (*layer_selected_thunk_)(void *) = nullptr;
        void (*right_click_thunk_)(void *) = nullptr;
        void (*color_picked_thunk_)(uchar, uchar, uchar, uchar, void *) = nullptr;

        void notify_view_changed()
        {
            if (view_changed_thunk_ && user_data_)
                view_changed_thunk_(user_data_);
        }
        void notify_layer_selected()
        {
            if (layer_selected_thunk_ && user_data_)
                layer_selected_thunk_(user_data_);
        }
        void notify_right_click()
        {
            if (right_click_thunk_ && user_data_)
                right_click_thunk_(user_data_);
        }

        void invalidate()
        {
            bg_dirty_ = true;
            redraw();
        }

        void mouse_to_world(int mx, int my, double &wx, double &wy) const noexcept
        {
            wx = view_x_ + (mx - x()) / scale_;
            wy = view_y_ + (my - y()) / scale_;
        }

        void world_to_screen(double wx, double wy, int &sx, int &sy) const noexcept
        {
            sx = x() + static_cast<int>(std::round((wx - view_x_) * scale_));
            sy = y() + static_cast<int>(std::round((wy - view_y_) * scale_));
        }

        void set_primary_selection(int id)
        {
            selected_layer_id_ = id;
            if (id == -1)
                return;
            selection_ids_.insert(id);
        }

        void clear_selection()
        {
            selected_layer_id_ = -1;
            selection_ids_.clear();
            multi_drag_origins_.clear();
        }

        bool is_in_selection(int layer_id) const noexcept
        {
            return selection_ids_.count(layer_id) > 0;
        }

        void toggle_selection(int layer_id)
        {
            if (selection_ids_.count(layer_id))
            {
                selection_ids_.erase(layer_id);
                if (selected_layer_id_ == layer_id)
                {
                    selected_layer_id_ = selection_ids_.empty() ? -1
                                                                : *selection_ids_.begin();
                }
            }
            else
            {
                selection_ids_.insert(layer_id);
                selected_layer_id_ = layer_id;
            }
        }

        void capture_multi_drag_origins()
        {
            multi_drag_origins_.clear();
            for (int id : selection_ids_)
            {
                int idx = document_->get_layer_index(id);
                if (idx == -1)
                    continue;
                if (auto l = get_image_layer(idx))
                    multi_drag_origins_.push_back({id, {l->x, l->y}});
            }
        }

    public:
        int get_selected_layer_index() const
        {
            if (selected_layer_id_ == -1)
                return -1;
            return document_->get_layer_index(selected_layer_id_);
        }

        const std::unordered_set<int> &selection_ids() const { return selection_ids_; }

        std::shared_ptr<ImageLayer> get_selected_image_layer() const
        {
            return get_image_layer(get_selected_layer_index());
        }

        std::shared_ptr<ImageLayer> get_image_layer(int index) const
        {
            if (index < 0 || index >= (int)document_->layer_count())
                return nullptr;
            return std::static_pointer_cast<ImageLayer>(document_->get_layer(index));
        }

        void world_to_layer_local(const ImageLayer &l, double wx, double wy, double &out_lx, double &out_ly)
        {
            const Point2D c = l.get_center();
            const Point2D local_pt = Transform::world_to_local(wx, wy, c.x, c.y, l.rotation_angle, l.flip_h, l.flip_v);
            out_lx = local_pt.x;
            out_ly = local_pt.y;
        }

        template <typename Action>
        void perform_heavy_undoable_action(Action action)
        {
            push_undo_record(
                std::make_unique<DocumentChangeRecord>(create_state_memento()));
            action();
            invalidate();
            notify_view_changed();
        }

        template <typename Action>
        void perform_light_undoable_action(const std::vector<int> &layer_indices, Action action)
        {
            auto record = std::make_unique<LayerPropsChangeRecord>();
            for (int index : layer_indices)
                if (auto l = get_image_layer(index))
                    record->props[l->id] = l->clone();

            push_undo_record(std::move(record));
            action();
            invalidate();
            notify_view_changed();
        }

        std::unique_ptr<StateMemento> create_state_memento()
        {
            auto memento = std::make_unique<StateMemento>();
            memento->document = std::make_shared<ImageDocument>(*document_);
            memento->selected_layer_id = selected_layer_id_;
            memento->selection_ids = selection_ids_;
            return memento;
        }

        void push_undo_record(std::unique_ptr<UndoRecord> record)
        {
            undo_stack_.push_back(std::move(record));
            redo_stack_.clear();
        }

        void apply_full_memento(std::unique_ptr<StateMemento> memento)
        {
            document_ = memento->document;
            selected_layer_id_ = memento->selected_layer_id;
            selection_ids_ = memento->selection_ids;
            invalidate();
            notify_view_changed();
            notify_layer_selected();
        }

        void undo()
        {
            if (undo_stack_.empty())
                return;
            auto record = std::move(undo_stack_.back());
            undo_stack_.pop_back();
            record->apply(this);
            redo_stack_.push_back(std::move(record));
        }

        void redo()
        {
            if (redo_stack_.empty())
                return;
            auto record = std::move(redo_stack_.back());
            redo_stack_.pop_back();
            record->apply(this);
            undo_stack_.push_back(std::move(record));
        }

        bool is_layer_visible(int index) const
        {
            if (index < 0 || index >= (int)document_->layer_count())
                return false;

            std::vector<int> visited;
            int current_id = document_->get_layer(index)->id;
            while (current_id != -1)
            {
                if (std::find(visited.begin(), visited.end(), current_id) != visited.end())
                    return false;
                visited.push_back(current_id);

                int ci = document_->get_layer_index(current_id);
                if (ci == -1)
                    return false;
                auto l = get_image_layer(ci);
                if (!l || !l->visible)
                    return false;
                current_id = l->parent_id;
            }
            return true;
        }

        void get_world_bounds(double &min_x, double &min_y, double &max_x, double &max_y)
        {
            if (document_->mode() == DocumentMode::FixedCanvas)
            {
                min_x = 0;
                min_y = 0;
                max_x = document_->canvas_width();
                max_y = document_->canvas_height();
                return;
            }
            if (document_->layer_count() == 0)
            {
                min_x = 0;
                min_y = 0;
                max_x = w() / scale_;
                max_y = h() / scale_;
                return;
            }

            min_x = min_y = 1e99;
            max_x = max_y = -1e99;
            for (size_t i = 0; i < document_->layer_count(); ++i)
            {
                auto layer = std::static_pointer_cast<ImageLayer>(document_->get_layer(i));
                Rect2D b = layer_effective_rect(*layer);
                Rect2D rot_b = Transform::get_rotated_bounds(b.x, b.y, b.w, b.h,
                                                             layer->rotation_angle);
                min_x = std::min(min_x, rot_b.x);
                max_x = std::max(max_x, rot_b.x + rot_b.w);
                min_y = std::min(min_y, rot_b.y);
                max_y = std::max(max_y, rot_b.y + rot_b.h);
            }
            if (min_x > max_x)
            {
                min_x = 0;
                min_y = 0;
                max_x = w() / scale_;
                max_y = h() / scale_;
            }
        }

        MinimapInfo get_minimap_info()
        {
            MinimapInfo info;
            info.w = minimap_size_;
            info.h = minimap_size_;
            info.x = x() + w() - info.w - minimap_margin_;
            info.y = y() + minimap_margin_;

            double max_x, max_y;
            get_world_bounds(info.min_x, info.min_y, max_x, max_y);

            double vw_w = w() / scale_, vw_h = h() / scale_;
            info.min_x = std::min(info.min_x, view_x_);
            info.min_y = std::min(info.min_y, view_y_);
            max_x = std::max(max_x, view_x_ + vw_w);
            max_y = std::max(max_y, view_y_ + vw_h);

            double world_w = std::max(max_x - info.min_x, 1.0);
            double world_h = std::max(max_y - info.min_y, 1.0);
            info.scale = std::min((info.w - 4.0) / world_w, (info.h - 4.0) / world_h);
            info.offset_x = info.x + (info.w - world_w * info.scale) * 0.5;
            info.offset_y = info.y + (info.h - world_h * info.scale) * 0.5;
            return info;
        }

        void clamp_view()
        {
            if (document_->layer_count() == 0 || w() == 0 || h() == 0 || scale_ <= 0.0)
                return;

            double min_x, min_y, max_x, max_y;
            get_world_bounds(min_x, min_y, max_x, max_y);

            double sw = w() / scale_, sh = h() / scale_;
            double world_w = max_x - min_x, world_h = max_y - min_y;

            view_x_ = (world_w <= sw) ? min_x - (sw - world_w) * 0.5
                                      : std::clamp(view_x_, min_x, max_x - sw);
            view_y_ = (world_h <= sh) ? min_y - (sh - world_h) * 0.5
                                      : std::clamp(view_y_, min_y, max_y - sh);
        }

        void sample_color(int mx, int my, double world_x, double world_y);
        virtual void draw_background(int cx, int cy, int cw, int ch);
        void draw_handle(int hx, int hy, bool is_hovered);
        virtual void draw_overlays(int cx, int cy, int cw, int ch);
        bool hit_test_minimap(int mx, int my);
        void pan_minimap_to(int mx, int my);
        void draw_minimap();
        void render_layer_to_buffer(const ImageLayer &layer, int layer_idx,
                                    int target_w, int target_h,
                                    double view_x, double view_y, double scale);
        void draw() override;
        void export_image(const char *filepath);
        int hit_test(double world_x, double world_y);
        int handle(int event) override;

    public:
        DragMode hit_test_gizmo(int mx, int my);

        void add_layer(std::shared_ptr<Layer> layer)
        {
            if (document_)
                document_->add_layer(layer);
            redraw();
        }
        const std::vector<std::shared_ptr<Layer>> &layers() const { return document_->layers(); }

        InternalImageViewer(int x, int y, int w, int h, const char *l = nullptr)
            : Fl_Widget(x, y, w, h, l),
              document_(std::make_shared<ImageDocument>())
        {
            grid_size_ = ThemeManager::get_palette().metrics.imageviewer_grid_size;
            solid_bg_color_ = ThemeManager::get_palette().bg_main;
            last_checker_color1_ = Fl::get_color(ThemeManager::get_palette().bg_main);
        }

        friend class ImageViewer;
    };
}

inline void mui::DocumentChangeRecord::apply(mui::InternalImageViewer *viewer)
{
    auto current_state = viewer->create_state_memento();
    viewer->apply_full_memento(std::move(state));
    state = std::move(current_state);
}

inline void mui::LayerPropsChangeRecord::apply(mui::InternalImageViewer *viewer)
{
    if (!viewer)
        return;
    for (auto &pair : props)
    {
        int layer_id = pair.first;
        std::shared_ptr<Layer> &stored_props = pair.second;

        int idx = viewer->document_->get_layer_index(layer_id);
        if (auto target_layer = viewer->get_image_layer(idx))
        {
            auto current_props_clone = target_layer->clone();
            *target_layer = *std::static_pointer_cast<ImageLayer>(stored_props);
            stored_props = current_props_clone;
        }
    }
    viewer->invalidate();
    viewer->notify_view_changed();
}

#include "Internal_Draw.hpp"
#include "Internal_Events.hpp"
