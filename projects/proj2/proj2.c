#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>

#define SEM_GETTING_ON_PIER "/xyadlo00_sem_getting_on_pier"
#define SEM_LOG "/xyadlo00_sem_log"
#define SEM_BOARDING "/xyadlo00_sem_boarding"
#define SEM_CONTROL "/xyadlo00_sem_control"
#define SEM_CROSSING "/xyadlo00_sem_crossing"

#define HACKER 1
#define SERF 2

//structure parameters
typedef struct{
    int count;   // p stores amount of children processes of each category generated.
    int max_time_hacker_gen;   // h stores value of maximum time to generate new hacker child-process.
    int max_time_serf_gen;   // s stores value of maximum time to generate new serf child-process.
    int max_crosssing_time;   // r stores value of maximum time to perform crossing the river.
    int max_wait_time;   // w stores valur of maximum time when person returns
    int pier_capacity;   // c stores value of capacity of pier
} param;

//structure of shared resources
typedef struct{
    int on_pier ;
    int action_number;
    int hacker_number ;
    int serf_number;
    int hacker_on_pier;
    int serf_on_pier;
    int in_boat;
}shared_data;

char *TYPE[] = {[HACKER] = "HACK", [SERF] = "SERF"};

sem_t *sem_getting_on_pier ;
sem_t *sem_log ;
sem_t *sem_control;
sem_t *sem_boarding;
sem_t *sem_crossing;


int process_args (int argc, char *argv[], param *params);

int wrong_args (char *trash);

void check_pier(shared_data *shared, param *params, int type, FILE *log);

void generate_child(shared_data *shared,param *params, int type, FILE *log);

void boarding(shared_data *shared,param *params, int type, FILE *log){
    if (shared->hacker_on_pier == 4){
        sem_wait(sem_control);
        sem_wait(sem_crossing);
        sem_wait(sem_log);
            shared->on_pier -= 4;
            shared->hacker_on_pier -= 4;
            fprintf(log,"%d: %s %d: boards: %d : %d\n",shared->action_number,TYPE[type],shared->hacker_number, shared->hacker_on_pier,shared->serf_on_pier);
            usleep(rand() % (params->max_crosssing_time));
        sem_post(sem_boarding);
            fprintf(log,"%d: member : exit\n",shared->action_number++);
        sem_post(sem_boarding);
            fprintf(log,"%d: member : exit\n",shared->action_number++);
        sem_post(sem_boarding);
            fprintf(log,"%d: member : exit\n",shared->action_number++);
            fprintf(log,"%d: capitan : exit\n",shared->action_number++);
        sem_post(sem_log);
        sem_post(sem_control);
        sem_post(sem_crossing);
    }
    else if (shared->serf_on_pier == 4){
        sem_wait(sem_control);
        sem_wait(sem_crossing);
            shared->on_pier -= 4;
            shared->serf_on_pier -= 4;
        sem_wait(sem_log);
            fprintf(log,"%d: %s %d: boards: %d : %d\n",shared->action_number,TYPE[type],shared->hacker_number, shared->hacker_on_pier,shared->serf_on_pier);
            usleep(rand() % (params->max_crosssing_time));
        sem_post(sem_boarding);
            fprintf(log,"%d: member : exit\n",shared->action_number++);
        sem_post(sem_boarding);
            fprintf(log,"%d: member : exit\n",shared->action_number++);
        sem_post(sem_boarding);
            fprintf(log,"%d: member : exit\n",shared->action_number++);
            fprintf(log,"%d: capitan : exit\n",shared->action_number++);
        sem_post(sem_log);
        sem_post(sem_control);
        sem_post(sem_crossing);
    }
    else if (shared->hacker_on_pier >= 2 && shared->serf_on_pier >= 2){
        sem_wait(sem_control);
        sem_wait(sem_crossing);
            shared->on_pier -= 4;
            shared->hacker_on_pier -= 2;
            shared->serf_on_pier -= 2;
        sem_wait(sem_log);
            fprintf(log,"%d: %s %d: boards: %d : %d\n",shared->action_number,TYPE[type],shared->hacker_number, shared->hacker_on_pier,shared->serf_on_pier);
            usleep(rand() % (params->max_crosssing_time));
        sem_post(sem_boarding);
            fprintf(log,"%d: member : exit\n",shared->action_number++);
        sem_post(sem_boarding);
            fprintf(log,"%d: member : exit\n",shared->action_number++);
        sem_post(sem_boarding);
            fprintf(log,"%d: member : exit\n",shared->action_number++);
            fprintf(log,"%d: capitan : exit\n",shared->action_number++);
        sem_post(sem_log);
        sem_post(sem_control);
        sem_post(sem_crossing);
    }
    else{
        sem_wait(sem_boarding);
    }
}

