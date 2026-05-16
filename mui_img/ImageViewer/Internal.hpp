#pragma once

#include "Types.hpp"
#include "Commands.hpp"
#include "Theme.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_draw.H>
#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include "ImageDocument.hpp"
#include "Tools/ViewerToolState.hpp"
#include "Transform.hpp"

namespace mui
{
    struct MinimapInfo
    {
        int x, y, w, h;
        double scale, offset_x, offset_y, min_x, min_y;
    };

    class InternalImageViewer : public Fl_Widget
    {
        friend struct CommandMove;
        friend struct CommandCrop;
        friend struct CommandFlip;
        friend struct CommandRotate;
        friend struct CommandDelete;
        friend struct CommandOpacity;
        friend struct CommandBlendMode;
        friend struct CommandVisibility;
        friend struct CommandLock;
        friend struct CommandParent;

    protected:
        std::vector<std::shared_ptr<ViewerCommand>> undo_stack_;
        std::vector<std::shared_ptr<ViewerCommand>> redo_stack_;
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

        int selected_layer_index_ = -1;
        enum DragMode
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
        DragMode drag_mode_ = None;
        DragMode hover_mode_ = None;

        bool bilinear_filtering_ = true;
        int grid_size_;
        bool use_solid_bg_ = false;
        Fl_Color solid_bg_color_;

        Fl_Offscreen bg_buffer_ = 0;
        int off_w_ = 0, off_h_ = 0;
        bool bg_dirty_ = true;
        Fl_Color last_checker_color1_ = 0;

        bool show_minimap_ = false;
        ViewerTool active_tool_ = ViewerTool::Select;
        int minimap_size_ = 150;
        int minimap_margin_ = 15;
        double crop_start_x_ = 0;
        double crop_start_y_ = 0;
        double crop_end_x_ = 0;
        double crop_end_y_ = 0;

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

    public:
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

        void undo()
        {
            if (undo_stack_.empty())
                return;
            auto cmd = undo_stack_.back();
            undo_stack_.pop_back();
            cmd->undo(this);
            redo_stack_.push_back(cmd);
            notify_view_changed();
        }

        void redo()
        {
            if (redo_stack_.empty())
                return;
            auto cmd = redo_stack_.back();
            redo_stack_.pop_back();
            cmd->execute(this);
            undo_stack_.push_back(cmd);
            notify_view_changed();
        }

        // Made public to facilitate external command injection, especially for testing.
        void push_command(std::shared_ptr<ViewerCommand> cmd)
        {
            cmd->execute(this);
            undo_stack_.push_back(cmd);
            redo_stack_.clear();
            notify_view_changed();
        }

        bool is_layer_visible(int index) const
        {
            if (index < 0 || index >= (int)document_->layer_count())
                return false;

            std::vector<int> visited_indices;
            int curr = index;
            while (curr != -1)
            {
                // The parent lookup is an O(N) scan. For deep hierarchies, this can be slow.
                // A map from layer ID to index would improve this to O(1).
                if (std::find(visited_indices.begin(), visited_indices.end(), curr) != visited_indices.end())
                    return false; // Cycle detected in parent hierarchy.
                visited_indices.push_back(curr);

                auto l = get_image_layer(curr);
                if (!l || !l->visible)
                    return false;

                int pid = l->parent_id;
                if (pid == -1)
                    break; // Reached root of this branch.

                int parent_idx = -1;
                for (int i = 0; i < (int)document_->layer_count(); ++i)
                {
                    if (document_->get_layer(i)->id == pid)
                    {
                        parent_idx = i;
                        break;
                    }
                }
                curr = parent_idx;
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
            min_x = 1e99;
            min_y = 1e99;
            max_x = -1e99;
            max_y = -1e99;

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

            double vw_w = w() / scale_;
            double vw_h = h() / scale_;
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

            double sw = w() / scale_;
            double sh = h() / scale_;

            double world_w = max_x - min_x;
            double world_h = max_y - min_y;

            if (world_w <= sw)
            {
                view_x_ = min_x - (sw - world_w) * 0.5;
            }
            else
            {
                view_x_ = std::clamp(view_x_, min_x, max_x - sw);
            }

            if (world_h <= sh)
            {
                view_y_ = min_y - (sh - world_h) * 0.5;
            }
            else
            {
                view_y_ = std::clamp(view_y_, min_y, max_y - sh);
            }
        }

        void sample_color(int mx, int my, double world_x, double world_y);
        virtual void draw_background(int cx, int cy, int cw, int ch);
        DragMode hit_test_gizmo(int mx, int my);
        void draw_handle(int hx, int hy, bool is_hovered);
        virtual void draw_overlays(int cx, int cy, int cw, int ch);
        bool hit_test_minimap(int mx, int my);
        void pan_minimap_to(int mx, int my);
        void draw_minimap();
        void render_layer_to_buffer(const ImageLayer &layer, int layer_idx, int target_w, int target_h, double view_x, double view_y, double scale);
        void draw() override;
        void export_image(const char *filepath);
        int hit_test(double world_x, double world_y);
        int handle(int event) override;

    public:
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

#include "Internal_Draw.hpp"
#include "Internal_Events.hpp"
#include "Internal_Commands.hpp"