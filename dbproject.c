#include <time.h>
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

Table db1;
Table db2;
Table result_db;

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

void load_folder(Table *db, char *folder) {
    DIR *dir;
    struct dirent *entry;
    char path[200], line[200];

    db->record_count = 0;
    strcpy(db->folder, folder);

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

        Record *r = &db->records[db->record_count];
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
        db->record_count++;
    }

    closedir(dir);

    printf("%d records loaded into RAM.\n", db->record_count);
}

void display_records(Table *db) {
    if(db->record_count == 0) {
        printf("No data loaded.\n");
        return;
    }

    for(int i=0;i<db->record_count;i++) {
        printf("\nRecord %d\n", i+1);

        for(int j=0;j<db->records[i].field_count;j++) {
            printf("%s : %s\n",
                   db->records[i].fields[j].header,
                   db->records[i].fields[j].value);
        }
    }
}

void display_sorted_column(char *header) {
    printf("\nValues sorted by %s:\n", header);

    for(int i=0;i<db1.record_count;i++) {
        printf("Record %d : %s\n",
               i+1,
               get_value(&db1.records[i], header));
    }
}

void sort_records(char *header) {
    clock_t start, end;
    double cpu_time;

    start = clock();     // timing starts here

    for(int i=0;i<db1.record_count-1;i++) {
        for(int j=i+1;j<db1.record_count;j++) {
            if(strcmp(get_value(&db1.records[i], header),
                      get_value(&db1.records[j], header)) > 0) {
                Record temp = db1.records[i];
                db1.records[i] = db1.records[j];
                db1.records[j] = temp;
            }
        }
    }

    end = clock();       // timing ends here

    cpu_time = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Sorted by %s\n", header);
    printf("Execution time: %f seconds\n", cpu_time);

    display_sorted_column(header);
}

void insert_record() {
    Record temp;
    temp.field_count = db1.records[0].field_count;

    for(int i=0;i<temp.field_count;i++) {
        strcpy(temp.fields[i].header, db1.records[0].fields[i].header);

        printf("Enter %s: ", temp.fields[i].header);
        fgets(temp.fields[i].value, MAX_TEXT, stdin);
        trim(temp.fields[i].value);
    }

    clock_t start = clock();

    db1.records[db1.record_count] = temp;
    sprintf(db1.records[db1.record_count].filename, "record%d.txt", db1.record_count+1);
    db1.record_count++;

    clock_t end = clock();

    double cpu_time = (double)(end-start)/CLOCKS_PER_SEC;

    printf("Execution time: %f seconds\n", cpu_time);
}

void delete_record(int index) {
    if(index < 0 || index >= db1.record_count) {
        printf("Invalid record number.\n");
        return;
    }

    clock_t start = clock();

    for(int i=index;i<db1.record_count-1;i++) {
        db1.records[i] = db1.records[i+1];
    }

    db1.record_count--;

    clock_t end = clock();

    printf("Record deleted.\n");
    printf("Execution time: %f seconds\n",
           (double)(end-start)/CLOCKS_PER_SEC);
}

void update_record(int index, char *header, char *newvalue) {
    if(index < 0 || index >= db1.record_count) {
        printf("Invalid record number.\n");
        return;
    }

    clock_t start = clock();

    for(int i=0;i<db1.records[index].field_count;i++) {
        if(strcmp(db1.records[index].fields[i].header, header)==0) {
            strcpy(db1.records[index].fields[i].value, newvalue);

            clock_t end = clock();
            double cpu_time = (double)(end-start)/CLOCKS_PER_SEC;

            printf("Record updated in RAM.\n");
            printf("Execution time: %f seconds\n", cpu_time);
            return;
        }
    }

    clock_t end = clock();
    double cpu_time = (double)(end-start)/CLOCKS_PER_SEC;

    printf("Header not found.\n");
    printf("Execution time: %f seconds\n", cpu_time);
}

void save_files() {
    char path[200];

    for(int i=0;i<db1.record_count;i++) {
        sprintf(path, "%s/%s", db1.folder, db1.records[i].filename);

        FILE *fp = fopen(path, "w");
        if(fp==NULL) continue;

        for(int j=0;j<db1.records[i].field_count;j++) {
            fprintf(fp,"%s: %s\n",
                    db1.records[i].fields[j].header,
                    db1.records[i].fields[j].value);
        }

        fclose(fp);
    }

    printf("Saved to files.\n");
}

