#ifndef UTIL_H
#define UTIL_H

#include <signal.h>

/* terminal color codes (source: https://stackoverflow.com/a/5947802) */
#ifdef _WIN32
	#define COLOR_RED ""
	#define COLOR_GREEN ""
	#define COLOR_BLUE ""
	#define TEXT_BOLD ""
	#define COLOR_NC ""
#else
	#define COLOR_RED "\033[0;31m"
	#define COLOR_GREEN "\033[0;32m"
	#define COLOR_BLUE "\033[0;34m"
	#define TEXT_BOLD "\033[1m"
	#define COLOR_NC "\033[0m"
#endif
#define AS_COLOR(c, of) c of COLOR_NC

#ifdef _WIN32
    #include <windows.h>
    // source: https://stackoverflow.com/a/3379146
    #define os_sleep(x) Sleep(x*1000) // time is in millisecs
#else
    #include <unistd.h>
    #define os_sleep(x) usleep(x*1000)
#endif

/* useful types (why not) */
#define uint unsigned int

struct SavedInstance {
    // simple file format for storing program data (why not?)
    int     app_ver;
    char    run_once;
};

/* utility functions (we know that's wrong but we target for a simple compilation) */
uint sum_of(uint *array, uint until){
    uint s = 0;
    for (uint i=0; i < until; i++){
        s += array[i];
    }
    return s;
}

void check_for_first_run(char *the_file_path, int min_ver){
    FILE *save_f; struct SavedInstance si;
    si.run_once = 0;
    // way to check if file exists: access( fname, F_OK ) == -1; // source: https://stackoverflow.com/a/230068
    if ( save_f = fopen(the_file_path, "rb") ){
        fread(&si, sizeof(struct SavedInstance), 1, save_f);
        fclose(save_f);
    }
    if (si.app_ver >= min_ver){
        if (si.run_once){
            return;
        }
    } else {
        printf("(I) Your saved data is from a previous version (%d). You are running: %d\n", si.app_ver, min_ver);
        printf("(I) The old data will be discarded.\n");
    }
    

    printf("(I) First time run. Any doubts, please read the source file for %s.\n",
        AS_COLOR(TEXT_BOLD, "documentation")
    );

    si.app_ver = min_ver;
    si.run_once = 1;
    if ( save_f = fopen(the_file_path, "wb") ){
        fwrite(&si, sizeof(struct SavedInstance), 1, save_f);
        fclose(save_f);
    }
}

void check_platform_compatibility(){
    // platform testing (source: https://stackoverflow.com/a/42040445)
    #if defined(_WIN32)
        printf("(W) You are running this program on Microsoft Windows. Some features will not work or might look awkward.\n");
    #endif
}

typedef void(cb_func)(int);
void attach_signal(cb_func *f){
    // atach simple Ctrl+C signal
    signal(SIGINT, f);
}

#endif