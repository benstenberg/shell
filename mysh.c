/*
 * Ben Stenberg
 *
 * CS441/541: Project 3
 *
 */
#include "mysh.h"

int main(int argc, char * argv[]) {
    int ret, i;
    char **files = NULL;

    /*
     * Parse Command line arguments to check if this is an interactive or batch
     * mode run.
     */
    if( 0 != (ret = parse_args_main(argc, argv)) ) {
        fprintf(stderr, "Error: Invalid command line!\n");
        return -1;
    }

    /*
     * If in batch mode then process all batch files
     */
    if( TRUE == is_batch) {
        if( TRUE == is_debug ) {
            printf("Batch Mode!\n");
        }

        /*
         * Get array of filenames
         */
        files = (char**)malloc((argc-1) * sizeof(char*));
        if(files == NULL) {
            printf("Error in allocating files.\n");
            exit(-1);
        }
        for(i = 0; i < argc-1; i++) {
            files[i] = argv[i+1];
        }

        if( 0 != (ret = batch_mode(files, argc-1)) ) {
            fprintf(stderr, "Error: Batch mode returned a failure!\n");
        }

    }
    /*
     * Otherwise proceed in interactive mode
     */
    else if( FALSE == is_batch ) {
        if( TRUE == is_debug ) {
            printf("Interactive Mode!\n");
        }

        if( 0 != (ret = interactive_mode()) ) {
            fprintf(stderr, "Error: Interactive mode returned a failure!\n");
        }
    }
    /*
     * This should never happen, but otherwise unknown mode
     */
    else {
        fprintf(stderr, "Error: Unknown execution mode!\n");
        return -1;
    }


    /*
     * Display counts
     */
    printf("-------------------------------\n");
    printf("Total number of jobs               = %d\n", total_jobs);
    printf("Total number of jobs in history    = %d\n", total_history);
    printf("Total number of jobs in background = %d\n", total_jobs_bg);

    /*
     * Cleanup
     */
    free_history();
    free_jobs();

    return 0;
}

int parse_args_main(int argc, char **argv)
{

    /*
     * If no command line arguments were passed then this is an interactive
     * mode run.
     */
    if(argc == 1) {
        is_batch = FALSE;
        return 0;
    }

    /*
     * If command line arguments were supplied then this is batch mode.
     */
    if(argc > 1) {
        is_batch = TRUE;
        return 0;
    }

    return -1;
}

int batch_mode(char **files, int num_files)
{
    int i;
    FILE *infile = NULL;
    char *buffer = NULL;
    size_t buffersize = MAX_COMMAND_LINE;
    size_t characters;
    char str[MAX_COMMAND_LINE];

    /*
     * Allocate buffer for reading input
     */
    buffer = (char *)malloc(buffersize * sizeof(char));
    if(buffer == NULL) {
        printf("Unable to allocate input buffer.\n");
        return(-1);
    }

    /*
     * For each file...
     */
    for(i = 0; i < num_files; i++) {
        /*
         * Open the batch file
         * If there was an error then print a message and move on to the next file.
         */
        infile = fopen(files[i], "r");
        if(infile == NULL) {
            printf("Unable to open file: %s\n", files[i]);
            continue;
        }

        /*
         * Otherwise, 
         *   - Read one line at a time.
         *   - strip off new line
         *   - parse and execute
         */
        characters = getline(&buffer, &buffersize, infile);
        while(characters != -1 && to_exit == FALSE) {
            if(is_blank(buffer) == FALSE) {
                strcpy(str, buffer);
                if(str[characters-1] == '\n') {
                    str[characters-1] = '\0';
                }
                buffer = strdup(str);
    
                parse_line(buffer);
            }
            characters = getline(&buffer, &buffersize, infile);
        }
        
        /*
         * Close the file
         */
        fclose(infile);
    }
    /*
     * Wait for jobs to finish before exiting
     */
    builtin_wait();

    /*
     * Cleanups
     */
    free(files);
    free(buffer);
    buffer = NULL;
    infile = NULL;
    files = NULL;
    return 0;
}

