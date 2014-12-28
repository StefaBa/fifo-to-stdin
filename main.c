#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <errno.h>

//usagen: fi2sti <fifo_file> <program> <args>

int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		printf("Usage: fi2sti <fifo_file> <program> [args]\n");
		return EXIT_SUCCESS;
	}

	//check if file fifo_file exists and if i have read access to it
	if(access(argv[1], R_OK) != 0)
	{
		int err = errno;
		fprintf(stderr, "Cannot read from file \"%s\":\n%s\n", argv[1], strerror(err));
		return EXIT_FAILURE;
	}


	//now going to start <tail -f <fifo_file> --pid=parrent_pid>
	//therefore tail is going to quit as soon as i quit
	int mypid = getpid();

	size_t mypid_str_len = (size_t)((ceil(log10(mypid))+1)*sizeof(char));
	char mypid_str[mypid_str_len];
	if(sprintf(mypid_str, "%i", mypid) < 0)
		return EXIT_FAILURE;
	
	//popen_cmd = tail --pid=PID -f argv[1]
	char tail_cmd_first[] = "tail --pid=";
	char tail_cmd_middle[] = " -f ";

	//usually you'd need malloc(strlen(a) + strlen(b) + ... + strlen(z) + 1)
	//as strlen does not contain the terminating null byte, but mypid_str_len contains it
	//so +1 is not needed
	char *popen_cmd = malloc(strlen(tail_cmd_first) + mypid_str_len + strlen(tail_cmd_middle) + strlen(argv[1]));
	if(popen_cmd == NULL)
	{
		fprintf(stderr, "memory allocation failed!\n");
		return EXIT_FAILURE;
	}

	strcat(popen_cmd, tail_cmd_first);
	strcat(popen_cmd + strlen(tail_cmd_first), mypid_str);
	strcat(popen_cmd + strlen(tail_cmd_first) + mypid_str_len -1, tail_cmd_middle);
	strcat(popen_cmd + strlen(tail_cmd_first) + mypid_str_len -1 + strlen(tail_cmd_middle), argv[1]);


	FILE* input_of_program = popen(popen_cmd, "r");
	if(input_of_program == NULL)
	{
		int err = errno;
		fprintf(stderr, "Cannot create subprocess \"tail\":\n%s\n", strerror(err));
		return EXIT_FAILURE;
	}

	int input_fd = fileno(input_of_program);
	if(input_fd == -1)
	{
		int err = errno;
		fprintf(stderr, "Cannot create file descriptor:\n%s\n", strerror(err));
		return EXIT_FAILURE;
	}

    if(dup2(input_fd, STDIN_FILENO) == -1)
	{
		int err = errno;
		fprintf(stderr, "Cannot change the stdin file descriptor to the newly generated file descriptor:\n%s\n", strerror(err));
		return EXIT_FAILURE;
	}

    execvp(argv[2], argv+2);

	//execvp only returns on error
	int err = errno;
	fprintf(stderr, "Cannot execute the specified program \"%s\" with arguments: [", argv[2]);
	for(int i = 3; i<argc; i++)
		fprintf(stderr, "\"%s\" ", argv[i]);
	fprintf(stderr, "] because of:\n%s\n", strerror(err));

	return EXIT_FAILURE;
}
