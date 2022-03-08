/*
 * Ben Stenberg
 *
 * CS441/541: Project 3
 *
 */
#ifndef MYSHELL_H
#define MYSHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* For fork, exec, sleep */
#include <sys/types.h>
#include <unistd.h>
/* For waitpid */
#include <sys/wait.h>

/******************************
 * Defines
 ******************************/
#define TRUE  1
#define FALSE 0

#define MAX_COMMAND_LINE 1024

#define PROMPT ("mysh$ ")


/******************************
 * Structures
 ******************************/
/*
 * A job struct.  Feel free to change as needed.
 */
struct job_t {
    char * full_command;
    int argc;
    char **argv;
    int is_background;
    char * binary;
    char *status;  // Running -> Done 
    int pid;
};
typedef struct job_t job_t;

/*
 * Linked List Node
 */
struct jobnode {
    job_t *job;
    struct jobnode *next;
};
 typedef struct jobnode jobnode;

struct histnode {
    char *command;
    struct histnode *next;
};
 typedef struct histnode histnode;


/******************************
 * Global Variables
 ******************************/
 
/*
 * Interactive or batch mode
 */
int is_batch = FALSE;
int to_exit = FALSE;

/*
 * Counts
 */
int total_jobs_display_ctr = 0;
int total_jobs    = 0;
int total_jobs_bg = 0;
int total_history = 0;
int current_jobs_bg = 0;

/*
 * Debugging mode
 */
int is_debug = TRUE;

/*
 * Linked List of Jobs
 */
histnode *history_head = NULL;
jobnode *jobs_head = NULL;

/******************************
 * Function declarations
 ******************************/
/*
 * Parse command line arguments passed to myshell upon startup.
 *
 * Parameters:
 *  argc : Number of command line arguments
 *  argv : Array of pointers to strings
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int parse_args_main(int argc, char **argv);

/*
 * Main routine for batch mode
 *
 * Parameters:
 *  None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int batch_mode(char **files, int num_files);

/*
 * Main routine for interactive mode
 *
 * Parameters:
 *  None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int interactive_mode(void);

/*
 * Launch a job
 *
 * Parameters:
 *   loc_job : job to execute
 *
 * Returns:
 *   0 on success
 *   Negative value on error 
 */
int launch_job(job_t * loc_job);

/*
 * Built-in 'exit' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_exit(void);

/*
 * Built-in 'jobs' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_jobs(void);

/*
 * Built-in 'history' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_history(void);

/*
 * Built-in 'wait' command
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_wait(void);

/*
 * Built-in 'fg' command
 *
 * Parameters:
 *   None (use default behavior)
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_fg(void);

/*
 * Built-in 'fg' command
 *
 * Parameters:
 *   Job id
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int builtin_fg_num(int job_num);

/*
 * Parses through input and takes appropriate action
 *
 * Parameters:
 *   line: line to be parsed
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int parse_line(char *line);

/*
 * Builds job based on given command
 *
 * Parameters:
 *   command: the command typed in
 *
 * Returns:
 *   Built job struct
 *   Negative value on error
 */
job_t* build_job(char *command);

/*
 * Insert jobnode into list
 *
 * Parameters:
 *   job: job to be added
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int insert_job(job_t *job);

/*
 * Insert histnode into list
 *
 * Parameters:
 *   command: string to be added
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int insert_history(char *command);

/*
 * Free jobs list data structures
 *
 * Parameters:
 *   none
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int free_jobs();

/*
 * Free history list data structures
 *
 * Parameters:
 *   none
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int free_history();

/*
 * Remove jobnode from list
 *
 * Parameters:
 *   job_num: pid of job to remove
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int remove_node(int job_num);

/*
 * Check jobs list for completed processes
 *
 * Parameters:
 *   none
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int check_bg();

/*
 * Check if a string is all spaces
 *
 * Parameters:
 *   line: string to check
 *
 * Returns:
 *   0 on success
 *   Negative value on error
 */
int is_blank(char *line);

#endif /* MYSHELL_H */
