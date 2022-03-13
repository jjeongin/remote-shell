// Remote shell project [Phase 1] 
// Created Date: Feb 26, 2022
// Latest Update : Mar 13, 2022
// Author : Jeongin Lee, Nouf Alabbasi
// Command List : ls, clear, pwd, mkdir, cat, echo, find, mv, rm, cd, grep

// Function Declaration

void get_user_input(char * buffer, size_t bufsize);

int get_argument_list(char * buffer, char ** args);

bool check_if_valid_command(char * command, char ** valid_commands);

int execute(char ** args, char ** valid_commands);

int redirect_input(char * filename);

int redirect_output(char * filename);

char * check_if_io_redirection(char * buffer, bool * redirect_input_found, bool * redirect_output_found);

void divide_buffer(char * buffer, char ** divided_buffers, int pipe_num);

int check_pipes(char * buffer);

int execute_pipes(char ** current_args, char ** valid_commands, char ** divided_buffers, int pipe_num);