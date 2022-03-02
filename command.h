/*
	Function: main_shell(char * buffer, char ** args);
	Usage: main_shell(char * buffer, char ** args);
	---------------------------
	main shell interface that reads user input into buffer and parse the string by white spaces
	and store the argument list into args
*/

// void main_shell(char * buffer, size_t bufsize, char ** args);

int main_shell(char * buffer, char ** args);

bool check_if_valid_command(char * command, char ** valid_commands);

int execute(char ** args, char ** valid_commands);