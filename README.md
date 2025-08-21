# Corrupted images

*move_bad_images* moves detected corrupted images to a destination folder. 
*image_fix* will be a try to fix these corrupted images.

Image_fix is not working yet!! 

# Usage

```bash
move_bad_images <source_folder> <destination_folder>
```

```bash
image_fix <source_folder>
```

# Dependencies

virtual/imagemagick-tools 
media-gfx/imagemagick
media-libs/exiftool        ????

# Build (on Linux)
gcc -o move_bad_images move_bad_images.c `pkg-config --cflags --libs MagickWand`


# Author

Raymond Marx
