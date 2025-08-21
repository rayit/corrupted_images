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

int has_supported_extensiona(const char* filename)
{

}

// magick -define jpeg:ignore-warnings=true 107-0736_IMG.jpg -colorspace sRGB -quality 95 fixed.jpg
int check_image(const char* filepath) 
{ 
  MagickWand *wand = NewMagickWand();
  
}

// dir_path is reused in while loop 
void process_directory(const char* dir_path, const char* start_dir) 
{
  DIR *dir = opendir(dir_path);
  if (!dir) return;

  struct dirent *entry;

  while ((entry = readdir(dir)) != NULL) {
    if( strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, ".." ) == 0) continue;
    
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);
    printf("Entry: %s \n ", path);

    // Rerun on foldes
    struct stat st;
    if (stat(path, &st) !=0) continue;
    printf("Filesize: %ld ,", st.st_size);

    if (S_ISDIR(st.st_mode)) {
      printf("Is directory: %s /n", path);
      process_directory(path, start_dir);
    } else if(S_ISREG(st.st_mode)) {
      printf("Is file: %s /n", path);
      // TODO do stuff
      // Check bad or not...
      // If bad try to fix?
      // Should I move to other folder?
    }
  }
}


int main(int argc, char **argv)
{
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <source_folder> \n", argv[0]);
    return -1;
  }

  const char* start_dir = argv[1];

  MagickWandGenesis();

  printf("Starting scan from directory: %s \n", start_dir);

  process_directory(start_dir, start_dir);

  MagickWandTerminus();
  printf("Fixind gone\n");
  return 0;
}

