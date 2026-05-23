#pragma once

#include "Internal.hpp"
#include <cmath>
#include <algorithm>
#include <memory>

namespace mui
{
    inline DragMode InternalImageViewer::hit_test_gizmo(int mx, int my)
    {
        auto l_ptr = get_selected_image_layer();
        if (!l_ptr || l_ptr->locked)
            return DragMode::None;
        const auto &l = *l_ptr;

        double world_mx, world_my;
        mouse_to_world(mx, my, world_mx, world_my);

        double local_wx, local_wy;
        world_to_layer_local(l, world_mx, world_my, local_wx, local_wy);

        Rect2D b = layer_effective_rect(l);
        double lx = b.x, ly = b.y, lw = b.w, lh = b.h;
        double r = (ThemeManager::get_palette().metrics.imageviewer_handle_size * 0.5 +
                    ThemeManager::get_palette().metrics.imageviewer_handle_hit_padding) /
                   scale_;

        auto near = [&](double px, double py)
        {
            return local_wx >= px - r && local_wx <= px + r &&
                   local_wy >= py - r && local_wy <= py + r;
        };

        if (near(lx, ly))
            return DragMode::ScaleTL;
        if (near(lx + lw, ly))
            return DragMode::ScaleTR;
        if (near(lx, ly + lh))
            return DragMode::ScaleBL;
        if (near(lx + lw, ly + lh))
            return DragMode::ScaleBR;
        return DragMode::None;
    }

