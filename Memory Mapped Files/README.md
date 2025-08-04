# File mapping
File mapping allows files to be treated as RAM. System creates a file mapping object and extends virtual address section of the device.

**Allows random IO and sequential IO**.
**I'll prolly use this for random async IO pointers.**

It is a known fact that programs reads files using pointers and offsets. The best example would be C in which we use FILE* to initiate file operations and that shows its a pointer. Advantages of mapping it to virtual memory is the speed it provides as the file lies on disk but is accessed via memory mapped addresses.

Note: **The file on disk can be any file that you want to map into memory, or it can be the system page file. The file mapping object can consist of all or only part of the file. It is backed by the file on disk. This means that when the system swaps out pages of the file mapping object, any changes made to the file mapping object are written to the file. When the pages of the file mapping object are swapped back in, they are restored from the file.**

---

Understandings:

* Any file can be mapped to memory.
* The system does not continuously swap the memory and file.
* The system lazy loads the pages. So when i request X blocks out of the file, it is loaded to the virtual memory (Memory mapped IO, RAM) associated with the process requesting the file.
* If any changes are done to the page, it is marked as dirty and OS later writes it off eventually or if flush is called.
* The changes will also be written if OS reclaims the said portion of memory.
* Once loaded, any changes done or any content read is read from the RAM and not from the disk. This boosts speed.

---

**Accessing**
A process can manipulates memory mapped file using a file view which contains a part of or the entirety of the file which is mapped to memory. A process can create multiple such file views and each file view resides in the virtual memory of that process. When it needs data from other portion of file, it can simply unmap the current section and create a new file view.

If multiple process use same mapping to create multiple file views, then the data is shared amongst all the processes.

## Further explanations

Mostly in specific .c or .cpp files unless mentioned below.

## Creating a memory map

* First step is to open the file via CreateFile and to block write access to other processes we must open it with exclusive access. The file handle used must remain **open** until the process no longer needs the memory map of file. The most easy way to generate such handle with exclusive rights is to pass 0 (false) to fdwShareMode.
* The handle obtained in step 1 is used to create a file map using CreateFileMapping function. This function also returns a handle which is used for accessing the file via a file view.
* CreateFileMapping requires an object name, number of bytes to be mapped and read-write permissions for the said memory.
* Only the first process creating a file map receives the unique handle. Any process attempting to access an existing file map receives the same handle.
* GetLastError helps to spot any failures. The main/first process receives NO_ERROR while the subsequent processes receive ERROR_ALREADY_EXISTS
* The CreateFileMapping function fails if the access parameters of the function varies form those of CreateFile.
* Goal:
```
    Specify the GENERIC_READ and GENERIC_WRITE values in the fdwAccess parameter of CreateFile.
    Specify the PAGE_READWRITE value in the fdwProtect parameter of CreateFileMapping.
```

### File size of memory map

The file size of memory map doesn't need to match the actual file size. **However, if the file size of memory map is specified to be larger than the actual file size, the file size is expanded.** Thus, in systems where accuracy is critical one must ensure the size match or is less than actual file size.

In systems where the file size should remain intact, prefer setting the dwMaximumSizeHigh and dwMaximumSizeLow parameters of CreateFileMapping to zero and this will create a memory map of exactly the file size. Otherwise, the program should calculate the file size manually.
**Note: dwMaximumSizeHigh and dwMaximumSizeLow help to modify the size of memory map.**

The size of file memory maps cannot be modified post their creation. Thus, allocation of zero-sized memory maps results in rejection of the system call with an error code of ERROR_FILE_INVALID.

The maximum reservable size for a file map for a named file on disk is limited by the disk space itself. However, The size of a file view is limited to the largest available contiguous memory blocks in the unreserved virtual memory. This is usually 2 gb minus the reserved virtual space for the process (in 32 bit systems).

**Note: In 32-bit systems, 2 gb of vspace is given to user mode and 2 gb for kernel mode.**

The size of the file map object that you select controls how far into the file you can view. Thus if you specify a file map object of 512 bytes then you can only view the first 512 bytes of the selected file irrespective to the actual file size. Thus it is almost always recommend to create a file map with the size of actual file size even if you do not expect to read full file. This is important because we can't expand the mapping object size later on.

---

**Difference between file view and file map**

File map only reserves the space while file view provides a way to interact with data inside the map, i.e., read and write.

---

## Creating a file view
To actually map data from file to virtual space, you must have a file view. Without file view we cannot access or modify the data in the file. The functions MapViewOfFile and MapViewOfFileEx are used to generate a file view from the handle returned by CreateFileMapping function for the whole file or a portion of it. These functions return a pointer to the file view. By dereferencing the file view in range of valid addresses specified in the MapViewOfFile, one can read and write data onto file.

**In the same way that CreateFileMapping fails if its access flags conflict those of CreateFile, the MapViewOfFile\* functions fail if the access flags conflict those of CreateFileMapping**

Writing data into the file view will update the file map object and the changes aren't immediately performed on the actual file. The Windows OS will slowly but surely write the changes later on. This is done to improve the general performance of system and the file map being used. Thus, any changes performed (I/O) are cached and written later by the system. However, to override this and make the system write the changes immediately, use **FlushViewOfFile** function.

MapViewOfFileEx allows to specify a base address for the file view to exist (not to be confused with offset) in the lpvBase parameter. This call fails if there is not enough space available at the specified address or if the address specified is not an integral multiple of the system memory allocation granularity. Thus, lpvBase sets the reference address (say R) such that R is linked to the logical address of file (say L) whose physical address is P such that when my process accesses R from the virtual space, the OS provides information from L which is given from the address P by the disk. 

Allocation granularity is the smallest allocatable unit in memory that Windows OS can provide for a memory map. As opposed to the page size (min 4kb), the allocation granularity for the file view handlers is 64kb. This can be confirmed using the GetSystemInfo call.
```
    Example: If granularity is 64 KB, valid addresses are:

0x00000000
0x00010000
0x00020000
...

    Passing 0x12345 (not divisible by 64 KB) → MapViewOfFile fails.

```

***GUIDELINES FOR PARAMETERS***

* A file view can be a different size than the file mapping object from which it is derived, but it must be smaller than the file mapping object.
* The offset specified by the dwOffsetHigh and dwOffsetLow parameters of MapViewOfFile must be a multiple of the allocation granularity of the system.