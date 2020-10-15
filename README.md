# Skeleton
Small shell in C++.



Description
For simplicity, you can assume that only up to 100 processes can run simultaneously and the
name of each process can contain up to 50 characters.
The program will work as follows:
● The program waits for commands that will be typed by the user and executes them
(and back forth).
● The program can execute a small number of built-in commands, the commands will
be listed below.
● When the program receives a command that is not one of the built-in commands
(we call it an external command), it tries to run it like a normal shell. The way to run
external commands will be described later.
● If one of the built-in commands was typed but with invalid parameters, the following
error message should be printed:
smash error: > “command-line”
where command-line is the command line as typed by the user (note that the error
message should be wrapped with double quotes)
you’ll see examples later on.
● Whenever an error occurs a proper error message should be printed and the
program should get back to parse and execute next typed commands.
● While the program is waiting for the next command to be entered, it should print the
following text:
smash> 

Built-in commands:
Your smash should support and implement a limited number of shell commands (features).
The commands you need to support will be listed below with a detailed description. Please
note that these commands (the built-in commands) should be executed from the smash
process: pwd, cd, history, jobs, kill, showpid, fg, bg, quit, cp.


External commands:
Besides the built-in commands, the smash should support executing external commands
that are not part of the built-in commands. External command is any command that is not a
built-in command.
The smash should execute the external command and wait until the external command is
executed.

Handling Ctrl+C, Ctrl+Z + Pipes.
