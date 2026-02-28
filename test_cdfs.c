#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <formats/cdfs.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <chd_file>\n", argv[1]);
        return 1;
    }

    const char* chd_path = argv[1];
    
    printf("Opening CHD file: %s\n", chd_path);
    
    cdfs_track_t* track = cdfs_open_data_track(chd_path);
    if (!track) {
        printf("Failed to open track\n");
        return 1;
    }
    
    printf("Track opened successfully!\n");
    
    /* Try to open with NULL path to get raw track */
    cdfs_file_t file;
    if (cdfs_open_file(&file, track, NULL)) {
        printf("Opened raw track via NULL path!\n");
    } else {
        printf("Failed to open raw track with NULL path\n");
    }
    
    cdfs_close_track(track);
    return 0;
}
