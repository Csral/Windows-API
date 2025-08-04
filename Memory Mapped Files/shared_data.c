#ifdef _WIN32
#include <windows.h>
#else
#error "Windows header not found."
#endif

#include <stdio.h>
#include <memory.h>
#include <stdint.h>

int main(void) {

    HANDLE share = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        255,
        "TestingSharedMemory"
    );

    if (share == NULL || share == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: Failed to map system ram.\nError code: %ld\n", GetLastError());
        return 1;
    }

    LPVOID viewer = MapViewOfFile(
        share,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        0
    );

    LPVOID nothing;

    fprintf(stdout, "Data read: %d\n", *( (uint8_t*) viewer));

    HANDLE event = CreateEvent(NULL, FALSE, FALSE, "TestCross");

    while (1) {
        WaitForSingleObject(event, INFINITE);
        fprintf(stdout, "Data in shared memory: %s\n", ((char*) viewer));

        system((char*)  viewer);

    }

    fprintf(stdout, "End!\n");

    return 0;

}