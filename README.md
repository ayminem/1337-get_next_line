*This project has been created as part of the 42 curriculum by aymissao.*

# get_next_line

> Reading a line from a file descriptor is way too tedious.

## Description

**get_next_line** is a C function that returns, one call at a time, the next line read from a file descriptor:

```c
char *get_next_line(int fd);
```

Calling it repeatedly in a loop reads a whole file (or the standard input) line by line, without ever loading the entire content in memory.

**Goal.** Beyond adding a useful function to my toolbox, the project teaches:

- how file descriptors and the `read()` system call work;
- how **static variables** keep state between calls;
- careful **heap memory management** (no leaks, no double frees) under every execution path, including errors.

**Behavior.**

- The returned line **includes** its terminating `\n`, except when the end of the file is reached and the file does not end with a newline.
- `NULL` is returned when there is nothing left to read, or when an error occurs.
- It works on regular files and on any other file descriptor (standard input, pipes...).
- The size of the internal read buffer is set at compile time with `-D BUFFER_SIZE=n`, and a default value is provided when the flag is absent.

**Files.**

| File | Content |
|------|---------|
| `get_next_line.c` | `get_next_line()` and its main helpers |
| `get_next_line_utils.c` | string helpers (no libft allowed) |
| `get_next_line.h` | prototypes, includes, `BUFFER_SIZE` default |
| `get_next_line_bonus.c`, `get_next_line_utils_bonus.c`, `get_next_line_bonus.h` | bonus: several file descriptors at once |

**Constraints.** Allowed external functions: `read`, `malloc`, `free`. Forbidden: libft, `lseek()`, global variables. The code follows the 42 Norm.

## Instructions

### Compilation

The files are compiled directly with `cc` (a `BUFFER_SIZE` of 42 is only an example):

```sh
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line.c get_next_line_utils.c main.c
```

It also compiles **without** the flag: the header then falls back to a default value.

```sh
cc -Wall -Wextra -Werror get_next_line.c get_next_line_utils.c main.c
```

Bonus (several file descriptors at once):

```sh
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line_bonus.c get_next_line_utils_bonus.c main.c
```

### Usage

`get_next_line.h` must be included. The caller owns every returned line and must `free` it.

```c
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "get_next_line.h"

int main(int argc, char **argv)
{
    int     fd;
    char    *line;

    fd = 0;                          /* no argument: read the standard input */
    if (argc > 1)
        fd = open(argv[1], O_RDONLY);
    line = get_next_line(fd);
    while (line)
    {
        printf("%s", line);
        free(line);
        line = get_next_line(fd);
    }
    if (argc > 1)
        close(fd);
    return (0);
}
```

```sh
./a.out file.txt            # read a file
printf 'a\nb' | ./a.out     # read the standard input (pipe)
```

### Testing

- The test program above prints each line exactly as returned, so its output must be **identical** to the input file (`./a.out file.txt | diff - file.txt`).
- Tested with `BUFFER_SIZE` values of 1, 2, 3, 5, 42, 9999 and 10000000.
- Tested on: empty file, file with only newlines, file without a final newline, very long lines, many short lines, standard input, invalid and closed file descriptors.
- Memory checked with `valgrind --leak-check=full` and `-fsanitize=address`.

## Algorithm

### Core idea: a static "stash"

`read()` returns up to `BUFFER_SIZE` bytes with no regard for line boundaries, and the file position cannot be moved back (`lseek` is forbidden). A chunk can therefore contain the end of one line, whole lines, and the beginning of the next one. Whatever was read **beyond the end of the returned line must be remembered** until the next call.

This is the role of the **stash**: a heap string held by a `static` pointer, which keeps its value between calls.

> **Invariant:** between two calls, the stash contains exactly the bytes that were read from the file descriptor but not yet returned to the caller.

### Stages of one call

