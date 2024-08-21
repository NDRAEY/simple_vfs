#include "debugfs.h"

#include <stdio.h>
#include <stdlib.h>

bool debugfs_probe(size_t disk_nr, fs_object_t* fs) {
    (void)fs;

    if(disk_nr == 0)
        return true;

    return false;
}

direntry_t* debugfs_diropen(fs_object_t* fs, const char* path) {
    (void)path;
    (void)fs;
    direntry_t* direntry = calloc(1, sizeof(direntry_t));

    direntry->name = "Pokemon.txt";
    direntry->type = ENT_FILE;
    direntry->size = 0;
    
    direntry->created.year = 2019;
    direntry->created.month = 1;
    direntry->created.day = 1;

    direntry->modified.year = 2019;
    direntry->modified.month = 1;
    direntry->modified.day = 1;
    
    direntry_t* another = calloc(1, sizeof(direntry_t));

    another->name = "Zeraora and Lucario.jpg";
    another->type = ENT_FILE;
    another->size = 4967308;

    another->created.year = 2024;
    another->created.month = 6;  // June
    another->created.day = 21;

    another->modified.year = 2024;
    another->modified.month = 6;
    another->modified.day = 22;

    direntry->next = another;

    return direntry;
}

NFILE* debugfs_fileopen(fs_object_t* fs, const char* path) {
    // TODO: Implement it

    NFILE* f = calloc(1, sizeof(NFILE));

    f->path = path;
    f->size = 2;

    return f;
}

void debugfs_fileclose(fs_object_t* fs, NFILE* file) {
    // TODO: Implement it

    if(file->priv_data) {
        free(file->priv_data);
    }
}

size_t debugfs_fileread(fs_object_t* fs, void* data, size_t size, size_t count, NFILE* file) {
    (void)data;
    (void)size;
    (void)count;

    printf("[debugfs] Read requested: size: %zu; count: %zu; pos: %zu\n", size, count, file->position);

    snprintf(data, size * count, "You're trying to read: %s", file->path);

    return 0;
}

size_t debugfs_filewrite(fs_object_t* fs, const void* data, size_t size, size_t count, NFILE* file) {
    printf("[debugfs] Write requested: size: %zu; count: %zu; pos: %zu\n", size, count, file->position);

    printf("[you're wasted your data] Content: %s\n", (char*)data);
        
    return 0;
}