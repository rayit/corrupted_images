/*
 * Copyright (C) 2025 Raymond Marx
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby 
 * granted. No representations are made about the suitability of this software 
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 */
/* move_bad_images.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <MagickWand/MagickWand.h>
#include <errno.h>

#define MAX_PATH 1024

// Supported image extensions
const char *extensions[] = {"jpg", "jpeg", "png", "gif", "tiff", "bmp", "webp", NULL};

// Create directories recursively
void mkdir_p(const char *path) 
{
    char tmp[MAX_PATH];
    snprintf(tmp, sizeof(tmp), "%s", path);
    size_t len = strlen(tmp);
    if(tmp[len-1] == '/') tmp[len-1] = 0;
    for(char *p = tmp + 1; *p; p++) {
        if(*p == '/') {
            *p = 0;
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
}

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

// Check if an image is corrupt or partially unreadable
int 
is_corrupt(const char *filepath) 
{
    MagickWand *wand = NewMagickWand();
    int bad = 0;

    MagickBooleanType status = MagickReadImage(wand, filepath);
    if(status == MagickFalse) {
        bad = 1; // cannot read image
    } else {
        ExceptionType severity;
        char *description = MagickGetException(wand, &severity);
        if(severity != UndefinedException) {
            bad = 1;
            printf("Warning detected in file: %s\n  %s\n", filepath, description);
        }
        description = (char *)MagickRelinquishMemory(description);
    }

    DestroyMagickWand(wand);
    return bad;
}

// Move bad file to mirrored bad_folder
void 
move_bad_file(const char *filepath, const char *start_dir, const char *bad_root) 
{
    // Compute relative path
    const char *relpath = filepath;
    size_t start_len = strlen(start_dir);
    if(strncmp(filepath, start_dir, start_len) == 0)
        relpath += start_len;
    while(*relpath == '/' || *relpath == '\\') relpath++; // skip leading slash

    char dest[MAX_PATH];
    snprintf(dest, sizeof(dest), "%s/%s", bad_root, relpath);

    char dest_dir[MAX_PATH];
    strcpy(dest_dir, dest);
    char *last_slash = strrchr(dest_dir, '/');
    if(last_slash) {
        *last_slash = 0;
        mkdir_p(dest_dir);
    }

    printf("Moving bad file: %s -> %s\n", filepath, dest);
    if(rename(filepath, dest) != 0) {
        fprintf(stderr, "Error moving file %s: %s\n", filepath, strerror(errno));
    }
}

// Recursively process directories
void 
process_directory(const char *dirpath, const char *start_dir, const char *bad_root) 
{
    DIR *dir = opendir(dirpath);
    if(!dir) return;

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        if(strcmp(entry->d_name, bad_root) == 0) continue; // skip bad folder itself

        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s/%s", dirpath, entry->d_name);

        struct stat st;
        if(stat(path, &st) != 0) continue;

        if(S_ISDIR(st.st_mode)) {
            process_directory(path, start_dir, bad_root);
        } else if(S_ISREG(st.st_mode)) {
            if(has_supported_extension(entry->d_name)) {
                printf("Checking file: %s\n", path);
                if(is_corrupt(path)) {
                    move_bad_file(path, start_dir, bad_root);
                }
            }
        }
    }
    closedir(dir);
}

int 
main(int argc, char **argv) 
{
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <source_folder> <bad_folder>\n", argv[0]);
        return 1;
    }

    const char *start_dir = argv[1];
    const char *bad_root = argv[2];

    MagickWandGenesis();
    mkdir_p(bad_root);

    printf("Starting scan from directory: %s\n", start_dir);
    printf("Bad images will be moved to: %s\n", bad_root);

    process_directory(start_dir, start_dir, bad_root);

    MagickWandTerminus();
    printf("Scan complete.\n");
    return 0;
}

