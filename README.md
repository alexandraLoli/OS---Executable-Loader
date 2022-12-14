# Skeleton for building a ELF loader

## Introduction
This project contains a skeleton for building an
[ELF](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format) binary
on-demand loader in Linux. The loader will provide two methods, defined in the
`loader.h` header:
* `int so_init_loader(void);` - initializes the on-demand loader
* `int so_execute(char *path, char *argv[]);` - executes the binary located in
`path` with the required `argv` arguments.

## Content
The project contains of three components, each of them in its own
directory:
* `loader` - a dynamic library that can be used to run ELF binaries. It
consists of the following files:
  * `exec_parser.c` - Implements an ELF binary parser.
  * `exec_parser.h` - The header exposed by the ELF parser.
  * `loader.h` - The interface of the loader, described in the
  [Introduction](#introduction) section.
  * `loader.c` - Contains functions to implement the loader:
    * 'so_execute'  - call so_execute function which parse 
                    an executable speficied in path;
                    - open the file specified in path;
                    - allocate memory for "data" parameter of a
                    segment (data is implemented as a data structure which contains an array of pages with values 0 and 1, that tells if a page has been mapped or not, and an integer that stores the number of pages) and for each array of pages;
                    - run the first instruction of the executable.
    * 'so_init_loader' - initializes the loader and calls the 
                      handler for SIGSEGV signal.
    * 'segv_handler'  - contains the actual loader impelentation;
                      - verify if the received signal is SIGSEV, if 
                      not - call the default handler;
                      - call function 'find_segment_with_segv' and store in variable index_segment the index of the segment where SIGSEGV happened
                      - knowing the segment, find the page where SIGSEGV happned;
                      - knowing the page, verify if it's mapped, if it is - call the default handler
                      - if the page is not mapped, use the mmap function to do so, and copy the information from the file opened in so_execute function (three cases: *first* when the whole page is copied, *second* when just a part of it is copied, *third* when there's nothing to be copied) and set the permissions 
  * `debug.h` - header for the `dprintf` function that can be used for logging
  and debugging.


## Notes
This skeleton is provided by the Operating System team from the University
Politehnica of Bucharest to their students to help them complete their
Executable Loader assignment.
