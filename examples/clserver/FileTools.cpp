#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>

typedef class FileTools {
public:
    static int addWatch(char *, ...);
    static void removeWatch(int);
    static bool hasChanged(int);

private:
    static int fd;
} *pFileTools;

// Parse variable arguments into 'buffer' character array
// This is just a macro I use often in my functions
#define parsevargs(buff, buffer)                    \
    char buffer[128];                               \
    memset(buffer, 0, sizeof(buffer));              \
    va_list ap;                                     \
    va_start(ap, buff);                             \
    vsnprintf(buffer, sizeof(buffer), buff, ap);    \
    va_end(ap);
//And on to the implementation:

int FileTools::fd = 0;

int FileTools::addWatch(char *path, ...)
{
    // Parse variable arguments into "buffer"
    parsevargs(path, buffer);

    // Initialize inotify if need be
    if (!FileTools::fd) FileTools::fd = inotify_init();

    printf("Adding Watch for %s..\n", buffer);

    // Return a new watch handle
    return inotify_add_watch(FileTools::fd, buffer, IN_CLOSE_WRITE);
}

void FileTools::removeWatch(int wd)
{
    printf("Removing Watch id = %i..\n", wd);
    inotify_rm_watch(FileTools::fd, wd); 
}

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN    (1024 * (EVENT_SIZE + 16))
bool FileTools::hasChanged(int wd)
{
    bool ret = false;

    // Return from select immediately if none available
    struct timeval time;
    time.tv_sec = 0;
    time.tv_usec = 0;
    fd_set rfds;

    FD_ZERO(&rfds);

    // Set up our fd_set with our inotify fd
    FD_SET(FileTools::fd, &rfds);

    // Do the select
    select(FileTools::fd + 1, &rfds, NULL, NULL, &time);

    if (FD_ISSET(FileTools::fd, &rfds)) {
        int length = 0;
        int i = 0;
        char buffer[BUF_LEN];

        length = read(FileTools::fd, buffer, BUF_LEN);

        while (i < length) {
            // Got a match
            struct inotify_event *event = (struct inotify_event*)&buffer[i];

            if (event->wd == wd &&
                event->mask & IN_CLOSE_WRITE) {
                // It was a file we were monitoring!
                printf("Watched file %i has changed\n", wd);
                ret = true;
            }

            i += EVENT_SIZE + event->len;
        }
    }

    return ret;
}
