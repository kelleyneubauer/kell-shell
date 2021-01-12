# KELL-SHELL

### Kelley Neubauer

kell-shell is my very own shell program written in C

<img src="/img/kell-shell.gif" width="300">

[Try it on repli.it!](https://repl.it/@kelleyneubauer/kell-shell)

---

**What does kell-shell do?**

kell-shell implements a subset of features found in well-known shells like bash.\
It:

1. Has a prompt `k$: `
2. Can handle comment lines that begin with `#`
3. Expands the variable `$$` to PID
4. Contains 3 built-in commands: `exit`, `cd`, and `status`
5. Can execute non-built-in commands as new processes
6. Works with input `<` and output `>` redirection
7. Supports running background processes with a last argument `&`
8. Uses custom signal handlers for `SIGINT` and `SIGTSTP`

---

**Running kell-shell from the command line:**

1. Navigate to src folder folder using `cd src`
2. To compile, type `make` \
    Program is compiled using GNU99 standard.\
    Executable is named `kell-shell`.
3. To run, type `./kell-shell` 
4. To clean up and remove executable and object files, type `make clean`

---

**Example usage:**

Input `<` and output `>` redirection:
```
k$: ls
kell-shell  main.c  main.o  makefile
k$: ls > junk
k$: cat junk
junk
kell-shell
main.c
main.o
makefile
k$: wc < junk > junk2
k$: wc < junk
 5  5 39
k$: cat junk2
 5  5 39
```

Background process using `&`:
```
k$: sleep 10 &
background pid is 34597
k$: ps
   PID TTY          TIME CMD
  2665 pts/35   00:00:00 bash
 20325 pts/35   00:00:00 kell-shell
 34597 pts/35   00:00:00 sleep
 34612 pts/35   00:00:00 ps
k$: 
k$: #that was a blank line, this is a comment
background pid 34597 is done: exit value 0
```

Built-in commands `cd`, `status`, `exit`, and `$$` expansion:
```
k$: pwd     
/nfs/stak/users/neubauek/kell-shell/src
k$: cd
k$: pwd
/nfs/stak/users/neubauek
k$: wc bad$$     
wc: bad46429: No such file or directory
k$: status
exit value 1
k$: mkdir bad$$
k$: status
exit value 0
k$: cd bad$$
k$: pwd
/nfs/stak/users/neubauek/bad46429
k$: echo $$
46429
k$: exit
os1 ~/kell-shell/src 1016$ 
```

Signal handlers for `SIGINT` and `SIGTSTP`:
```
k$: sleep 5
^Cterminated by signal 2
k$: ^C
k$: ^Z
Entering foreground-only mode (& is now ignored)
k$: date
Thu Jan  7 17:06:22 PST 2021
k$: sleep 5 &
date
k$: Thu Jan  7 17:06:27 PST 2021
k$: ^Z
Exiting foreground-only mode
```