void inner_join(char *header) {
    clock_t start = clock();

    result_db.record_count = 0;

    for(int i=0;i<db1.record_count;i++) {
        for(int j=0;j<db2.record_count;j++) {

            if(strcmp(get_value(&db1.records[i], header),
                      get_value(&db2.records[j], header)) == 0) {

                Record *r = &result_db.records[result_db.record_count];
                r->field_count = 0;

                for(int k=0;k<db1.records[i].field_count;k++) {
                    r->fields[r->field_count++] = db1.records[i].fields[k];
                }

                for(int k=0;k<db2.records[j].field_count;k++) {
                    if(strcmp(db2.records[j].fields[k].header, header)!=0) {
                        r->fields[r->field_count++] = db2.records[j].fields[k];
                    }
                }

                result_db.record_count++;
            }
        }
    }

    clock_t end = clock();
    double cpu_time = (double)(end-start)/CLOCKS_PER_SEC;

    printf("Inner join completed.\n");
    printf("Execution time: %f seconds\n", cpu_time);
}

void outer_join(char *header) {
    clock_t start = clock();

    result_db.record_count = 0;

    for(int i=0;i<db1.record_count;i++) {
        int found = 0;

        for(int j=0;j<db2.record_count;j++) {
            if(strcmp(get_value(&db1.records[i], header),
                      get_value(&db2.records[j], header)) == 0) {

                Record *r = &result_db.records[result_db.record_count];
                r->field_count = 0;

                for(int k=0;k<db1.records[i].field_count;k++)
                    r->fields[r->field_count++] = db1.records[i].fields[k];

                for(int k=0;k<db2.records[j].field_count;k++)
                    if(strcmp(db2.records[j].fields[k].header, header) != 0)
                        r->fields[r->field_count++] = db2.records[j].fields[k];

                result_db.record_count++;
                found = 1;
            }
        }

        if(found == 0) {
            result_db.records[result_db.record_count++] = db1.records[i];
        }
    }

    clock_t end = clock();
    double cpu_time = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Outer join completed.\n");
    printf("Execution time: %f seconds\n", cpu_time);
}

void full_join(char *header) {
    clock_t start = clock();

    int matched[MAX_RECORDS] = {0};
    result_db.record_count = 0;

    for(int i=0;i<db1.record_count;i++) {
        int found = 0;

        for(int j=0;j<db2.record_count;j++) {
            if(strcmp(get_value(&db1.records[i], header),
                      get_value(&db2.records[j], header)) == 0) {

                Record *r = &result_db.records[result_db.record_count];
                r->field_count = 0;

                for(int k=0;k<db1.records[i].field_count;k++)
                    r->fields[r->field_count++] = db1.records[i].fields[k];

                for(int k=0;k<db2.records[j].field_count;k++)
                    if(strcmp(db2.records[j].fields[k].header, header) != 0)
                        r->fields[r->field_count++] = db2.records[j].fields[k];

                matched[j] = 1;
                result_db.record_count++;
                found = 1;
            }
        }

        if(found == 0) {
            result_db.records[result_db.record_count++] = db1.records[i];
        }
    }

    for(int j=0;j<db2.record_count;j++) {
        if(matched[j] == 0) {
            result_db.records[result_db.record_count++] = db2.records[j];
        }
    }

    clock_t end = clock();
    double cpu_time = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Full join completed.\n");
    printf("Execution time: %f seconds\n", cpu_time);
}

void display_result() {
    for(int i=0;i<result_db.record_count;i++) {
        printf("\nResult Record %d\n", i+1);

        for(int j=0;j<result_db.records[i].field_count;j++) {
            printf("%s : %s\n",
                   result_db.records[i].fields[j].header,
                   result_db.records[i].fields[j].value);
        }
    }
}
void save_result(char *folder) {
    char path[200];

    for(int i=0;i<result_db.record_count;i++) {
        sprintf(path,"%s/result%d.txt",folder,i+1);

        FILE *fp = fopen(path,"w");
        if(fp==NULL) continue;

        for(int j=0;j<result_db.records[i].field_count;j++) {
            fprintf(fp,"%s: %s\n",
                    result_db.records[i].fields[j].header,
                    result_db.records[i].fields[j].value);
        }

        fclose(fp);
    }

    printf("Result saved.\n");
}
void select_records(char *header, char *value) {
    result_db.record_count = 0;

    for(int i=0;i<db1.record_count;i++) {
        if(strcmp(get_value(&db1.records[i], header), value)==0) {
            result_db.records[result_db.record_count++] = db1.records[i];
        }
    }

    printf("Selection completed.\n");
}
void project_records(char *header) {
    result_db.record_count = 0;

    for(int i=0;i<db1.record_count;i++) {
        Record *r = &result_db.records[result_db.record_count];
        r->field_count = 0;

        for(int j=0;j<db1.records[i].field_count;j++) {
            if(strcmp(db1.records[i].fields[j].header, header)==0) {
                r->fields[r->field_count++] = db1.records[i].fields[j];
            }
        }

        result_db.record_count++;
    }

    printf("Projection completed.\n");
}

