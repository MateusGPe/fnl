#pragma once

#include "Internal.hpp"
#include <cmath>
#include "PixelBlender.hpp"
#include <algorithm>

namespace mui
{
    inline void InternalImageViewer::draw_background(int cx, int cy, int cw, int ch)
    {
        fl_push_clip(cx, cy, cw, ch);

        int bg_cx = cx, bg_cy = cy, bg_cw = cw, bg_ch = ch;

        if (state_->document()->mode() == DocumentMode::FixedCanvas)
        {
            fl_color(ThemeManager::get_palette().bg_main);
            fl_rectf(cx, cy, cw, ch);

            bg_cx = cx + static_cast<int>(std::round((0 - view_x_) * scale_));
            bg_cy = cy + static_cast<int>(std::round((0 - view_y_) * scale_));
            bg_cw = std::max(1, static_cast<int>(std::round(state_->document()->canvas_width() * scale_)));
            bg_ch = std::max(1, static_cast<int>(std::round(state_->document()->canvas_height() * scale_)));

            fl_push_clip(bg_cx, bg_cy, bg_cw, bg_ch);
        }

        if (use_solid_bg_)
        {
            fl_color(solid_bg_color_);
            fl_rectf(bg_cx, bg_cy, bg_cw, bg_ch);
        }
        else
        {
            const int checker_size = grid_size_;
            draw_checker(bg_cx, bg_cy, bg_cw, bg_ch, checker_size);
        }

        if (state_->document()->mode() == DocumentMode::FixedCanvas)
        {
            fl_pop_clip();
            fl_color(ThemeManager::get_palette().btn_frame.out_top);
            fl_rect(bg_cx, bg_cy, bg_cw, bg_ch);
        }

        fl_pop_clip();
    }

    inline void InternalImageViewer::draw_handle(int hx, int hy, bool is_hovered)
    {
        const int r = ThemeManager::get_palette().metrics.imageviewer_handle_size / 2;
        fl_color(is_hovered ? ThemeManager::get_palette().selection : ThemeManager::get_palette().bg_main);
        fl_rectf(hx - r, hy - r, r * 2, r * 2);
        fl_color(ThemeManager::get_palette().btn_frame.out_top);
        fl_rect(hx - r, hy - r, r * 2, r * 2);
    }

    inline void InternalImageViewer::draw_snap_lines()
    {
        if (snap_lines_.empty()) return;
        fl_color(ThemeManager::get_palette().focus_ring);
        fl_line_style(FL_SOLID, 1);
        for (const auto &line : snap_lines_)
        {
            int x1, y1, x2, y2;
            world_to_screen(line.p1.x, line.p1.y, x1, y1);
            world_to_screen(line.p2.x, line.p2.y, x2, y2);
            fl_line(x1, y1, x2, y2);
        }
        fl_line_style(0);
    }

