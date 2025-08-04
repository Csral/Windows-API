#ifdef _WIN32
#include <windows.h>
#else
#error "Windows header not found."
#endif

#include <stdio.h>
#include <memory.h>
#include <stdint.h>

int main(void) {

    /*
    
        * We must map the file 
    
    */

    HANDLE test_file;
    DWORD status;
    test_file = CreateFile(
        "test.txt",
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        1,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    status = GetLastError();

    if (status == 80) {
        /* File exists, open it */
        test_file = CreateFile(
            "test.arcln",
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            3,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        status = GetLastError();
    }

    if (status != 0) {
        fprintf(stderr, "The last error code (File creation/opening): %lu\n", status);
        return 1;
    }

    HANDLE test_mapping;
    test_mapping = CreateFileMapping(test_file, NULL, PAGE_READWRITE, 0, 0, "Testing_File_Mapping");

    status = GetLastError();

    if (status != 0) {
        fprintf(stderr, "The last error code (Map creation): %lu\n", status);
        return 2;
    }

    LPVOID view = MapViewOfFile(test_mapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    char buffer[8];
    memcpy(buffer, view, 8);
    
    char tes;

    for (DWORD i = 0; i < 20; i++) {

        memcpy(
            &tes,
            (void*) ((uint8_t*) view + i + 0x10000),
            sizeof(uint8_t)
        );
        fprintf(stdout, "The character at position %lu is: %c\n", i, tes);

    };

    //* Testing write.
    fprintf(stdout, "DEBUG: Printing -> %c\n", *(( (uint8_t*) view + 1)));
    
    for (DWORD i = 0; i < 10; i++)
        *(( (uint8_t*) view + 10 + i)) = 'c';

    *((uint8_t*) view + 21) = 'N';
    *((uint8_t*) view + 22) = 'I';
    *((uint8_t*) view + 23) = 'G';
    *((uint8_t*) view + 24) = 'G';
    *((uint8_t*) view + 25) = 'A';
    // memset(
    //     (void*) (((uint8_t*) view + 1)),
    //     (int) 'c',
    //     sizeof(char)
    // );

    // tes = 'W';
    // memset(
    //     (void*) ((uint8_t*) view + 8),
    //     (int) tes,
    //     sizeof(uint8_t)
    // );

    // tes = 'S';
    // memcpy(
    //     (void*) ((uint8_t*) view + 9),
    //     &tes,
    //     sizeof(uint8_t)
    // );
    fprintf(stdout, "Flushing\n");
    FlushViewOfFile((uint8_t*) view + 6, 10);

    /* Close active files */
    CloseHandle(test_file);
    CloseHandle(test_mapping);

    fprintf(stdout, "Finished executing.\n");

    return 0;

}