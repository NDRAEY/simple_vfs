#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct datetime {
    // Date
    uint8_t day : 5;
    uint8_t month : 4;
    uint16_t year;

    // Time
    uint8_t hour : 5;
    uint8_t minute : 6;
    uint8_t second : 6;
    uint32_t millis;
} datetime_t;

typedef enum direntry_type {
    ENT_UNKNOWN = 0,
    ENT_DIRECTORY,
    ENT_FILE
} direntry_type_t;

typedef struct direntry {
    char* name;
    direntry_type_t type;

    size_t size;
    datetime_t created;
    datetime_t modified;

    struct direntry* next;
} direntry_t;

struct fs_object;

typedef struct file {
    char* name;
    size_t position;
    size_t size;

    struct fs_object* _obj;

    void* priv_data;  // FS-specific data can be stored here.
} NFILE;

struct filesystem;

typedef struct fs_object {
    bool valid;
    size_t disk_nr;  // Disk number (used by fs to read from that disk)
    struct filesystem* filesystem;

    void* priv_data;  // FS-specific data can be stored here.
} fs_object_t;

typedef bool (*probe_fn_t)(size_t, fs_object_t*);
typedef direntry_t* (*diropen_fn_t)(fs_object_t* fs, const char* path);
typedef NFILE* (*fileopen_fn_t)(fs_object_t* fs, const char* path);
typedef size_t (*fileread_fn_t)(fs_object_t* fs, void* data, size_t size, size_t count, NFILE* fp);
typedef size_t (*filewrite_fn_t)(fs_object_t* fs, const void* data, size_t size, size_t count, NFILE* fp);
typedef void (*fileclose_fn_t)(fs_object_t* fs, NFILE* file);
//typedef void (*dirclose_fn_t)(fs_object_t* fs, direntry_t* entry);

typedef struct filesystem {
    bool valid;
    const char* name;
    probe_fn_t probe;
    diropen_fn_t diropen;
    fileopen_fn_t fileopen;
    fileread_fn_t fileread;
    filewrite_fn_t filewrite;
    fileclose_fn_t fileclose;
} filesystem_t;

#define FILESYSTEM_MAX_COUNT 32
#define MOUNTPOINTS_MAX_COUNT 32

static filesystem_t registered_filesystems[FILESYSTEM_MAX_COUNT] = {0};
static fs_object_t registered_mountpoints[MOUNTPOINTS_MAX_COUNT] = {0};

int find_free_fs_nr() {
    for (int i = 0; i < 32; i++) {
        if (!registered_filesystems[i].valid) {
            return i;
        }
    }

    return -1;
}

int register_filesystem(const char* name, probe_fn_t probe, diropen_fn_t diropen, fileopen_fn_t fileopen,
                fileread_fn_t fileread, filewrite_fn_t filewrite, fileclose_fn_t fileclose) {
    int fs_nr = find_free_fs_nr();

    if (fs_nr == -1) {
        return -1;
    }

    registered_filesystems[fs_nr].valid = true;
    registered_filesystems[fs_nr].name = name;
    registered_filesystems[fs_nr].probe = probe;
    registered_filesystems[fs_nr].diropen = diropen;
    registered_filesystems[fs_nr].fileopen = fileopen;
    registered_filesystems[fs_nr].fileread = fileread;
    registered_filesystems[fs_nr].filewrite = filewrite;
    registered_filesystems[fs_nr].fileclose = fileclose;

    return fs_nr;
}

int find_free_mountpoint_nr() {
    for (int i = 0; i < 32; i++) {
        if (!registered_mountpoints[i].valid) {
            return i;
        }
    }

    return -1;
}

int register_mountpoint(size_t disk_nr, filesystem_t* fs, void* priv_data) {
    int mp_nr = find_free_mountpoint_nr();

    if (mp_nr == -1) {
        return -1;
    }

    registered_mountpoints[mp_nr].valid = true;
    registered_mountpoints[mp_nr].disk_nr = disk_nr;
    registered_mountpoints[mp_nr].filesystem = fs;
    registered_mountpoints[mp_nr].priv_data = priv_data;

    return mp_nr;
}

void vfs_scan() {
    // TODO: Really scan disks on filesystems, but it's a simple test, so we just add a mountpoint here
    
    for(int disk = 0; disk < 32; disk++) {
        // Probe every filesystem
        for(int fsn = 0; fsn < 32; fsn++) {
            if(!registered_filesystems[fsn].valid) {
                continue;
            }

            bool result = registered_filesystems[fsn].probe(disk, registered_mountpoints + disk);

            if(result) {
                register_mountpoint(disk, registered_filesystems + fsn, NULL);
                printf("Filesystem %s registered on disk %d!\n", registered_filesystems[fsn].name, disk);
            }
        }
    }
}

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

    f->name = "Unknown.txt";
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

    return 0;
}