void get_on_pier(shared_data *shared, param *params, int type, FILE *log){
    sem_wait(sem_getting_on_pier);
    sem_wait(sem_control);
    sem_wait(sem_log);
            shared->action_number++;
            shared->on_pier++;
            type == HACKER ? shared->hacker_on_pier++ : shared->serf_on_pier++;
        fprintf(log,"%d: %s %d: waits: %d : %d\n",shared->action_number,TYPE[type],type == HACKER ? shared->hacker_number : shared->serf_number , shared->hacker_on_pier,shared->serf_on_pier);
    sem_post(sem_log);
    sem_post(sem_control);
    sem_post(sem_getting_on_pier);
    (void)params;
}


void action(shared_data *shared, param *params, int type, FILE *log){

    check_pier(shared, params, type, log);

    get_on_pier(shared, params, type, log);

    if (shared->hacker_on_pier == 3 && shared->serf_on_pier == 1){
        sem_wait(sem_boarding);
    }
    else if(shared->on_pier >= 4 && ((shared->hacker_on_pier == 4) || (shared->serf_on_pier == 4) || (shared->hacker_on_pier >= 2 && shared->hacker_on_pier >= 2))) {
        boarding(shared, params, type, log);
    }
    else{
        sem_post(sem_log);
        sem_post(sem_control);
        sem_wait(sem_boarding);
}
}

int main(int argc, char *argv[]) {
    pid_t serf_pid;
    pid_t hacker_pid;
    int pid;
    setbuf(stdout,NULL);

    param params;
    if (process_args(argc, argv, &params) == 1){
        return 1;
    }

    //creating semaphpores
    sem_getting_on_pier = sem_open(SEM_GETTING_ON_PIER, O_CREAT, 0666,1);
    sem_close(sem_getting_on_pier);
    sem_log = sem_open(SEM_LOG, O_CREAT, 0666, 1);
    sem_close(sem_log);
    sem_control = sem_open(SEM_CONTROL, O_CREAT, 0666, 1);
    sem_close(sem_control);
    sem_boarding = sem_open(SEM_BOARDING, O_CREAT, 0666, 4);
    sem_close(sem_boarding);
    sem_crossing = sem_open(SEM_CROSSING, O_CREAT, 0666, 1);
    sem_close(sem_crossing);


    FILE *log;
    if ((log = fopen("proj2.out","w+")) == NULL){
        perror("fopen");
        fclose(log);
        return 1;
    }

    //creating shared memory
    int shm_fd;
    if ((shm_fd = shm_open("/xyadlo00_share_memory",O_CREAT | O_EXCL | O_RDWR, 0644)) < 0){
        perror("shm_open");
        shm_unlink("/xyadlo00_share_memory");
        close(shm_fd);
        fclose(log);
        return 1;
    }

    if (ftruncate(shm_fd, sizeof(shared_data)) < 0){
        perror("ftruncate");
        shm_unlink("/xyadlo00_share_memory");
        fclose(log);
        return 1;
    }

    shared_data *shared;
    shared = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);

    shared->on_pier = 0;
    shared->action_number = 0;
    shared->hacker_number = 0;
    shared->hacker_on_pier = 0;
    shared->serf_on_pier = 0;
    shared->serf_number = 0;
    shared->in_boat = 0;


    //forking main process
    if ((pid = fork()) < 0){
        perror("fork");
        exit(2);
    }

    sem_getting_on_pier = sem_open(SEM_GETTING_ON_PIER, O_RDWR);
    sem_log = sem_open(SEM_LOG, O_RDWR);
    sem_control = sem_open(SEM_CONTROL, O_RDWR);
    sem_boarding = sem_open(SEM_CONTROL, O_RDWR);
    sem_crossing = sem_open(SEM_CROSSING, O_RDWR);
    if (pid == 0){
        generate_child(shared,&params, HACKER, log);
        exit(0);
    }
    else {

        hacker_pid = pid;
        if ((pid = fork()) < 0){
            perror("fork");
            exit(2);
        }
        if (pid == 0){
            generate_child(shared, &params, SERF, log);
            exit(0);
        }
        else {
            serf_pid = pid;
        }
    }

    waitpid(hacker_pid, NULL,0);
    waitpid(serf_pid, NULL, 0);


    fclose(log);
    sem_close(sem_getting_on_pier);
    sem_close(sem_log);
    sem_close(sem_control);
    sem_close(sem_boarding);
    sem_close(sem_crossing);

    sem_unlink(SEM_GETTING_ON_PIER);
    sem_unlink(SEM_LOG);
    sem_unlink(SEM_CONTROL);
    sem_unlink(SEM_BOARDING);
    sem_unlink(SEM_CROSSING);

    munmap(shared, sizeof(shared_data));
    shm_unlink("/xyadlo00_share_memory");

    return 0;
}

