# KELL-SHELL

### Kelley Neubauer

Minesweeper for your terminal with ncurses GUI interface!

<img src="/img/minesweeper_gameplay.png" width="600">

[Play on repli.it!](https://repl.it/@kelleyneubauer/minesweeper)

---

**Running Kell-Shell from the command line:**

1. Navigate to folder neubauek_program3 folder using `cd src`
2. To compile, type `make` \
    Program is compiled using GNU99 standard.\
    Executable is named `kell-shell`.
3. To run, type `./kell-shell` 
4. To clean up and remove executable and object files, type `make clean`.`

---

**What it does:**

This program will:
1. Provide a prompt for running commands
2. Handle blank lines and comments, which are lines beginning with the # character
3. Provide expansion for the variable $$
4. Execute 3 commands exit , cd , and via code built into the shell
5. Execute other commands by creating new processes using a function from the exec family of
functions
6. Support input and output redirection
7. Support running commands in foreground and background processes
8. Implement custom handlers for 2 signals, SIGINT and SIGTSTP
