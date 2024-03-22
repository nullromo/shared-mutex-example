#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define NAME "/my_shm_test"

pthread_mutex_t* mutex;

void quit(const char* message, int returnCode = 1) {
    printf("%s\n", message);
    exit(returnCode);
}

void exitHandler(int signalCode) {
    if ((signalCode == SIGTERM) || (signalCode == SIGINT) ||
        (signalCode == SIGQUIT) || (signalCode == SIGSEGV)) {
        printf("Caught signal.\n");
        printf("Unlocking mutex.\n");
        int status = pthread_mutex_unlock(mutex);
        if (status != 0) {
            quit("Error unlocking mutex.");
        }
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    signal(SIGTERM, exitHandler);
    signal(SIGINT, exitHandler);
    signal(SIGQUIT, exitHandler);
    signal(SIGSEGV, exitHandler);
    int status = 0;

    if (argc != 1) {
        quit("No arguments allowed.");
    }

    printf("Starting\n");

    bool shouldInitialize = true;

    // create or obtain shared file descriptor
    int fileDescriptor =
            shm_open(NAME, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fileDescriptor == -1) {
        if (errno == EEXIST) {
            // the shared file already exists
            printf("Shared file already exists.\n");
            // get a file descriptor again, but this time don't try to create a
            // new file. Also set the shouldInitialize flag to false
            shouldInitialize = false;
            fileDescriptor = shm_open(NAME, O_RDWR, S_IRUSR | S_IWUSR);
            if (fileDescriptor == -1) {
                quit("Error during shm_open.", fileDescriptor);
            }
        } else {
            // other error
            quit("Error during shm_open.", fileDescriptor);
        }
    } else {
        printf("Created shared file.\n");
    }

    if (shouldInitialize) {
        // created, need to truncate
        printf("Truncating shared file.\n");
        int truncateResult = ftruncate(fileDescriptor, sizeof(pthread_mutex_t));
        if (truncateResult == -1) {
            quit("Error during ftruncate.", truncateResult);
        }
    } else {
        printf("No need to truncate shared file.\n");
    }

    printf("Using file descriptor %d for shared file.\n", fileDescriptor);

    // map shared file into memory
    mutex = (pthread_mutex_t*)mmap(NULL,
                                   sizeof(*mutex),
                                   PROT_READ | PROT_WRITE,
                                   MAP_SHARED,
                                   fileDescriptor,
                                   0);
    if (mutex == MAP_FAILED) {
        quit("Error during mmap", errno);
    }
    printf("Memory mapped shared file to mutex object.\n");

    status = close(fileDescriptor);
    if (status != 0) {
        quit("Error during close.");
    }

    if (shouldInitialize) {
        printf("Initializing mutex.\n");
        pthread_mutexattr_t mutexAttributes;
        status = pthread_mutexattr_setpshared(&mutexAttributes,
                                              PTHREAD_PROCESS_SHARED);
        if (status != 0) {
            quit("Error during pthread attribute set.");
        }

        status = pthread_mutex_init(mutex, &mutexAttributes);
        if (status != 0) {
            quit("Error during pthread init.");
        }
    } else {
        printf("No need to initialize mutex object.\n");
    }

    printf("Shared mutex ready.\n");

    while (true) {
        printf("Acquiring mutex lock...\n");
        int status = pthread_mutex_lock(mutex);
        if (status != 0) {
            quit("Error during mutex lock.");
        }
        for (uint i = 0; i < 5; i++) {
            printf("Simulating processing (%d/5)...\n", i + 1);
            sleep(1);
        }
        printf("Done processing.\n");
        status = pthread_mutex_unlock(mutex);
        if (status != 0) {
            quit("Error during mutex unlock.");
        }
        sleep(1);
    }
}
