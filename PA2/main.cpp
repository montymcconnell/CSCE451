/*
 * Copyright (c) 2022, Justin Bradley
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Author: Monty McConnell
 * Using source code provided by Yutaka Tsutano and Prof. Justin Bradley
 *
 * Note: Sorry if this code is overcommented/messy
 */

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "command.hpp"
#include "parser.hpp"

#define MAX_ALLOWED_LINES 25

int exec(const std::string& cmd, const std::vector<std::string>& args);


void runcmd(std::vector<shell_command> shell_commands)
{
    
    next_command_mode previousMode = next_command_mode::always;
    int pipe1 = 0;
    int pipe2 = 0;
    int status = 0;
    int child_status = 0;

    for(shell_command command: shell_commands) 
    {
        // If it should not continue, break loop 
        if(child_status != 0 && previousMode == next_command_mode::on_success)
        {
            break;
        }
        if(child_status == 0 && previousMode == next_command_mode::on_fail)
        {
            break;
        }

        int pipes[2];
        if(pipe(pipes) == -1)
        {
            fprintf(stderr, "Fail");
            exit(1);
        }

        int pid;
        pid = fork();

        if(pid == 0) // Child
        {            
            // Check for input/output and set file descriptors
            // for redirection.
            if(command.cout_mode == ostream_mode::append) 
            {
                // Convert output file name to a c-string and open file for file descriptor.
                int fd = open(command.cout_file.c_str(), O_APPEND | O_WRONLY);
                dup2(fd, 1);
                close(fd);
            }

            if(command.cout_mode == ostream_mode::file) 
            {
                // Convert output file name to a c-string and open file for file descriptor
                // and set permissions accordingly.
                int fd = open(command.cout_file.c_str(), O_CREAT | O_RDWR, 0666);
                dup2(fd, 1);
                close(fd);
            }

            if(command.cin_mode == istream_mode::file) 
            {
                // Convert input file name to a c-string and open file for file descriptor.
                int fd = open(command.cin_file.c_str(), O_RDONLY );
                dup2(fd, 0);
                close(fd);
            }

            // Pipes //
            // Input
            if(command.cin_mode == istream_mode::pipe)
            {
                dup2(pipe1,0);
                close(pipes[0]);
            }

            // Output
            if(command.cout_mode == ostream_mode::pipe)
            {
                dup2(pipes[1],1);
                close(pipes[1]);
            }
            
            exec(command.cmd, command.args);

            exit(1);
        }
        
        else if(pid > 0) // Parent
        {
            wait(&status);
            pipe1 = pipes[0];
            pipe2 = pipes[1];


            // Logical AND
            if(command.next_mode == next_command_mode::on_success) 
            {   
                previousMode = command.next_mode;
                child_status = status;
            }

            // Logical OR
            if(command.next_mode == next_command_mode::on_fail) 
            {
                previousMode = command.next_mode;
                child_status = status;
            }


            if(command.cout_mode != ostream_mode::term)
            {
                // Duplicate output end to stdout
                dup2(1,pipe2);
            }

        }

        else // Yikers
        {
            fprintf(stderr, "Yikes, fork failed\n");
            exit(1);
        }
    }
}

int main(int argc, char *argv[])
{
    std::string input_line;
    std::string t = "-t";

    // Check for '-t' and don't print 'osh' prompt.
    if(argc > 1 && argv[1] == t) 
    {
        // **Everything below was either given as starter code or adapted from it**//
        for (int i=0;i<MAX_ALLOWED_LINES;i++) // Limits the shell to MAX_ALLOWED_LINES
        { 
            // Read a single line.
            if (!std::getline(std::cin, input_line) || input_line == "exit") 
            {
                break;
            }

            try 
            {
                // Parse the input line.
                std::vector<shell_command> shell_commands
                    = parse_command_string(input_line);

                runcmd(shell_commands);
            }
            catch (const std::runtime_error& e) 
            {
                std::cout << e.what() << "\n";
            }
        }
    }
    // Since '-t' flag is not used, continue as normal.
    else 
    {
        for (int i=0;i<MAX_ALLOWED_LINES;i++) // Limits the shell to MAX_ALLOWED_LINES
        { 
            // Print the prompt.
            std::cout << "osh> " << std::flush;

            // Read a single line.
            if (!std::getline(std::cin, input_line) || input_line == "exit") 
            {
                break;
            }

            try 
            {
                // Parse the input line.
                std::vector<shell_command> shell_commands
                    = parse_command_string(input_line);

                runcmd(shell_commands);
            }
            catch (const std::runtime_error& e) 
            {
                std::cout << "osh: " << e.what() << "\n";
            }
        }
    }
    
}

// Code from Yutaka Tsutano's README for the given parser 
// https://github.com/ytsutano/osh-parser
int exec(const std::string& cmd, const std::vector<std::string>& args)
{
    // Make an ugly C-style args array.
    std::vector<char*> c_args = {const_cast<char*>(cmd.c_str())};
    for (const auto& a : args) 
    {
        c_args.push_back(const_cast<char*>(a.c_str()));
    }
    c_args.push_back(nullptr);

    return execvp(cmd.c_str(), c_args.data());
}