    inline void InternalImageViewer::draw_selection_gizmo(const ImageLayer& l, bool is_primary)
    {
        // 1. Draw dashed crop full-boundary reference (if primary & cropping tool active)
        if (is_primary && active_tool_ == ViewerTool::Crop)
        {
            Point2D c = Transform::get_bounds_center(l.get_effective_bounds());
            Rect2D full_b = {l.x, l.y, l.original_w * l.scale_x, l.original_h * l.scale_y};
            auto full_corners = Transform::get_bounds_corners(full_b, c.x, c.y, l.rotation_angle, l.flip_h, l.flip_v);

            fl_color(ThemeManager::get_palette().btn_frame.out_top);
            fl_line_style(FL_DASH, 1);
            fl_begin_loop();
            for (const auto& pt : full_corners)
            {
                int sx, sy; world_to_screen(pt.x, pt.y, sx, sy);
                fl_vertex(sx, sy);
            }
            fl_end_loop();
            fl_line_style(0);
        }

        // Get unified gizmo math mapping 
        double rot_offset = (ThemeManager::get_palette().metrics.imageviewer_handle_size * 2) / scale_;
        auto gizmo = Transform::get_gizmo_handles(l, rot_offset);

        int scx[4], scy[4];
        for (int i = 0; i < 4; ++i) 
        {
            world_to_screen(gizmo.corners[i].x, gizmo.corners[i].y, scx[i], scy[i]);
        }

        // 2. Draw standard selection outline
        fl_color(is_primary ? ThemeManager::get_palette().selection : fl_color_average(ThemeManager::get_palette().selection, ThemeManager::get_palette().bg_main, 0.5f));
        fl_line_style(FL_DASH, ThemeManager::get_palette().metrics.imageviewer_selection_dash_width);
        fl_begin_loop();
        for (int i = 0; i < 4; ++i) fl_vertex(scx[i], scy[i]);
        fl_end_loop();

        // 3. Draw resize and rotation handles (if primary and unlocked)
        if (is_primary && !l.locked)
        {
            fl_line_style(FL_SOLID, 1);
            draw_handle(scx[0], scy[0], hover_mode_ == DragMode::ScaleTL);
            draw_handle(scx[1], scy[1], hover_mode_ == DragMode::ScaleTR);
            draw_handle(scx[2], scy[2], hover_mode_ == DragMode::ScaleBR);
            draw_handle(scx[3], scy[3], hover_mode_ == DragMode::ScaleBL);

            int r_sx, r_sy;
            world_to_screen(gizmo.rot_handle.x, gizmo.rot_handle.y, r_sx, r_sy);

            fl_color(ThemeManager::get_palette().btn_frame.out_top);
            fl_line((scx[0] + scx[1]) / 2, (scy[0] + scy[1]) / 2, r_sx, r_sy);

            const int r = ThemeManager::get_palette().metrics.imageviewer_handle_size / 2;
            fl_color(hover_mode_ == DragMode::Rotate ? ThemeManager::get_palette().selection : ThemeManager::get_palette().bg_main);
            fl_pie(r_sx - r, r_sy - r, r * 2, r * 2, 0, 360);
            fl_color(ThemeManager::get_palette().btn_frame.out_top);
            fl_arc(r_sx - r, r_sy - r, r * 2, r * 2, 0, 360);
        }
        fl_line_style(0);
    }

    inline void InternalImageViewer::draw_crop_gizmo()
    {
        int sel_idx = get_selected_layer_index();
        if (drag_mode_ != DragMode::Crop || sel_idx < 0) return;
        
        if (auto l_ptr = get_image_layer(sel_idx)) 
        {
            auto &l = *l_ptr;
            Point2D c = Transform::get_bounds_center(l.get_effective_bounds());
            CropDragBox box = get_crop_drag_box();
            Rect2D drag_b = {box.min_lx, box.min_ly, box.w(), box.h()};
            
            auto corners = Transform::get_bounds_corners(drag_b, c.x, c.y, l.rotation_angle, l.flip_h, l.flip_v);

            fl_color(ThemeManager::get_palette().fg_main);
            fl_line_style(FL_DASH, 2);
            fl_begin_loop();
            for (const auto& pt : corners) 
            {
                int sx, sy; world_to_screen(pt.x, pt.y, sx, sy);
                fl_vertex(sx, sy);
            }
            fl_end_loop();
            fl_line_style(0);
        }
    }

    inline void InternalImageViewer::draw_overlays()
    {
        draw_snap_lines();

        int primary_sel_idx = get_selected_layer_index();
        for (int layer_id : state_->selection_ids())
        {
            int sel_idx = state_->document()->get_layer_index(layer_id);
            if (auto l_ptr = get_image_layer(sel_idx))
            {
                if (is_layer_visible(sel_idx))
                    draw_selection_gizmo(*l_ptr, sel_idx == primary_sel_idx);
            }
        }
        
        draw_crop_gizmo();
    }

