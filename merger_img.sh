#!/bin/bash

# --- CONFIGURATION ---
OUTPUT="merged_code.md"
SOURCE_DIR="./mui_img/"

# --- FILTER SETTINGS ---
# 1. Allowed file extensions (without the dot). Leave empty () to include all files.
ALLOWED_EXTENSIONS=("cpp" "h" "hpp")

# 2. Directories or files to completely ignore (e.g., build folders, git)
IGNORE_DIRS=(".git" "build" "node_modules" "todo" "ImageViewer")

# 3. Specific files to ignore (exact names)
IGNORE_FILES=("stb_image.h" "IconsFontAwesome6.h" "fa_solid_900.hpp")


# Clear the output file if it already exists (start fresh)
> "$OUTPUT"

# --- BUILD THE 'FIND' COMMAND DYNAMICALLY ---
FIND_CMD=(find "$SOURCE_DIR")

# 1. Add directory exclusions
if [ ${#IGNORE_DIRS[@]} -gt 0 ]; then
    FIND_CMD+=("(")
    for i in "${!IGNORE_DIRS[@]}"; do
        FIND_CMD+=("-name" "${IGNORE_DIRS[$i]}")
        [ $i -lt $((${#IGNORE_DIRS[@]} - 1)) ] && FIND_CMD+=("-o")
    done
    FIND_CMD+=(")" "-prune" "-o")
fi

# 2. Only look for files
FIND_CMD+=("-type" "f")

# 3. Add specific file exclusions
if [ ${#IGNORE_FILES[@]} -gt 0 ]; then
    for file in "${IGNORE_FILES[@]}"; do
        FIND_CMD+=("!" "-name" "$file")
    done
fi

# 4. Add extension inclusions
if [ ${#ALLOWED_EXTENSIONS[@]} -gt 0 ]; then
    FIND_CMD+=("(")
    for i in "${!ALLOWED_EXTENSIONS[@]}"; do
        FIND_CMD+=("-name" "*.${ALLOWED_EXTENSIONS[$i]}")
        [ $i -lt $((${#ALLOWED_EXTENSIONS[@]} - 1)) ] && FIND_CMD+=("-o")
    done
    FIND_CMD+=(")")
fi

FIND_CMD+=("-print")

# --- EXECUTE AND PROCESS ---
# Execute the find command and loop through results safely
"${FIND_CMD[@]}" | while IFS= read -r file; do
    echo "Processing $file..."

    # 1. Write the filename aImageViewers a Markdown Header (Level 2)
    relpath="${file#$SOURCE_DIR}"
    echo "## $relpath" >> "$OUTPUT"
    echo "" >> "$OUTPUT"

    # 2. Determine language for markdown code block
    extension="${file##*.}"
    lang="text"
    case "$extension" in
        cpp|c|h|hpp) lang="cpp" ;;
        py)          lang="python" ;;
        js|jsx)      lang="javascript" ;;
        ts|tsx)      lang="typescript" ;;
        sh)          lang="bash" ;;
        json)        lang="json" ;;
    esac

    # 3. Open the code block
    echo "\`\`\`$lang" >> "$OUTPUT"
    # 4. Append the content of the file
    echo "// $relpath" >> "$OUTPUT"
    cat "$file" >> "$OUTPUT"

    # 5. Close the code block (ensure a newline before the closing ticks)
    echo -e "\n\`\`\`" >> "$OUTPUT"

    # 6. Add some spacing between files
    echo -e "\n---\n" >> "$OUTPUT"

done

echo "Done! All files merged into $OUTPUT"
