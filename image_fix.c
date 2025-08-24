/*
 * Copyright (C) 2025 Raymond Marx
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby 
 * granted. No representations are made about the suitability of this software 
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 */
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <MagickWand/MagickWand.h>

#define MAX_PATH 1024

// Extensions supported
const char *extensions[] = {"jpg", "jpeg", "png", "gif", "tiff", "bmp", "webp", NULL};

// Check file extension
int 
has_supported_extension(const char *filename) 
{
    const char **ext = extensions;
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return 0;
    dot++; // skip dot
    for(; *ext; ext++) {
        if(strcasecmp(dot, *ext) == 0) return 1;
    }
    return 0;
}

// magick -define jpeg:ignore-warnings=true 107-0736_IMG.jpg -colorspace sRGB -quality 95 fixed.jpg
// TODO other types of files like png
int check_image(const char* filepath, const char* destination_dir) 
{ 
  MagickWand *wand = NewMagickWand();
  
  MagickSetOption(wand, "jpeg:ingore-warnings", "true");
  
  // Read image
  if (MagickReadImage(wand, filepath) == MagickFalse) {
    fprintf(stderr, "Failed to read image %s \n", filepath); 
    return -1;
  }

  if (MagickSetImageColorspace(wand, sRGBColorspace) == MagickFalse) {
    fprintf(stderr, "Failed set image colorspace %s \n", filepath);
    return -1;
  }

  MagickSetImageCompressionQuality(wand, 95);

  // Write output image
  char *fixed_file_path;
  fixed_file_path = malloc(strlen(destination_dir) + strlen(filepath) +1);
  strcpy(fixed_file_path, destination_dir);
  strcat(fixed_file_path, filepath);
  
  if (MagickWriteImage(wand, fixed_file_path ) == MagickFalse) {
    fprintf(stderr, "Failed to write image\n");
    return 1;
  }

  // Clean up
  if (wand) wand = DestroyMagickWand(wand);
}

// dir_path is reused in while loop 
void process_directory(const char* dir_path, const char* start_dir, const char* destination_dir) 
{
  DIR *dir = opendir(dir_path);
  if (!dir) return;

  struct dirent *entry;

  while ((entry = readdir(dir)) != NULL) {
    if( strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, ".." ) == 0) continue;
    
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

    // Rerun on foldes
    struct stat st;
    if (stat(path, &st) !=0) continue;

    if (S_ISDIR(st.st_mode)) {
      printf("Directory: %s \n", path);
      process_directory(path, start_dir, destination_dir);
    } else if(S_ISREG(st.st_mode)) {
        if (has_supported_extension( entry->d_name)) {
          check_image( path, destination_dir); 
        }
    }
  }
}


int main(int argc, char **argv)
{
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <source_folder> <destination_folder \n", argv[0]);
    return -1;
  }

  const char* start_dir = argv[1];
  const char* destination_dir = argv[2];

  MagickWandGenesis();

  printf("Starting scan from directory: %s \n", start_dir);

  process_directory(start_dir, start_dir, destination_dir);

  MagickWandTerminus();
  printf("Fixing done\n");
  return 0;
}

