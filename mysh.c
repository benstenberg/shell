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

        // get array of files
        files = (char**)malloc((argc-1) * sizeof(char*));
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
    free_list(&history_head);
    free_list(&jobs_head);

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
    }

    /*
     * If command line arguments were supplied then this is batch mode.
     */
    if(argc > 1) {
        is_batch = TRUE;
    }

    return 0;
}

int batch_mode(char **files, int num_files)
{
    int i;
    FILE *infile = NULL;
    char *buffer = NULL;
    size_t buffersize = MAX_COMMAND_LINE;
    size_t characters;

    /*
     * For each file...
     */
    for(i = 0; i < num_files; i++) {
            /*
         * Open the batch file
         * If there was an error then print a message and move on to the next file.
         * Otherwise, 
         *   - Read one line at a time.
         *   - strip off new line
         *   - parse and execute
         * Close the file
         */
        printf("%s\n", files[i]);
        infile = fopen(files[i], "r");
        if(infile == NULL) {
            printf("Unable to open file\n");
            continue;
        }

        /*
         * Allocate buffer for reading input
        */
        buffer = (char *)malloc(buffersize * sizeof(char));
        if(buffer == NULL) {
            printf("Unable to allocate input buffer.\n");
            return(-1);
        }

        do {
            characters = getline(&buffer, &buffersize, infile);
            parse_line(buffer);
        }      
        while(characters != 0);
        

        fclose(infile);
    }
    

    /*
     * Cleanups
     */


    free(files);
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
        if(characters == 0) {
            continue;
         }

        /* Strip off the newline */
        strcpy(str, buffer);
        str[characters-1] = '\0';
        buffer = strdup(str);

        /*
         * Parse and execute the command
         */
         check_bg();
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
    /*
     * Loop through all commands in line, separated by ;
     */
    
    char *save = NULL;
    char *command = strtok_r(line, ";", &save);

    while(command != NULL) {
        //printf("%s\n", command);
        if(strcmp(command, "exit") == 0) {
            builtin_exit();
            insert_node(build_job("exit"), &history_head);
        }
        else if(strcmp(command, "jobs") == 0) {
            builtin_jobs();
            insert_node(build_job("jobs"), &history_head);
        }
        else if(strcmp(command, "history") == 0) {
            insert_node(build_job("history"), &history_head);
            builtin_history();
        }
        else if(strcmp(command, "wait") == 0) {
            builtin_wait();
            insert_node(build_job("wait"), &history_head);
        }
        else if(strcmp(command, "fg") == 0) {
            builtin_fg();
            insert_node(build_job("fg"), &history_head);
        }
        else if(strncmp(command, "fg ", 3) == 0) {
            // NEEDS REVISION
            char * args = strtok(command, " ");
            args = strtok(NULL, " ");
            builtin_fg_num(atoi(args));
            // insert_node(?)
        }
        else {
            // build job_t
            job_t *job = build_job(command);
            insert_node(job, &history_head);
            launch_job(job);
        } 
        command = strtok_r(NULL, ";", &save);
    }
    return 0;
}


job_t* build_job(char *command) {
    job_t *job = (job_t *)malloc(sizeof(job_t));
    
    // copy to use in strtok
    char* copy = strdup(command);
    
    // initialize
    job->full_command = strdup(command); 
    job->argc = 0;
    job->argv = NULL;
    job->is_background = FALSE;
    job->binary = NULL;
    job->status = NULL;
    job->pid = 0;

    // fill in job info on first pass
    char *cur = strtok(copy, " ");
    job->binary = strdup(cur);
    while(cur != NULL) {
        if(strcmp(cur, "&") == 0) {
            job->is_background = TRUE;
            cur = strtok(NULL, " ");
            continue;
        }
        job->argc = job->argc+1;
        cur = strtok(NULL, " ");
    }

    // reset copy
    strcpy(copy, command);

    // create argv on second pass
    char **argv = (char**)malloc(sizeof(char*)*job->argc+1);
    cur = strtok(copy, " ");
    for(int i = 0; i < job->argc; i++) {
        argv[i] = strdup(cur);
        cur = strtok(NULL, " ");
    }
    argv[job->argc] = NULL;
    job->argv = argv;
 
    copy = NULL;
    return job;
}

/*
 * You will want one or more helper functions for parsing the commands 
 * and then call the following functions to execute them
 */

