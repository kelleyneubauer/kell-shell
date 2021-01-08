/*******************************************************************************
* 
* File:     main.c
* Author:   Kelley Neubauer
* Date:     10/23/2020
* 
* Description: kell-shell implements a subset of features found in well-known
*   shells, such as bash. kell-shell:
*       1. Provides a prompt for running commands
*       2. Handles blank lines and comments, which are lines beginning with 
*           the # character
*       3. Provides expansion for the variable $$ to PID
*       4. Executes 3 commands exit, cd, and status by code built into the shell
*       5. Executes other commands by creating new processes using a function 
*           from the exec family of functions
*       6. Supports input and output redirection
*       7. Supports running commands in foreground and background processes
*       8. Implements custom handlers for 2 signals, SIGINT and SIGTSTP 
* 
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/types.h>  // system calls
#include <sys/wait.h>   // waitpid()
#include <unistd.h>     // system calls
#include <fcntl.h>      // files
#include <signal.h>     // signal handlers

#define MAX_CHAR 2048 
#define MAX_ARG 512 

struct process {
    int processId;
    struct process *next;
};

_Bool foregroundOnly = 0;

/**
* 
* void catchSIGTSTP(int signo) 
* 
* Summary: 
*       Custom ignal handler for SIGTSTP
* 
* Parameters:   int for signal number
* 				
* Returns:      nothing.
*
* Description:
*       This signal handler toggles foreground only mode when user enters CTRL+Z
*       and sends SIGTSTP. Foreground only mode will disable background procs.
* 
* ---
*
* Elements of the following code have been adapted from:
*
* - Title: OSU CS344 archived lectures: 3.3 Signals
*   Author: Benjamin Brewster
*   Date: 4/10/2019
*   Availability: https://www.youtube.com/channel/UCqiv0C67MA6NOiusl5NLXIQ
*   Additional Info: first accessed 10/21/2020
*
* - Title: OSU CS344 course materials: 5_3_siguser.c
*   Author: Unknown
*   Date: Unknown
*   Availability: canvas.oregonstate.edu
*   Additional Info: first accessed 9/24/2020
*
**/
void catchSIGTSTP(int signo) 
{
    if (foregroundOnly) {
        char *message = "\nExiting foreground-only mode\n";
        write(STDOUT_FILENO, message, 30);
        fflush(stdout);
        foregroundOnly = 0;
    }
    else {
        char *message = "\nEntering foreground-only mode (& is now ignored)\n";
        write(STDOUT_FILENO, message, 50);
        fflush(stdout);
        foregroundOnly = 1;
    }
}

/**
* 
* void expandInput(char *str, int pid) 
* 
* Summary: 
*       Replaces all instances of $$ with process id
* 
* Parameters:   char* for the string to expand
*               int for the process id
* 				
* Returns:      nothing. string is modified
*
* Description:
*       This function takes advantage of a while loop and strstr to locate 
*       all instancees of $$ in the string. It creates a new string to hold
*       the expanded version and uses strcat to modify the new string.
*       Finally, it copies the modified string back to the original to 
*       be returned.
*
*       Note: It has been assumed that when instances of $$mare replaced by 
*       process ID, the length of the command line will not go above 2048 char.
*       (piazza post @240)
*
*       Note: Process ID must be 10 digits or fewer (I believe Linux limits
*       processid to 2^22 on 64 bit systems but extra room doesn't hurt here).
* 
* ---
*  
* Elements of this code have been adapted from:
*
* - Title: C - how to convert a pointer in an array to an index?
*   Author: AraK
*   Date: 4/26/2010
*   Availability: https://stackoverflow.com/questions/2711653
*   Additional Info: First accessed 10/26/2020
*
**/
void expandInput(char *str, int pid) 
{
    // convert process id from int to string
    char procId[11]; // 1 extra space for \0
    sprintf(procId, "%d", pid);

    // locate $$ substring if exists
    // repeat until there are no substrings containing $$
    char *loc;
    while ((loc = strstr(str, "$$"))) {
        // create copy of input with enough room for pid
        char *expandedStr = calloc(strlen(str) + strlen(procId) + 1, sizeof(char));
        strcpy(expandedStr, str);

        // calculate index of initial $ and index after second $ for srtcat
        size_t startIndex = loc - str;
        size_t endIndex = startIndex + 2;

        // replace the starting $ with \0 so we can cat and overwrite with pid
        expandedStr[startIndex] = '\0';	
        strcat(expandedStr, procId);
        // sprintf has written trailing \0 to procId so we can 
        // cat what's left after second $ in original string
        strcat(expandedStr, str + endIndex);

        // copy the expanded string to the original location and free memory
        strcpy(str, expandedStr);
        free(expandedStr);
    }
}