void generate_child(shared_data *shared,param *params, int type, FILE *log){
    int pid;
    pid_t id;

    for(int i = 1; i <= params->count ; i++){
        if ((pid = fork()) < 0){
            perror("fork");
            exit(2);}

        if (pid == 0){
            sem_wait(sem_control);
            sem_wait(sem_log);
                shared->action_number++;
                type == HACKER ? shared->hacker_number++ : shared->serf_number++;
                fprintf(log,"%d: %s %d: starts\n",shared->action_number,TYPE[type],type == HACKER ? shared->hacker_number : shared->serf_number);
            sem_post(sem_log);
            sem_post(sem_control);
            action(shared, params, type, log);
            exit(0);
        }
        else{
            id = pid;
        }
        type == HACKER ? usleep(rand() % (params->max_time_hacker_gen)) : usleep(rand() % (params->max_time_serf_gen));
    }
    waitpid(id, NULL,0);
}

void check_pier(shared_data *shared, param *params, int type, FILE *log){
    if (shared->on_pier >= params->pier_capacity){
        sem_wait(sem_control);
        sem_wait(sem_log);
            shared->action_number++;
            fprintf(log, "%d: %s %d: leave queue\n",shared->action_number, TYPE[type],shared->hacker_number);
            usleep(rand() % (params->max_wait_time));
            shared->action_number++;
            fprintf(log, "%d: %s %d: is back: %d : %d \n",shared->action_number, TYPE[type],shared->hacker_number, shared->hacker_on_pier,shared->serf_on_pier);
        sem_post(sem_log);
        sem_post(sem_control);

        check_pier(shared, params, type, log);
    }
}

int process_args (int argc, char *argv[], param *params){

    if (argc  != 7){
        printf("THERE NOT ENOUGHT ARGUMENTS. EXIT...\n");
        return 1;
    }

    char *trash = NULL;

    params->count = strtol(argv[1], &trash, 10);
    if (*trash != '\0'){
        return wrong_args(trash);
    }

    params->max_time_hacker_gen =  strtol(argv[2], &trash, 10);
    if (*trash != '\0'){
        return wrong_args(trash);
    }
    if (params->max_time_hacker_gen == 0){
        params->max_time_hacker_gen++;
    }

    params->max_time_serf_gen =  strtol(argv[3], &trash, 10);
    if (*trash != '\0'){
        return wrong_args(trash);
    }
    if (params->max_time_serf_gen == 0){
        params->max_time_serf_gen++;
    }


    params->max_crosssing_time =  strtol(argv[4], &trash, 10);
    if (*trash != '\0'){
        return wrong_args(trash);
    }
    if (params->max_crosssing_time == 0){
        params->max_crosssing_time++;
    }

    params->max_wait_time =  strtol(argv[5], &trash, 10);
    if (*trash != '\0'){
        return wrong_args(trash);
    }
    if (params->max_wait_time == 0){
        params->max_wait_time++;
    }

    params->pier_capacity =  strtol(argv[6], &trash, 10);
    if (*trash != '\0'){
        return wrong_args(trash);
    }

    if((params->count % 2 != 0)
        || (params->max_time_hacker_gen > 2000)
        || (params->max_time_serf_gen > 2000)
        || (params->max_crosssing_time > 2000)
        || (params->max_wait_time > 2000 )){
            printf("WRONG MESUARE OF ARGUMENT\n");
	        return 1;
        }

	if((params->count < 2)
        || (params->max_time_hacker_gen < 0)
        || (params->max_time_serf_gen < 0)
        || (params->max_crosssing_time < 0)
        || (params->max_wait_time < 20 )
        || (params->pier_capacity < 5)){
            printf("WRONG MESUARE OF ARGUMENT\n");
            return 1;
        }

    return 0;
}

int wrong_args (char *trash){
    printf("THERE WAS WRONG PARAMETR: %s\n",trash);
    return 1;
}
