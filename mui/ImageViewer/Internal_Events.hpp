#pragma once

#include "Internal.hpp"
#include <cmath>
#include <algorithm>
#include <memory>

namespace mui
{
    inline InternalImageViewer::DragMode InternalImageViewer::hit_test_gizmo(int mx, int my)
    {
        if (selected_layer_index_ >= 0 && selected_layer_index_ < (int)document_->layer_count())
        {
            auto l_ptr = std::static_pointer_cast<ImageLayer>(document_->get_layer(selected_layer_index_));
            if (l_ptr->locked)
                return None;
            const auto &l = *l_ptr;

            double world_mx = view_x_ + (mx - x()) / scale_;
            double world_my = view_y_ + (my - y()) / scale_;

            double local_wx, local_wy;
            world_to_layer_local(l, world_mx, world_my, local_wx, local_wy);

            double lx = l.x;
            double ly = l.y;
            double lw = l.original_w * l.scale_x;
            double lh = l.original_h * l.scale_y;
            if (l.crop_w >= 0 && l.crop_h >= 0)
            {
                lx += l.crop_x * l.scale_x;
                ly += l.crop_y * l.scale_y;
                lw = l.crop_w * l.scale_x;
                lh = l.crop_h * l.scale_y;
            }

            double r_world = (ThemeManager::get_palette().metrics.imageviewer_handle_size * 0.5 + ThemeManager::get_palette().metrics.imageviewer_handle_hit_padding) / scale_;

            if (local_wx >= lx - r_world && local_wx <= lx + r_world && local_wy >= ly - r_world && local_wy <= ly + r_world)
                return ScaleTL;
            if (local_wx >= lx + lw - r_world && local_wx <= lx + lw + r_world && local_wy >= ly - r_world && local_wy <= ly + r_world)
                return ScaleTR;
            if (local_wx >= lx - r_world && local_wx <= lx + r_world && local_wy >= ly + lh - r_world && local_wy <= ly + lh + r_world)
                return ScaleBL;
            if (local_wx >= lx + lw - r_world && local_wx <= lx + lw + r_world && local_wy >= ly + lh - r_world && local_wy <= ly + lh + r_world)
                return ScaleBR;
        }
        return None;
    }

    inline bool InternalImageViewer::hit_test_minimap(int mx, int my)
    {
        if (!show_minimap_)
            return false;
        int mm_x = x() + w() - minimap_size_ - minimap_margin_;
        int mm_y = y() + minimap_margin_;
        return (mx >= mm_x && mx <= mm_x + minimap_size_ && my >= mm_y && my <= mm_y + minimap_size_);
    }

    inline void InternalImageViewer::pan_minimap_to(int mx, int my)
    {
        MinimapInfo mi = get_minimap_info();
        double world_click_x = mi.min_x + (mx - mi.offset_x) / mi.scale;
        double world_click_y = mi.min_y + (my - mi.offset_y) / mi.scale;

        view_x_ = world_click_x - ((w() / scale_) * 0.5);
        view_y_ = world_click_y - ((h() / scale_) * 0.5);

        clamp_view();
        invalidate();
        notify_view_changed();
    }

    inline int InternalImageViewer::hit_test(double world_x, double world_y)
    {
        for (int i = (int)document_->layer_count() - 1; i >= 0; --i)
        {
            const auto &l = *std::static_pointer_cast<ImageLayer>(document_->get_layer(i));
            if (!is_layer_visible(i))
                continue;

            double lx = l.x;
            double ly = l.y;
            double lw = l.original_w * l.scale_x;
            double lh = l.original_h * l.scale_y;
            if (l.crop_w >= 0 && l.crop_h >= 0)
            {
                lx += l.crop_x * l.scale_x;
                ly += l.crop_y * l.scale_y;
                lw = l.crop_w * l.scale_x;
                lh = l.crop_h * l.scale_y;
            }

            double local_wx, local_wy;
            world_to_layer_local(l, world_x, world_y, local_wx, local_wy);

            if (local_wx >= lx && local_wx <= lx + lw && local_wy >= ly && local_wy <= ly + lh)
            {
                return i;
            }
        }
        return -1;
    }