int interactive_mode(void)
{
    char *buffer = NULL;
    size_t buffersize = MAX_COMMAND_LINE;
    size_t characters;
    char str[MAX_COMMAND_LINE];

    /*
     * Allocate buffer for reading input
     */
    buffer = (char *)malloc(buffersize * sizeof(char));
    if(buffer == NULL) {
        printf("Unable to allocate input buffer.\n");
        return(-1);
    }

    do {
        /*
         * Print the prompt
         */
        printf("\x1B[36m%s\x1B[0m", PROMPT);
        
        /*
         * Read stdin, break out of loop if Ctrl-D
         */
        characters = getline(&buffer, &buffersize, stdin);
        if(characters == -1) {
            /* Ctrl-D */
            break;
        }
        if(characters == 0 || is_blank(buffer) == TRUE) {
            continue;
        }

        /* Strip off the newline */
        strcpy(str, buffer);
        str[characters-1] = '\0';
        buffer = strdup(str);

        /*
         * Parse and execute the command
         */
         parse_line(buffer);

    } while(to_exit == FALSE);

    /*
     * Cleanup
     */
    free(buffer);
    buffer = NULL;
    return 0;
}

int parse_line(char *line) {
    char block[strlen(line)];
    char tmp[strlen(line)];
    char *save = NULL;
    
    /*
     * Loop through all commands in line, separated by ;
     */
    char *command = strtok_r(line, ";", &save);

    while(command != NULL) {
        /*
         * Within that chunk, look for &
         */
        strcpy(block, command);
        strcpy(tmp, "");

        for(int i = 0; i < strlen(command); i++) {
            if(block[i] == '&') {
                do_command(trimwhitespace(tmp), TRUE);
                strcpy(tmp, "");
            }
            else {
                /*
                 * Keep track of command up until & is found
                 */
                strncat(tmp, &block[i], 1);
            }
        }

        /*
         * If there's still something in tmp, no & was found
         */
        if(strcmp(tmp, "") != 0) {
            do_command(trimwhitespace(tmp), FALSE);
            strcpy(tmp, "");
        }
        
        /*
         *  Go to next block
         */
        command = strtok_r(NULL, ";", &save);
    }
    return 0;
}

int do_command(char *command, int is_bg) {
    char *args = NULL;

    if(command == NULL) {
        return -1;
    }

    /* Update bg list */
    check_bg();

    if(strcmp(command, "exit") == 0) {
        builtin_exit();
        insert_history("exit");
    }
    else if(strcmp(command, "jobs") == 0) {
        builtin_jobs();
        insert_history("jobs");
    }
    else if(strcmp(command, "history") == 0) {
        insert_history("history");
        builtin_history();
    }
    else if(strcmp(command, "wait") == 0) {
        builtin_wait();
        insert_history("wait");
    }
    else if(strcmp(command, "fg") == 0) {
        builtin_fg();
        insert_history("fg");
    }
    else if(strncmp(command, "fg ", 3) == 0) {
        args = strtok(strdup(command), " ");
        args = strtok(NULL, " ");
        builtin_fg_num(atoi(args));
        insert_history(strdup(command));
    }
    else {
        /*
         * Build a job
         */
        job_t *job = build_job(command, is_bg);
        if(job != NULL) {
            insert_history(strdup(job->full_command));
            launch_job(job);
        }
    } 
    return 0;
}

job_t* build_job(char *command, int is_bg) {
    /*
     * Sanity check
     */
    if(command == NULL || is_blank(command) == 1 || strcmp(command, "") == 0) {
        return NULL;
    }

    job_t *job = (job_t *)malloc(sizeof(job_t));
    if(job == NULL) {
        printf("Error in allocating job.\n");
        return NULL;
    }
    char* copy = strdup(command);
    
    /*
     * Initialize job struct
     */
    job->full_command = strdup(command); 
    job->argc = 0;
    job->argv = NULL;
    job->is_background = is_bg;
    job->binary = NULL;
    job->status = NULL;
    job->pid = 0;

    /*
     * Populate job with data
     */
    char *cur = strtok(copy, " ");
    job->binary = strdup(cur);
    while(cur != NULL) {
        job->argc = job->argc+1;
        cur = strtok(NULL, " ");
    }

    /*
     * Reset string
     */
    strcpy(copy, command);

    /*
     * Create argv
     */
    char **argv = (char**)malloc(sizeof(char*)*job->argc+1);
    if(argv == NULL) {
        printf("Error in allocating argv.\n");
        return NULL;
    }

    cur = strtok(copy, " ");
    for(int i = 0; i < job->argc; i++) {
        argv[i] = strdup(trimwhitespace(cur));
        cur = strtok(NULL, " ");
    }
    argv[job->argc] = NULL;
    job->argv = argv;
 
    copy = NULL;
    return job;
}