void run_query(char *query) {
    char header[100], value[100];

    clock_t start = clock();

    if(strncmp(query, "JOIN ", 5)==0) {
        sscanf(query + 5, "%s", header);
        inner_join(header);
    }

    else if(strncmp(query, "PROJECT ", 8)==0) {
        sscanf(query + 8, "%s", header);
        project_records(header);
    }

    else if(strncmp(query, "SELECT WHERE ", 13)==0) {
        sscanf(query + 13, "%[^=]=%s", header, value);
        select_records(header, value);
    }

    else {
        printf("Invalid query.\n");
        return;
    }

    clock_t end = clock();
    double cpu_time = (double)(end-start)/CLOCKS_PER_SEC;

    display_result();

    printf("Query executed successfully.\n");
    printf("Execution time: %f seconds\n", cpu_time);
}



int main() {
    setbuf(stdout, NULL);

    int choice;
    char input[100];

    while(1) {
        printf("\n===== MINI DATABASE SYSTEM =====\n");
        printf("1. Load first folder (DB1)\n");
        printf("2. Load second folder (DB2)\n");
        printf("3. Display first database\n");
        printf("4. Sort DB1 by header\n");
        printf("5. Insert new record in DB1\n");
        printf("6. Delete record from DB1\n");
        printf("7. Update record in DB1\n");
        printf("8. Save DB1 files\n");
        printf("9. Inner Join\n");
        printf("10. Outer Join\n");
        printf("11. Full Join\n");
        printf("12. Run Query Language\n");
        printf("13. Save join/query result\n");
        printf("14. Exit\n");
        printf("Enter your choice: ");

        fgets(input, sizeof(input), stdin);
        choice = atoi(input);

if(choice == 1) {
    printf("Enter folder for DB1: ");
    fgets(input, sizeof(input), stdin);
    trim(input);
    load_folder(&db1, input);
}

else if(choice == 2) {
    printf("Enter folder for DB2: ");
    fgets(input, sizeof(input), stdin);
    trim(input);
    load_folder(&db2, input);
}

else if(choice == 3) {
    display_records(&db1);
}

else if(choice == 4) {
    printf("Enter header name: ");
    fgets(input, sizeof(input), stdin);
    trim(input);
    sort_records(input);
}

else if(choice == 5) {
    insert_record();
}

else if(choice == 6) {
    printf("Enter record number: ");
    fgets(input, sizeof(input), stdin);
    delete_record(atoi(input)-1);
}

else if(choice == 7) {
    char header[100], value[100];

    printf("Enter record number: ");
    fgets(input, sizeof(input), stdin);
    int index = atoi(input)-1;

    printf("Enter header name: ");
    fgets(header, sizeof(header), stdin);
    trim(header);

    printf("Enter new value: ");
    fgets(value, sizeof(value), stdin);
    trim(value);

    update_record(index, header, value);
}

else if(choice == 8) {
    save_files();
}

else if(choice == 9) {
    printf("Enter common header: ");
    fgets(input, sizeof(input), stdin);
    trim(input);
    inner_join(input);
    display_result();
}

else if(choice == 10) {
    printf("Enter common header: ");
    fgets(input, sizeof(input), stdin);
    trim(input);
    outer_join(input);
    display_result();
}

else if(choice == 11) {
    printf("Enter common header: ");
    fgets(input, sizeof(input), stdin);
    trim(input);
    full_join(input);
    display_result();
}

else if(choice == 12) {
    printf("Enter query: ");
    fgets(input, sizeof(input), stdin);
    trim(input);
    run_query(input);
}

else if(choice == 13) {
    printf("Enter folder to save result: ");
    fgets(input, sizeof(input), stdin);
    trim(input);
    save_result(input);
}

else if(choice == 14) {
    break;
}
        else {
            printf("Invalid choice.\n");
        }
    }

    return 0;
}
