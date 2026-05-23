#pragma once

#include "Internal.hpp"
#include <cmath>
#include <algorithm>
#include <memory>

namespace mui
{
    struct SnapCandidate
    {
        double delta;      
        double line_coord; 
        double line_start; 
        double line_end;   
        bool is_grid_snap = false;
    };

    inline DragMode InternalImageViewer::hit_test_gizmo(int mx, int my)
    {
        auto l_ptr = get_selected_image_layer();
        if (!l_ptr || l_ptr->locked) return DragMode::None;
        
        double wx, wy;
        mouse_to_world(mx, my, wx, wy);
        
        double r_world = (ThemeManager::get_palette().metrics.imageviewer_handle_size * 0.5 +
                          ThemeManager::get_palette().metrics.imageviewer_handle_hit_padding) / scale_;

        auto near = [&](const Point2D& p) {
            return std::abs(wx - p.x) <= r_world && std::abs(wy - p.y) <= r_world;
        };

        // Reuse identical unified math used for drawing bounds
        double rot_offset = (ThemeManager::get_palette().metrics.imageviewer_handle_size * 2) / scale_;
        auto gizmo = Transform::get_gizmo_handles(*l_ptr, rot_offset);

        if (near(gizmo.corners[0])) return DragMode::ScaleTL;
        if (near(gizmo.corners[1])) return DragMode::ScaleTR;
        if (near(gizmo.corners[2])) return DragMode::ScaleBR;
        if (near(gizmo.corners[3])) return DragMode::ScaleBL;
        if (near(gizmo.rot_handle)) return DragMode::Rotate;

        return DragMode::None;
    }

    inline bool InternalImageViewer::hit_test_minimap(int mx, int my)
    {
        if (!show_minimap_) return false;
        int mm_x = x() + w() - minimap_size_ - minimap_margin_;
        int mm_y = y() + minimap_margin_;
        return mx >= mm_x && mx <= mm_x + minimap_size_ &&
               my >= mm_y && my <= mm_y + minimap_size_;
    }

    inline void InternalImageViewer::pan_minimap_to(int mx, int my)
    {
        MinimapInfo mi = get_minimap_info();
        view_x_ = mi.min_x + (mx - mi.offset_x) / mi.scale - (w() / scale_) * 0.5;
        view_y_ = mi.min_y + (my - mi.offset_y) / mi.scale - (h() / scale_) * 0.5;
        clamp_view();
        invalidate();
        notify_view_changed();
    }

    inline int InternalImageViewer::hit_test(double world_x, double world_y)
    {
        for (int i = static_cast<int>(state_->document()->layer_count()) - 1; i >= 0; --i)
        {
            if (!is_layer_visible(i)) continue;
            auto l_ptr = get_image_layer(i);
            if (!l_ptr) continue;
            const auto &l = *l_ptr;

            Rect2D b = layer_effective_rect(l);
            double local_wx, local_wy;
            world_to_layer_local(l, world_x, world_y, local_wx, local_wy);

            if (local_wx >= b.x && local_wx <= b.x + b.w &&
                local_wy >= b.y && local_wy <= b.y + b.h)
                return i;
        }
        return -1;
    }

