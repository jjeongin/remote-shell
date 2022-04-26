## Phase 1: Local Shell
In phase 1, we created a local version of our shell that supports single command execution, input/output redirection, and composed command execution with upto 3 pipes. The implemented command list is following:

```
ls
cd
cat
find
clear
pwd 
mkdir
rm
echo
move
grep
```

I/O redirection examples

```
echo hello > new-file
ls frhtrhrjy 2 > errout-file
cat < new-file
```

Composed command examples

```
echo hello world | grep hello
ls | grep f | grep n | grep ne
```

To execute the program, download all files and execute the following commands in your shell.
```
$ make
$ ./shell
```