int launch_job(job_t * loc_job)
{
    pid_t c_pid = 0;

    /*
     * Display the job
     */
    /*printf("Full command: %s\n", loc_job->full_command);
    printf("argc: %d\n", loc_job->argc);
    printf("binary: %s\n", loc_job->binary);
    for(int i = 0; i < loc_job->argc; i++) {
        printf("argv[%d]: %s\n", i, loc_job->argv[i]);
    }
    printf("isBackground: %d\n", loc_job->is_background);
    */

    /*
     * Launch the job in either the foreground or background
     */
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
            insert_node(loc_job, &jobs_head);
            current_jobs_bg++;
        }
        else {
            waitpid(c_pid, NULL, 0);
        }
     }
    /*
     * Some accounting
     */
    

    return 0;
}

int builtin_exit(void)
{
    // wait for all jobs to finish then exit
    if(current_jobs_bg > 0) {
        printf("Waiting on %d jobs before exiting.\n", current_jobs_bg);
        builtin_wait();
    }
    to_exit = TRUE;
    return 0;
}

int builtin_jobs(void)
{
    // print background jobs in list
    node *cur = jobs_head;
    node *next = NULL;
    int i = 1;

    if(cur == NULL) {
        return 0;
    }
    while(cur != NULL) {
        // print
        printf("[%d]  %s  %s\n", i, cur->job->status, cur->job->full_command);
        // remove finished jobs once viewed
        next = cur->next;
        if(strcmp(cur->job->status, "Done") == 0) {
            remove_node(cur->job->pid);
            current_jobs_bg--;
        }
        i++;
        cur = next;
    }
    return 0;
}

int builtin_history(void)
{
    // print linked list of commands
    node *cur = history_head;
    int i = 1;
    while(cur != NULL) {
        printf("%d  %s\n", i, cur->job->full_command);
        i++;
        cur = cur->next;
    }

    return 0;
}

int builtin_wait(void)
{
    node *cur = jobs_head;

    if(cur == NULL) {
        return 0;
    }

    while(cur != NULL) {
        // wait
        waitpid(cur->job->pid, NULL, 0);
        // mark as done
        cur->job->status = "Done";
        cur = cur->next;
    }

    return 0;
}

int builtin_fg(void)
{
    // wait on background job
    // go to end of jobs list for most recent process
    node *cur = jobs_head;
    if(cur == NULL) {
        fprintf(stderr, "--mysh: no such job!\n");
        return -1;
    }

    while(cur->next != NULL) {
        cur = cur->next;
    }

    // pass pid
    builtin_fg_num(cur->job->pid);
    return 0;
}

int builtin_fg_num(int job_num)
{
    // wait on pid
    if(waitpid(job_num, NULL, 0) > 0) {
        // remove node from list
        remove_node(job_num);
        current_jobs_bg--;
        return 0;
    }
    fprintf(stderr, "--mysh: no such job!\n");
    return -1;
}

int insert_node(job_t *job, node **head) {
    // make node
    node *new = (node*)malloc(sizeof(node));
    new->job = job;
    new->next = NULL;
    new->job->status = "Running";

    // initialize
    if(*head == NULL) {
        *head = new;
        //printf("First node\n");
    }
    else {
        // get to end of linked list
        node *cur = *head;
        while(cur->next != NULL) {
            cur = cur->next;
        }

        // set pointer
        cur->next = new;
        //printf("Next node\n");
    }

    if(*head == history_head) {
        total_history++;
    }
    else if(*head == jobs_head) {
        total_jobs_bg++;
    }

    return 0;
}


int free_list(node **head) {
    // go down list and free nodes
    if(head != NULL) {
        node *cur = *head;
        node *next = NULL;

        while(cur != NULL) {
            next = cur->next;
            free(cur->job);
            free(cur);
            cur = next;
        }
  
        *head = NULL;
        next = NULL;
    }
    return 0;
}

int remove_node(int job_num) {
    // remove node
    node *cur = jobs_head;
    node *prev = NULL;
    
    if(cur == NULL) {
        return -1;
    }

    // target is head
    if(cur->job->pid == job_num) {
        jobs_head = cur->next;
        free(cur->job);
        free(cur);
        return 0;
    }

    // find node
    while(cur != NULL && cur->job->pid != job_num) {
        prev = cur;
        cur = cur->next;
    }

    // not found
    if(cur == NULL) {
        return -1;
    }   
        
    // set pointer, free
    prev->next = cur->next;
    free(cur->job);
    free(cur);
    return 0;
}

int check_bg() {
    // check if any background jobs have completed
    node *cur = jobs_head;
    int status;

    if(cur == NULL) {
        return 0;
    }

    while(cur != NULL) {
        status = waitpid(cur->job->pid, NULL, WNOHANG);
        if(status != 0) {
            cur->job->status = "Done";
        }
        cur = cur->next;
    }
    return 0;
}