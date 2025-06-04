import os
from PIL import Image

songs_folder = "songs/"
illegal_chars = ["–", "•", "ø", "$"]
replace_chars = ["-", "-", "o", "S"]
valid_extensions = [".mp3", ".raw"]

def clean_name(name):
    for i, char in enumerate(illegal_chars):
        name = name.replace(char, replace_chars[i])
    return name

for foldername, subfolders, filenames in os.walk(songs_folder, topdown=False):
    base_folder_name = os.path.basename(foldername)
    parent_path = os.path.dirname(foldername)

    # Clean folder name
    cleaned_folder_name = clean_name(base_folder_name)

    # If the folder name needs to be changed
    if cleaned_folder_name != base_folder_name:
        new_folder_path = os.path.join(parent_path, cleaned_folder_name)
        try:
            os.rename(foldername, new_folder_path)
            print(f"Renamed folder: {foldername} -> {new_folder_path}")
            foldername = new_folder_path  # update path to renamed folder
        except Exception as e:
            print(f"Error renaming folder {foldername}: {e}")

    # Rename files in the folder
    for filename in os.listdir(foldername):
        file_path = os.path.join(foldername, filename)
        if os.path.isfile(file_path):
            name, ext = os.path.splitext(filename)
            ext_lower = ext.lower()

            if ext_lower in valid_extensions:
                new_name = cleaned_folder_name
            else:
                new_name = clean_name(name)

            new_filename = new_name + ext_lower
            new_file_path = os.path.join(foldername, new_filename)

            if new_filename != filename:
                try:
                    os.rename(file_path, new_file_path)
                    print(f"Renamed file: {file_path} -> {new_file_path}")
                except Exception as e:
                    print(f"Error renaming file {file_path}: {e}")

# Loop through all folders and files
for foldername, subfolders, filenames in os.walk(songs_folder):
    for filename in filenames:
        if filename.lower().endswith(".jpg"):
            filepath = os.path.join(foldername, filename)
            try:
                with Image.open(filepath) as img:
                    width, height = img.size

                    # Center crop to 360x360
                    left = (width - 360) // 2
                    top = (height - 360) // 2
                    right = left + 360
                    bottom = top + 360
                    img_cropped = img.crop((left, top, right, bottom))

                    # Resize to 180x180
                    img_resized = img_cropped.resize((180, 180), Image.LANCZOS)

                    # Save the processed image (overwrite)
                    img_resized.save(filepath)
                    print(f"Processed: {filepath}")

            except Exception as e:
                print(f"Failed to process {filepath}: {e}")


def convert_jpg_to_rgb565(image_path, output_path):
    with Image.open(image_path) as img:
        img = img.convert("RGB")
        width, height = img.size
        with open(output_path, "wb") as f:
            for y in range(height):
                for x in range(width):
                    r, g, b = img.getpixel((x, y))
                    rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
                    f.write(rgb565.to_bytes(2, "big"))
    print(f"Converted JPG to RGB565: {output_path}")

for foldername, subfolders, filenames in os.walk(songs_folder):
    for filename in filenames:
        filepath = os.path.join(foldername, filename)
        name, ext = os.path.splitext(filename)
        ext = ext.lower()

        try:
            if ext == ".jpg":
                output_path = os.path.join(foldername, f"{name}.raw")
                convert_jpg_to_rgb565(filepath, output_path)
                os.remove(filepath)

        except Exception as e:
            print(f"Failed to convert {filepath}: {e}")

os.makedirs("song_cache")
file = open('song_cache/cache.json', 'w')
file.close()