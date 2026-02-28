#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <rc_hash.h>
#include <rc_consoles.h>
#include <formats/cdfs.h>

static cdfs_track_t* g_track = NULL;
static cdfs_file_t g_file;
static int g_file_opened = 0;

static void* RC_CCONV cdreader_open_track(const char* path, uint32_t track) {
    (void)track;
    
    if (g_track)
        cdfs_close_track(g_track);
        
    g_track = cdfs_open_data_track(path);
    if (!g_track)
        return NULL;
    
    g_file_opened = cdfs_open_file(&g_file, g_track, NULL);
    if (!g_file_opened) {
        cdfs_close_track(g_track);
        g_track = NULL;
        return NULL;
    }
    
    return g_track;
}

static size_t RC_CCONV cdreader_read_sector(void* track_handle, uint32_t sector, void* buffer, size_t requested_bytes) {
    (void)track_handle;
    
    if (!g_track || !g_file_opened)
        return 0;
    
    cdfs_seek_sector(&g_file, sector);
    
    int64_t bytes_read = cdfs_read_file(&g_file, buffer, requested_bytes);
    if (bytes_read < 0)
        return 0;
    
    return (size_t)bytes_read;
}

static void RC_CCONV cdreader_close_track(void* track_handle) {
    (void)track_handle;
    if (g_track) {
        cdfs_close_track(g_track);
        g_track = NULL;
        g_file_opened = 0;
    }
}

static uint32_t RC_CCONV cdreader_first_track_sector(void* track_handle) {
    (void)track_handle;
    return 0;
}

static void* RC_CCONV cdreader_open_track_iterator(const char* path, uint32_t track, const rc_hash_iterator_t* iterator) {
    (void)iterator;
    return cdreader_open_track(path, track);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <chd_file>\n", argv[0]);
        return 1;
    }

    const char* chd_path = argv[1];

    rc_hash_cdreader_t cdreader;
    cdreader.open_track = cdreader_open_track;
    cdreader.read_sector = cdreader_read_sector;
    cdreader.close_track = cdreader_close_track;
    cdreader.first_track_sector = cdreader_first_track_sector;
    cdreader.open_track_iterator = cdreader_open_track_iterator;

    rc_hash_init_custom_cdreader(&cdreader);

    char hash[33];
    if (!rc_hash_generate_from_file(hash, RC_CONSOLE_PLAYSTATION, chd_path)) {
        printf("Failed to generate hash for %s\n", chd_path);
        return 1;
    }

    printf("%s\n", hash);
    return 0;
}