int launch_job(job_t * loc_job)
{
    pid_t c_pid = 0;

    /* fork child process */
    c_pid = fork();
    
    /* error */
     if(c_pid < 0) {
        fprintf(stderr, "Error: Fork failed!\n");
        return -1;
     }
     /* child */
     else if(c_pid == 0) {
        execvp(loc_job->binary, loc_job->argv);
        fprintf(stderr, "--mysh: Invalid command!\n");
        exit(-1);
     }
     /* parent */
     else {
        if(loc_job->is_background == TRUE) {
            waitpid(c_pid, NULL, WNOHANG);
            loc_job->pid = c_pid;
            insert_job(loc_job);
            current_jobs_bg++;
        }
        else {
            waitpid(c_pid, NULL, 0);
        }
        total_jobs++;
     }

    return 0;
}

int builtin_exit(void)
{
    /*
     * Wait for bg processes to finish, then exit
     */
    check_bg();
    if(current_jobs_bg > 0) {
        printf("Waiting on %d jobs before exiting.\n", current_jobs_bg);
        builtin_wait();
    }
    to_exit = TRUE;
    return 0;
}

int builtin_jobs(void)
{
    jobnode *cur = jobs_head;
    jobnode *next = NULL;
    int i = 1;

    /*
     * No background jobs in list
     */
    if(cur == NULL) {
        return 0;
    }

    while(cur != NULL) {
        printf("[%d]  %s", i, cur->job->status);
        for(int i = 0; i < cur->job->argc; i++) {
            printf(" %s", cur->job->argv[i]);
        }
        printf("\n");
        
        /*
         * Remove jobs once viewed
         */
        next = cur->next;
        if(strcmp(cur->job->status, "Done") == 0) {
            if(-1 == remove_node(cur->job->pid)) {
                printf("Couldn't remove job node!\n");
            }
        }
        i++;
        cur = next;
    }
    return 0;
}

int builtin_history(void)
{   
    /*
     * Traverse list of commands
     */
    histnode *cur = history_head;
    int i = 1;
    while(cur != NULL) {
        printf("%d  %s\n", i, cur->command);
        i++;
        cur = cur->next;
    }

    return 0;
}

int builtin_wait(void)
{
    jobnode *cur = jobs_head;
    jobnode *next = NULL;

    /*
     * No jobs in bg
     */
    if(cur == NULL) {
        return 0;
    }

    while(cur != NULL) {
        /*
         *  Wait and then remove
         */
        waitpid(cur->job->pid, NULL, 0);
        next = cur->next;
        //cur->job->status = "Done";
        if(-1 == remove_node(cur->job->pid)) {
            printf("Couldn't remove job node!\n");
        }
        cur = next;
    }

    return 0;
}

int builtin_fg(void)
{
    jobnode *cur = jobs_head;
    jobnode *to_fg = NULL;

    /* 
     * No jobs to fg
     */
    if(cur == NULL) {
        fprintf(stderr, "--mysh: no such job!\n");
        return -1;
    }

    /*
     * Traverse list to get most recent backgrounded running job
     */
    while(cur != NULL) {
        if(strcmp(cur->job->status, "Running") == 0) {
            to_fg = cur;
        }
        cur = cur->next;
    }

    if(to_fg == NULL) {
        fprintf(stderr, "--mysh: no such job!\n");
        return -1;
    }


    /*
     * Get pid of job to fg
     */
    builtin_fg_num(to_fg->job->pid);
    return 0;
}

int builtin_fg_num(int job_num)
{
    /*
     * waitpid returns pid of child that exited
     */
    if(waitpid(job_num, NULL, 0) > 0) {
        /*
         *  Remove job from bg list
         */
        if(-1 == remove_node(job_num)) {
            printf("Couldn't remove job node!\n");
        }
        current_jobs_bg--;
        return 0;
    }

    fprintf(stderr, "--mysh: no such job!\n");
    return -1;
}