/**
* 
* int tokenize(char *str, char *args[], char *io[]) 
* 
* Summary: 
*       Tokenizes user input and locates locations of all user arguments.
*       Marks end of list with NULL arg.
* 
* Parameters:   char* for the user input string
*               array of char* to hold pointers to individual arguments
*               array of char* to hold pointers to io files
* 				
* Returns:      an int for the number of arguments found
*
* Description:
*       For simplicity, this function does not allocate new memory for tokens, 
*       rather it simply points to their location in the original string. 
*       If the original string is altered after this function is called, 
*       the argument and io arrays will no longer be accurate which could 
*       lead to invalid memory reads or unexpected behavior. 
* 
* ---
*
* Elements of the following code have been adapted from:
*
* - Title: OSU CS344 course materials including studentsc & sample repl
*   Author: Unknown
*   Date: Unknown
*   Availability: canvas.oregonstate.edu
*   Additional Info: first accessed 9/24/2020
*
* - Kelley Neubauer CS344 Assignment1 & Assignment2
* 
**/
int tokenize(char *str, char *args[], char *io[]) 
{
    int count = 0;
    char *saveptr;
    char *token = strtok_r(str, " ", &saveptr);
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            // redirect input: save name of next token, add neither to arg list
            token = strtok_r(NULL, " ", &saveptr);
            io[0] = token;
        }
        else if (strcmp(token, ">") == 0) {
            // redirect output: save name of next token, add neither to arg list
            token = strtok_r(NULL, " ", &saveptr);
            io[1] = token;
        }
        else {
            // save token to arg list
            args[count] = token;
            count++;
        }
        // get next token
        token = strtok_r(NULL, " ", &saveptr);
    }
    // add NULL terminator so we can find end of list
    args[count] = NULL;

    return count;
}

/**
* 
* void printStatus(int status)
* 
* Summary: 
*       Prints an exit status
* 
* Parameters:   an int for the exit status to print
* 				
* Returns:      nothing. prints exit status
*
* Description:
*       This function prints the exit status of the int passed in. It determines
*       whether termination was normal or by signal and prints the status.
* 
* ---
*
* Elements of the following code have been adapted from:
*
* - Title: OSU CS344 archived lectures: 3.1 Processes
*   Author: Benjamin Brewster
*   Date: 4/10/2019
*   Availability: https://www.youtube.com/channel/UCqiv0C67MA6NOiusl5NLXIQ
*   Additional Info: first accessed 10/21/2020
*
**/
void printStatus(int status) {
    if (WIFEXITED(status)) {
        //terminated normally
        printf("exit value %d\n", WEXITSTATUS(status));
        fflush(stdout);
    }
    else if (WIFSIGNALED(status)) {
        // terminated by signal
        printf("terminated by signal %d\n", WTERMSIG(status));
        fflush(stdout);
    } 
}

/**
* 
* _Bool backgroundCheck(char *args[], int *numArgs)
* 
* Summary: 
*       Checks is the last argument in a command is &
* 
* Parameters:   array of pointer to char for the argument list
*               a pointer to int for the number of args in the list
* 				
* Returns:      a bool. 
*               true if process should run in background
*               false if process should not run in background
*
* Description:
*       Checks if the last argument is & in a list that has more than one 
*       argument. If an & is found in the last position, it will be replaced
*       by NULL and the arg count value will be reduced by one.
*
**/
_Bool backgroundCheck(char *args[], int *numArgs) {
    if ((*numArgs > 1) && (strcmp(args[*numArgs - 1], "&") == 0)) {
        // set list end indicator in place of & and reduce count by 1
        args[*numArgs - 1] = NULL;
        *numArgs = *numArgs - 1;

        return 1;
    }
    return 0;
}

/**
* 
* void addProcess(struct process **list, int procId)
* 
* Summary: 
*       Adds a process to a linked list. New process will be added to front.
* 
* Parameters:   pointer to pointer to process struct
*               int for the process id to add
* 				
* Returns:      nothing. list is modified
*
* ---
*
* Elements of the following code have been adapted from:
*
* - Title: Linked List | Set 3 (Deleting a node)
*   Author: Geeks for Geeks
*   Date: 10/26/2020
*   Availability: https://www.geeksforgeeks.org/linked-list-set-3-deleting-node/
*   Additional Info: first accessed 11/2/2020
*
**/
void addProcess(struct process **list, int procId) {
    struct process *newProcess = malloc(sizeof(struct process));
    newProcess->processId = procId;
    newProcess->next = *list;  
    *list = newProcess;
}

