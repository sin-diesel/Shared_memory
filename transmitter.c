#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define PROJ_ID 10 // for key generation


int main (int argc, char** argv) {

    assert(argc == 2);
    int fd = open(argv[1], O_RDONLY);
    struct stat input;
    fstat(fd, &input);
    int input_size = input.st_size;

    char* buf = (char*) calloc(input_size, sizeof(char));
    assert(buf);
    int n_read = read(fd, buf, input_size);
    assert(n_read == input_size);
    fprintf(stderr, "Bytes read: %d\n", n_read);
    fprintf(stderr, "Data: %s\n", buf);

    // transfering input_size to receiver
    mkfifo("/Users/stassidelnikov/Documents/MIPT/Operating_systems/Shared_memory/mf", 0666);
    int fifo = open("/Users/stassidelnikov/Documents/MIPT/Operating_systems/Shared_memory/mf", O_WRONLY);
    assert(fifo > 0);
    int n_write = write(fifo, &input_size, sizeof(int));
    assert(n_write == sizeof(int));
    close(fifo);
    fprintf(stderr, "Buf size put in pipe transmitter:%d\n", input_size);

    const char* path = "/Users/stassidelnikov/Documents/MIPT/Operating_systems/Shared_memory/sync.txt";
    key_t key = ftok(path, PROJ_ID);
    assert(key != -1);
    fprintf(stderr, "Key: %d\n", key);
    

    struct sembuf semafor_1 = {0, -1, 0}; // sub
    int sem_id = semget(key, 1, IPC_CREAT | 0666);
    assert(sem_id != -1);

    int id = shmget(key, input_size * sizeof(char), IPC_CREAT | 0666); // 10 bytes
    assert(id != -1);

    void* memory = shmat(id, NULL, 0);
    assert(memory != (void*) -1);

    int res = semop(sem_id, &semafor_1, 1); // waiting until memory is ready for access

    void* dest = memcpy(memory, buf, input_size);
    semafor_1.sem_op = 1;
    res = semop(sem_id, &semafor_1, 1);
    assert(res != -1);

    fprintf(stderr, "After copying to shm: %s\n", (char*) memory);

    int det_res = shmdt(memory);
    assert(det_res != -1);
    free(buf);
    close(fd);
    return 0;
}