int insert_job(job_t *job) {
    /* 
     * Create node
     */
    jobnode *new = (jobnode*)malloc(sizeof(jobnode));
    if(new == NULL) {
        printf("Error in allocating jobnode.\n");
        return -1;
    }
    new->job = job;
    new->next = NULL;
    new->job->status = "Running";

    /*
     * Empty list
     */
    if(jobs_head == NULL) {
        jobs_head = new;
    }
    else {
        /*
         * Go to end of list
         */
        jobnode *cur = jobs_head;
        while(cur->next != NULL) {
            cur = cur->next;
        }

        /*
         * Update
         */
        cur->next = new;
    }

    total_jobs_bg++;
    return 0;
}

int insert_history(char *com) {
    /*
     * Create node
     */
    histnode *new = (histnode*)malloc(sizeof(histnode));
    if(new == NULL) {
        printf("Error in allocating histnode.\n");
        return -1;
    }
    new->command = com;
    new->next = NULL;

    /*
     * Empty list
     */
    if(history_head == NULL) {
        history_head = new;
    }
    else {
        /*
         * Go to end of list
         */
        histnode *cur = history_head;
        while(cur->next != NULL) {
            cur = cur->next;
        }

        /*
         * Update
         */
        cur->next = new;
    }

    total_history++;
    return 0;
}

int free_jobs() {
    /*
     * Nonempty list
     */
    if(jobs_head != NULL) {
        jobnode *cur = jobs_head;
        jobnode *next = NULL;

        while(cur != NULL) {
            /*
             * Free and move on
             */
            next = cur->next;
            free(cur->job->argv);
            free(cur->job);
            free(cur);
            cur = next;
        }
  
        jobs_head = NULL;
        next = NULL;
    }
    return 0;
}


int free_history() {
    /*
     * Nonempty list
     */
    if(history_head != NULL) {
        histnode *cur = history_head;
        histnode *next = NULL;

        while(cur != NULL) {
            /*
             * Free and move on
             */
            next = cur->next;
            free(cur);
            cur = next;
        }
  
        history_head = NULL;
        next = NULL;
    }
    return 0;
}

int remove_node(int job_num) {
    jobnode *cur = jobs_head;
    jobnode *prev = NULL;
    
    /*
     * Nothing to remove
     */
    if(cur == NULL) {
        return -1;
    }

    /*
     * Target is head
     */
    if(cur->job->pid == job_num) {
        jobs_head = cur->next;
        free(cur->job->argv);
        free(cur->job);
        free(cur);
        return 0;
    }

    /*
     * Traverse to find target
     */
    while(cur != NULL && cur->job->pid != job_num) {
        prev = cur;
        cur = cur->next;
    }

    /*
     * Target not found
     */
    if(cur == NULL) {
        return -1;
    }   
        
    /*
     * Update
     */
    prev->next = cur->next;
    free(cur->job->argv);
    free(cur->job);
    free(cur);
    return 0;
}

int check_bg() {
    jobnode *cur = jobs_head;
    int status;

    /*
     * No jobs in background list
     */
    if(cur == NULL) {
        return 0;
    }

    /*
     * Check all bg processes
     * status will be the pid of the process if process is done
     */
    while(cur != NULL) {
        status = waitpid(cur->job->pid, NULL, WNOHANG);
        if(status > 0) {
            cur->job->status = "Done";
            current_jobs_bg--;   
        }
        cur = cur->next;
    }
    return 0;
}

int is_blank(char *line) {
    /*
     * Check if there are any non-space characters
     */
    for(int i = 0; i < strlen(line); i++) {
        if(isspace(line[i]) == 0) {
            return FALSE;
        }
    }
    return TRUE;
}



char* trimwhitespace(char *str)
{
  char *cur;

  /*
   * If everything is whitespace, just return null
   */
  if(is_blank(str)) {
    return NULL;
  }

  /*
   * Leading whitespace: progress string until non-space character
   */
  while(isspace(*str)) {
    str++;
  }

  /*
   * Trailing whitespace: start at end and go until non-space character
   */
  cur = str + strlen(str) - 1;
  while(isspace(*cur)) {
    cur--;
  }

  /*
   * Null terminator
   */
  cur[1] = '\0';

  return str;
}