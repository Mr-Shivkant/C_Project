#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_RECORDS 100
#define MAX_FIELDS 20
#define MAX_TEXT 100

/* One field = header + value */
typedef struct {
    char header[MAX_TEXT];
    char value[MAX_TEXT];
} Field;

/* One file = one record */
typedef struct {
    Field fields[MAX_FIELDS];
    int field_count;
    char filename[MAX_TEXT];
} Record;

/* Whole folder data in RAM */
typedef struct {
    Record records[MAX_RECORDS];
    int record_count;
    char folder[MAX_TEXT];
} Table;

Table db;

/* remove newline */
void trim(char *s) {
    s[strcspn(s, "\n")] = 0;
}

/* find value of given header */
char* get_value(Record *r, char *header) {
    for(int i=0;i<r->field_count;i++) {
        if(strcmp(r->fields[i].header, header)==0)
            return r->fields[i].value;
    }
    return "";
}

/* load all files from folder */
void load_folder(char *folder) {
    DIR *dir;
    struct dirent *entry;
    char path[200];
    char line[200];

    db.record_count = 0;
    strcpy(db.folder, folder);

    dir = opendir(folder);
    if(dir == NULL) {
        printf("Cannot open folder\n");
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

                while(r->fields[r->field_count].value[0] == ' ')
                    memmove(r->fields[r->field_count].value,
                            r->fields[r->field_count].value + 1,
                            strlen(r->fields[r->field_count].value));

                r->field_count++;
            }
        }

        fclose(fp);
        db.record_count++;
    }

    closedir(dir);

    printf("Loaded %d records into RAM\n", db.record_count);
}

/* display all records */
void display() {
    for(int i=0;i<db.record_count;i++) {
        printf("\nRecord %d:\n", i+1);

        for(int j=0;j<db.records[i].field_count;j++) {
            printf("%s : %s\n",
                db.records[i].fields[j].header,
                db.records[i].fields[j].value);
        }
    }
}

/* sort by header */
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
}

/* insert new record */
void insert_record() {
    Record *r = &db.records[db.record_count];
    r->field_count = db.records[0].field_count;

    for(int i=0;i<r->field_count;i++) {
        strcpy(r->fields[i].header, db.records[0].fields[i].header);

        printf("Enter %s: ", r->fields[i].header);
        fgets(r->fields[i].value, MAX_TEXT, stdin);
        trim(r->fields[i].value);
    }

    sprintf(r->filename, "record%d.txt", db.record_count+1);
    db.record_count++;

    printf("Record inserted in RAM\n");
}

/* delete record */
void delete_record(int index) {
    if(index<0 || index>=db.record_count) {
        printf("Invalid index\n");
        return;
    }

    for(int i=index;i<db.record_count-1;i++) {
        db.records[i] = db.records[i+1];
    }

    db.record_count--;

    printf("Record deleted from RAM\n");
}

/* update record */
void update_record(int index, char *header, char *newvalue) {
    if(index<0 || index>=db.record_count) {
        printf("Invalid index\n");
        return;
    }

    for(int i=0;i<db.records[index].field_count;i++) {
        if(strcmp(db.records[index].fields[i].header, header)==0) {
            strcpy(db.records[index].fields[i].value, newvalue);
            printf("Updated successfully\n");
            return;
        }
    }

    printf("Header not found\n");
}

/* save RAM data back to files */
void save_files() {
    char path[200];

    for(int i=0;i<db.record_count;i++) {
        sprintf(path, "%s/%s", db.folder, db.records[i].filename);

        FILE *fp = fopen(path, "w");
        if(fp == NULL)
            continue;

        for(int j=0;j<db.records[i].field_count;j++) {
            fprintf(fp, "%s: %s\n",
                db.records[i].fields[j].header,
                db.records[i].fields[j].value);
        }

        fclose(fp);
    }

    printf("Changes saved to files\n");
}

/* menu */
int main() {
    int choice, index;
    char folder[100], header[100], value[100];

    while(1) {
        printf("\n1.Load\n2.Display\n3.Sort\n4.Insert\n5.Delete\n6.Update\n7.Save\n8.Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);
        getchar();

        switch(choice) {
            case 1:
                printf("Folder: ");
                fgets(folder, 100, stdin);
                trim(folder);
                load_folder(folder);
                break;

            case 2:
                display();
                break;

            case 3:
                printf("Header: ");
                fgets(header,100,stdin);
                trim(header);
                sort_records(header);
                break;

            case 4:
                insert_record();
                break;

            case 5:
                printf("Record number: ");
                scanf("%d",&index);
                getchar();
                delete_record(index-1);
                break;

            case 6:
                printf("Record number: ");
                scanf("%d",&index);
                getchar();

                printf("Header: ");
                fgets(header,100,stdin);
                trim(header);

                printf("New value: ");
                fgets(value,100,stdin);
                trim(value);

                update_record(index-1, header, value);
                break;

            case 7:
                save_files();
                break;

            case 8:
                return 0;
        }
    }
}