    inline void InternalImageViewer::sample_color(int mx, int my, double world_x, double world_y)
    {
        if (!color_picked_thunk_ || !user_data_) return;

        if (auto l = get_selected_image_layer())
        {
            int sel_idx = get_selected_layer_index();
            if (l->image && l->image->handle() && is_layer_visible(sel_idx))
            {
                double local_wx, local_wy;
                world_to_layer_local(*l, world_x, world_y, local_wx, local_wy);

                int px = static_cast<int>(std::floor((local_wx - l->x) / l->scale_x));
                int py = static_cast<int>(std::floor((local_wy - l->y) / l->scale_y));
                int iw = l->image->data_w();
                int ih = l->image->data_h();

                if (px >= 0 && px < iw && py >= 0 && py < ih)
                {
                    Fl_Image *img = l->image->handle();
                    if (img->count() > 0 && img->d() >= 3)
                    {
                        const uchar *data = reinterpret_cast<const uchar *>(img->data()[0]);
                        int d = img->d();
                        int ld = img->ld() ? img->ld() : iw * d;
                        int idx = py * ld + px * d;
                        color_picked_thunk_(data[idx], data[idx + 1], data[idx + 2], (d == 4) ? data[idx + 3] : 255, user_data_);
                        return;
                    }
                }
            }
        }

        if (bg_buffer_ && mx >= 0 && mx < w() && my >= 0 && my < h())
        {
            fl_begin_offscreen(bg_buffer_);
            uchar pixel[3];
            fl_read_image(pixel, mx, my, 1, 1, 0);
            fl_end_offscreen();
            color_picked_thunk_(pixel[0], pixel[1], pixel[2], 255, user_data_);
        }
    }

    inline int InternalImageViewer::handle_keydown(int key)
    {
        if (state_->selection_ids().empty()) return 0;

        if (key == FL_Delete || key == FL_BackSpace)
        {
            perform_heavy_undoable_action([this]() {
                std::vector<int> to_delete;
                for (int id : state_->selection_ids())
                    if (int idx = state_->document()->get_layer_index(id); idx >= 0)
                        to_delete.push_back(idx);
                std::sort(to_delete.rbegin(), to_delete.rend());
                for (int idx : to_delete) state_->document()->remove_layer(idx);
                state_->clear_selection(); 
            });
            return 1;
        }

        double nudge = Fl::event_state(FL_SHIFT) ? 10.0 : 1.0;
        double dx = 0.0, dy = 0.0;
        if (key == FL_Left) dx = -nudge / scale_;
        else if (key == FL_Right) dx = nudge / scale_;
        else if (key == FL_Up) dy = -nudge / scale_;
        else if (key == FL_Down) dy = nudge / scale_;

        if (dx != 0.0 || dy != 0.0)
        {
            std::vector<int> indices;
            for (int id : state_->selection_ids())
                if (int idx = state_->document()->get_layer_index(id); idx >= 0)
                    indices.push_back(idx);

            perform_light_undoable_action(indices, [this, dx, dy]() {
                for (int id : state_->selection_ids()) {
                    int idx = state_->document()->get_layer_index(id);
                    if (auto lp = get_image_layer(idx); lp && !lp->locked) {
                        lp->x += dx;
                        lp->y += dy;
                        clamp_world_coords(lp->x, lp->y);
                    }
                } 
            });
            return 1;
        }
        return 0;
    }