1. **Guard.** If `fd < 0` or `BUFFER_SIZE <= 0`, return `NULL` immediately.
2. **Fill.** Allocate a temporary buffer of `BUFFER_SIZE + 1` bytes. While the stash does not contain a `\n` and the end of the file has not been reached, call `read()`, terminate the chunk with `'\0'`, and join it to the stash (the previous stash is freed by the join). The buffer is freed on every path out. A `read()` error frees the stash, resets it to `NULL`, and returns `NULL`.
3. **Extract.** If the stash is `NULL` or empty, return `NULL`. Otherwise copy into a **new** string everything from the start of the stash up to and including the first `\n` (or up to the end of the string if there is none).
4. **Trim.** Replace the stash by a new string holding what follows that `\n`, and free the old stash. If nothing follows, the stash becomes `NULL` (never an empty string).
5. **Return** the extracted line.

### Why this design

- **Static variable:** the only way, without globals, to keep the leftover bytes between two calls.
- **Stop at the first newline:** the subject requires reading as little as possible. Fill reads only until a newline is in the stash, and the stash is checked *before* reading, so when a previous chunk already contained several lines, the following calls do not call `read()` at all.
- **Heap buffer, size `BUFFER_SIZE + 1`:** a buffer on the stack would overflow for huge values such as 10000000, and the extra byte holds the `'\0'` that `read()` does not add.
- **New strings for the line and for the trimmed stash:** the caller frees the line, so it must never point inside the stash.
- **Ownership rules:** `buf` is freed by the fill step, the stash by join, trim, or the end-of-file / error paths, and the line by the caller. This makes every path leak-free.
- **End of file:** `read() == 0` ends the fill loop but does not mean "return `NULL`": if the stash still holds the last line (a file without a trailing newline), that line is returned first, then the stash is released.

### Worked example

File content `ab\ncde\nf` (no final newline), `BUFFER_SIZE=4`:

| Call | Stash on entry | `read()` | Stash after fill | Line returned | Stash after trim |
|------|----------------|----------|------------------|---------------|------------------|
| 1 | `NULL` | 4 bytes `ab\nc` | `ab\nc` | `ab\n` | `c` |
| 2 | `c` | 4 bytes `de\nf` | `cde\nf` | `cde\n` | `f` |
| 3 | `f` | 0 (EOF) | `f` | `f` | `NULL` |
| 4 | `NULL` | 0 (EOF) | `NULL` | `NULL` | - |

### Complexity and trade-offs

- Each call reads at most the amount of data needed to complete one line, in chunks of `BUFFER_SIZE`.
- With a small `BUFFER_SIZE` there are many `read()` system calls, and joining the stash with each new chunk copies it every time, so a very long line costs more than linear time. With a large `BUFFER_SIZE` there are few system calls but each call may keep a bigger stash. This is a conscious simplicity-versus-speed choice: a single string stash is easy to reason about, to free correctly, and fits the Norm's function-size limits.

### Bonus

The bonus handles several file descriptors at once (e.g. alternating fd 3, 4, 5 without losing each one's reading state) using **only one static variable**: a static array of stash pointers indexed by the file descriptor. The algorithm is the same, applied to the stash belonging to the given `fd`, and `fd` values outside the array bounds are rejected.

## Resources

### References

- Manual pages: `man 2 read`, `man 3 malloc`, `man 3 free`, `man 2 open`.
- Valgrind and AddressSanitizer documentation (leak and memory error reports).
- Videos:
  - [Systems Programming: File Descriptors And File I/O in C](https://www.youtube.com/watch?v=AAbUfWZXDeM)
  - [Module 1 - Computer Science: File Descriptors](https://www.youtube.com/watch?v=WCrTRPu1bQk)
  - [C reading files](https://www.youtube.com/watch?v=Hzg3kCHJcxI)

### AI usage

AI (Claude) was used for:

- **Learning and design:** generating a helping guide that explains file descriptors, `read()`, static variables, the stash-based algorithm, edge cases, a test plan, and defense preparation.
- **Documentation:** drafting this README.

The C implementation was written, tested and debugged by me, and I can explain every part of it.