    inline void InternalImageViewer::draw_minimap()
    {
        if (!show_minimap_ || w() <= 0 || h() <= 0) return;

        MinimapInfo mi = get_minimap_info();
        fl_color(fl_color_average(ThemeManager::get_palette().bg_main, FL_BLACK, 0.7f));
        fl_rectf(mi.x, mi.y, mi.w, mi.h);
        fl_color(ThemeManager::get_palette().btn_frame.out_top);
        fl_rect(mi.x, mi.y, mi.w, mi.h);

        fl_push_clip(mi.x, mi.y, mi.w, mi.h);

        for (size_t i = 0; i < state_->document()->layer_count(); ++i)
        {
            if (!is_layer_visible(i)) continue;
            if (auto l_ptr = get_image_layer(i))
            {
                auto corners = Transform::get_layer_corners(*l_ptr);
                fl_color(ThemeManager::get_palette().inactive);
                fl_begin_polygon();
                for (int j = 0; j < 4; ++j)
                {
                    fl_vertex(mi.offset_x + (corners[j].x - mi.min_x) * mi.scale, 
                              mi.offset_y + (corners[j].y - mi.min_y) * mi.scale);
                }
                fl_end_polygon();
            }
        }

        int vx = static_cast<int>(mi.offset_x + (view_x_ - mi.min_x) * mi.scale);
        int vy = static_cast<int>(mi.offset_y + (view_y_ - mi.min_y) * mi.scale);
        int vw = std::max(1, static_cast<int>((w() / scale_) * mi.scale));
        int vh = std::max(1, static_cast<int>((h() / scale_) * mi.scale));

        fl_color(ThemeManager::get_palette().selection);
        fl_line_style(FL_SOLID, 2);
        fl_rect(vx, vy, vw, vh);
        fl_line_style(0);

        fl_pop_clip();
    }