/**
* 
* void removeProcess(struct process **list, int procId)
* 
* Summary: 
*       Removes a specified process id from a linked list, if exists
* 
* Parameters:   pointer to pointer to process struct
*               int for the process id to remove
* 				
* Returns:      nothing.
*
* ---
*
* Elements of the following code have been adapted from:
*
* - Title: Linked List | Set 3 (Deleting a node)
*   Author: Geeks for Geeks
*   Date: 10/26/2020
*   Availability: https://www.geeksforgeeks.org/linked-list-set-3-deleting-node/
*   Additional Info: first accessed 11/2/2020
*
**/
void removeProcess(struct process **list, int procId) {
    struct process *prev = NULL;
    struct process *temp = *list;

    if (temp && temp->processId == procId) {
        // special case: first item is one to be removed (prev does not exist)
        *list = temp->next;
        free(temp);
    }
    else {
        // locate the link if exists
        while (temp && temp->processId != procId) {
            prev = temp;
            temp = temp->next;
        }

        if (temp != NULL) {
            // process id was found, remove the link
            prev->next = temp->next;
            free(temp);
        }
    }
}

/**
* 
* void freeMemory(struct process **list)
* 
* Summary: 
*       Frees all dynamically allocated memory for a linked list of process
* 
* Parameters:   pointer to pointer to process struct
* 				
* Returns:      nothing.
*
**/
void freeMemory(struct process **list) {
    struct process *temp = *list;
    while (temp) {
        struct process *garbage = temp;
        temp = temp->next;
        free(garbage);
    }
}