    inline int InternalImageViewer::handle_mouse_push()
    {
        last_mouse_x_ = Fl::event_x();
        last_mouse_y_ = Fl::event_y();
        double world_x, world_y;
        mouse_to_world(last_mouse_x_, last_mouse_y_, world_x, world_y);
        drag_start_x_ = world_x;
        drag_start_y_ = world_y;
        drag_mode_ = DragMode::None;
        drag_undo_state_pushed_ = false;

        if (Fl::event_button() == FL_RIGHT_MOUSE)
        {
            int hit_idx = hit_test(world_x, world_y);
            if (hit_idx >= 0 && hit_idx != get_selected_layer_index())
            {
                state_->clear_selection();
                state_->set_primary_selection(state_->document()->get_layer(hit_idx)->id);
            }
            notify_right_click();
            return 1;
        }

        if (Fl::event_button() == FL_MIDDLE_MOUSE)
        {
            drag_mode_ = DragMode::Pan;
            return 1;
        }

        if (Fl::event_button() == FL_LEFT_MOUSE)
        {
            if (hit_test_minimap(last_mouse_x_, last_mouse_y_))
            {
                drag_mode_ = DragMode::MinimapPan;
                pan_minimap_to(last_mouse_x_, last_mouse_y_);
                return 1;
            }

            if (active_tool_ == ViewerTool::Select || active_tool_ == ViewerTool::Move)
            {
                if (auto lp = get_selected_image_layer(); lp && !lp->locked)
                {
                    DragMode gizmo = hit_test_gizmo(last_mouse_x_, last_mouse_y_);
                    if (is_scale_mode(gizmo) || gizmo == DragMode::Rotate)
                    {
                        drag_mode_ = gizmo;
                        capture_multi_drag_origins();
                        orig_group_bounds_ = get_selection_world_bounds();
                        if (gizmo == DragMode::Rotate)
                            group_drag_center_ = Transform::get_bounds_center(orig_group_bounds_);
                        return 1;
                    }
                }
            }

            if (active_tool_ == ViewerTool::Pan) { drag_mode_ = DragMode::Pan; return 1; }

            if (active_tool_ == ViewerTool::Crop)
            {
                if (auto lp = get_selected_image_layer())
                {
                    double local_wx, local_wy;
                    world_to_layer_local(*lp, world_x, world_y, local_wx, local_wy);
                    drag_mode_ = DragMode::Crop;
                    crop_start_x_ = crop_end_x_ = local_wx;
                    crop_start_y_ = crop_end_y_ = local_wy;
                }
                return 1;
            }

            if (active_tool_ == ViewerTool::Eyedropper)
            {
                drag_mode_ = DragMode::Eyedropper;
                sample_color(last_mouse_x_ - x(), last_mouse_y_ - y(), world_x, world_y);
                return 1;
            }

            // Fallback Layer selection
            int hit_idx = hit_test(world_x, world_y);
            bool ctrl = Fl::event_state(FL_CTRL) || Fl::event_state(FL_META);

            if (hit_idx >= 0)
            {
                int hit_id = state_->document()->get_layer(hit_idx)->id;
                if (ctrl) state_->toggle_selection(hit_id);
                else if (!state_->is_in_selection(hit_id))
                {
                    state_->clear_selection();
                    state_->set_primary_selection(hit_id);
                }
                else state_->set_primary_selection(hit_id);
            }
            else if (!ctrl) state_->clear_selection();

            if (auto lp = get_selected_image_layer(); lp && !lp->locked)
            {
                drag_mode_ = DragMode::MoveLayer;
                capture_multi_drag_origins();
            }
            else if (state_->selection_ids().empty())
            {
                drag_mode_ = DragMode::Pan;
            }
            return 1;
        }
        return 0;
    }