    inline void InternalImageViewer::render_layer_to_buffer(const ImageLayer &layer, int layer_idx, int target_w, int target_h, double view_x, double view_y, double scale)
    {
        if (!is_layer_visible(layer_idx) || !layer.image || !layer.image->handle())
            return;

        const int screen_x = static_cast<int>(std::round((layer.x - view_x) * scale));
        const int screen_y = static_cast<int>(std::round((layer.y - view_y) * scale));
        const int screen_w = std::max(1, static_cast<int>(std::round(layer.original_w * layer.scale_x * scale)));
        const int screen_h = std::max(1, static_cast<int>(std::round(layer.original_h * layer.scale_y * scale)));

        int draw_sx = screen_x, draw_sy = screen_y, draw_sw = screen_w, draw_sh = screen_h;
        int crop_off_x = 0, crop_off_y = 0;

        if (layer.crop_w >= 0 && layer.crop_h >= 0)
        {
            crop_off_x = static_cast<int>(std::round(layer.crop_x * layer.scale_x * scale));
            crop_off_y = static_cast<int>(std::round(layer.crop_y * layer.scale_y * scale));
            draw_sw = std::max(1, static_cast<int>(std::round(layer.crop_w * layer.scale_x * scale)));
            draw_sh = std::max(1, static_cast<int>(std::round(layer.crop_h * layer.scale_y * scale)));
            draw_sx += crop_off_x;
            draw_sy += crop_off_y;
        }

        double dst_cx = draw_sx + draw_sw * 0.5;
        double dst_cy = draw_sy + draw_sh * 0.5;
        double hw = draw_sw * 0.5, hh = draw_sh * 0.5;

        Rect2D b = Transform::get_rotated_bounds(draw_sx, draw_sy, draw_sw, draw_sh, layer.rotation_angle);
        int bg_x = static_cast<int>(std::floor(b.x));
        int bg_y = static_cast<int>(std::floor(b.y));
        int bg_w = static_cast<int>(std::ceil(b.w));
        int bg_h = static_cast<int>(std::ceil(b.h));

        if (bg_x + bg_w < 0 || bg_x > target_w || bg_y + bg_h < 0 || bg_y > target_h)
            return;

        fl_push_clip(0, 0, target_w, target_h);

        bool use_software_blend = (layer.alpha < 1.0 || layer.blend_mode != BlendMode::Normal || layer.flip_h || layer.flip_v || layer.rotation_angle != 0.0);
        Fl_Image *base_img = layer.image->handle();

        if (use_software_blend && base_img->count() > 0 && base_img->d() >= 3)
        {
            const int img_w = layer.image->data_w();
            const int img_h = layer.image->data_h();

            if (img_w > 0 && img_h > 0)
            {
                const uchar *fg = (const uchar *)base_img->data()[0];
                int fg_d = base_img->d();

                int clip_x = std::max(0, bg_x);
                int clip_y = std::max(0, bg_y);
                int clip_r = std::min(target_w, bg_x + bg_w);
                int clip_b = std::min(target_h, bg_y + bg_h);
                int clip_w = clip_r - clip_x;
                int clip_h = clip_b - clip_y;

                if (clip_w > 0 && clip_h > 0)
                {
                    uchar *bg = fl_read_image(nullptr, clip_x, clip_y, clip_w, clip_h, 0);
                    std::unique_ptr<uchar[]> bg_guard(bg); 
                    if (bg && fg)
                    {
                        size_t buffer_size = static_cast<size_t>(clip_w) * static_cast<size_t>(clip_h) * 3;
                        std::unique_ptr<uchar[]> blended(new uchar[buffer_size]);
                        int fg_ld = base_img->ld() ? base_img->ld() : img_w * fg_d;
                        double actual_crop_x = (layer.crop_w >= 0) ? layer.crop_x : 0.0;
                        double actual_crop_y = (layer.crop_h >= 0) ? layer.crop_y : 0.0;
                        double actual_crop_w = (layer.crop_w >= 0) ? layer.crop_w : img_w;
                        double actual_crop_h = (layer.crop_h >= 0) ? layer.crop_h : img_h;

                        for (int yy = 0; yy < clip_h; ++yy)
                        {
                            for (int xx = 0; xx < clip_w; ++xx)
                            {
                                size_t bg_idx = (static_cast<size_t>(yy) * static_cast<size_t>(clip_w) + static_cast<size_t>(xx)) * 3;
                                uchar br = bg[bg_idx], bg_g = bg[bg_idx + 1], bb = bg[bg_idx + 2];
                                blended[bg_idx] = br;
                                blended[bg_idx + 1] = bg_g;
                                blended[bg_idx + 2] = bb;

                                double screen_px = clip_x + xx;
                                double screen_py = clip_y + yy;

                                Point2D loc = Transform::world_to_local(screen_px, screen_py, dst_cx, dst_cy, layer.rotation_angle, layer.flip_h, layer.flip_v);
                                double norm_x = (loc.x - dst_cx + hw) / draw_sw;
                                double norm_y = (loc.y - dst_cy + hh) / draw_sh;

                                if (norm_x >= 0.0 && norm_x < 1.0 && norm_y >= 0.0 && norm_y < 1.0)
                                {
                                    double src_px = actual_crop_x + norm_x * actual_crop_w;
                                    double src_py = actual_crop_y + norm_y * actual_crop_h;
                                    float fr, fg_g_val, fb, fa;

                                    if (bilinear_filtering_)
                                        PixelBlender::bilerp(fg, src_px, src_py, img_w, img_h, fg_ld, fg_d, fr, fg_g_val, fb, fa);
                                    else
                                    {
                                        int src_x_int = std::clamp(static_cast<int>(std::round(src_px)), 0, img_w - 1);
                                        int src_y_int = std::clamp(static_cast<int>(std::round(src_py)), 0, img_h - 1);

                                        const int fg_idx = src_y_int * fg_ld + src_x_int * fg_d;
                                        fr = fg[fg_idx];
                                        fg_g_val = fg[fg_idx + 1];
                                        fb = fg[fg_idx + 2];
                                        fa = (fg_d == 4) ? fg[fg_idx + 3] : 255;
                                    }

                                    const float alpha = (fa / 255.0f) * layer.alpha;
                                    if (alpha > 0.0f)
                                        PixelBlender::blend_pixels(layer.blend_mode, fr, fg_g_val, fb, br, bg_g, bb, alpha, &blended[bg_idx]);
                                }
                            }
                        }
                        fl_draw_image(blended.get(), clip_x, clip_y, clip_w, clip_h, 3);
                    }
                }
            }
        }
        else
        {
            layer.image->scale(screen_w, screen_h, false, true);

            bool apply_matrix = layer.rotation_angle != 0.0 || layer.flip_h || layer.flip_v;
            if (apply_matrix)
            {
                fl_push_matrix();
                fl_translate(dst_cx, dst_cy);
                if (layer.rotation_angle != 0.0)
                    fl_rotate(layer.rotation_angle);
                if (layer.flip_h || layer.flip_v)
                    fl_scale(layer.flip_h ? -1.0 : 1.0, layer.flip_v ? -1.0 : 1.0);
                fl_translate(-dst_cx, -dst_cy);
            }

            layer.image->handle()->draw(draw_sx, draw_sy, draw_sw, draw_sh, crop_off_x, crop_off_y);

            if (apply_matrix) fl_pop_matrix();
        }

        fl_pop_clip();
    }

