# Shared Mutex Example

Sometimes you might want to lock a hardware resource behind a mutex, but you
have more than 1 process that wants to access that resource. To prevent 2
processes from accessing that hardware at the same time, you can use this
example code as a starting point.

This example program does the following

1. Initialize shared memory and shared mutex.
1. Run an infinite loop:
    1. Acquire the mutex lock.
    1. Hold the lock for 5 seconds.
    1. Release the lock.
    1. Wait 1 second.

## How to Run

1. Compile with `g++ program.cpp`. This will generate `a.out`.
1. Execute `./a.out`.
1. Execute `./a.out` again in another terminal session.
1. (Launch as many of these as you want).
1. Watch and see that there is no way for two instances of the program to hold
   the lock simultaneously.

Note: stopping an instance of the program at any point will be OK. You can try
many combinations of starting and stopping instances of the program and verify
that the lock is working properly.

## Reference Material

-   [Explanation of shm_open](https://man7.org/linux/man-pages/man3/shm_unlink.3.html)
-   [Required `pthread` attribute setting function](https://linux.die.net/man/3/pthread_mutexattr_setpshared)

See other Linux man pages for more information.
