#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#define DEFAULT_COMMANDS_FILE_PATH "/etc/usbremote_commands"

char **read_code_commands(char *);
void free_commands(char **);

#endif
