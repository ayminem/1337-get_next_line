# get_next_line

## Purpose

`get_next_line` reads a file descriptor and returns one complete line per call.

A line:

- Ends with `\n` when present.
- Ends at EOF when no newline remains.
- Is returned once and not repeated.
- Is allocated dynamically and must be freed by the caller.

## Main Variables

- `fd`: The file descriptor to read.
- `BUFFER_SIZE`: Number of bytes read per system call.
- `remainder`: Static variable preserving unread data between calls.

The static variable is retained for the lifetime of the process. It is not reset before each call, so leftover data can be used by later calls.

## Header

The header declares:

- `<stdlib.h>` for `malloc` and `free`
- `<unistd.h>` for `read`
- `BUFFER_SIZE`
- All required function prototypes

A default `BUFFER_SIZE` of `32` is used if it is not provided by the compiler.

## Core Algorithm

`get_next_line(fd)` performs these steps:

1. Verify that `fd` is valid and `BUFFER_SIZE` is positive.
2. Read data into the static remainder until a newline or EOF is found.
3. Extract one line from the remainder.
4. Remove that line from the remainder.
5. Return the extracted line.

If EOF is reached and no data remains, `get_next_line` returns `NULL`.

## Reading Data

`read_to_remainder(fd, remainder)`:

1. Allocates a fixed-size buffer of `BUFFER_SIZE + 1`.
2. Reads at most `BUFFER_SIZE` bytes.
3. Terminates the newly read string with `'\0'`.
4. Appends the buffer to the existing remainder.
5. Continues until a newline exists or `read` returns `0`.

If `read` fails, the function frees the buffer and remainder and returns `NULL`.

This loop allows lines larger than `BUFFER_SIZE` to be handled.

## Extracting the Line

`extract_line(remainder)`:

1. Finds the index of `'\n'` or the end of the string.
2. Allocates enough memory for the line.
3. Copies characters into the allocated buffer.
4. Includes `'\n'` when one is found.
5. Adds the terminating null character.

The caller must free the returned line.

## Updating the Remainder

`update_remainder(remainder)`:

1. Finds the newline in the remainder.
2. If there is no newline, frees and clears the remainder.
3. Otherwise, copies everything after the newline.
4. Frees the old remainder.
5. Saves only the unused text for the next call.

For example, when remainder contains:

```text
first\nsecond\nthird
```

The first call returns:

```text
first\n
```

and the remainder becomes:

```text
second\nthird
```

The next call returns `second\n`, and the remainder becomes `third`.

## Utility Functions

### ft_strlen

Counts characters until `'\0'` and returns the length.

### ft_strchr

Searches for a character in a string.

It returns:

- A pointer to the matching character
- `NULL` if the character is absent
- A pointer to the end for `'\0'`

### ft_strjoin

Allocates and concatenates two strings:

```text
copy(s1) + copy(s2) + '\0'
```

It also accepts `NULL` strings in the implementation by treating them as empty strings.

### ft_memcpy

Copies `n` bytes from source to destination.

### ft_strdup

Allocates and copies a complete string, including its terminating null character.

### ft_substr

Creates a substring beginning at `start`.

It returns:

- An empty string if `start` is beyond the source length
- A shortened substring if insufficient characters remain
- `NULL` if allocation fails

## Memory Management

All dynamic memory must be released for normal or error conditions:

- Buffer data is freed after reading.
- Old remainder values are freed after joining or updating.
- The remainder is freed at EOF.
- Returned line strings must be freed by the caller.

## Error and EOF Handling

`get_next_line` returns `NULL` when:

- `fd` is negative.
- `BUFFER_SIZE` is zero or negative.
- Memory allocation fails.
- `read` fails.
- EOF is reached with no remaining content.

A read error clears the remainder to prevent stale data from being returned later.

## Compilation Example

```sh
cc -Wall -Wextra -Werror -D BUFFER_SIZE=32 \
	get_next_line.c get_next_line_utils.c main.c
```

## Usage Example

```c
#include "get_next_line.h"
#include <fcntl.h>
#include <stdio>

int	main(void)
{
	int		fd;
	char	*line;

	fd = open("example.txt", O_RDONLY);
	if (fd < 0)
		return (1);

	line = get_next_line(fd);
	while (line)
	{
		printf("%s", line);
		free(line);
		line = get_next_line(fd);
	}
	close(fd);
	return (0);
}
```
