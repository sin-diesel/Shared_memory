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
    char* buf = (char*) (MAX_SIZE, sizeof(char));
    assert(buf != NULL);

    const char* path = "/home/stanislav/Documents/MIPT/Operating_systems/shm_transfer/sync.txt";
    key_t key = ftok(path, PROJ_ID);
    assert(key != -1);

    struct sembuf semafor = {0, -1, 0};
    int sem_id = semget(key, 1, 0);
    assert(sem_id != -1);
    int res = semop(sem_id, &semafor, 1); // waiting until memory is ready for access
    assert(res != -1);

    int id = shmget(key, 10 * sizeof(char), 0); // ten bytes
    assert(id != -1);
    void* memory = shmat(id, NULL, 0); 
    assert(memory != (void*) -1);


    int det_res = shmdt(memory);
    assert(det_res != -1);
    free(buf);

    //semop(id, &semafor, 1);

    //memcpy(buf, memory, 10);
    //buf[9] = '\0';
    //fprintf(stderr, "Buf:%s\n", buf);

    return 0;

}