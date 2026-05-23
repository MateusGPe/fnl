# **AdvancedImageViewer Developer Manual**

This manual provides a guide for extending the AdvancedImageViewer class. Following a recent refactor, the architecture now strictly separates the UI logic from the application state, utilizing Dependency Injection (DI) and the Command/Memento patterns for an efficient and robust Undo/Redo system.

## **1. Architectural Overview**

To safely add features without breaking the app, it is crucial to understand the three main pillars of the viewer's architecture:

1. **EditorState (The Model):** A purely FLTK-agnostic class holding the ImageDocument and selection IDs. It is observable.  
2. **InternalImageViewer (The View/Controller):** An FLTK widget that observes EditorState. It acts as a bridge, forwarding UI events to state changes and reacting to state changes by scheduling redraws.  
3. **IUndoManager & ICommand (The History):** Injected into the viewer. Commands mutate the EditorState and save before/after snapshots (Mementos).

## **2. The Undo / Redo Subsystem**

The Undo/Redo system is designed around the **Command Pattern**. However, you rarely need to write your own Command classes. The base class (InternalImageViewer) provides two highly optimized wrapper methods that do the heavy lifting for you via closures (lambdas).

### **A. Lightweight Actions (perform_light_undoable_action)**

Use this when you are **modifying existing properties** of a specific layer or layers (e.g., opacity, rotation, scale, position).

* **How it works:** It uses LayerPropsCommand. It takes a snapshot of *only* the specific layers you are modifying before and after the action.  
* **Why use it:** Extremely memory efficient. It avoids copying the entire document layer list.

### **B. Heavy Actions (perform_heavy_undoable_action)**

Use this when you are **changing the structural state of the document**, such as adding, deleting, duplicating, or reordering layers.

* **How it works:** It uses DocumentStateCommand. It clones the entire ImageDocument layer list and the current selection state.  
* **Why use it:** Structural changes cannot be tracked by individual layer properties. (Note: Due to std::shared_ptr, cloning the document is still very fast because it only copies pointers, not the heavy pixel data).

## **3. Step-by-Step: Adding a New Feature**

Here are practical examples of how to safely extend AdvancedImageViewer.hpp.

### **Scenario 1: Adding a simple layer toggle or property change**

Suppose you want to add a feature to invert a layer's colors (assuming ImageLayer has a boolean invert_colors).

**Step 1:** Add your property to Types.hpp in the ImageLayer struct.

**Step 2:** Add your fluent API method in AdvancedImageViewer.hpp.

**Step 3:** Wrap your logic in perform_light_undoable_action.

```cpp
// Inside AdvancedImageViewer.hpp  
AdvancedImageViewer &toggle_layer_invert(int index)  
{  
    // 1. Get the layer securely  
    if (auto l = get_image_layer(index))  
    {  
        // 2. Wrap in a light undoable action, passing the indices to track  
        perform_light_undoable_action({index}, [l]() {  
            // 3. Mutate the property  
            l->invert_colors = !l->invert_colors;   
        });  
    }  
    return *this;  
}
```

*Note: The system will automatically capture the layer's state before and after your lambda runs, update the Undo Manager, and trigger invalidate() and notify_view_changed().*

### **Scenario 2: Adding a feature that affects multiple layers**

Suppose you want to align all selected layers to the left edge of the primary selection.

```cpp
AdvancedImageViewer &align_selection_left()  
{  
    if (state_->selection_ids().empty()) return *this;

    // 1. Collect indices of all selected layers  
    std::vector<int> indices;  
    for (int id : state_->selection_ids()) {  
        if (int idx = state_->document()->get_layer_index(id); idx >= 0)  
            indices.push_back(idx);  
    }

    // 2. Calculate your logic before mutating (find the left-most coordinate)  
    double target_x = get_image_layer(get_selected_layer_index())->x;

    // 3. Wrap in a light action, passing ALL affected indices  
    perform_light_undoable_action(indices, [this, target_x]() {  
        for (int id : state_->selection_ids()) {  
            if (auto l = get_image_layer(state_->document()->get_layer_index(id))) {  
                l->x = target_x; // Mutate  
            }  
        }  
    });

    return *this;  
}
```

### **Scenario 3: Adding a structural change (Heavy Action)**

Suppose you want to implement a "Duplicate Layer" feature. Because you are adding a new item to the document list, this is a structural change.

```cpp
AdvancedImageViewer &duplicate_layer(int index)  
{  
    if (index < 0 || index >= (int)state_->document()->layer_count()) return *this;

    // Wrap the structural change in a heavy action  
    perform_heavy_undoable_action([this, index]() {  
        // 1. Grab original layer  
        auto original = get_image_layer(index);  
          
        // 2. Clone it (which creates a new ImageLayer but shares the pixel data ptr)  
        auto duplicate = std::static_pointer_cast<ImageLayer>(original->clone());  
          
        // 3. Reset the ID so the document assigns a fresh one  
        duplicate->id = -1;   
        duplicate->name += " (Copy)";  
        duplicate->x += 20; // Offset visually  
        duplicate->y += 20;

        // 4. Mutate the document structure  
        state_->document()->insert_layer(index + 1, duplicate);  
          
        // 5. Update selection  
        state_->clear_selection();  
        state_->set_primary_selection(duplicate->id);  
    });

    return *this;  
}
```

## **4. Object Lifetime and Memory Management**

When adding features, keep these memory paradigms in mind to prevent leaks or massive RAM spikes:

1. **Shared Memory (std::shared_ptr):**  
   * Both the ImageDocument (the layer list) and the actual pixel buffers (Image objects) are wrapped in std::shared_ptr.  
   * When the undo system captures a "Memento" (snapshot), it creates a *new* ImageDocument vector, but the std::shared_ptr<Layer> inside points to clones of the layers.  
   * Crucially, ImageLayer::clone() performs a *shallow copy* of the std::shared_ptr<Image>. **Pixel data is NEVER duplicated in RAM during an undo step.**  
2. **Undo Caps (max_undo_records):**  
   * The DefaultUndoManager enforces a cap (default 100). If you perform 101 actions, the oldest command is dropped, and the std::shared_ptr references it holds are destroyed. If no other Memento holds those layers, they are safely freed from RAM.  
3. **UI Redraws:**  
   * Never call redraw() or invalidate() manually inside your feature logic if you are using the perform_* wrappers. The wrappers automatically trigger UI updates via the observer pattern (EditorState::notify_changed).