    inline void InternalImageViewer::sample_color(int mx, int my, double world_x, double world_y)
    {
        if (selected_layer_index_ >= 0 && selected_layer_index_ < (int)document_->layer_count())
        {
            auto l = std::static_pointer_cast<ImageLayer>(document_->get_layer(selected_layer_index_));
            if (l->image && l->image->handle() && l->visible)
            {
                double local_wx, local_wy;
                world_to_layer_local(*l, world_x, world_y, local_wx, local_wy);

                double dx = local_wx - l->x;
                double dy = local_wy - l->y;

                int pixel_x = static_cast<int>(std::floor(dx / l->scale_x));
                int pixel_y = static_cast<int>(std::floor(dy / l->scale_y));

                int img_w = l->image->data_w();
                int img_h = l->image->data_h();

                if (pixel_x >= 0 && pixel_x < img_w && pixel_y >= 0 && pixel_y < img_h)
                {
                    Fl_Image *base_img = l->image->handle();
                    if (base_img->count() > 0 && base_img->d() >= 3)
                    {
                        const uchar *fg = (const uchar *)base_img->data()[0];
                        int fg_d = base_img->d();
                        int fg_ld = base_img->ld() ? base_img->ld() : img_w * fg_d;

                        int fg_idx = pixel_y * fg_ld + pixel_x * fg_d;
                        if (color_picked_thunk_ && user_data_)
                        {
                            color_picked_thunk_(fg[fg_idx], fg[fg_idx + 1], fg[fg_idx + 2], (fg_d == 4) ? fg[fg_idx + 3] : 255, user_data_);
                            return;
                        }
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
            if (color_picked_thunk_ && user_data_)
                color_picked_thunk_(pixel[0], pixel[1], pixel[2], 255, user_data_);
        }
    }

    inline int InternalImageViewer::handle(int event)
    {
        if (active_tool_state_)
        {
            double world_x = view_x_ + (Fl::event_x() - x()) / scale_;
            double world_y = view_y_ + (Fl::event_y() - y()) / scale_;
            int res = 0;
            switch (event)
            {
            case FL_PUSH:
                res = active_tool_state_->on_mouse_down(this, Fl::event_x(), Fl::event_y(), world_x, world_y);
                break;
            case FL_DRAG:
                res = active_tool_state_->on_mouse_drag(this, Fl::event_x(), Fl::event_y(), world_x, world_y, Fl::event_dx(), Fl::event_dy());
                break;
            case FL_RELEASE:
                res = active_tool_state_->on_mouse_up(this, Fl::event_x(), Fl::event_y(), world_x, world_y);
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
            if (selected_layer_index_ >= 0 && selected_layer_index_ < (int)document_->layer_count() && !document_->get_layer(selected_layer_index_)->locked)
            {
                auto l = std::static_pointer_cast<ImageLayer>(document_->get_layer(selected_layer_index_));
                if (Fl::event_key() == FL_Delete || Fl::event_key() == FL_BackSpace)
                {
                    auto cmd = std::make_shared<CommandDelete>(selected_layer_index_, document_->get_layer(selected_layer_index_));
                    push_command(cmd);
                    return 1;
                }
                double nudge = Fl::event_state(FL_SHIFT) ? 10.0 : 1.0;
                if (Fl::event_key() == FL_Left)
                {
                    l->x -= nudge / scale_;
                    bg_dirty_ = true;
                    redraw();
                    return 1;
                }
                if (Fl::event_key() == FL_Right)
                {
                    l->x += nudge / scale_;
                    bg_dirty_ = true;
                    redraw();
                    return 1;
                }
                if (Fl::event_key() == FL_Up)
                {
                    l->y -= nudge / scale_;
                    bg_dirty_ = true;
                    redraw();
                    return 1;
                }
                if (Fl::event_key() == FL_Down)
                {
                    l->y += nudge / scale_;
                    bg_dirty_ = true;
                    redraw();
                    return 1;
                }
            }
            break;
        }
        case FL_PUSH:
        {
            last_mouse_x_ = Fl::event_x();
            last_mouse_y_ = Fl::event_y();
            drag_start_x_ = view_x_ + (last_mouse_x_ - x()) / scale_;
            drag_start_y_ = view_y_ + (last_mouse_y_ - y()) / scale_;
            drag_mode_ = None;

            double world_x = view_x_ + (last_mouse_x_ - x()) / scale_;
            double world_y = view_y_ + (last_mouse_y_ - y()) / scale_;

            if (Fl::event_button() == FL_RIGHT_MOUSE)
            {
                int hit_index = hit_test(world_x, world_y);
                if (hit_index != -1 && hit_index != selected_layer_index_)
                {
                    selected_layer_index_ = hit_index;
                    notify_layer_selected();
                    redraw();
                }
                notify_right_click();
                return 1;
            }

            if (Fl::event_button() == FL_LEFT_MOUSE)
            {
                if (hit_test_minimap(last_mouse_x_, last_mouse_y_))
                {
                    drag_mode_ = MinimapPan;
                    pan_minimap_to(last_mouse_x_, last_mouse_y_);
                    return 1;
                }
                if ((active_tool_ == ViewerTool::Select || active_tool_ == ViewerTool::Move) &&
                    selected_layer_index_ >= 0 && selected_layer_index_ < (int)document_->layer_count() && !document_->get_layer(selected_layer_index_)->locked)
                {
                    DragMode mode = hit_test_gizmo(last_mouse_x_, last_mouse_y_);
                    if (mode == ScaleTL || mode == ScaleTR || mode == ScaleBL || mode == ScaleBR)
                    {
                        drag_mode_ = mode;
                        auto l = std::static_pointer_cast<ImageLayer>(document_->get_layer(selected_layer_index_));
                        orig_layer_sx_ = l->scale_x;
                        orig_layer_sy_ = l->scale_y;
                        orig_layer_x_ = l->x;
                        orig_layer_y_ = l->y;
                        return 1;
                    }
                }

                if (active_tool_ == ViewerTool::Pan)
                {
                    drag_mode_ = Pan;
                }
                else if (active_tool_ == ViewerTool::Crop)
                {
                    if (selected_layer_index_ >= 0 && selected_layer_index_ < (int)document_->layer_count())
                    {
                        const auto &l = *std::static_pointer_cast<ImageLayer>(document_->get_layer(selected_layer_index_));
                        double local_wx, local_wy;
                        world_to_layer_local(l, world_x, world_y, local_wx, local_wy);

                        drag_mode_ = Crop;
                        crop_start_x_ = local_wx;
                        crop_start_y_ = local_wy;
                        crop_end_x_ = local_wx;
                        crop_end_y_ = local_wy;
                    }
                }
                else if (active_tool_ == ViewerTool::Eyedropper)
                {
                    drag_mode_ = Eyedropper;
                    sample_color(last_mouse_x_ - x(), last_mouse_y_ - y(), world_x, world_y);
                }
                else if (active_tool_ == ViewerTool::Move)
                {
                    int hit_index = hit_test(world_x, world_y);
                    if (hit_index != selected_layer_index_)
                    {
                        selected_layer_index_ = hit_index;
                        notify_layer_selected();
                        redraw();
                    }
                    if (selected_layer_index_ != -1 && selected_layer_index_ < (int)document_->layer_count() && !document_->get_layer(selected_layer_index_)->locked)
                    {
                        drag_mode_ = MoveLayer;
                        auto l = std::static_pointer_cast<ImageLayer>(document_->get_layer(selected_layer_index_));
                        orig_layer_x_ = l->x;
                        orig_layer_y_ = l->y;
                    }
                }
                else
                {
                    int hit_index = hit_test(world_x, world_y);

                    if (hit_index != selected_layer_index_)
                    {
                        selected_layer_index_ = hit_index;
                        notify_layer_selected();
                        redraw();
                    }

                    if (selected_layer_index_ != -1 && selected_layer_index_ < (int)document_->layer_count() && !document_->get_layer(selected_layer_index_)->locked)
                    {
                        drag_mode_ = MoveLayer;
                        auto l = std::static_pointer_cast<ImageLayer>(document_->get_layer(selected_layer_index_));
                        orig_layer_x_ = l->x;
                        orig_layer_y_ = l->y;
                    }
                    else
                    {
                        drag_mode_ = Pan;
                    }
                }
                return 1;
            }

            if (Fl::event_button() == FL_MIDDLE_MOUSE)
            {
                drag_mode_ = Pan;
                return 1;
            }
            break;
        }
        case FL_DRAG:
        {
            double world_x = view_x_ + (Fl::event_x() - x()) / scale_;
            double world_y = view_y_ + (Fl::event_y() - y()) / scale_;
            double dx = Fl::event_x() - last_mouse_x_;
            double dy = Fl::event_y() - last_mouse_y_;

            if (drag_mode_ == MinimapPan)
            {
                pan_minimap_to(Fl::event_x(), Fl::event_y());
                return 1;
            }
            else if (drag_mode_ == MoveLayer && selected_layer_index_ >= 0 && selected_layer_index_ < (int)document_->layer_count())
            {
                double target_x = orig_layer_x_ + (world_x - drag_start_x_);
                double target_y = orig_layer_y_ + (world_y - drag_start_y_);
                const double SNAP_THRESHOLD = 10.0 / scale_;
                for (size_t i = 0; i < document_->layer_count(); ++i)
                {
                    if ((int)i == selected_layer_index_)
                        continue;
                    auto other_l = std::static_pointer_cast<ImageLayer>(document_->get_layer(i));
                    if (std::abs(target_x - other_l->x) < SNAP_THRESHOLD)
                        target_x = other_l->x;
                    if (std::abs(target_y - other_l->y) < SNAP_THRESHOLD)
                        target_y = other_l->y;
                }

                auto l = std::static_pointer_cast<ImageLayer>(document_->get_layer(selected_layer_index_));
                l->x = target_x;
                l->y = target_y;
                bg_dirty_ = true;
                clamp_view();
            }
            else if (drag_mode_ == Crop)
            {
                const auto &l = *std::static_pointer_cast<ImageLayer>(document_->get_layer(selected_layer_index_));
                double local_wx, local_wy;
                world_to_layer_local(l, world_x, world_y, local_wx, local_wy);

                crop_end_x_ = local_wx;
                crop_end_y_ = local_wy;
                redraw();
                return 1;
            }
            else if (drag_mode_ == Eyedropper)
            {
                sample_color(Fl::event_x() - x(), Fl::event_y() - y(), world_x, world_y);
                return 1;
            }
            else if ((drag_mode_ == ScaleBR || drag_mode_ == ScaleTL || drag_mode_ == ScaleTR || drag_mode_ == ScaleBL) &&
                     selected_layer_index_ >= 0 && selected_layer_index_ < (int)document_->layer_count())
            {
                auto &l = *std::static_pointer_cast<ImageLayer>(document_->get_layer(selected_layer_index_));
                Rect2D b = l.get_effective_bounds();
                double orig_draw_w = b.w * orig_layer_sx_ / l.scale_x;
                double orig_draw_h = b.h * orig_layer_sy_ / l.scale_y;
                double hw = orig_draw_w * 0.5, hh = orig_draw_h * 0.5;
                double fx = (drag_mode_ == ScaleTL || drag_mode_ == ScaleBL) ? hw : -hw;
                double fy = (drag_mode_ == ScaleTL || drag_mode_ == ScaleTR) ? hh : -hh;
                double d_x = (drag_mode_ == ScaleTL || drag_mode_ == ScaleBL) ? -1.0 : 1.0;
                double d_y = (drag_mode_ == ScaleTL || drag_mode_ == ScaleTR) ? -1.0 : 1.0;

                Point2D orig_c = {(orig_layer_x_ + (b.x - l.x)) + hw, (orig_layer_y_ + (b.y - l.y)) + hh};
                Point2D w_fixed = Transform::local_to_world(orig_c.x + fx, orig_c.y + fy, orig_c.x, orig_c.y, l.rotation_angle, l.flip_h, l.flip_v);
                Point2D drag_local = Transform::world_to_local(world_x, world_y, w_fixed.x, w_fixed.y, l.rotation_angle, l.flip_h, l.flip_v);

                double new_w = std::max(5.0 / scale_, (drag_local.x - w_fixed.x) * d_x);
                double new_h = std::max(5.0 / scale_, (drag_local.y - w_fixed.y) * d_y);

                l.scale_x = new_w / (b.w / l.scale_x);
                l.scale_y = new_h / (b.h / l.scale_y);

                double nfx = (drag_mode_ == ScaleTL || drag_mode_ == ScaleBL) ? (new_w / 2) : -(new_w / 2);
                double nfy = (drag_mode_ == ScaleTL || drag_mode_ == ScaleTR) ? (new_h / 2) : -(new_h / 2);

                Point2D r_n = Transform::rotate_point(l.flip_h ? -nfx : nfx, l.flip_v ? -nfy : nfy, 0, 0, l.rotation_angle);

                l.x = (w_fixed.x - r_n.x) - (new_w * 0.5) - (l.crop_w >= 0 ? l.crop_x * l.scale_x : 0);
                l.y = (w_fixed.y - r_n.y) - (new_h * 0.5) - (l.crop_h >= 0 ? l.crop_y * l.scale_y : 0);

                invalidate();
            }
            else if (drag_mode_ == Pan || Fl::event_state(FL_MIDDLE_MOUSE))
            {
                view_x_ -= dx / scale_;
                view_y_ -= dy / scale_;
                clamp_view();
                bg_dirty_ = true;
            }

            last_mouse_x_ = Fl::event_x();
            last_mouse_y_ = Fl::event_y();
            redraw();
            notify_view_changed();
            return 1;
        }
        case FL_RELEASE:
        {
            if (drag_mode_ == MoveLayer && selected_layer_index_ >= 0 && selected_layer_index_ < (int)document_->layer_count())
            {
                auto l = std::static_pointer_cast<ImageLayer>(document_->get_layer(selected_layer_index_));
                auto cmd = std::make_shared<CommandMove>(selected_layer_index_, orig_layer_x_, orig_layer_y_, l->x, l->y);
                undo_stack_.push_back(cmd);
                redo_stack_.clear();
            }
            else if (drag_mode_ == Crop && selected_layer_index_ >= 0 && selected_layer_index_ < (int)document_->layer_count())
            {
                auto &l = *std::static_pointer_cast<ImageLayer>(document_->get_layer(selected_layer_index_));
                if (l.image && l.image->handle())
                {
                    const int img_w = l.image->data_w();
                    const int img_h = l.image->data_h();

                    double min_lx = std::min(crop_start_x_, crop_end_x_);
                    double min_ly = std::min(crop_start_y_, crop_end_y_);
                    double max_lx = std::max(crop_start_x_, crop_end_x_);
                    double max_ly = std::max(crop_start_y_, crop_end_y_);

                    double cx_crop = (min_lx - l.x) / l.scale_x;
                    double cy_crop = (min_ly - l.y) / l.scale_y;
                    double cw_crop = (max_lx - min_lx) / l.scale_x;
                    double ch_crop = (max_ly - min_ly) / l.scale_y;

                    cx_crop = std::clamp(cx_crop, 0.0, (double)img_w);
                    cy_crop = std::clamp(cy_crop, 0.0, (double)img_h);
                    cw_crop = std::clamp(cw_crop, 0.0, (double)img_w - cx_crop);
                    ch_crop = std::clamp(ch_crop, 0.0, (double)img_h - cy_crop);

                    if (cw_crop > 1.0 && ch_crop > 1.0)
                    {
                        double current_lx = l.x;
                        double current_ly = l.y;
                        double current_lw = l.original_w * l.scale_x;
                        double current_lh = l.original_h * l.scale_y;
                        if (l.crop_w >= 0 && l.crop_h >= 0)
                        {
                            current_lx += l.crop_x * l.scale_x;
                            current_ly += l.crop_y * l.scale_y;
                            current_lw = l.crop_w * l.scale_x;
                            current_lh = l.crop_h * l.scale_y;
                        }
                        double C_x = current_lx + current_lw * 0.5;
                        double C_y = current_ly + current_lh * 0.5;

                        double new_lx = l.x + cx_crop * l.scale_x;
                        double new_ly = l.y + cy_crop * l.scale_y;
                        double new_lw = cw_crop * l.scale_x;
                        double new_lh = ch_crop * l.scale_y;

                        double C_new_unrot_x = new_lx + new_lw * 0.5;
                        double C_new_unrot_y = new_ly + new_lh * 0.5;

                        Point2D W = Transform::local_to_world(C_new_unrot_x, C_new_unrot_y, C_x, C_y, l.rotation_angle, l.flip_h, l.flip_v);

                        double final_l_x = l.x + (W.x - C_new_unrot_x);
                        double final_l_y = l.y + (W.y - C_new_unrot_y);

                        auto cmd = std::make_shared<CommandCrop>(selected_layer_index_, l.crop_x, l.crop_y, l.crop_w, l.crop_h, cx_crop, cy_crop, cw_crop, ch_crop, l.x, l.y, final_l_x, final_l_y);
                        push_command(cmd);
                    }
                }
                bg_dirty_ = true;
                redraw();
            }
            drag_mode_ = None;
            return 1;
        }
        case FL_MOUSEWHEEL:
        {
            double old_scale = scale_;
            double mouse_world_x = view_x_ + (Fl::event_x() - x()) / old_scale;
            double mouse_world_y = view_y_ + (Fl::event_y() - y()) / old_scale;

            scale_ *= (Fl::event_dy() > 0) ? 0.8 : 1.25;
            scale_ = std::clamp(scale_, 0.01, 100.0);

            view_x_ = mouse_world_x - (Fl::event_x() - x()) / scale_;
            view_y_ = mouse_world_y - (Fl::event_y() - y()) / scale_;

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
            notify_view_changed();
            return 1;
        }
        case FL_ENTER:
            notify_view_changed();
            return 1;
        case FL_LEAVE:
            if (hover_mode_ != None)
            {
                hover_mode_ = None;
                redraw();
            }
            notify_view_changed();
            return 1;
        }
        return Fl_Widget::handle(event);
    }
}