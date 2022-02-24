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
        printf("%s", PROMPT);
        
        /*
         * Read stdin, break out of loop if Ctrl-D
         */
        characters = getline(&buffer, &buffersize, stdin);
        

        /* Strip off the newline */
        

        /*
         * Parse and execute the command
         */
         if(characters == 0) {
            continue;
         }
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
    char *command = strtok(line, ";&");
    while(command != NULL) {
        //printf("%s", command);
        if(strcmp(command, "exit") == 0) {
            builtin_exit();
            goto CONTINUE;
        }
        else if(strcmp(command, "jobs") == 0) {
            builtin_jobs();
            goto CONTINUE;
        }
        else if(strcmp(command, "history") == 0) {
            builtin_history();
            goto CONTINUE;
        }
        else if(strcmp(command, "wait") == 0) {
            builtin_wait();
            goto CONTINUE;
        }
        else if(strcmp(command, "fg") == 0) {
            builtin_fg();
            goto CONTINUE;
        }
        else if(strncmp(command, "fg ", 3) == 0) {
            char * args = strtok(command, " ");
            args = strtok(NULL, " ");
            builtin_fg_num(atoi(args));
            goto CONTINUE;
        }
        else {
            // build job_t
            job_t *job = (job_t *)malloc(sizeof(job_t));
            job->full_command = strdup(command); 
            //job->argc =
            //job->argv = 
            //job->is_background =
            //job->binary = 
            launch_job(job);
        }

        CONTINUE: command = strtok(NULL, ";&");
    }
    return 0;
}

/*
 * You will want one or more helper functions for parsing the commands 
 * and then call the following functions to execute them
 */

int launch_job(job_t * loc_job)
{

    /*
     * Display the job
     */


    /*
     * Launch the job in either the foreground or background
     */

    /*
     * Some accounting
     */
     free(loc_job);

    return 0;
}

int builtin_exit(void)
{

    return 0;
}

int builtin_jobs(void)
{

    return 0;
}

int builtin_history(void)
{

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
