#pragma once

#include "Internal.hpp"
#include "Commands.hpp"

namespace mui
{
    inline void CommandMove::execute(InternalImageViewer *v)
    {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx))
        {
            l->x = nx;
            l->y = ny;
            v->invalidate();
        }
    }

    inline void CommandMove::undo(InternalImageViewer *v)
    {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx))
        {
            l->x = ox;
            l->y = oy;
            v->invalidate();
        }
    }

    inline void CommandCrop::execute(InternalImageViewer *v)
    {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx))
        {
            l->crop_x = nx;
            l->crop_y = ny;
            l->crop_w = nw;
            l->crop_h = nh;
            l->x = nlx;
            l->y = nly;
            v->invalidate();
        }
    }

    inline void CommandCrop::undo(InternalImageViewer *v)
    {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx))
        {
            l->crop_x = ox;
            l->crop_y = oy;
            l->crop_w = ow;
            l->crop_h = oh;
            l->x = olx;
            l->y = oly;
            v->invalidate();
        }
    }

    inline void CommandFlip::execute(InternalImageViewer *v)
    {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx))
        {
            l->flip_h = nh;
            l->flip_v = nv;
            v->invalidate();
        }
    }

    inline void CommandFlip::undo(InternalImageViewer *v)
    {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx))
        {
            l->flip_h = oh;
            l->flip_v = ov;
            v->invalidate();
        }
    }

    inline void CommandRotate::execute(InternalImageViewer *v)
    {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx))
        {
            l->rotation_angle = n_angle;
            v->invalidate();
        }
    }

    inline void CommandRotate::undo(InternalImageViewer *v)
    {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx))
        {
            l->rotation_angle = o_angle;
            v->invalidate();
        }
    }

    inline void CommandScale::execute(InternalImageViewer *v)
    {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx))
        {
            l->x = nx;
            l->y = ny;
            l->scale_x = nsx;
            l->scale_y = nsy;
            v->invalidate();
        }
    }

    inline void CommandScale::undo(InternalImageViewer *v)
    {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx))
        {
            l->x = ox;
            l->y = oy;
            l->scale_x = osx;
            l->scale_y = osy;
            v->invalidate();
        }
    }

    inline void CommandDelete::execute(InternalImageViewer *v)
    {
        v->document_->remove_layer(idx);
        v->selected_layer_id_ = -1;
        v->invalidate();
    }

    inline void CommandDelete::undo(InternalImageViewer *v)
    {
        if (idx <= (int)v->document_->layer_count())
        {
            v->document_->insert_layer(idx, layer);
            v->selected_layer_id_ = layer->id;
        }
        else
        {
            v->document_->add_layer(layer);
            v->selected_layer_id_ = layer->id;
        }
        v->invalidate();
    }

    inline void CommandOpacity::execute(InternalImageViewer *v)
    {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx))
        {
            l->alpha = new_val;
            v->invalidate();
        }
    }
    inline void CommandOpacity::undo(InternalImageViewer *v)
    {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx))
        {
            l->alpha = old_val;
            v->invalidate();
        }
    }

    inline void CommandBlendMode::execute(InternalImageViewer *v)
    {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx))
        {
            l->blend_mode = new_val;
            v->invalidate();
        }
    }
    inline void CommandBlendMode::undo(InternalImageViewer *v)
    {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx))
        {
            l->blend_mode = old_val;
            v->invalidate();
        }
    }

    inline void CommandVisibility::execute(InternalImageViewer *v)
    {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx))
        {
            l->visible = new_val;
            v->invalidate();
        }
    }
    inline void CommandVisibility::undo(InternalImageViewer *v)
    {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx))
        {
            l->visible = old_val;
            v->invalidate();
        }
    }

    inline void CommandLock::execute(InternalImageViewer *v) {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx)) { l->locked = new_val; v->invalidate(); }
    }
    inline void CommandLock::undo(InternalImageViewer *v) {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx)) { l->locked = old_val; v->invalidate(); }
    }

    inline void CommandParent::execute(InternalImageViewer *v) {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx)) { l->parent_id = new_val; v->invalidate(); }
    }
    inline void CommandParent::undo(InternalImageViewer *v) {
        int idx = v->document_->get_layer_index(layer_id);
        if (idx == -1) return;
        if (auto l = v->get_image_layer(idx)) { l->parent_id = old_val; v->invalidate(); }
    }

    inline void CommandMoveLayer::execute(InternalImageViewer *v)
    {
        if (from_idx >= 0 && from_idx < (int)v->document_->layer_count() &&
            to_idx >= 0 && to_idx < (int)v->document_->layer_count())
        {
            v->document_->swap_layers(from_idx, to_idx);
            v->invalidate();
        }
    }

    inline void CommandMoveLayer::undo(InternalImageViewer *v)
    {
        // Swapping is its own inverse.
        execute(v);
    }
}