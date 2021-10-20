# Project 1: System Inspector

See: https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-1.html 

To compile and run:

```bash
make
./inspector
```

## Description

This program reads the `/proc` directory, parsing various files, interpreting the usage, and showing in a tabular format. It mimics and serves as a simpler version of `top` or `htop` on *nix machines.

The `/proc` directory on a linux machine stores the important information of the machine, including but not limited to:

- uptime

    - a file containing the current uptime statistics of the machine

- meminfo

    - a file containing the information of memory usage on the machine

- cpuinfo

    - a file containing information on the cpu of the machine

## Building

In order to build it, go to the main folder and run `make`. This will build all the files.

## Running

In order to run it, build the files, then input `./inspector` into the terminal. This will run the executable file of the inspector.

### Running Options

There are various options for running the inspector.

    ./inspector -h
    Usage: ./inspector [-ahlrst] [-p procfs_dir]

    Options:
        * -a              Display all (equivalent to -lrst, default)
        * -h              Help/usage information
        * -l              Task List
        * -p procfs_dir   Change the expected procfs mount point (default: /proc)
        * -r              Hardware Information
        * -s              System Information
        * -t              Task Information

## Files Included

There are several files included:

- display.c: The c file in charge of running a terminal display
- display.h: The header file containing definitions for display.c
- inspector.c: The c file in charge of pulling in our command line arguments, parsing data, and printing data
- logger.h: The header file used for logging
- procfs.c: The primary c file in charge of parsing various files within the `/proc` directory, returning data for our inspector and display files to read and show
- procfs.h: The header file for procfs.c
- util.c: The c file containing utility functions to help with parsing and reading data, used in other files
- util.h: The header file for util.c containing function definitions and structures.

## Program Output

    Hostname: erees | Kernel Version: 5.8.1
    CPU: AMD EPYC Processor (with IBPB), Processing Units: 2
    Uptime: 25 days, 5 hours, 23 minutes, 9 seconds

    Load Average (1/5/15 min): 0.00 0.01 0.05
    CPU Usage:    [--------------------] 0.5%
    Memory Usage: [#######-------------] 36.6% (0.3 / 1.0 GB)

    Tasks: 118 total
    1 running, 0 waiting, 117 sleeping, 0 stopped, 0 zombie

          PID |                 Task Name |        State |            User
    ----------+---------------------------+--------------+-----------------
      144334 |                 inspector |      running |           erees

## Testing

To execute the test cases, use `make test`. To pull in updated test cases, run `make testupdate`. You can also run a specific test case instead of all of them:

```
# Run all test cases:
make test

# Run a specific test case:
make test run=4

# Run a few specific test cases (4, 8, and 12 in this case):
make test run='4 8 12'
```