    inline bool InternalImageViewer::hit_test_minimap(int mx, int my)
    {
        if (!show_minimap_)
            return false;
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
            if (!is_layer_visible(i))
                continue;
            auto l_ptr = get_image_layer(i);
            if (!l_ptr)
                continue;
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
        if (!color_picked_thunk_ || !user_data_)
            return;

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
                        color_picked_thunk_(data[idx], data[idx + 1], data[idx + 2],
                                            (d == 4) ? data[idx + 3] : 255, user_data_);
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
            case FL_PUSH:
                res = active_tool_state_->on_mouse_down(e);
                break;
            case FL_DRAG:
                res = active_tool_state_->on_mouse_drag(e, Fl::event_dx(), Fl::event_dy());
                break;
            case FL_RELEASE:
                res = active_tool_state_->on_mouse_up(e);
                break;
            case FL_KEYBOARD:
                res = active_tool_state_->on_key_press(this, Fl::event_key());
                break;
            }
            if (res)
            {
                redraw();
                return res;
            }
        }

        switch (event)
        {
        case FL_KEYBOARD:
        {
            if (state_->selection_ids().empty())
                return 0;

            int key = Fl::event_key();

            if (key == FL_Delete || key == FL_BackSpace)
            {
                if (!state_->selection_ids().empty())
                {
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
                }
                return 1;
            }

            double nudge = Fl::event_state(FL_SHIFT) ? 10.0 : 1.0;
            double dx = 0.0, dy = 0.0;
            switch (key)
            {
            case FL_Left:
                dx = -nudge / scale_;
                break;
            case FL_Right:
                dx = nudge / scale_;
                break;
            case FL_Up:
                dy = -nudge / scale_;
                break;
            case FL_Down:
                dy = nudge / scale_;
                break;
            }
            if (dx != 0.0 || dy != 0.0)
            {
                std::vector<int> indices;
                for (int id : state_->selection_ids())
                    if (int idx = state_->document()->get_layer_index(id); idx >= 0)
                        indices.push_back(idx);

                perform_light_undoable_action(indices, [this, dx, dy]()
                                              {
                    for (int id : state_->selection_ids()) {
                        int idx = state_->document()->get_layer_index(id);
                        auto lp = get_image_layer(idx);
                        if (lp && !lp->locked) {
                            lp->x += dx;
                            lp->y += dy;
                            clamp_world_coords(lp->x, lp->y);
                        }
                    } });
                return 1;
            }
            break;
        }

        case FL_PUSH:
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
                        if (is_scale_mode(gizmo))
                        {
                            drag_mode_ = gizmo;
                            orig_layer_sx_ = lp->scale_x;
                            orig_layer_sy_ = lp->scale_y;
                            orig_layer_x_ = lp->x;
                            orig_layer_y_ = lp->y;
                            return 1;
                        }
                    }
                }

                if (active_tool_ == ViewerTool::Pan)
                {
                    drag_mode_ = DragMode::Pan;
                    return 1;
                }

                if (active_tool_ == ViewerTool::Crop)
                {
                    if (auto lp = get_selected_image_layer())
                    {
                        double local_wx, local_wy;
                        world_to_layer_local(*lp, world_x, world_y, local_wx, local_wy);
                        drag_mode_ = DragMode::Crop;
                        crop_start_x_ = local_wx;
                        crop_end_x_ = local_wx;
                        crop_start_y_ = local_wy;
                        crop_end_y_ = local_wy;
                    }
                    return 1;
                }

                if (active_tool_ == ViewerTool::Eyedropper)
                {
                    drag_mode_ = DragMode::Eyedropper;
                    sample_color(last_mouse_x_ - x(), last_mouse_y_ - y(), world_x, world_y);
                    return 1;
                }

                {
                    int hit_idx = hit_test(world_x, world_y);
                    bool ctrl = Fl::event_state(FL_CTRL) || Fl::event_state(FL_META);

                    if (hit_idx >= 0)
                    {
                        int hit_id = state_->document()->get_layer(hit_idx)->id;
                        if (ctrl)
                        {
                            state_->toggle_selection(hit_id);
                        }
                        else if (!state_->is_in_selection(hit_id))
                        {
                            state_->clear_selection();
                            state_->set_primary_selection(hit_id);
                        }
                        else
                        {
                            state_->set_primary_selection(hit_id);
                        }
                    }
                    else if (!ctrl)
                    {
                        state_->clear_selection();
                    }

                    if (auto lp = get_selected_image_layer(); lp && !lp->locked)
                    {
                        drag_mode_ = DragMode::MoveLayer;
                        orig_layer_x_ = lp->x;
                        orig_layer_y_ = lp->y;
                        capture_multi_drag_origins();
                    }
                    else if (state_->selection_ids().empty())
                    {
                        drag_mode_ = DragMode::Pan;
                    }
                    return 1;
                }
            }
            break;
        }

        case FL_DRAG:
        {
            double world_x, world_y;
            mouse_to_world(Fl::event_x(), Fl::event_y(), world_x, world_y);
            double dx = Fl::event_x() - last_mouse_x_;
            double dy = Fl::event_y() - last_mouse_y_;

            if ((is_scale_mode(drag_mode_) || drag_mode_ == DragMode::MoveLayer) && !drag_undo_record_)
            {
                std::vector<int> indices;
                for (int id : state_->selection_ids())
                {
                    int idx = state_->document()->get_layer_index(id);
                    if (idx >= 0)
                        indices.push_back(idx);
                }
                drag_undo_record_ = std::make_unique<LayerPropsCommand>(*state_, indices);
                drag_undo_state_pushed_ = true;
            }

            if (drag_mode_ == DragMode::MinimapPan)
            {
                pan_minimap_to(Fl::event_x(), Fl::event_y());
                return 1;
            }

            if (drag_mode_ == DragMode::MoveLayer)
            {
                double ddx = world_x - drag_start_x_;
                double ddy = world_y - drag_start_y_;

                if (state_->selection_ids().size() > 1)
                {
                    for (auto &[id, origin] : multi_drag_origins_)
                    {
                        int idx = state_->document()->get_layer_index(id);
                        if (idx < 0)
                            continue;
                        if (auto lp = get_image_layer(idx))
                        {
                            lp->x = origin.first + ddx;
                            lp->y = origin.second + ddy;
                            clamp_world_coords(lp->x, lp->y);
                        }
                    }
                }
                else if (auto lp = get_selected_image_layer())
                {
                    double target_x = orig_layer_x_ + ddx;
                    double target_y = orig_layer_y_ + ddy;
                    const double SNAP = 10.0 / scale_;
                    for (size_t i = 0; i < state_->document()->layer_count(); ++i)
                    {
                        if ((int)i == get_selected_layer_index())
                            continue;
                        if (auto other = get_image_layer(i))
                        {
                            if (std::abs(target_x - other->x) < SNAP)
                                target_x = other->x;
                            if (std::abs(target_y - other->y) < SNAP)
                                target_y = other->y;
                        }
                    }
                    lp->x = target_x;
                    lp->y = target_y;
                    clamp_world_coords(lp->x, lp->y);
                }
                bg_dirty_ = true;
                last_mouse_x_ = Fl::event_x();
                last_mouse_y_ = Fl::event_y();
                redraw();
                return 1;
            }

            if (drag_mode_ == DragMode::Crop)
            {
                if (auto lp = get_selected_image_layer())
                {
                    double local_wx, local_wy;
                    world_to_layer_local(*lp, world_x, world_y, local_wx, local_wy);
                    crop_end_x_ = local_wx;
                    crop_end_y_ = local_wy;
                }
                last_mouse_x_ = Fl::event_x();
                last_mouse_y_ = Fl::event_y();
                redraw();
                return 1;
            }

            if (drag_mode_ == DragMode::Eyedropper)
            {
                sample_color(Fl::event_x() - x(), Fl::event_y() - y(), world_x, world_y);
                last_mouse_x_ = Fl::event_x();
                last_mouse_y_ = Fl::event_y();
                return 1;
            }

            if (is_scale_mode(drag_mode_) && get_selected_image_layer())
            {
                auto &l = *get_selected_image_layer();
                double base_w = (l.crop_w >= 0) ? l.crop_w : l.original_w;
                double base_h = (l.crop_h >= 0) ? l.crop_h : l.original_h;
                Rect2D b = l.get_effective_bounds();
                double orig_draw_w = base_w * orig_layer_sx_;
                double orig_draw_h = base_h * orig_layer_sy_;
                double hw = orig_draw_w * 0.5, hh = orig_draw_h * 0.5;

                double fx = (drag_mode_ == DragMode::ScaleTL || drag_mode_ == DragMode::ScaleBL) ? hw : -hw;
                double fy = (drag_mode_ == DragMode::ScaleTL || drag_mode_ == DragMode::ScaleTR) ? hh : -hh;
                double d_x = (drag_mode_ == DragMode::ScaleTL || drag_mode_ == DragMode::ScaleBL) ? -1.0 : 1.0;
                double d_y = (drag_mode_ == DragMode::ScaleTL || drag_mode_ == DragMode::ScaleTR) ? -1.0 : 1.0;

                Point2D orig_c = {(orig_layer_x_ + (b.x - l.x)) + hw, (orig_layer_y_ + (b.y - l.y)) + hh};
                Point2D w_fixed = Transform::local_to_world(orig_c.x + fx, orig_c.y + fy,
                                                            orig_c.x, orig_c.y,
                                                            l.rotation_angle, l.flip_h, l.flip_v);
                Point2D drag_loc = Transform::world_to_local(world_x, world_y,
                                                             w_fixed.x, w_fixed.y,
                                                             l.rotation_angle, l.flip_h, l.flip_v);

                double new_w = std::max(5.0 / scale_, (drag_loc.x - w_fixed.x) * d_x);
                double new_h = std::max(5.0 / scale_, (drag_loc.y - w_fixed.y) * d_y);

                l.scale_x = new_w / base_w;
                l.scale_y = new_h / base_h;

                double nfx = (drag_mode_ == DragMode::ScaleTL || drag_mode_ == DragMode::ScaleBL) ? new_w / 2 : -new_w / 2;
                double nfy = (drag_mode_ == DragMode::ScaleTL || drag_mode_ == DragMode::ScaleTR) ? new_h / 2 : -new_h / 2;
                Point2D r_n = Transform::rotate_point(l.flip_h ? -nfx : nfx,
                                                      l.flip_v ? -nfy : nfy, 0, 0, l.rotation_angle);
                l.x = (w_fixed.x - r_n.x) - new_w * 0.5 - (l.crop_w >= 0 ? l.crop_x * l.scale_x : 0);
                l.y = (w_fixed.y - r_n.y) - new_h * 0.5 - (l.crop_h >= 0 ? l.crop_y * l.scale_y : 0);
                invalidate();
            }

            if (drag_mode_ == DragMode::Pan || Fl::event_state(FL_MIDDLE_MOUSE))
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

        case FL_RELEASE:
        {
            if (drag_undo_state_pushed_ && drag_undo_record_)
            {
                drag_undo_record_->capture_after(*state_);
                undo_mgr_->push(std::move(drag_undo_record_));
            }
            drag_undo_record_ = nullptr;
            drag_undo_state_pushed_ = false;

            if (drag_mode_ == DragMode::Crop)
            {
                int sel_idx = get_selected_layer_index();
                if (sel_idx >= 0)
                {
                    auto &l = *get_image_layer(sel_idx);
                    if (l.image && l.image->handle())
                    {
                        int iw = l.image->data_w();
                        int ih = l.image->data_h();

                        double min_lx = std::min(crop_start_x_, crop_end_x_);
                        double min_ly = std::min(crop_start_y_, crop_end_y_);
                        double max_lx = std::max(crop_start_x_, crop_end_x_);
                        double max_ly = std::max(crop_start_y_, crop_end_y_);

                        double cx_c = std::clamp((min_lx - l.x) / l.scale_x, 0.0, (double)iw);
                        double cy_c = std::clamp((min_ly - l.y) / l.scale_y, 0.0, (double)ih);
                        double cw_c = std::clamp((max_lx - min_lx) / l.scale_x, 0.0, (double)iw - cx_c);
                        double ch_c = std::clamp((max_ly - min_ly) / l.scale_y, 0.0, (double)ih - cy_c);

                        if (cw_c > 1.0 && ch_c > 1.0)
                        {
                            Rect2D cur = layer_effective_rect(l);
                            double C_x = cur.x + cur.w * 0.5;
                            double C_y = cur.y + cur.h * 0.5;

                            double new_lx = l.x + cx_c * l.scale_x;
                            double new_ly = l.y + cy_c * l.scale_y;
                            double new_lw = cw_c * l.scale_x;
                            double new_lh = ch_c * l.scale_y;
                            double C_ux = new_lx + new_lw * 0.5;
                            double C_uy = new_ly + new_lh * 0.5;

                            Point2D W = Transform::local_to_world(C_ux, C_uy, C_x, C_y,
                                                                  l.rotation_angle, l.flip_h, l.flip_v);
                            perform_heavy_undoable_action(
                                [&]()
                                {
                                    l.crop_x = cx_c;
                                    l.crop_y = cy_c;
                                    l.crop_w = cw_c;
                                    l.crop_h = ch_c;
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

        case FL_MOUSEWHEEL:
        {
            if (Fl::event_dy() == 0)
                return 0;

            double old_scale = scale_;
            double mouse_wx = view_x_ + (Fl::event_x() - x()) / old_scale;
            double mouse_wy = view_y_ + (Fl::event_y() - y()) / old_scale;

            scale_ *= std::pow(1.25, -Fl::event_dy());
            scale_ = std::clamp(scale_, 0.01, 100.0);

            if (scale_ == old_scale)
                return 1;

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
            if (new_hover != hover_mode_)
            {
                hover_mode_ = new_hover;
                redraw();
            }
            return 1;
        }

        case FL_ENTER:
            return 1;

        case FL_LEAVE:
            if (hover_mode_ != DragMode::None)
            {
                hover_mode_ = DragMode::None;
                redraw();
            }
            return 1;
        }

        return Fl_Widget::handle(event);
    }
}