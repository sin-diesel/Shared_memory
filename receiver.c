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


#define MAX_SIZE 2048

int main () {
    char* buf = (char*) calloc(MAX_SIZE, sizeof(char));
    assert(buf != NULL);

    const char* path = "/Users/stassidelnikov/Documents/MIPT/Operating_systems/Shared_memory/sync.txt";
    key_t key = ftok(path, PROJ_ID);
    assert(key != -1);
    fprintf(stderr, "Key: %d\n", key);

    struct sembuf semafor_1 = {0, 1, 0};
    int sem_id = semget(key, 1, IPC_CREAT | 0666);
    assert(sem_id != -1);

    int input_size = 0;
    const char* fifo_path = "/Users/stassidelnikov/Documents/MIPT/Operating_systems/Shared_memory/mf";
    int fifo = open(fifo_path, O_RDONLY);
    assert(fifo > 0);
    int n_read = read(fifo, &input_size, sizeof(int));
    assert(n_read == sizeof(int));
    close(fifo);
    fprintf(stderr, "Buf size retrieved from pipe by receiver:%d\n", input_size);


    int id = shmget(key, input_size * sizeof(char), IPC_CREAT | 0666);
    assert(id != -1);
    int res = 0;

    //int res = semop(sem_id, &semafor_1, 1); // allowing transmitter to continue
    //assert(res != -1);

    void* memory = shmat(id, NULL, 0); 
    assert(memory != (void*) -1);
    semafor_1.sem_op = -1;
    res = semop(sem_id, &semafor_1, 1);
    assert(res != -1);
    //fprintf(stderr, "Mem[0]: %c\n", *((char*) memory + 1));
    memcpy(buf, memory, input_size);
    buf[input_size - 1] = '\0';
    fprintf(stderr, "Buf received: %s\n", buf);

    int det_res = shmdt(memory);
    assert(det_res != -1);

    free(buf);
    return 0;

}