size_t debugfs_filewrite(fs_object_t* fs, const void* data, size_t size, size_t count, NFILE* file) {
    printf("[debugfs] Write requested: size: %zu; count: %zu; pos: %zu\n", size, count, file->position);
    return 0;
}

void vfs_parse_path(const char* full_path, size_t* out_disk_nr, char** out_path) {
    char* path = NULL;

    int disk_nr = strtol(full_path, &path, 10);

    if (path[0] == ':') {
        path++;
    } else {
        path = NULL;
    }

    *out_disk_nr = disk_nr;
    *out_path = path;
}

direntry_t* diropen(const char* path) {
    size_t disk_nr;
    char* div_path;

    vfs_parse_path(path, &disk_nr, &div_path);

    if (disk_nr >= 32) {
        return NULL;
    }

    fs_object_t* mt;
    
    for(int i = 0; i < 32; i++) {
        if(registered_mountpoints[i].disk_nr == disk_nr) {
            mt = registered_mountpoints + i;
            break;
        }
    }

    if(!mt) {
        return NULL;
    }

    if(!mt->valid) {
        return NULL;
    }

    return mt->filesystem->diropen(mt, div_path);
}

void dirclose(direntry_t* direntry) {
    if(!direntry) {
        return;
    }
    
    do {
        direntry_t* k = direntry;
        direntry = direntry->next;
        free(k);
    } while(direntry);
};

NFILE* nfopen(const char* path) {
    size_t disk_nr;
    char* div_path;

    vfs_parse_path(path, &disk_nr, &div_path);

    if (disk_nr >= 32) {
        return NULL;
    }

    fs_object_t* mt;
    
    for(int i = 0; i < MOUNTPOINTS_MAX_COUNT; i++) {
        if(registered_mountpoints[i].disk_nr == disk_nr) {
            mt = registered_mountpoints + i;
            break;
        }
    }

    if(!mt) {
        return NULL;
    }

    if(!mt->valid) {
        return NULL;
    }

    NFILE* fp = mt->filesystem->fileopen(mt, div_path);
    fp->_obj = mt;

    return fp;
}

void nfclose(NFILE* file) {
    fs_object_t* mt = file->_obj;
    
    mt->filesystem->fileclose(mt, file);

    free(file);
}

size_t nfread(void* buffer, size_t size, size_t count, NFILE* file) {
    if (!file) {
        return 0;
    }

    fs_object_t* mt = file->_obj;

    size_t a = mt->filesystem->fileread(mt, buffer, size, count, file);

    /*size_t bytes_to_read = size * count;
    if (file->position + bytes_to_read > file->size) {
        bytes_to_read = file->size - file->position;
    }

    if (bytes_to_read == 0) {
        return 0;
    }*/

    return a;
}

size_t nfwrite(const void* buffer, size_t size, size_t count, NFILE* file) {
    if (!file) {
        return 0;
    }

    fs_object_t* mt = file->_obj;

    size_t a = mt->filesystem->filewrite(mt, buffer, size, count, file);
    return a;
}


int main() {
    const char* path = "0:/";

    // Test path parsing
    size_t disk;
    char* div_path;
    vfs_parse_path(path, &disk, &div_path);

    printf("Disk nr: %zu; Path: %s\n", disk, div_path);

    // Register and scan filesystems
    register_filesystem("debugfs",
            &debugfs_probe, &debugfs_diropen, debugfs_fileopen,
            debugfs_fileread, debugfs_filewrite,
            debugfs_fileclose);
    vfs_scan();

    // Open a directory and list its contents
    direntry_t* entry = diropen(path);

    printf("dir: %p\n", entry);
    
    if(entry == 0) {
        printf("invalid entry\n");
        return 1;
    }

    direntry_t* original_pointer = entry;

    do {
        printf("[%s] %ld: %s\n",
                    original_pointer->type == ENT_DIRECTORY ? "DIR" : "FILE",
                    original_pointer->size,
                    original_pointer->name);
        original_pointer = original_pointer->next;
    } while(original_pointer);

    dirclose(entry);

    // Test nfwrite and nfread
    const char* file_path = "0:/testfile.txt";
    NFILE* test_file = nfopen(file_path);

    if (!test_file) {
        printf("Failed to open or create file: %s\n", file_path);
        return 1;
    }

    printf("Opened file: %s\n", file_path);

    const char* test_data = "Hello, VFS!";
    size_t data_length = strlen(test_data);

    // Write data to file
    size_t written_elements = nfwrite(test_data, sizeof(char), data_length, test_file);
    printf("Written elements: %zu\n", written_elements);

    // Reset the file position to the beginning
    test_file->position = 0;

    // Read data from file
    char read_buffer[128] = {0};
    size_t read_elements = nfread(read_buffer, sizeof(char), data_length, test_file);
    printf("Read elements: %zu\n", read_elements);
    printf("Read data: %s\n", read_buffer);

    // Close the file
    nfclose(test_file);

    return 0;
}
