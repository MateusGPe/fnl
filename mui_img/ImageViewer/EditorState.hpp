#pragma once

#include "ImageDocument.hpp"
#include <unordered_set>
#include <memory>
#include <vector>
#include <algorithm>
#include <functional>

namespace mui
{
    // Observer pattern to safely decouple state mutations from the FLTK frontend logic
    class IEditorObserver
    {
    public:
        virtual ~IEditorObserver() = default;
        virtual void on_document_changed() = 0;
        virtual void on_selection_changed() = 0;
    };

    struct StateMemento
    {
        std::shared_ptr<ImageDocument> document;
        int selected_layer_id;
        std::unordered_set<int> selection_ids;
    };

    // The centralized, framework-agnostic store of current logic and app state
    class EditorState
    {
    private:
        std::shared_ptr<ImageDocument> document_;
        int selected_layer_id_ = -1;
        std::unordered_set<int> selection_ids_;
        std::vector<IEditorObserver *> observers_;

    public:
        EditorState() : document_(std::make_shared<ImageDocument>()) {}

        void add_observer(IEditorObserver *obs) { observers_.push_back(obs); }
        void remove_observer(IEditorObserver *obs)
        {
            observers_.erase(std::remove(observers_.begin(), observers_.end(), obs), observers_.end());
        }

        void notify_changed()
        {
            for (auto o : observers_)
                o->on_document_changed();
        }

        StateMemento create_memento() const
        {
            return {std::make_shared<ImageDocument>(*document_), selected_layer_id_, selection_ids_};
        }

        void restore_memento(const StateMemento &m)
        {
            document_ = std::make_shared<ImageDocument>(*m.document);
            selected_layer_id_ = m.selected_layer_id;
            selection_ids_ = m.selection_ids;
            notify_changed();
            for (auto o : observers_)
                o->on_selection_changed();
        }

        // --- Document ---
        std::shared_ptr<ImageDocument> document() const { return document_; }
        void set_document(std::shared_ptr<ImageDocument> doc)
        {
            document_ = doc;
            notify_changed();
        }

        // --- Selection ---
        int selected_layer_id() const { return selected_layer_id_; }
        const std::unordered_set<int> &selection_ids() const { return selection_ids_; }

        void clear_selection()
        {
            selected_layer_id_ = -1;
            selection_ids_.clear();
            for (auto o : observers_)
                o->on_selection_changed();
        }

        void set_primary_selection(int id)
        {
            selected_layer_id_ = id;
            if (id != -1)
                selection_ids_.insert(id);
            for (auto o : observers_)
                o->on_selection_changed();
        }

        void toggle_selection(int id)
        {
            if (selection_ids_.count(id))
            {
                selection_ids_.erase(id);
                if (selected_layer_id_ == id)
                {
                    selected_layer_id_ = selection_ids_.empty() ? -1 : *selection_ids_.begin();
                }
            }
            else
            {
                selection_ids_.insert(id);
                selected_layer_id_ = id;
            }
            for (auto o : observers_)
                o->on_selection_changed();
        }

        bool is_in_selection(int layer_id) const
        {
            return selection_ids_.count(layer_id) > 0;
        }
    };
}