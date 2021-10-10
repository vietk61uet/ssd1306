#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
 
#define REG_CURRENT_TASK _IOR('a','a',int32_t*) //button -> user
#define LCD_CMD_1 _IOW('b','b',int32_t*) // user -> lcd
 
#define SIGETX 44
#define LCD_MODE 22
 
static int done = 0;
int32_t value = 0;
int check = 0;
 
void ctrl_c_handler(int n, siginfo_t *info, void *unused)
{
    if (n == SIGINT) {
        printf("\nrecieved ctrl-c\n");
        done = 1;
    }
}
 
void sig_event_handler(int n, siginfo_t *info, void *unused)
{
    if (n == SIGETX) {
        check = info->si_int;
        printf ("Received signal from kernel : Value =  %u\n", check);
        value = LCD_MODE;
        printf ("Sent signal from user : Value = %u\n", value);
    }
}
 
int main()
{
    int fd_read, fd_write;
    int32_t number;
    struct sigaction act;
 
    /* install ctrl-c interrupt handler to cleanup at exit */
    sigemptyset (&act.sa_mask);
    act.sa_flags = (SA_SIGINFO | SA_RESETHAND);
    act.sa_sigaction = ctrl_c_handler;
    sigaction (SIGINT, &act, NULL);
 
    /* install custom signal handler */
    sigemptyset(&act.sa_mask);
    act.sa_flags = (SA_SIGINFO | SA_RESTART);
    act.sa_sigaction = sig_event_handler;
    sigaction(SIGETX, &act, NULL);
 
    printf("Installed signal handler for SIGETX = %d\n", SIGETX);
 
    printf("\nOpening Driver\n");
    fd_read = open("/dev/reboot_3", O_RDWR);
    if(fd_read < 0) {
        printf("Cannot open device file...\n");
        return 0;
    }
 
    printf("Registering application ...");
    /* register this task with kernel for signal */
    if (ioctl(fd_read, REG_CURRENT_TASK,(int32_t*) &number)) {
        printf("Failed 1\n");
        close(fd_read);
        exit(1);
    }
    else {
        printf("Done from button to user!!!\n");
    }
   
    while(!done) {
        printf("Waiting for signal...\n");

        if (value != 0)
        {
            fd_write = open("/dev/lcd_class", O_RDWR );
            if(fd_write < 0) {
                printf("Cannot open file device file...\n");
                return 0;
            }
            if (ioctl(fd_write, LCD_CMD_1,(int32_t*) &value)) {
                printf("Failed 2\n");
                close(fd_write);
                exit(1);
            }
            else {
                printf("Done from user to lcd!!!\n");
                value = 0;
            }
            close(fd_write);
        }
 
        //blocking check
        while (!done && !check);
        check = 0;
    }
 
    printf("Closing Driver\n");
    close(fd_read);


}