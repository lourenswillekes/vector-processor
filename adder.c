/*************************************************
 * Lourens Willekes
 *
 *
 *
 *
 *************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1
#define MAX 66
#define ATOI 48

void ctrlcHandler(int sig);

int main(int argc, char **argv) {

    int j;
    int carry;
    int status;
    int length;

    int pipe1fd[2];
    int pipe2fd[2];

    pid_t complpid;
    pid_t increpid;
    pid_t adderpid;

    char bufb[MAX];
    char bufa[MAX];
    char bufsum[MAX]; 

    FILE *fpb, *fpa, *fpout;

    struct sigaction ctrlc;
    ctrlc.sa_handler = ctrlcHandler;
    ctrlc.sa_flags = 0;
    sigemptyset(&ctrlc.sa_mask);
    sigaction(SIGINT, &ctrlc, NULL);


    // make sure user has supplied both files
    if (3 > argc) {
        perror("two input files required");
        exit(1);
    }

    // create the pipe between complementer and incrementer
    fprintf(stderr, "Creating pipe between complementer and incrementer\n");
    if (0 > pipe(pipe1fd)) {
        perror("first pipe plumbing problem");
        exit(1);
    }


    /* complementer process */
    fprintf(stderr, "Spawning new complementer process\n");
    if (0 > (complpid = fork())) {
        perror("complementer fork failed");
        exit(1);
    }
    fprintf(stderr, "PID Compl: %d\n", complpid);
    if (0 == complpid) {

        fprintf(stderr, "Pausing, resume with Ctrl-C"); fflush(stderr);
        // pause();

        fprintf(stderr, "Opening file B for reading\n");
        fpb = fopen(argv[2], "r");
        close(pipe1fd[READ]);

        // read from file line by line until EOF
        while (NULL != fgets(bufb, MAX, fpb)) {

            fprintf(stderr, "Reading value from file B\n");
            length = strlen(bufb) - 1;
            // remove newline
            bufb[length] = '\0';
            //printf("1: %s\n", bufb);

            // complementer
            fprintf(stderr, "Calculating One's Complement\n");
            for (j = 0; j < length; j++) {
                if ('1' == bufb[j]) {
                    bufb[j] = '0';
                } else {
                    bufb[j] = '1';
                }
            }
            //printf("2: %s\n", bufb);

            fprintf(stderr, "Writing complement to pipe 1\n");
            write(pipe1fd[WRITE], bufb, length + 1);

        }

        fclose(fpb);
        close(pipe1fd[WRITE]);
        exit(1);

    }

    // create the pipe between incrementer and adder
    fprintf(stderr, "Creating pipe between incrementer and adder\n");
    if (0 > pipe(pipe2fd)) {
        perror("second pipe plumbing problem");
        exit(1);
    }


    /* incrementer process */
    fprintf(stderr, "Spawning new incrementer process\n");
    if (0 > (increpid = fork())) {
        perror("incrementer fork failed");
        exit(1);
    }
    fprintf(stderr, "PID Incre: %d\n", increpid);
    if (0 == increpid) {

        close(pipe1fd[WRITE]);
        close(pipe2fd[READ]);

        // read from pipe1
        while (read(pipe1fd[READ], bufb, MAX)) {

            fprintf(stderr, "Reading complement from pipe 1\n");
            length = strlen(bufb);
            //printf("3: %s %d\n", bufb, length);

            // incrementer
            fprintf(stderr, "Incrementing\n");
            for (j = length - 1; j >= 0; j--) {
                if ('1' == bufb[j]) {
                    bufb[j] = '0';
                } else {
                    bufb[j] = '1';
                    break;
                }
            }
            //printf("4: %s\n", bufb);

            fprintf(stderr, "Writing incremented value to pipe 2\n");
            write(pipe2fd[WRITE], bufb, length + 1);

        }

        close(pipe1fd[READ]);
        close(pipe2fd[WRITE]);
        exit(1);

    }


    /* adder process */
    fprintf(stderr, "Spawning new adder process\n");
    if (0 > (adderpid = fork())) {
        perror("adder fork failed");
        exit(1);
    }
    fprintf(stderr, "PID Adder: %d\n", adderpid);
    if (0 == adderpid) {
 
        fprintf(stderr, "Opening file A for reading\n");
        fpa = fopen(argv[1], "r");
        fprintf(stderr, "Opening output file for writing\n");
        fpout = fopen("out.txt", "w");
        close(pipe2fd[WRITE]);

        // read from pipe2
        while (read(pipe2fd[READ], bufb, MAX)) {

            fprintf(stderr, "Reading incremented value from pipe 2\n");
            length = strlen(bufb);
            //printf("5: %s %d\n", bufb, length);

            fprintf(stderr, "Reading value from file A\n");
            fgets(bufa, MAX, fpa);
            // remove newline
            bufa[length] = '\0';
            //printf("6: %s %d\n", bufa, strlen(bufa));

            // adder
            fprintf(stderr, "Adding the values\n");
            carry = 0;
            for (j = length - 1; j >= 0; j--) {

                //printf("sum %d\n", (bufb[j] - ATOI) + (bufa[j] - ATOI) + carry );
                switch ( (bufb[j] - ATOI) + (bufa[j] - ATOI) + carry) {
                    case 0:
                        bufsum[j] = '0';
                        carry = 0;
                        break;
                    case 1:
                        bufsum[j] = '1';
                        carry = 0;
                        break;
                    case 2:
                        bufsum[j] = '0';
                        carry = 1;
                        break;
                    case 3:
                        bufsum[j] = '1';
                        carry = 1;
                        break;
                    default:
                        perror("impossible case");
                        exit(1);
                }

            }
            bufsum[length] = '\n';
            bufsum[length + 1] = '\0';
            printf("%s\n", bufsum);

            fprintf(stderr, "Writing difference to output file\n");
            fprintf(fpout, "%s", bufsum);


        }

        fclose(fpa);
        fclose(fpout);
        close(pipe2fd[READ]);
        exit(1);

    }


    //waitpid(complpid, &status, 0);
    //waitpid(increpid, &status, 0);
    //waitpid(adderpid, &status, 0);


    return 0;

}


void ctrlcHandler(int sig) {

    return;

}
