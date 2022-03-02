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
    free_history();

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
         parse_line(buffer);

    
       
    } while( 1/* end condition */);

    /*
     * Cleanup
     */
    free(buffer);
    buffer = NULL;

    return 0;
}

int parse_line(char *line) {
    /*
     * Loop through all commands in line, separated by ; or &
     */
    char *command = strtok(line, ";");
    while(command != NULL) {
        if(strcmp(command, "exit") == 0) {
            builtin_exit();
            insert_node("exit");
            goto CONTINUE;
        }
        else if(strcmp(command, "jobs") == 0) {
            builtin_jobs();
            insert_node("jobs");
            goto CONTINUE;
        }
        else if(strcmp(command, "history") == 0) {
            insert_node("history");
            builtin_history();
            goto CONTINUE;
        }
        else if(strcmp(command, "wait") == 0) {
            builtin_wait();
            insert_node("wait");
            goto CONTINUE;
        }
        else if(strcmp(command, "fg") == 0) {
            builtin_fg();
            insert_node("fg");
            goto CONTINUE;
        }
        else if(strncmp(command, "fg ", 3) == 0) {
            // NEEDS REVISION
            char * args = strtok(command, " ");
            args = strtok(NULL, " ");
            builtin_fg_num(atoi(args));
            // insert_node(?)
            goto CONTINUE;
        }
        else {
            // build job_t
            job_t *job = build_job(command);
            insert_node(strdup(job->full_command));
            launch_job(job);
        }

        CONTINUE: command = strtok(NULL, ";");
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
    char **argv = (char**)malloc(sizeof(char*)*job->argc);
    cur = strtok(copy, " ");
    for(int i = 0; i < job->argc; i++) {
        argv[i] = strdup(cur);
        cur = strtok(NULL, " ");
    }
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
        }
        else {
            waitpid(c_pid, NULL, 0);
        }
     }

    /*
     * Some accounting
     */
     free(loc_job);

    return 0;
}

int builtin_exit(void)
{
    builtin_wait();
    exit(0);
    return 0;
}

int builtin_jobs(void)
{

    return 0;
}

int builtin_history(void)
{
    // print linked list of commands
    node *cur = head;
    int i = 1;
    while(cur != NULL) {
        printf("%d  %s\n", i, cur->job);
        i++;
        cur = cur->next;
    }

    return 0;
}

int builtin_wait(void)
{

    return 0;
}

int builtin_fg(void)
{

    return 0;
}

int builtin_fg_num(int job_num)
{

    return 0;
}

int insert_node(char *command) {
    // make node
    node *new = (node*)malloc(sizeof(node));
    new->job = strdup(command);
    new->next = NULL;

    // initialize
    if(head == NULL) {
        head = new;
    }
    else {
        // get to end of linked list
        node *cur = head;
        while(cur->next != NULL) {
            cur = cur->next;
        }

        // set pointer
        cur->next = new;
    }

    return 0;
}

int free_history() {
    // go down list and free nodes
    node *cur = head;
    node *next = NULL;

    while(cur->next != NULL) {
        next = cur->next;
        free(cur);
        cur = next;
    }

    head = NULL;
    next = NULL;

    return 0;
}
