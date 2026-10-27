/* Keep Windows SDK types/macros out of the legacy numerical translation unit. */
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <errno.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
FILE *jip_windows_io_open_exclusive(const char *path) {
 int fd=_open(path,_O_WRONLY|_O_CREAT|_O_EXCL|_O_BINARY,_S_IREAD|_S_IWRITE);
 if(fd<0)return NULL;FILE *file=_fdopen(fd,"wb");
 if(!file){int saved=errno;_close(fd);remove(path);errno=saved;}return file;
}
int jip_windows_io_replace(const char *from,const char *to) {
 return MoveFileExA(from,to,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)?0:-1;
}
