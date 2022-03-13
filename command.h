/*
	Function: main_shell(char * buffer, char ** args);
	Usage: main_shell(char * buffer, char ** args);
	---------------------------
	main shell interface that reads user input into buffer and parse the string by white spaces
	and store the argument list into args
*/

void get_user_input(char * buffer, size_t bufsize);

int get_argument_list(char * buffer, char ** args);

bool check_if_valid_command(char * command, char ** valid_commands);

int execute(char ** args);

int redirect_input(char * filename);

int redirect_output(char * filename);

char * check_if_io_redirection(char * buffer, bool * redirect_input_found, bool * redirect_output_found);

char ** divide_buffer(char * buffer, char ** divided_buffers, int pipe_num);

int check_pipes(char * buffer);

int execute_pipes(char ** current_args, char ** divided_buffers, int pipe_num);