    inline void InternalImageViewer::apply_snapping(double& final_ddx, double& final_ddy, const Rect2D& target_bounds)
    {
        std::vector<SnapCandidate> x_snaps, y_snaps;
        const double SNAP_DIST = 10.0 / scale_;

        for (size_t i = 0; i < state_->document()->layer_count(); ++i)
        {
            auto other_layer = get_image_layer(i);
            if (!other_layer || state_->is_in_selection(other_layer->id) || !is_layer_visible(i)) continue;

            Rect2D other_bounds = get_layer_world_bounds(*other_layer);
            double line_y_start = std::min(target_bounds.y, other_bounds.y);
            double line_y_end = std::max(target_bounds.y + target_bounds.h, other_bounds.y + other_bounds.h);
            double line_x_start = std::min(target_bounds.x, other_bounds.x);
            double line_x_end = std::max(target_bounds.x + target_bounds.w, other_bounds.x + other_bounds.w);

            double h_coords[] = {target_bounds.x, target_bounds.x + target_bounds.w / 2, target_bounds.x + target_bounds.w};
            double other_h_coords[] = {other_bounds.x, other_bounds.x + other_bounds.w / 2, other_bounds.x + other_bounds.w};
            for (double hc : h_coords) for (double ohc : other_h_coords)
                if (std::abs(hc - ohc) < SNAP_DIST) x_snaps.push_back({ohc - hc, ohc, line_y_start, line_y_end, false});

            double v_coords[] = {target_bounds.y, target_bounds.y + target_bounds.h / 2, target_bounds.y + target_bounds.h};
            double other_v_coords[] = {other_bounds.y, other_bounds.y + other_bounds.h / 2, other_bounds.y + other_bounds.h};
            for (double vc : v_coords) for (double ovc : other_v_coords)
                if (std::abs(vc - ovc) < SNAP_DIST) y_snaps.push_back({ovc - vc, ovc, line_x_start, line_x_end, false});
        }

        if (!use_solid_bg_ && grid_size_ > 0)
        {
            double h_coords[] = {target_bounds.x, target_bounds.x + target_bounds.w / 2, target_bounds.x + target_bounds.w};
            for (double hc : h_coords) {
                double grid_line = std::round(hc / grid_size_) * grid_size_;
                if (std::abs(hc - grid_line) < SNAP_DIST) x_snaps.push_back({grid_line - hc, grid_line, target_bounds.y, target_bounds.y + target_bounds.h, true});
            }
            double v_coords[] = {target_bounds.y, target_bounds.y + target_bounds.h / 2, target_bounds.y + target_bounds.h};
            for (double vc : v_coords) {
                double grid_line = std::round(vc / grid_size_) * grid_size_;
                if (std::abs(vc - grid_line) < SNAP_DIST) y_snaps.push_back({grid_line - vc, grid_line, target_bounds.x, target_bounds.x + target_bounds.w, true});
            }
        }

        if (snap_to_canvas_ && state_->document()->mode() == DocumentMode::FixedCanvas)
        {
            const double canvas_w = state_->document()->canvas_width();
            const double canvas_h = state_->document()->canvas_height();
            double canvas_h_coords[] = {0, canvas_w / 2, canvas_w};
            double h_coords[] = {target_bounds.x, target_bounds.x + target_bounds.w / 2, target_bounds.x + target_bounds.w};
            for (double hc : h_coords) for (double chc : canvas_h_coords)
                if (std::abs(hc - chc) < SNAP_DIST) x_snaps.push_back({chc - hc, chc, 0, canvas_h, false});

            double canvas_v_coords[] = {0, canvas_h / 2, canvas_h};
            double v_coords[] = {target_bounds.y, target_bounds.y + target_bounds.h / 2, target_bounds.y + target_bounds.h};
            for (double vc : v_coords) for (double cvc : canvas_v_coords)
                if (std::abs(vc - cvc) < SNAP_DIST) y_snaps.push_back({cvc - vc, cvc, 0, canvas_w, false});
        }

        if (!x_snaps.empty())
        {
            const auto best_snap = *std::min_element(x_snaps.begin(), x_snaps.end(), [](const auto &a, const auto &b){ return std::abs(a.delta) < std::abs(b.delta); });
            final_ddx += best_snap.delta;
            for (const auto &snap : x_snaps) if (std::abs(snap.delta - best_snap.delta) < 1e-6)
            {
                double start = snap.line_start, end = snap.line_end;
                if (snap.is_grid_snap) { start = view_y_; end = view_y_ + h() / scale_; }
                snap_lines_.push_back({{snap.line_coord, start}, {snap.line_coord, end}});
            }
        }

        if (!y_snaps.empty())
        {
            const auto best_snap = *std::min_element(y_snaps.begin(), y_snaps.end(), [](const auto &a, const auto &b){ return std::abs(a.delta) < std::abs(b.delta); });
            final_ddy += best_snap.delta;
            for (const auto &snap : y_snaps) if (std::abs(snap.delta - best_snap.delta) < 1e-6)
            {
                double start = snap.line_start, end = snap.line_end;
                if (snap.is_grid_snap) { start = view_x_; end = view_x_ + w() / scale_; }
                snap_lines_.push_back({{start, snap.line_coord}, {end, snap.line_coord}});
            }
        }
    }

