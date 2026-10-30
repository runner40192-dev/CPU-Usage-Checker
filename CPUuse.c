#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

void get_process_cpu_time(int pid, unsigned long *utime, unsigned long *stime) {
    char path[512];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *fp = fopen(path, "r");
    if (!fp) return;

    // Read necessary fields from /proc/[pid]/stat
    // The 14th and 15th fields are utime and stime
    unsigned long dummy;
    char comm[256];
    char state;
    fscanf(fp, "%d %s %c", &pid, comm, &state);
    for (int i = 4; i <= 13; i++) {
        fscanf(fp, "%lu", &dummy);
    }
    fscanf(fp, "%lu %lu", utime, stime);
    fclose(fp);
}

int main() {
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    printf("%-10s %-20s %-10s\n", "PID", "Command", "CPU Time (jiffies)");

    while ((entry = readdir(dir)) != NULL) {
        // Check if the directory name is a number (PID)
        if (entry->d_type == DT_DIR) {
            int pid = atoi(entry->d_name);
            if (pid > 0) {
                unsigned long utime = 0, stime = 0;
                get_process_cpu_time(pid, &utime, &stime);
                if (utime > 0 || stime > 0) {
                    char path[512];
                    char comm[256] = "unknown";
                    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
                    FILE *fp = fopen(path, "r");
                    if (fp) {
                        fgets(comm, sizeof(comm), fp);
                        comm[strcspn(comm, "\n")] = 0;
                        fclose(fp);
                    }
                    printf("%-10d %-20s %-10lu\n", pid, comm, utime + stime);
                }
            }
        }
    }
    closedir(dir);
    return 0;
}
