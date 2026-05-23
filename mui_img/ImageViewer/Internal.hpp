#pragma once

#include "Types.hpp"
#include "Theme.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include "EditorState.hpp"
#include "Commands.hpp"
#include "Transform.hpp"

namespace mui
{
    class InternalImageViewer;

    struct ViewerMouseEvent
    {
        InternalImageViewer *viewer;
        int mx, my;
        double world_x, world_y;
    };

    class ViewerToolState
    {
    public:
        virtual ~ViewerToolState() = default;
        virtual int on_mouse_down(const ViewerMouseEvent &e) { return 0; }
        virtual int on_mouse_drag(const ViewerMouseEvent &e, double dx, double dy) { return 0; }
        virtual int on_mouse_up(const ViewerMouseEvent &e) { return 0; }
        virtual int on_key_press(InternalImageViewer *viewer, int key) { return 0; }
        virtual void on_render_overlay(InternalImageViewer *viewer, int cx, int cy, int cw, int ch) {}
    };

    constexpr double MIN_WORLD_COORD = -32768.0;
    constexpr double MAX_WORLD_COORD = 32768.0;

    inline void clamp_world_coords(double &x, double &y)
    {
        x = std::clamp(x, MIN_WORLD_COORD, MAX_WORLD_COORD);
        y = std::clamp(y, MIN_WORLD_COORD, MAX_WORLD_COORD);
    }

    struct DragOriginState
    {
        double x, y, sx, sy, rot;
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

    class InternalImageViewer : public Fl_Widget, public IEditorObserver
    {
    protected:
        std::shared_ptr<EditorState> state_;
        std::shared_ptr<IUndoManager> undo_mgr_;

        std::unique_ptr<ViewerToolState> active_tool_state_;

        double scale_ = 1.0;
        double view_x_ = 0.0;
        double view_y_ = 0.0;

        int last_mouse_x_ = 0;
        int last_mouse_y_ = 0;
        double drag_start_x_ = 0;
        double drag_start_y_ = 0;
        std::unordered_map<int, DragOriginState> multi_drag_origins_;
        Rect2D orig_group_bounds_;
        Point2D group_drag_center_;
        std::vector<Line2D> snap_lines_;
        std::unique_ptr<LayerPropsCommand> drag_undo_record_;
        DragMode drag_mode_ = DragMode::None;
        DragMode hover_mode_ = DragMode::None;
        bool drag_undo_state_pushed_ = false;

        bool bilinear_filtering_ = true;
        int grid_size_;
        bool snap_to_canvas_ = true;
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

        void capture_multi_drag_origins()
        {
            multi_drag_origins_.clear();
            for (int id : state_->selection_ids())
                if (auto l = get_image_layer(state_->document()->get_layer_index(id)))
                    multi_drag_origins_[id] = {l->x, l->y, l->scale_x, l->scale_y, l->rotation_angle};
        }

        // Helper struct to abstract crop drags 
        struct CropDragBox {
            double min_lx, min_ly, max_lx, max_ly;
            double w() const { return max_lx - min_lx; }
            double h() const { return max_ly - min_ly; }
        };

        CropDragBox get_crop_drag_box() const {
            double min_lx, min_ly, max_lx, max_ly;
            if (Fl::event_state(FL_CTRL)) {
                double dx = std::abs(crop_end_x_ - crop_start_x_);
                double dy = std::abs(crop_end_y_ - crop_start_y_);
                min_lx = crop_start_x_ - dx; min_ly = crop_start_y_ - dy;
                max_lx = crop_start_x_ + dx; max_ly = crop_start_y_ + dy;
            } else {
                min_lx = std::min(crop_start_x_, crop_end_x_); min_ly = std::min(crop_start_y_, crop_end_y_);
                max_lx = std::max(crop_start_x_, crop_end_x_); max_ly = std::max(crop_start_y_, crop_end_y_);
            }
            return {min_lx, min_ly, max_lx, max_ly};
        }

        // Extracted Drawing Methods
        void draw_snap_lines();
        void draw_selection_gizmo(const ImageLayer& l, bool is_primary);
        void draw_crop_gizmo();

        // Extracted Event Handlers
        int handle_keydown(int key);
        int handle_mouse_push();
        int handle_mouse_drag();
        int handle_mouse_release();
        void apply_snapping(double& final_ddx, double& final_ddy, const Rect2D& target_bounds);
        void handle_scale_drag(double world_x, double world_y);
        void handle_rotate_drag(double world_x, double world_y);

    public:
        void set_undo_manager(std::shared_ptr<IUndoManager> um) { undo_mgr_ = um; }
        std::shared_ptr<IUndoManager> undo_manager() const { return undo_mgr_; }
        std::shared_ptr<EditorState> state() const { return state_; }

        void on_document_changed() override
        {
            invalidate();
            notify_view_changed();
        }
        void on_selection_changed() override
        {
            invalidate();
            notify_layer_selected();
        }

        int get_selected_layer_index() const
        {
            if (state_->selected_layer_id() == -1)
                return -1;
            return state_->document()->get_layer_index(state_->selected_layer_id());
        }

        std::shared_ptr<ImageLayer> get_selected_image_layer() const
        {
            return get_image_layer(get_selected_layer_index());
        }

        std::shared_ptr<ImageLayer> get_image_layer(int index) const
        {
            if (index < 0 || index >= (int)state_->document()->layer_count())
                return nullptr;
            return std::static_pointer_cast<ImageLayer>(state_->document()->get_layer(index));
        }

