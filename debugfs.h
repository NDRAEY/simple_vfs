#pragma once

#include <stdbool.h>

#include "vfs.h"

bool debugfs_probe(size_t disk_nr, fs_object_t* fs);
direntry_t* debugfs_diropen(fs_object_t* fs, const char* path);
NFILE* debugfs_fileopen(fs_object_t* fs, const char* path);
void debugfs_fileclose(fs_object_t* fs, NFILE* file);
size_t debugfs_fileread(fs_object_t* fs, void* data, size_t size, size_t count, NFILE* file);
size_t debugfs_filewrite(fs_object_t* fs, const void* data, size_t size, size_t count, NFILE* file);
