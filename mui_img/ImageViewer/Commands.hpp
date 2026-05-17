#pragma once

namespace mui
{
    class Layer;
    class InternalImageViewer;

    struct ViewerCommand
    {
        virtual ~ViewerCommand() = default;
        virtual void execute(InternalImageViewer *v) = 0;
        virtual void undo(InternalImageViewer *v) = 0;
    };

    struct CommandMove : public ViewerCommand
    {
        int layer_id;
        double ox, oy, nx, ny;
        CommandMove(int id, double ox, double oy, double nx, double ny) : layer_id(id), ox(ox), oy(oy), nx(nx), ny(ny) {}
        void execute(InternalImageViewer *v) override;
        void undo(InternalImageViewer *v) override;
    };

    struct CommandCrop : public ViewerCommand
    {
        int layer_id;
        double ox, oy, ow, oh, nx, ny, nw, nh;
        double olx, oly, nlx, nly;
        CommandCrop(int id, double ox, double oy, double ow, double oh, double nx, double ny, double nw, double nh, double olx, double oly, double nlx, double nly)
            : layer_id(id), ox(ox), oy(oy), ow(ow), oh(oh), nx(nx), ny(ny), nw(nw), nh(nh), olx(olx), oly(oly), nlx(nlx), nly(nly) {}
        void execute(InternalImageViewer *v) override;
        void undo(InternalImageViewer *v) override;
    };

    struct CommandFlip : public ViewerCommand
    {
        int layer_id;
        bool oh, ov, nh, nv;
        CommandFlip(int id, bool oh, bool ov, bool nh, bool nv)
            : layer_id(id), oh(oh), ov(ov), nh(nh), nv(nv) {}
        void execute(InternalImageViewer *v) override;
        void undo(InternalImageViewer *v) override;
    };

    struct CommandRotate : public ViewerCommand
    {
        int layer_id;
        double o_angle, n_angle;
        CommandRotate(int id, double oa, double na)
            : layer_id(id), o_angle(oa), n_angle(na) {}
        void execute(InternalImageViewer *v) override;
        void undo(InternalImageViewer *v) override;
    };

    struct CommandScale : public ViewerCommand
    {
        int layer_id;
        double ox, oy, osx, osy;
        double nx, ny, nsx, nsy;
        CommandScale(int id, double ox, double oy, double osx, double osy, double nx, double ny, double nsx, double nsy)
            : layer_id(id), ox(ox), oy(oy), osx(osx), osy(osy), nx(nx), ny(ny), nsx(nsx), nsy(nsy) {}
        void execute(InternalImageViewer *v) override;
        void undo(InternalImageViewer *v) override;
    };

    struct CommandDelete : public ViewerCommand
    {
        int idx;
        std::shared_ptr<Layer> layer;
        CommandDelete(int i, std::shared_ptr<Layer> l) : idx(i), layer(l) {}
        void execute(InternalImageViewer *v) override;
        void undo(InternalImageViewer *v) override;
    };

    struct CommandOpacity : public ViewerCommand
    {
        int layer_id;
        double old_val, new_val;
        CommandOpacity(int id, double o, double n) : layer_id(id), old_val(o), new_val(n) {}
        void execute(InternalImageViewer *v) override;
        void undo(InternalImageViewer *v) override;
    };

    struct CommandBlendMode : public ViewerCommand
    {
        int layer_id;
        BlendMode old_val, new_val;
        CommandBlendMode(int id, BlendMode o, BlendMode n) : layer_id(id), old_val(o), new_val(n) {}
        void execute(InternalImageViewer *v) override;
        void undo(InternalImageViewer *v) override;
    };

    struct CommandVisibility : public ViewerCommand
    {
        int layer_id;
        bool old_val, new_val;
        CommandVisibility(int id, bool o, bool n) : layer_id(id), old_val(o), new_val(n) {}
        void execute(InternalImageViewer *v) override;
        void undo(InternalImageViewer *v) override;
    };

    struct CommandLock : public ViewerCommand
    {
        int layer_id;
        bool old_val, new_val;
        CommandLock(int id, bool o, bool n) : layer_id(id), old_val(o), new_val(n) {}
        void execute(InternalImageViewer *v) override;
        void undo(InternalImageViewer *v) override;
    };

    struct CommandParent : public ViewerCommand
    {
        int layer_id;
        int old_val, new_val;
        CommandParent(int id, int o, int n) : layer_id(id), old_val(o), new_val(n) {}
        void execute(InternalImageViewer *v) override;
        void undo(InternalImageViewer *v) override;
    };

    struct CommandMoveLayer : public ViewerCommand
    {
        int from_idx, to_idx;
        CommandMoveLayer(int from, int to) : from_idx(from), to_idx(to) {}
        void execute(InternalImageViewer *v) override;
        void undo(InternalImageViewer *v) override;
    };

}