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
        int idx;
        double ox, oy, nx, ny;
        CommandMove(int i, double ox, double oy, double nx, double ny) : idx(i), ox(ox), oy(oy), nx(nx), ny(ny) {}
        void execute(InternalImageViewer *v) override;
        void undo(InternalImageViewer *v) override;
    };

    struct CommandCrop : public ViewerCommand
    {
        int idx;
        double ox, oy, ow, oh, nx, ny, nw, nh;
        double olx, oly, nlx, nly;
        CommandCrop(int i, double ox, double oy, double ow, double oh, double nx, double ny, double nw, double nh, double olx, double oly, double nlx, double nly)
            : idx(i), ox(ox), oy(oy), ow(ow), oh(oh), nx(nx), ny(ny), nw(nw), nh(nh), olx(olx), oly(oly), nlx(nlx), nly(nly) {}
        void execute(InternalImageViewer *v) override;
        void undo(InternalImageViewer *v) override;
    };

    struct CommandFlip : public ViewerCommand
    {
        int idx;
        bool oh, ov, nh, nv;
        CommandFlip(int i, bool oh, bool ov, bool nh, bool nv)
            : idx(i), oh(oh), ov(ov), nh(nh), nv(nv) {}
        void execute(InternalImageViewer *v) override;
        void undo(InternalImageViewer *v) override;
    };

    struct CommandRotate : public ViewerCommand
    {
        int idx;
        double o_angle, n_angle;
        CommandRotate(int i, double oa, double na)
            : idx(i), o_angle(oa), n_angle(na) {}
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
}