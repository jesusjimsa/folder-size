/**
 * Copyright 2020 Jesús Jiménez Sánchez
 *
 */

#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#endif

#ifndef WIN32

off_t FileSize(const char * path) {
    struct stat buf;
    return stat(path, &buf) ? -1 : buf.st_size;
}

off_t dirSize(const char *path) {
    struct dirent *dir;
    DIR *d;
    off_t folder_size = 0;
    char *new_path;

    d = opendir(path);

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            // Ignore . and ..
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
                continue;
            }

            new_path = malloc(strlen(path) + strlen(dir->d_name) + 2);
            strcpy(new_path, path);
            strcat(new_path, "/");
            strcat(new_path, dir->d_name);

            // Recursion if the path points to a directory
            if (dir->d_type == DT_DIR) {
                folder_size += dirSize(new_path);
            }
            else {
                folder_size += FileSize(new_path);
            }

            if (new_path) {
                free(new_path);
            }
        }

        closedir(d);
    }

    return folder_size;
}

#else

DWORD FileSizeWin(const char * file) {
    HANDLE h1;
    BY_HANDLE_FILE_INFORMATION lpFileInfo;

    h1 = CreateFile(file, GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, NULL);

    if (h1 == INVALID_HANDLE_VALUE) {
        printf("Error handling file '%s'", file);
    }
    else if (GetFileInformationByHandle(h1, &lpFileInfo) == 0) {
        CloseHandle(h1);
        printf("Error handling file '%s'", file);
    }
    else {
        CloseHandle(h1);
        return lpFileInfo.nFileSizeHigh + lpFileInfo.nFileSizeLow;
    }

    return -1;
}

float dirSize(const char *sDir) {
    WIN32_FIND_DATA fdFile;
    HANDLE hFind = NULL;
    float folder_size = 0.0;

    char sPath[2048];

    // Specify a file mask. *.* = We want everything!
    sprintf(sPath, "%s\\*.*", sDir);

    if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE) {
        printf("Path not found: [%s]\n", sDir);
        return 0;
    }

    do {
        // Find first file will always return "."
        //    and ".." as the first two directories.
        if (strcmp(fdFile.cFileName, ".") != 0 && strcmp(fdFile.cFileName, "..") != 0) {
            // Build up our file path using the passed in
            //  [sDir] and the file/foldername we just found:
            sprintf(sPath, "%s\\%s", sDir, fdFile.cFileName);

            // Is the entity a File or Folder?
            if (fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY) {
                folder_size += dirSize(sPath);
            }
            else {
                folder_size += FileSizeWin(sPath);
            }
        }
    } while (FindNextFile(hFind, &fdFile));     // Find the next file.

    FindClose(hFind);       // Always, Always, clean things up!

    return folder_size;
}

#endif

int main(int argc, char **argv) {
    clock_t begin, end;
    float folder_size = 0.0;
    char *path;

    path = (char *)malloc(sizeof(char) * 6556);

#ifndef WIN32
    if (argc < 2) {
        printf("Write the complete path: ");
        scanf("%s", path);
    }
    else {
        path = argv[1];
    }

    if (path[0] != '/') {
        printf("Please, write the full path.\n");
        printf("Example: /var/lib/...\n");
        return 0;
    }
#else
    printf("Write the complete path: ");
    gets(path);

    if (path[0] != 'c' && path[1] != ':' && path[2] != '\\') {
        printf("Please, write the full path.\n");
        printf("Example: c:\\windows\\");
        return 0;
    }
#endif

    begin = clock();

#ifndef WIN32
    folder_size = dirSize(path);
#else
    folder_size = dirSize(path);
#endif

    end = clock();

    folder_size = folder_size / 1024;

    printf("\nTotal folder size --> %.5f KB\n\n", folder_size);
    printf("Total time --> %.6f seconds\n\n", (float)(end - begin) / CLOCKS_PER_SEC);

#ifdef WIN32
    printf("Press ENTER key to Continue\n");
    getchar();
#endif

    return 0;
}