    inline void InternalImageViewer::handle_scale_drag(double world_x, double world_y)
    {
        bool proportional = Fl::event_state(FL_SHIFT);
        Point2D fixed_point;

        if (Fl::event_state(FL_CTRL)) {
            fixed_point = Transform::get_bounds_center(orig_group_bounds_);
        } else {
            switch(drag_mode_) {
                case DragMode::ScaleTL: fixed_point = {orig_group_bounds_.x + orig_group_bounds_.w, orig_group_bounds_.y + orig_group_bounds_.h}; break;
                case DragMode::ScaleTR: fixed_point = {orig_group_bounds_.x, orig_group_bounds_.y + orig_group_bounds_.h}; break;
                case DragMode::ScaleBL: fixed_point = {orig_group_bounds_.x + orig_group_bounds_.w, orig_group_bounds_.y}; break;
                case DragMode::ScaleBR: fixed_point = {orig_group_bounds_.x, orig_group_bounds_.y}; break;
                default: fixed_point = Transform::get_bounds_center(orig_group_bounds_); break;
            }
        }

        double new_group_w = std::max(5.0 / scale_, std::abs(world_x - fixed_point.x));
        double new_group_h = std::max(5.0 / scale_, std::abs(world_y - fixed_point.y));

        if (proportional && orig_group_bounds_.h > 1e-6)
        {
            double group_aspect = orig_group_bounds_.w / orig_group_bounds_.h;
            if (new_group_w / group_aspect > new_group_h) new_group_h = new_group_w / group_aspect;
            else new_group_w = new_group_h * group_aspect;
        }

        double scale_factor_x = (orig_group_bounds_.w > 1e-6) ? new_group_w / orig_group_bounds_.w : 1.0;
        double scale_factor_y = (orig_group_bounds_.h > 1e-6) ? new_group_h / orig_group_bounds_.h : 1.0;

        for (auto const &[id, origin] : multi_drag_origins_)
        {
            if (auto l = get_image_layer(state_->document()->get_layer_index(id)))
            {
                l->scale_x = origin.sx * scale_factor_x;
                l->scale_y = origin.sy * scale_factor_y;
                l->x = fixed_point.x + (origin.x - fixed_point.x) * scale_factor_x;
                l->y = fixed_point.y + (origin.y - fixed_point.y) * scale_factor_y;
            }
        }
        invalidate();
    }

    inline void InternalImageViewer::handle_rotate_drag(double world_x, double world_y)
    {
        double start_angle = atan2(drag_start_y_ - group_drag_center_.y, drag_start_x_ - group_drag_center_.x) * 180.0 / M_PI;
        double current_angle = atan2(world_y - group_drag_center_.y, world_x - group_drag_center_.x) * 180.0 / M_PI;
        double angle_delta = current_angle - start_angle;

        if (Fl::event_state(FL_SHIFT))
            angle_delta = std::round(angle_delta / 15.0) * 15.0;

        for (auto const &[id, origin] : multi_drag_origins_)
        {
            if (auto l = get_image_layer(state_->document()->get_layer_index(id)))
            {
                Point2D new_pos = Transform::rotate_point(origin.x, origin.y, group_drag_center_.x, group_drag_center_.y, angle_delta);
                l->x = new_pos.x;
                l->y = new_pos.y;
                l->rotation_angle = origin.rot + angle_delta;
            }
        }
        invalidate();
    }