        void world_to_layer_local(const ImageLayer &l, double wx, double wy, double &out_lx, double &out_ly)
        {
            const Point2D c = l.get_center();
            const Point2D local_pt = Transform::world_to_local(wx, wy, c.x, c.y, l.rotation_angle, l.flip_h, l.flip_v);
            out_lx = local_pt.x;
            out_ly = local_pt.y;
        }

        Rect2D get_layer_world_bounds_at(const ImageLayer &l, double at_x, double at_y) const
        {
            Rect2D b = l.get_effective_bounds();
            double dx = b.x - l.x;
            double dy = b.y - l.y;
            return Transform::get_rotated_bounds(at_x + dx, at_y + dy, b.w, b.h, l.rotation_angle);
        }

        Rect2D get_layer_world_bounds(const ImageLayer &l) const
        {
            return get_layer_world_bounds_at(l, l.x, l.y);
        }

        Rect2D get_selection_world_bounds()
        {
            if (state_->selection_ids().empty())
                return {0, 0, 0, 0};
            double min_x = 1e99, min_y = 1e99, max_x = -1e99, max_y = -1e99;
            bool first = true;
            for (int id : state_->selection_ids())
            {
                if (auto l = get_image_layer(state_->document()->get_layer_index(id)))
                {
                    Rect2D b = get_layer_world_bounds(*l);
                    if (first)
                    {
                        min_x = b.x;
                        min_y = b.y;
                        max_x = b.x + b.w;
                        max_y = b.y + b.h;
                        first = false;
                    }
                    else
                    {
                        min_x = std::min(min_x, b.x);
                        min_y = std::min(min_y, b.y);
                        max_x = std::max(max_x, b.x + b.w);
                        max_y = std::max(max_y, b.y + b.h);
                    }
                }
            }
            if (first)
                return {0, 0, 0, 0};
            return {min_x, min_y, max_x - min_x, max_y - min_y};
        }

        template <typename Action>
        void perform_heavy_undoable_action(Action action)
        {
            auto cmd = std::make_unique<DocumentStateCommand>(*state_);
            action();
            cmd->capture_after(*state_);
            undo_mgr_->push(std::move(cmd));
            state_->notify_changed();
        }

        template <typename Action>
        void perform_light_undoable_action(const std::vector<int> &layer_indices, Action action)
        {
            auto cmd = std::make_unique<LayerPropsCommand>(*state_, layer_indices);
            action();
            cmd->capture_after(*state_);
            undo_mgr_->push(std::move(cmd));
            state_->notify_changed();
        }

        void undo() { undo_mgr_->undo(*state_); }
        void redo() { undo_mgr_->redo(*state_); }

        bool is_layer_visible(int index) const
        {
            if (index < 0 || index >= (int)state_->document()->layer_count())
                return false;

            std::vector<int> visited;
            int current_id = state_->document()->get_layer(index)->id;
            while (current_id != -1)
            {
                if (std::find(visited.begin(), visited.end(), current_id) != visited.end())
                    return false;
                visited.push_back(current_id);

                int ci = state_->document()->get_layer_index(current_id);
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
            if (state_->document()->mode() == DocumentMode::FixedCanvas)
            {
                min_x = 0;
                min_y = 0;
                max_x = state_->document()->canvas_width();
                max_y = state_->document()->canvas_height();
                return;
            }
            if (state_->document()->layer_count() == 0)
            {
                min_x = 0;
                min_y = 0;
                max_x = w() / scale_;
                max_y = h() / scale_;
                return;
            }

            min_x = min_y = 1e99;
            max_x = max_y = -1e99;
            for (size_t i = 0; i < state_->document()->layer_count(); ++i)
            {
                auto layer = std::static_pointer_cast<ImageLayer>(state_->document()->get_layer(i));
                Rect2D b = layer_effective_rect(*layer);
                Rect2D rot_b = Transform::get_rotated_bounds(b.x, b.y, b.w, b.h, layer->rotation_angle);
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
            if (state_->document()->layer_count() == 0 || w() == 0 || h() == 0 || scale_ <= 0.0)
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
        virtual void draw_overlays();
        bool hit_test_minimap(int mx, int my);
        void pan_minimap_to(int mx, int my);
        void draw_minimap();
        void render_layer_to_buffer(const ImageLayer &layer, int layer_idx, int target_w, int target_h, double view_x, double view_y, double scale);
        void draw() override;
        void export_image(const char *filepath);
        int hit_test(double world_x, double world_y);
        int handle(int event) override;

    public:
        DragMode hit_test_gizmo(int mx, int my);

        const std::vector<std::shared_ptr<Layer>> &layers() const { return state_->document()->layers(); }

        InternalImageViewer(int x, int y, int w, int h, const char *l = nullptr)
            : Fl_Widget(x, y, w, h, l),
              state_(std::make_shared<EditorState>()),
              undo_mgr_(std::make_shared<DefaultUndoManager>())
        {
            state_->add_observer(this);
            grid_size_ = ThemeManager::get_palette().metrics.imageviewer_grid_size;
            solid_bg_color_ = ThemeManager::get_palette().bg_main;
            last_checker_color1_ = Fl::get_color(ThemeManager::get_palette().bg_main);
        }

        ~InternalImageViewer()
        {
            state_->remove_observer(this);
        }

        friend class ImageViewer;
    };
}

#include "Internal_Draw.hpp"
#include "Internal_Events.hpp"