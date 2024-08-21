#include "vfs.h"
#include "debugfs.h"
#include <stdio.h>
#include <string.h>

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
    size_t read_elements = nfread(read_buffer, sizeof(char), 64, test_file);
    printf("Read elements: %zu\n", read_elements);
    printf("Read data: %s\n", read_buffer);

    // Close the file
    nfclose(test_file);

    return 0;
}