    inline int InternalImageViewer::handle_mouse_drag()
    {
        double world_x, world_y;
        mouse_to_world(Fl::event_x(), Fl::event_y(), world_x, world_y);
        double dx = Fl::event_x() - last_mouse_x_;
        double dy = Fl::event_y() - last_mouse_y_;

        if ((is_scale_mode(drag_mode_) || drag_mode_ == DragMode::MoveLayer || drag_mode_ == DragMode::Rotate) && !drag_undo_record_)
        {
            std::vector<int> indices;
            for (int id : state_->selection_ids())
                if (int idx = state_->document()->get_layer_index(id); idx >= 0)
                    indices.push_back(idx);
            if (!indices.empty()) drag_undo_record_ = std::make_unique<LayerPropsCommand>(*state_, indices);
            drag_undo_state_pushed_ = true;
        }

        if (drag_mode_ == DragMode::MinimapPan) pan_minimap_to(Fl::event_x(), Fl::event_y());
        else if (drag_mode_ == DragMode::MoveLayer)
        {
            snap_lines_.clear();
            double ddx = world_x - drag_start_x_;
            double ddy = world_y - drag_start_y_;

            if (Fl::event_state(FL_SHIFT))
            {
                if (std::abs(ddx) > std::abs(ddy)) ddy = 0; else ddx = 0;
            }

            double final_ddx = ddx;
            double final_ddy = ddy;

            if (auto primary_layer = get_selected_image_layer())
            {
                const auto &origin = multi_drag_origins_.at(primary_layer->id);
                Rect2D target_bounds = get_layer_world_bounds_at(*primary_layer, origin.x + ddx, origin.y + ddy);
                apply_snapping(final_ddx, final_ddy, target_bounds);
            }

            for (auto const &[id, origin] : multi_drag_origins_)
                if (auto lp = get_image_layer(state_->document()->get_layer_index(id)))
                {
                    lp->x = origin.x + final_ddx;
                    lp->y = origin.y + final_ddy;
                    clamp_world_coords(lp->x, lp->y);
                }
            bg_dirty_ = true;
        }
        else if (drag_mode_ == DragMode::Crop)
        {
            if (auto lp = get_selected_image_layer())
            {
                double local_wx, local_wy;
                world_to_layer_local(*lp, world_x, world_y, local_wx, local_wy);

                bool proportional = Fl::event_state(FL_SHIFT);
                double cdx = local_wx - crop_start_x_;
                double cdy = local_wy - crop_start_y_;

                if (proportional) {
                    double side = std::max(std::abs(cdx), std::abs(cdy));
                    crop_end_x_ = crop_start_x_ + side * (cdx > 0 ? 1 : -1);
                    crop_end_y_ = crop_start_y_ + side * (cdy > 0 ? 1 : -1);
                } else {
                    crop_end_x_ = local_wx;
                    crop_end_y_ = local_wy;
                }
            }
        }
        else if (drag_mode_ == DragMode::Eyedropper) sample_color(Fl::event_x() - x(), Fl::event_y() - y(), world_x, world_y);
        else if (is_scale_mode(drag_mode_) && !multi_drag_origins_.empty()) handle_scale_drag(world_x, world_y);
        else if (drag_mode_ == DragMode::Rotate && !multi_drag_origins_.empty()) handle_rotate_drag(world_x, world_y);
        else if (drag_mode_ == DragMode::Pan || Fl::event_state(FL_MIDDLE_MOUSE))
        {
            view_x_ -= dx / scale_;
            view_y_ -= dy / scale_;
            clamp_view();
            bg_dirty_ = true;
        }

        last_mouse_x_ = Fl::event_x();
        last_mouse_y_ = Fl::event_y();
        redraw();
        return 1;
    }

