#pragma once

#include "EditorState.hpp"
#include <memory>
#include <deque>
#include <vector>
#include <unordered_map>

namespace mui
{
    // Interface for an undoable action executed upon the EditorState
    class ICommand
    {
    public:
        virtual ~ICommand() = default;
        virtual void execute(EditorState &state) = 0;
        virtual void undo(EditorState &state) = 0;
    };

    // Dependency Injection target for the undo/redo subsystem
    class IUndoManager
    {
    public:
        virtual ~IUndoManager() = default;
        virtual void push(std::unique_ptr<ICommand> cmd) = 0;
        virtual void undo(EditorState &state) = 0;
        virtual void redo(EditorState &state) = 0;
        virtual void set_max_records(size_t max) = 0;
        virtual size_t max_records() const = 0;
        virtual void clear() = 0;
    };

    class DefaultUndoManager : public IUndoManager
    {
        std::deque<std::unique_ptr<ICommand>> undo_stack_;
        std::vector<std::unique_ptr<ICommand>> redo_stack_;
        size_t max_records_ = 100;

    public:
        void push(std::unique_ptr<ICommand> cmd) override
        {
            undo_stack_.push_back(std::move(cmd));
            while (undo_stack_.size() > max_records_)
                undo_stack_.pop_front();
            redo_stack_.clear();
        }
        void undo(EditorState &state) override
        {
            if (undo_stack_.empty())
                return;
            auto cmd = std::move(undo_stack_.back());
            undo_stack_.pop_back();
            cmd->undo(state);
            redo_stack_.push_back(std::move(cmd));
        }
        void redo(EditorState &state) override
        {
            if (redo_stack_.empty())
                return;
            auto cmd = std::move(redo_stack_.back());
            redo_stack_.pop_back();
            cmd->execute(state);
            undo_stack_.push_back(std::move(cmd));
        }
        void set_max_records(size_t max) override { max_records_ = max; }
        size_t max_records() const override { return max_records_; }
        void clear() override
        {
            undo_stack_.clear();
            redo_stack_.clear();
        }
    };

    // Generic command for a full document state swap
    class DocumentStateCommand : public ICommand
    {
        std::unique_ptr<StateMemento> before_;
        std::unique_ptr<StateMemento> after_;

    public:
        DocumentStateCommand(const EditorState &state) : before_(std::make_unique<StateMemento>(state.create_memento())) {}
        void capture_after(const EditorState &state) { after_ = std::make_unique<StateMemento>(state.create_memento()); }

        void execute(EditorState &state) override
        {
            if (after_)
                state.restore_memento(*after_);
        }
        void undo(EditorState &state) override
        {
            if (before_)
                state.restore_memento(*before_);
        }
    };

    // Generic lightweight command using property swapping for specified layers
    class LayerPropsCommand : public ICommand
    {
        std::unordered_map<int, std::shared_ptr<Layer>> before_props_;
        std::unordered_map<int, std::shared_ptr<Layer>> after_props_;

    public:
        LayerPropsCommand(const EditorState &state, const std::vector<int> &layer_indices)
        {
            for (int idx : layer_indices)
            {
                if (auto l = state.document()->get_layer(idx))
                {
                    before_props_[l->id] = l->clone();
                }
            }
        }
        void capture_after(const EditorState &state)
        {
            for (auto &pair : before_props_)
            {
                int id = pair.first;
                int idx = state.document()->get_layer_index(id);
                if (auto l = state.document()->get_layer(idx))
                {
                    after_props_[id] = l->clone();
                }
            }
        }
        void execute(EditorState &state) override { restore(state, after_props_); }
        void undo(EditorState &state) override { restore(state, before_props_); }

    private:
        void restore(EditorState &state, const std::unordered_map<int, std::shared_ptr<Layer>> &props)
        {
            for (const auto &pair : props)
            {
                int idx = state.document()->get_layer_index(pair.first);
                if (idx >= 0)
                {
                    if (auto target_layer = std::dynamic_pointer_cast<ImageLayer>(state.document()->get_layer(idx)))
                    {
                        if (auto source_layer = std::dynamic_pointer_cast<ImageLayer>(pair.second))
                        {
                            *target_layer = *source_layer;
                        }
                    }
                }
            }
            state.notify_changed();
        }
    };
}