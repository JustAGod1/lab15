#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ListNode {
    struct ListNode *previous;
    char* value;
};

typedef struct ListNode ListNode;

typedef struct {
    ListNode *head;
    unsigned count;
} List;

struct SetNode {
    struct SetNode *previous;
    unsigned char value;
};

typedef struct SetNode SetNode;

typedef struct {
    SetNode* head;
    long long flags[4];
} Set;

typedef struct {
    Set set;
    unsigned buckets[256];
} Map;

void add(Set *set, unsigned char idx) {
    long long *target = &set->flags[idx / 64];
    long long bit = ((long long) 1) << idx % 64;
    if (((*target) & bit) != 0) return;
    *target = *target | bit;
    SetNode *node = malloc(sizeof(SetNode));
    node->previous = set->head;
    set->head = node;
}

void increase(Map *map, unsigned char idx) {
    add(&map->set, idx);
    map->buckets[idx]++;
}

unsigned get(Map *map, unsigned char idx) {
    return map->buckets[idx];
}

Map* createDictionary(FILE* input) {
    Map *result = calloc(sizeof(Map), 1);
    int a;
    while ((a = fgetc(input)) != EOF) {
        unsigned char c = a;
        increase(result, a);
    }
    return result;
}

void addToList(List *list, char *value) {
    ListNode *node = malloc(sizeof(ListNode));
    node->previous = list->head;
    node->value = value;
    list->head = node;
    list->count++;
}

int main(int argc, char **args) {
    List inputFiles;
    inputFiles.head = 0;
    inputFiles.count = 0;
    char* fileName = 0;
    int list = 0;
    int create = 0;
    int extract = 0;

    for (int i = 1; i < argc; ++i) {
        char *arg = args[i];
        if (strcmp("--file", arg) == 0) {
            if ((i + 1) >= argc) {
                printf("--file was found but it's last argument\n");
                exit(1);
            }
            if (fileName != 0) {
                printf("File name redefinition\n");
                exit(1);
            }
            fileName = args[i + 1];
            i++;
        } else if (strcmp("--create", arg) == 0) {
            create = 1;
        } else if (strcmp("--extract", arg) == 0) {
            extract = 1;
        } else if (strcmp("--list", arg) == 0) {
            list = 1;
        } else {
            addToList(&inputFiles, arg);
        }
    }

    if (fileName == 0) {
        printf("File name!\n");
        exit(1);
    }

    if (create && extract) {
        printf("Can't create and extract simultaneously\n");
        exit(1);
    }

    if (list && create) {
        printf("Where is the point to create and list archive\n");
        exit(1);
    }

    if (create && inputFiles.count <= 0) {
        printf("You've requested to create archive so you have to add some files\n");
        exit(1);
    }

    if (create) {
        FILE *output = fopen(fileName, "wb");
        fwrite(&inputFiles.count, sizeof(unsigned), 1, output);
        ListNode *node = inputFiles.head;
        while (node != 0) {
            FILE* input = fopen(node->value, "rb");
            if (input == 0) {
                printf("Error while opening file %s", node->value);
                exit(0);
            }

            size_t l = strlen(node->value);
            fwrite(&l, sizeof(size_t), 1, output);
            fwrite(node->value, 1, l, output);
            fseek(input, 0, SEEK_END);
            long size = ftell(input);
            fwrite(&size, sizeof(long), 1, output);
            rewind(input);
            int c;
            while ((c = fgetc(input)) != EOF) fputc(c, output);
            node = node->previous;
        }
    } else {
        FILE *input = fopen(fileName, "rb");
        if (input == 0) {
            printf("Error while opening file %s", fileName);
            exit(0);
        }
        unsigned count;
        fread(&count, sizeof(unsigned), 1, input);
        for (int i = 0; i < count; i++) {
            size_t nameSize;
            fread(&nameSize, sizeof(size_t), 1, input);
            char *name = malloc((nameSize + 1) * sizeof(char));
            name[nameSize] = '\0';
            fread(name, 1, nameSize, input);

            if (list) {
                printf("%s\n", name);
            }
            long fileSize;
            fread(&fileSize, sizeof(long), 1, input);
            if (extract) {
                FILE *output = fopen(name, "wb");
                for (long j = 0; j < fileSize; ++j) {
                    fputc(fgetc(input), output);
                }
                fclose(output);
            } else {
                for (long j = 0; j < fileSize; ++j) {
                    fgetc(input);
                }
            }
        }
    }

    printf("Hello, World!\n");
    return 0;
}