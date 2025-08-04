#ifdef _WIN32
#include <windows.h>
#else
#error "Windows.h not found."
#endif

#include <stdio.h>
#include <memory.h>
#include <stdint.h>

int main(void) {

    HANDLE mem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 256, "Server_Csral");

    if (mem == NULL) {
        fprintf(stderr, "Failed to created a map: %ld", GetLastError());
        return 1;
    }

    LPVOID data = MapViewOfFile(mem, FILE_MAP_ALL_ACCESS, 0, 0, 0);

    if (data == NULL) {
        fprintf(stderr, "Failed to created a view: %ld", GetLastError());
        return 1;
    }

    const char proc_end_cmd[256] = { 'e', 'n', 'd'};
    uint8_t run = 1;
    char collect[256];

    HANDLE event = CreateEvent(NULL, FALSE, FALSE, "Server_event_Csral");

    while (run) {

        printf("Input some value> ");
        scanf_s("%256s", collect, 256);

        if (memcmp(collect, proc_end_cmd, 3) == 0)
            run = 0;

        memcpy(
            data,
            (void*) collect,
            256
        ); //* send cmd

        //* Trigger event
        SetEvent(event);

    }

    printf("Process terminated!");

    return 0;

}