/**
* 
* int main (int argc, char* argv[])
* 
* Summary: 
*       Program driver. See description at top of file.
* 
* Parameters:   none
* 				
* Returns:      nothing
* 
**/
int main (int argc, char* argv[])
{
    _Bool exitShell = 0;
    int foregroundStatus = 0;
    int backgroundStatus = 0;

    struct process *backgroundProcsList = NULL;

    // register action for parent process to ignore SIGINT
    // source: OSU CS344 course materials: 5_3_siguser.c
    struct sigaction SIGINT_action = {{0}}; 
    SIGINT_action.sa_handler = SIG_IGN;
    sigaction(SIGINT, &SIGINT_action, NULL);

    // register custom handler for SIGTSTP
    // source: OSU CS344 course materials: 5_3_siguser.c
    struct sigaction SIGTSTP_action = {{0}};
    SIGTSTP_action.sa_handler = catchSIGTSTP;
    SIGTSTP_action.sa_flags = SA_RESTART;   // auto restart interrupted things
    sigfillset(&SIGTSTP_action.sa_mask);    // block all catchable while running
    SIGTSTP_action.sa_flags = 0;            // no flags
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);


    // repeat shell prompt until exit command is received
    while (!exitShell) {
        char buffer[MAX_CHAR + 2]; // 2 extra: one for \n and one for \0
        char *userInput;
        char *userArgs[MAX_ARG + 1]; // 1 extra: space for null arg at end
        int numArgs = 0;
        char *ioFiles[2] = { NULL };

        _Bool runInBackground = 0;


        // print prompt and get user input
        printf("k$: ");
        fflush(stdout);
        userInput = fgets(buffer, sizeof(buffer), stdin); // fgets reads \n

        if (!userInput) {
            // if there was an error with fgets, clear it and move on
            clearerr(stdin);
        }
        else if (userInput[0] == '#' || userInput[0] == '\n') {
            // skip comments and lines with no input
        }
        else {
            // remove extraneous newline char read by fgets
            userInput[strlen(userInput) - 1] = '\0';

            // expand $$ to pid
            expandInput(userInput, getpid());

            // tokenize input, locate each argument & io files
            numArgs = tokenize(userInput, userArgs, ioFiles);

            // process the user arguments
            if (strcmp(userArgs[0], "exit") == 0) {
                // kill background processes and exit shell
                struct process *temp = backgroundProcsList;
                while (temp) {
                    kill(temp->processId, 1);
                    temp = temp->next;
                }
                exitShell = 1;
            }
            else if (strcmp(userArgs[0], "status") == 0) {
                // print last foreground status
                printStatus(foregroundStatus);
            }
            else if (strcmp(userArgs[0], "cd") == 0) {
                int result = -1;
                if (!userArgs[1]) {
                    // cd is the only command, go to HOME
                    result = chdir(getenv("HOME"));
                }
                else {
                    result = chdir(userArgs[1]);
                }

                if (result == -1){
                    perror("cd error");
                    fflush(stdout);
                }
            }
            else {
                // check if process should be run in background (last arg is &)
                runInBackground = backgroundCheck(userArgs, &numArgs);

                // background processes are not allowed in foreground only mode
                if (foregroundOnly) { runInBackground = 0; }

                if (runInBackground) {
                    // if input is not redirected, direct to /dev/null
                    if (!ioFiles[0]) {
                        ioFiles[0] = "/dev/null";
                    }
                    // if output is not redirected, direct to /dev/null
                    if (!ioFiles[1]) {
                        ioFiles[1] = "/dev/null";
                    }
                }

                // fork and pass command to exec
                // source: Benjamin Breswter OSU CS344 lectures: 3.1 Processes
                pid_t spawnPid = fork();

                switch (spawnPid) {
                    case -1:
                        perror("fork()\n");
                        exit(1);
                        break;
                    case 0:
                        // open files for input & output redirection
                        // source: OSU CS344 course materials: 5_4_sortViaFiles.c

                        // input redirection. input name is in ioFiles[0]
                        if (ioFiles[0]) {
                            // open file as read only
                            int inputFd = open(ioFiles[0], O_RDONLY);
                            if (inputFd == -1) {
                                printf("cannot open %s for input\n", ioFiles[0]);
                                fflush(stdout);
                                exit(1);
                            }

                            // copy file descriptor to stdin. input is 0
                            int result = dup2(inputFd, 0);
                            if (result == -1) {
                                printf("error redirecting stdin to %s\n", ioFiles[0]);
                                fflush(stdout);
                                exit(2);
                            }
                        }
                        // output redirection. output name is in ioFiles[1]
                        if (ioFiles[1]) {
                            // open file as write only, truncate, create if not exist
                            int outputFd = open(ioFiles[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                            if (outputFd == -1) {
                                printf("cannot open %s for output\n", ioFiles[1]);
                                fflush(stdout);
                                exit(1);
                            }

                            // copy file descriptor to stdout. output is 1
                            int result = dup2(outputFd, 1);
                            if (result == -1) {
                                printf("error redirecting stdout to %s\n", ioFiles[1]);
                                fflush(stdout);
                                exit(2);
                            }
                        }

                        // foreground children do not ignore SIGINT
                        if (!runInBackground) {
                            SIGINT_action.sa_handler = SIG_DFL;
                            sigaction(SIGINT, &SIGINT_action, NULL);
                        }

                        // all children ignore SIGTSTP
                        SIGTSTP_action.sa_handler = SIG_IGN;
                        sigaction(SIGTSTP, &SIGTSTP_action, NULL);

                        // pass the command and args to exec()
                        execvp(userArgs[0], userArgs);

                        // exec will only make it here on error
                        printf("%s: no such file or directory\n", userArgs[0]);
                        fflush(stdout);
                        exit(1);
                        break;
                    default:
                        if (runInBackground) {
                            // add child process to list of background processes
                            addProcess(&backgroundProcsList, spawnPid);

                            printf("background pid is %d\n", spawnPid);
                            fflush(stdout);
                        }
                        else {
                            // foreground, blocking wait, set foreground status
                            spawnPid = waitpid(spawnPid, &foregroundStatus, 0);

                            // if foreground child is terminated by signal, print status immediately
                            if (WIFSIGNALED(foregroundStatus)) {
                                printStatus(foregroundStatus);
                            }
                        }
                        break;
                } 
            } 
        } 

        // reap all zombie children that have finished with nonblocking wait call
        // waitpid(-1,.. ) does not work properly with the grading script 
        // so I've switched to iterating through the backgroundProcsList
        // source: Michael Kerrisk - The Linux Programming Interface pg. 557-558
        struct process *temp = backgroundProcsList;
        while (temp) {
            if (waitpid(temp->processId, &backgroundStatus, WNOHANG) > 0) {
                printf("background pid %d is done: ", temp->processId);
                fflush(stdout);
                printStatus(backgroundStatus);

                // remove process and move to next while preventing invalid reads
                int deletePid = temp->processId;
                temp = temp->next;
                removeProcess(&backgroundProcsList, deletePid);
            }
            else {
                temp = temp->next;
            }
        }
    } 

    // free memory associated with background process linked list
    freeMemory(&backgroundProcsList);

    return 0;
}
