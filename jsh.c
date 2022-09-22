/*
Title: jsh.c -- main.c
Dev:   Gabriel Stowe
Date:  4/18/22
Description: This program illistrates how the command shell works. When running the program, the program takes in
commands as if they are commands from the actual command line. It will parses the input and forks and pipes the procs
in a proper manner to insure it passes the gradescripts.
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*Include Planks libs to make life easier*/
#include "fields.h"
#include "jrb.h"


int main(int argc, char** argv,char **envp)
{
    /*This is to check if argv[1] should be prompted to the console or not*/
    char *prompt = malloc(sizeof(char*)*5);
    strcat(prompt, "jsh3:");
    if(strcmp(argv[1],"-") == 0)
    {
        prompt = "";
    }
    else 
    {
        prompt = malloc(sizeof(char*)*(strlen(argv[1])));
        prompt = strdup(argv[1]);
    }
    
    fprintf(stdout, "%s", prompt);

    /*struct provided by Dr. Plank for input*/
    IS input_stream = new_inputstruct(NULL);
    JRB jrb_dummy_ids, jrb_tmp_dummy_ids;
    int pipe_fd[2],dummy;


    while(get_line(input_stream) > 0 && strcmp(input_stream->fields[0],"exit") != 0)
    {
        /*This is used to store if the current fork is a child*/
        int child; 
        /*This is set to !child*/
        int parent;
        //Stores the process id for the files
        int input_file = -1;
        int output_file = -1;
        /*State weather the program should wait for child process to die*/
        int wait_for_children = 1;
        /*Stores are */
        char ** arg = malloc(sizeof(char *) * input_stream->NF+1);
        int arg_index = 0;

        jrb_dummy_ids = make_jrb();
    
        for(int i = 0; i < input_stream->NF; i++)
        {
            if(strcmp(input_stream->fields[i],">")==0)
            {
                output_file = open(input_stream->fields[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if(output_file < 0)
                {
                    perror(input_stream->fields[i+1]);
                    exit(1);
                }
                i++;
            }
            else if(strcmp(input_stream->fields[i],">>") == 0)
            {
                output_file = open(input_stream->fields[i+1], O_WRONLY | O_APPEND | O_CREAT, 0666);
                if(output_file < 0)
                {
                    perror(input_stream->fields[i+1]);
                    exit(1);
                }
                i++;
            }
            else if(strcmp(input_stream->fields[i],"<") == 0)
            {
                input_file = open(input_stream->fields[i+1], O_RDONLY);
                if(input_file < 0)
                {
                    perror(input_stream->fields[i+1]);
                    exit(1);
                }
                i++;
            }
            else if(strcmp(input_stream->fields[i],"|") == 0)
            {
                arg[arg_index] = NULL;
                arg_index = 0;

                //when there is a pipe, need to use pipe in order to communicate between procs
                if(pipe(pipe_fd) < 0) 
                {
                    perror("pipe(pipefd)");
                    exit(1);
                }
                /*Flush everything brfore fork do to buffer*/
                fflush(stderr);
                fflush(stdout);
                fflush(stdin);

                child = fork();
                jrb_insert_int(jrb_dummy_ids,child,JNULL);
                parent != child;

                if(child == 0)
                {
                    if(input_file > 0)
                    {
                        if(dup2(input_file,0)!=0)
                        {
                            perror("ERROR");
                            exit(1);
                        }
                        close(input_file);
                    }
                    if(dup2(pipe_fd[1],1) != 1)
                    {
                        perror("ERROR");
                        exit(1);
                    }
                    close(pipe_fd[1]);
                    close(pipe_fd[0]);
                    execvp(arg[0],arg);
                    perror(arg[0]);
                    exit(1);
                }
                else if(parent)
                {
                    if(input_file > 0) close(input_file);
                    input_file = pipe_fd[0];
                    close(pipe_fd[1]);
                }
            }
            else if(strcmp(input_stream->fields[i],"&") == 0 && i == input_stream->NF - 1)
            {
                wait_for_children = 0;   
                i++;
            }
            else
            {
                arg[arg_index] = input_stream->fields[i];
                arg_index++; 
            }
        }
        
        arg[arg_index] = NULL;

        //Flush everything to clear out buffer in between procs.
        fflush(stderr);
        fflush(stdout);
        fflush(stdin);

        child = fork();
        jrb_insert_int(jrb_dummy_ids,child,JNULL);
        parent != child;

        if(child == 0)
        {
            if(input_file > 0)
            {
                if(dup2(input_file,0)!= 0)
                {
                    perror("ERROR");
                    exit(1);
                }
                close(input_file);
            }
            if(output_file > 0)
            {
                if(dup2(output_file,1)!=1)
                {
                    perror("ERROR");
                    exit(1);
                }
                close(output_file);
            }
            
            execvp(arg[0],arg);
            perror(arg[0]);
            exit(1);
        }
        else if(parent)
        {
            if(wait_for_children)
            {
                jrb_traverse(jrb_tmp_dummy_ids,jrb_dummy_ids)
                {
                    while (1)
                    {
                        int Wait = wait(&dummy);
                        if(Wait == jrb_tmp_dummy_ids->key.i || Wait < 0) break;
                    }
                    
                }
            }
            fprintf(stdout, "%s", prompt);
            close(input_file);
            close(output_file);
        }
    }  
}