    inline int InternalImageViewer::handle_mouse_release()
    {
        if (drag_undo_state_pushed_ && drag_undo_record_)
        {
            drag_undo_record_->capture_after(*state_);
            undo_mgr_->push(std::move(drag_undo_record_));
        }
        drag_undo_record_ = nullptr;
        drag_undo_state_pushed_ = false;

        if (!snap_lines_.empty()) { snap_lines_.clear(); redraw(); }

        if (drag_mode_ == DragMode::Crop)
        {
            int sel_idx = get_selected_layer_index();
            if (sel_idx >= 0)
            {
                auto &l = *get_image_layer(sel_idx);
                if (l.image && l.image->handle())
                {
                    CropDragBox box = get_crop_drag_box();
                    int iw = l.image->data_w(), ih = l.image->data_h();
                    double cx_c = std::clamp((box.min_lx - l.x) / l.scale_x, 0.0, (double)iw);
                    double cy_c = std::clamp((box.min_ly - l.y) / l.scale_y, 0.0, (double)ih);
                    double cw_c = std::clamp(box.w() / l.scale_x, 0.0, (double)iw - cx_c);
                    double ch_c = std::clamp(box.h() / l.scale_y, 0.0, (double)ih - cy_c);

                    if (cw_c > 1.0 && ch_c > 1.0)
                    {
                        Point2D C = Transform::get_bounds_center(layer_effective_rect(l));
                        double C_ux = l.x + cx_c * l.scale_x + (cw_c * l.scale_x) * 0.5;
                        double C_uy = l.y + cy_c * l.scale_y + (ch_c * l.scale_y) * 0.5;

                        Point2D W = Transform::local_to_world(C_ux, C_uy, C.x, C.y, l.rotation_angle, l.flip_h, l.flip_v);
                        perform_heavy_undoable_action([&]() {
                            l.crop_x = cx_c; l.crop_y = cy_c;
                            l.crop_w = cw_c; l.crop_h = ch_c;
                            l.x += (W.x - C_ux);
                            l.y += (W.y - C_uy);
                        });
                    }
                }
            }
            bg_dirty_ = true;
            redraw();
        }

        drag_mode_ = DragMode::None;
        notify_view_changed();
        return 1;
    }

    inline int InternalImageViewer::handle(int event)
    {
        if (active_tool_state_)
        {
            double wx, wy;
            mouse_to_world(Fl::event_x(), Fl::event_y(), wx, wy);
            ViewerMouseEvent e = {this, Fl::event_x(), Fl::event_y(), wx, wy};
            int res = 0;
            switch (event)
            {
            case FL_PUSH: res = active_tool_state_->on_mouse_down(e); break;
            case FL_DRAG: res = active_tool_state_->on_mouse_drag(e, Fl::event_dx(), Fl::event_dy()); break;
            case FL_RELEASE: res = active_tool_state_->on_mouse_up(e); break;
            case FL_KEYBOARD: res = active_tool_state_->on_key_press(this, Fl::event_key()); break;
            }
            if (res) { redraw(); return res; }
        }

        switch (event)
        {
            case FL_KEYBOARD: return handle_keydown(Fl::event_key());
            case FL_PUSH: return handle_mouse_push();
            case FL_DRAG: return handle_mouse_drag();
            case FL_RELEASE: return handle_mouse_release();
            case FL_MOUSEWHEEL:
            {
                if (Fl::event_dy() == 0) return 0;
                double old_scale = scale_;
                double mouse_wx = view_x_ + (Fl::event_x() - x()) / old_scale;
                double mouse_wy = view_y_ + (Fl::event_y() - y()) / old_scale;

                scale_ *= std::pow(1.25, -Fl::event_dy());
                scale_ = std::clamp(scale_, 0.01, 100.0);

                if (scale_ == old_scale) return 1;

                view_x_ = mouse_wx - (Fl::event_x() - x()) / scale_;
                view_y_ = mouse_wy - (Fl::event_y() - y()) / scale_;
                clamp_view();
                bg_dirty_ = true;
                redraw();
                notify_view_changed();
                return 1;
            }
            case FL_MOVE:
            {
                DragMode new_hover = hit_test_gizmo(Fl::event_x(), Fl::event_y());
                if (new_hover != hover_mode_) { hover_mode_ = new_hover; redraw(); }
                return 1;
            }
            case FL_ENTER: return 1;
            case FL_LEAVE:
            {
                if (hover_mode_ != DragMode::None) { hover_mode_ = DragMode::None; redraw(); }
                return 1;
            }
        }
        return Fl_Widget::handle(event);
    }
}