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

static const char* g_console_shortcodes[][2] = {
    {"megadrive", "1"},
    {"nintendo64", "2"},
    {"superNintendo", "3"},
    {"snes", "3"},
    {"gameboy", "4"},
    {"gameboyadvance", "5"},
    {"gba", "5"},
    {"gameboycolor", "6"},
    {"gbc", "6"},
    {"nintendo", "7"},
    {"nes", "7"},
    {"pcengine", "8"},
    {"pce", "8"},
    {"segacd", "9"},
    {"sega32x", "10"},
    {"mastersystem", "11"},
    {"sms", "11"},
    {"playstation", "12"},
    {"ps1", "12"},
    {"psx", "12"},
    {"atarilynx", "13"},
    {"lynx", "13"},
    {"neogeopocket", "14"},
    {"ngp", "14"},
    {"gamegear", "15"},
    {"gg", "15"},
    {"gamecube", "16"},
    {"gc", "16"},
    {"atarijaguar", "17"},
    {"jaguar", "17"},
    {"nintendods", "18"},
    {"nds", "18"},
    {"wii", "19"},
    {"wiiu", "20"},
    {"playstation2", "21"},
    {"ps2", "21"},
    {"xbox", "22"},
    {"magnavoxodyssey2", "23"},
    {"odyssey2", "23"},
    {"pokemonmini", "24"},
    {"atari2600", "25"},
    {"atari", "25"},
    {"msdos", "26"},
    {"dos", "26"},
    {"arcade", "27"},
    {"virtualboy", "28"},
    {"vb", "28"},
    {"msx", "29"},
    {"commodore64", "30"},
    {"c64", "30"},
    {"zx81", "31"},
    {"oric", "32"},
    {"sg1000", "33"},
    {"vic20", "34"},
    {"amiga", "35"},
    {"atarist", "36"},
    {"amstradpc", "37"},
    {"appleii", "38"},
    {"saturn", "39"},
    {"dreamcast", "40"},
    {"dc", "40"},
    {"psp", "41"},
    {"cdi", "42"},
    {"3do", "43"},
    {"colecovision", "44"},
    {"intellivision", "45"},
    {"vectrex", "46"},
    {"pc8800", "47"},
    {"pc9800", "48"},
    {"pcfx", "49"},
    {"atari5200", "50"},
    {"atari7800", "51"},
    {"x68k", "52"},
    {"wonderswan", "53"},
    {"ws", "53"},
    {"cassettevision", "54"},
    {"supercassettevision", "55"},
    {"neogeocd", "56"},
    {"ngcd", "56"},
    {"fairchildchannelf", "57"},
    {"fmTowns", "58"},
    {"zxspectrum", "59"},
    {"spectrum", "59"},
    {"gameandwatch", "60"},
    {"nandwatch", "60"},
    {"nokiangage", "61"},
    {"nintendo3ds", "62"},
    {"n3ds", "62"},
    {"supervision", "63"},
    {"sharpx1", "64"},
    {"tic80", "65"},
    {"thomsonto8", "66"},
    {"pc6000", "67"},
    {"pico", "68"},
    {"megaduck", "69"},
    {"zeebo", "70"},
    {"arduboy", "71"},
    {"wasm4", "72"},
    {"arcadia2001", "73"},
    {"intertonvc4000", "74"},
    {"elektortvgamescomputer", "75"},
    {"pcenginecd", "76"},
    {"pcecd", "76"},
    {"atarijaguarcd", "77"},
    {"nintendodsi", "78"},
    {"ndsi", "78"},
    {"ti83", "79"},
    {"uzebox", "80"},
    {"famicomdisksystem", "81"},
    {"fds", "81"},
    {NULL, NULL}
};

static uint32_t parse_console_id(const char* shortcode) {
    if (!shortcode)
        return RC_CONSOLE_UNKNOWN;
    
    for (int i = 0; g_console_shortcodes[i][0] != NULL; i++) {
        if (strcasecmp(shortcode, g_console_shortcodes[i][0]) == 0)
            return (uint32_t)atoi(g_console_shortcodes[i][1]);
    }
    
    return RC_CONSOLE_UNKNOWN;
}

static void* RC_CCONV cdreader_open_track(const char* path, uint32_t track) {
    if (g_track)
        cdfs_close_track(g_track);
        
    g_track = cdfs_open_track(path, track);
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

static void RC_CCONV cdreader_close_track(void* track_handle) {
    (void)track_handle;
    if (g_track) {
        cdfs_close_track(g_track);
        g_track = NULL;
        g_file_opened = 0;
    }
}

static size_t RC_CCONV cdreader_read_sector(void* track_handle, uint32_t sector, void* buffer, size_t requested_bytes) {
    cdfs_track_t* track = (cdfs_track_t*)track_handle;
    uint32_t first_sector = track ? track->first_sector_index : 0;
    uint32_t rel_sector = sector >= first_sector ? sector - first_sector : sector;
    
    if (!g_track || !g_file_opened)
        return 0;
    
    cdfs_seek_sector(&g_file, rel_sector);
    
    int64_t bytes_read = cdfs_read_file(&g_file, buffer, requested_bytes);
    if (bytes_read < 0)
        return 0;
    
    return (size_t)bytes_read;
}

static uint32_t RC_CCONV cdreader_first_track_sector(void* track_handle) {
    cdfs_track_t* track = (cdfs_track_t*)track_handle;
    if (!track)
        return 0;
    return track->first_sector_index;
}

static void* RC_CCONV cdreader_open_track_iterator(const char* path, uint32_t track, const rc_hash_iterator_t* iterator) {
    (void)iterator;
    return cdreader_open_track(path, track);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <console_shortcode> <chd_file>\n", argv[0]);
        printf("Example: %s dreamcast game.chd\n", argv[0]);
        return 1;
    }

    const char* console_shortcode = argv[1];
    const char* chd_path = argv[2];

    uint32_t console_id = parse_console_id(console_shortcode);
    if (console_id == RC_CONSOLE_UNKNOWN) {
        printf("Unknown console: %s\n", console_shortcode);
        return 1;
    }

    rc_hash_cdreader_t cdreader;
    cdreader.open_track = cdreader_open_track;
    cdreader.read_sector = cdreader_read_sector;
    cdreader.close_track = cdreader_close_track;
    cdreader.first_track_sector = cdreader_first_track_sector;
    cdreader.open_track_iterator = cdreader_open_track_iterator;

    rc_hash_init_custom_cdreader(&cdreader);

    char hash[33];
    if (!rc_hash_generate_from_file(hash, console_id, chd_path)) {
        printf("Failed to generate hash for %s\n", chd_path);
        return 1;
    }

    printf("%s\n", hash);
    return 0;
}
