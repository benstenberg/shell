# CS441/541 Shell Project

## Author(s):

Ben Stenberg


## Date:

Last Modified: 3/10/2022


## Description:

Basic shell program written in C featuring both interactive and batch modes.


## How to build the software

Type "make" to compile the software.


## How to use the software

Run the software from the command line as usual. To run the shell in interactive mode, provide no other arguments.
To run the shell in batch mode, provide one or more batch files as arguments. These batch files should have a command for each line.
When running multiple batch files at the same time, the program will execute all of the commands in all files in one run, without providing
any separation between file runs.


## How the software was tested

Within the tests directory, there are 5 txt files testing various elements of the shell. Listed are their names and purposes:
- redir.txt -- Tests file redirection. Echos into a file, and uses that file as input for print.c.
- fg-test.txt -- Tests the foreground functionality.
- jobs-test.txt -- Tests the jobs functionality.
- parsing-test.txt -- Provides messy commands to test the shell's parsing ability.
- wait-test.txt -- Tests the wait functionality.

print.c is a simple program created to test file redirection. Its executable is compiled in the name "print".
These files can be run in batch mode by passing them in as arguments when running mysh. Or, you can run the commands in interactive
mode by either typing them in manually, or by directing them into mysh like "./mysh < file.txt".


## Known bugs and problem areas

File redirection is not yet implemented.