    inline void InternalImageViewer::draw()
    {
        if (w() <= 0 || h() <= 0) return;

        if (!use_solid_bg_)
        {
            Fl_Color current_checker_color = Fl::get_color(ThemeManager::get_palette().bg_main);
            if (last_checker_color1_ != current_checker_color)
            {
                bg_dirty_ = true;
                last_checker_color1_ = current_checker_color;
            }
        }

        fl_push_clip(x(), y(), w(), h());
        if (bg_dirty_ || !bg_buffer_ || off_w_ != w() || off_h_ != h())
        {
            if (bg_buffer_) fl_delete_offscreen(bg_buffer_);
            bg_buffer_ = fl_create_offscreen(w(), h());
            off_w_ = w();
            off_h_ = h();
            bg_dirty_ = false;

            fl_begin_offscreen(bg_buffer_);
            draw_background(0, 0, w(), h());

            if (state_->document()->mode() == DocumentMode::FixedCanvas)
            {
                int bg_cx = static_cast<int>(std::round((0 - view_x_) * scale_));
                int bg_cy = static_cast<int>(std::round((0 - view_y_) * scale_));
                int bg_cw = std::max(1, static_cast<int>(std::round(state_->document()->canvas_width() * scale_)));
                int bg_ch = std::max(1, static_cast<int>(std::round(state_->document()->canvas_height() * scale_)));
                fl_push_clip(bg_cx, bg_cy, bg_cw, bg_ch);
            }

            for (size_t i = 0; i < state_->document()->layer_count(); ++i)
            {
                if (auto l = get_image_layer(i))
                    render_layer_to_buffer(*l, i, w(), h(), view_x_, view_y_, scale_);
            }

            if (state_->document()->mode() == DocumentMode::FixedCanvas) fl_pop_clip();
            fl_end_offscreen();
        }

        fl_copy_offscreen(x(), y(), w(), h(), bg_buffer_, 0, 0);
        draw_overlays();
        draw_minimap();
        fl_pop_clip();
    }

    inline void InternalImageViewer::export_image(const char *filepath)
    {
        double min_x, min_y, max_x, max_y;
        get_world_bounds(min_x, min_y, max_x, max_y);
        int world_w = std::max(1, static_cast<int>(std::round(max_x - min_x)));
        int world_h = std::max(1, static_cast<int>(std::round(max_y - min_y)));

        if (world_w <= 0 || world_h <= 0 || world_w > 16384 || world_h > 16384)
            return;

        Fl_Offscreen off = fl_create_offscreen(world_w, world_h);
        if (!off) return;

        fl_begin_offscreen(off);
        if (use_solid_bg_)
        {
            fl_color(solid_bg_color_);
            fl_rectf(0, 0, world_w, world_h);
        }
        else
        {
            const int checker_size = grid_size_;
            for (int j = 0; j < world_h; j += checker_size)
                for (int i = 0; i < world_w; i += checker_size)
                {
                    fl_color((((i / checker_size) + (j / checker_size)) % 2 == 0) ? ThemeManager::get_palette().bg_main : ThemeManager::get_palette().bg_sec);
                    fl_rectf(i, j, checker_size, checker_size);
                }
        }

        for (size_t i = 0; i < state_->document()->layer_count(); ++i)
        {
            if (auto l = get_image_layer(i))
                render_layer_to_buffer(*l, i, world_w, world_h, min_x, min_y, 1.0);
        }

        uchar *pixels_raw = fl_read_image(nullptr, 0, 0, world_w, world_h, 0);
        fl_end_offscreen();
        fl_delete_offscreen(off);

        if (pixels_raw)
        {
            std::unique_ptr<uchar[]> pixels(pixels_raw);
            FILE *fp = fopen(filepath, "wb");
            if (fp)
            {
                fprintf(fp, "P6\n%d %d\n255\n", world_w, world_h);
                fwrite(pixels.get(), 1, world_w * world_h * 3, fp);
                fclose(fp);
            }
        }
    }
}