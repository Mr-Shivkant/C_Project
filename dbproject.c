#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_RECORDS 100
#define MAX_FIELDS 20
#define MAX_TEXT 100

typedef struct {
    char header[MAX_TEXT];
    char value[MAX_TEXT];
} Field;

typedef struct {
    Field fields[MAX_FIELDS];
    int field_count;
    char filename[MAX_TEXT];
} Record;

typedef struct {
    Record records[MAX_RECORDS];
    int record_count;
    char folder[MAX_TEXT];
} Table;

Table db;

void trim(char *s) {
    s[strcspn(s, "\n")] = 0;
}

char* get_value(Record *r, char *header) {
    for(int i=0;i<r->field_count;i++) {
        if(strcmp(r->fields[i].header, header)==0)
            return r->fields[i].value;
    }
    return "";
}

void load_folder(char *folder) {
    DIR *dir;
    struct dirent *entry;
    char path[200], line[200];

    db.record_count = 0;
    strcpy(db.folder, folder);

    dir = opendir(folder);
    if(dir == NULL) {
        printf("Folder not found.\n");
        return;
    }

    while((entry = readdir(dir)) != NULL) {
        if(entry->d_name[0] == '.')
            continue;

        sprintf(path, "%s/%s", folder, entry->d_name);

        FILE *fp = fopen(path, "r");
        if(fp == NULL)
            continue;

        Record *r = &db.records[db.record_count];
        r->field_count = 0;
        strcpy(r->filename, entry->d_name);

        while(fgets(line, sizeof(line), fp)) {
            trim(line);

            char *colon = strchr(line, ':');
            if(colon) {
                *colon = '\0';
                strcpy(r->fields[r->field_count].header, line);
                strcpy(r->fields[r->field_count].value, colon + 1);

                while(r->fields[r->field_count].value[0]==' ')
                    memmove(r->fields[r->field_count].value,
                            r->fields[r->field_count].value+1,
                            strlen(r->fields[r->field_count].value));

                r->field_count++;
            }
        }

        fclose(fp);
        db.record_count++;
    }

    closedir(dir);

    printf("%d records loaded into RAM.\n", db.record_count);
}

void display_records() {
    if(db.record_count==0) {
        printf("No data loaded.\n");
        return;
    }

    for(int i=0;i<db.record_count;i++) {
        printf("\nRecord %d\n", i+1);
        for(int j=0;j<db.records[i].field_count;j++) {
            printf("%s : %s\n",
                   db.records[i].fields[j].header,
                   db.records[i].fields[j].value);
        }
    }
}

void sort_records(char *header) {
    for(int i=0;i<db.record_count-1;i++) {
        for(int j=i+1;j<db.record_count;j++) {
            if(strcmp(get_value(&db.records[i], header),
                      get_value(&db.records[j], header)) > 0) {
                Record temp = db.records[i];
                db.records[i] = db.records[j];
                db.records[j] = temp;
            }
        }
    }

    printf("Sorted by %s\n", header);
    display_records();
}

void insert_record() {
    if(db.record_count == 0) {
        printf("Load data first.\n");
        return;
    }

    Record *r = &db.records[db.record_count];
    r->field_count = db.records[0].field_count;

    for(int i=0;i<r->field_count;i++) {
        strcpy(r->fields[i].header, db.records[0].fields[i].header);

        printf("Enter %s: ", r->fields[i].header);
        fgets(r->fields[i].value, MAX_TEXT, stdin);
        trim(r->fields[i].value);
    }

    sprintf(r->filename, "record%d.txt", db.record_count + 1);
    db.record_count++;

    printf("Record inserted into RAM.\n");
}

void delete_record(int index) {
    if(index < 0 || index >= db.record_count) {
        printf("Invalid record number.\n");
        return;
    }

    for(int i=index;i<db.record_count-1;i++) {
        db.records[i] = db.records[i+1];
    }

    db.record_count--;

    printf("Record deleted from RAM.\n");
}

void update_record(int index, char *header, char *newvalue) {
    if(index < 0 || index >= db.record_count) {
        printf("Invalid record number.\n");
        return;
    }

    for(int i=0;i<db.records[index].field_count;i++) {
        if(strcmp(db.records[index].fields[i].header, header)==0) {
            strcpy(db.records[index].fields[i].value, newvalue);
            printf("Record updated in RAM.\n");
            return;
        }
    }

    printf("Header not found.\n");
}

void save_files() {
    char path[200];

    for(int i=0;i<db.record_count;i++) {
        sprintf(path, "%s/%s", db.folder, db.records[i].filename);

        FILE *fp = fopen(path, "w");
        if(fp==NULL) continue;

        for(int j=0;j<db.records[i].field_count;j++) {
            fprintf(fp,"%s: %s\n",
                    db.records[i].fields[j].header,
                    db.records[i].fields[j].value);
        }

        fclose(fp);
    }

    printf("Saved to files.\n");
}

int main() {
    setbuf(stdout, NULL);

    int choice;
    char input[100];

    while(1) {
        printf("\n===== MINI DATABASE SYSTEM =====\n");
        printf("1. Load records from folder\n");
        printf("2. Display records\n");
        printf("3. Sort by header\n");
        printf("4. Insert new record\n");
        printf("5. Delete record\n");
        printf("6. Update record\n");
        printf("7. Save changes to files\n");
        printf("8. Exit\n");
        printf("Enter your choice: ");

        fgets(input, sizeof(input), stdin);
        choice = atoi(input);

        if(choice==1) {
            printf("Enter folder name: ");
            fgets(input,sizeof(input),stdin);
            trim(input);
            load_folder(input);
        }
        else if(choice==2) {
            display_records();
        }
        else if(choice==3) {
            printf("Enter header name: ");
            fgets(input,sizeof(input),stdin);
            trim(input);
            sort_records(input);
        }
        else if(choice==4) {
            insert_record();
        }
        else if(choice==5) {
            printf("Enter record number: ");
            fgets(input,sizeof(input),stdin);
            delete_record(atoi(input)-1);
        } 
        else if(choice==6) {
            char header[100], value[100];

            printf("Enter record number: ");
            fgets(input,sizeof(input),stdin);
            int index = atoi(input)-1;

            printf("Enter header name: ");
            fgets(header,sizeof(header),stdin);
            trim(header);

            printf("Enter new value: ");
            fgets(value,sizeof(value),stdin);
            trim(value);

            update_record(index, header, value);
        }
        else if(choice==7) {
            save_files();
        }
        else if(choice==8) {
            break;
        }
        else {
            printf("Invalid choice.\n");
        }
    }

    return 0;
}
