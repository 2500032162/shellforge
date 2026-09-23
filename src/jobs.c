#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "../include/jobs.h"

static job_t job_table[MAX_JOBS];
static int job_count = 0;
static int next_job_id = 1;

void jobs_init(void)
{
    memset(job_table, 0, sizeof(job_table));
    job_count = 0;
    next_job_id = 1;
}

int job_add(pid_t pgid, const char *command, job_state_t state)
{
    if (job_count >= MAX_JOBS) {
        fprintf(stderr, "jobs: too many jobs\n");
        return -1;
    }

    int idx = -1;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_table[i].job_id == 0) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return -1;

    int id = next_job_id++;
    job_table[idx].job_id = id;
    job_table[idx].pgid = pgid;
    job_table[idx].state = state;
    strncpy(job_table[idx].command, command, MAX_JOB_COMMAND - 1);
    job_table[idx].command[MAX_JOB_COMMAND - 1] = '\0';

    job_count++;
    return id;
}

job_t *job_find(int job_id)
{
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_table[i].job_id == job_id)
            return &job_table[i];
    }
    return NULL;
}

job_t *job_find_by_pgid(pid_t pgid)
{
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_table[i].job_id != 0 && job_table[i].pgid == pgid)
            return &job_table[i];
    }
    return NULL;
}

void job_remove(int job_id)
{
    job_t *job = job_find(job_id);
    if (!job) return;
    job->job_id = 0;
    job->pgid = 0;
    job->state = JOB_DONE;
    job->command[0] = '\0';
    if (job_count > 0) job_count--;
}

void jobs_print(void)
{
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_table[i].job_id == 0) continue;

        const char *state_str;
        switch (job_table[i].state) {
            case JOB_RUNNING: state_str = "Running"; break;
            case JOB_STOPPED: state_str = "Stopped"; break;
            case JOB_DONE: state_str = "Done"; break;
            default: state_str = "Unknown"; break;
        }
        printf("[%d] %s %s\n",
               job_table[i].job_id,
               state_str,
               job_table[i].command);
    }
}

void job_stop(pid_t pgid)
{
    job_t *job = job_find_by_pgid(pgid);
    if (job) job->state = JOB_STOPPED;
}

void job_continue(pid_t pgid)
{
    job_t *job = job_find_by_pgid(pgid);
    if (job) job->state = JOB_RUNNING;
}

void job_done(pid_t pgid)
{
    job_t *job = job_find_by_pgid(pgid);
    if (job) job->state = JOB_DONE;
}
