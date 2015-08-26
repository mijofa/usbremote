#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "commands.h"

char **read_code_commands(char *path)
{
	int fd;
	fd = open(path, 0);
	if(fd == -1) {
		return NULL;
	}

	struct stat fs;
	if(fstat(fd, &fs) != 0) {
		close(fd);
		return NULL;
	}

	char *contents = malloc(fs.st_size);
	if(read(fd, contents, fs.st_size) != fs.st_size) {
		close(fd);
		free(contents);
		return NULL;
	}

	int commands_size = sizeof(char*) * 255;
	char **commands = (char **)malloc(commands_size);
	memset(commands, 0, commands_size);

	char *token;
	token = strtok(contents, "\n");
	if(token != NULL) {
		do {
			char *sep_offset;
			sep_offset = strstr(token, ":");
			if(sep_offset == NULL) {
				continue;
			}

			int index;
			char *index_str;
			index_str = malloc(sep_offset - token + 1);
			memcpy(index_str, token, sep_offset - token);
			index = atoi(index_str);
			free(index_str);

			commands[index] = strdup(sep_offset + 1);
		} while((token = strtok(NULL, "\n")));
	}

	close(fd);
	free(contents);
	return commands;
}

void free_commands(char **commands) {
	int x;
	for(x = 0; x < 255; x++) {
		if(commands[x] != NULL) {
			free(commands[x]);
		}
	}

	free(commands);
}
