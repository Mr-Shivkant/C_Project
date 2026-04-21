#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

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

void display_sorted_column(Table *db, char *header) {
    printf("\nValues sorted by %s:\n", header);

    for(int i=0;i<db->record_count;i++) {
        printf("Record %d : %s\n",
               i+1,
               get_value(&db->records[i], header));
    }
}

void sort_records(Table *db, char *header) {
    clock_t start = clock();

    for(int i=0;i<db->record_count-1;i++) {
        for(int j=i+1;j<db->record_count;j++) {
            if(strcmp(get_value(&db->records[i], header),
                      get_value(&db->records[j], header)) > 0) {
                Record temp = db->records[i];
                db->records[i] = db->records[j];
                db->records[j] = temp;
            }
        }
    }

    clock_t end = clock();

    printf("Sorted by %s\n", header);
    printf("Execution time: %f seconds\n",
           (double)(end-start)/CLOCKS_PER_SEC);

    display_sorted_column(db, header);
}

void insert_record(Table *db) {
    if(db->record_count == 0) {
        printf("No database loaded.\n");
        return;
    }

    Record temp;
    temp.field_count = db->records[0].field_count;

    for(int i=0;i<temp.field_count;i++) {
        strcpy(temp.fields[i].header, db->records[0].fields[i].header);

        printf("Enter %s: ", temp.fields[i].header);
        fgets(temp.fields[i].value, MAX_TEXT, stdin);
        trim(temp.fields[i].value);
    }

    clock_t start = clock();

    db->records[db->record_count] = temp;
    sprintf(db->records[db->record_count].filename,
            "record%d.txt", db->record_count+1);
    db->record_count++;

    clock_t end = clock();

    printf("Record inserted.\n");
    printf("Execution time: %f seconds\n",
           (double)(end-start)/CLOCKS_PER_SEC);
}

void delete_record(Table *db, int index) {
    if(index < 0 || index >= db->record_count) {
        printf("Invalid record number.\n");
        return;
    }

    clock_t start = clock();

    for(int i=index;i<db->record_count-1;i++) {
        db->records[i] = db->records[i+1];
    }

    db->record_count--;

    clock_t end = clock();

    printf("Record deleted.\n");
    printf("Execution time: %f seconds\n",
           (double)(end-start)/CLOCKS_PER_SEC);
}

void update_record(Table *db, int index, char *header, char *newvalue) {
    if(index < 0 || index >= db->record_count) {
        printf("Invalid record number.\n");
        return;
    }

    clock_t start = clock();

    for(int i=0;i<db->records[index].field_count;i++) {
        if(strcmp(db->records[index].fields[i].header, header)==0) {
            strcpy(db->records[index].fields[i].value, newvalue);

            clock_t end = clock();

            printf("Record updated.\n");
            printf("Execution time: %f seconds\n",
                   (double)(end-start)/CLOCKS_PER_SEC);
            return;
        }
    }

    printf("Header not found.\n");
}
void save_files(Table *db) {
    char path[200];

    for(int i=0;i<db->record_count;i++) {
        sprintf(path, "%s/%s", db->folder, db->records[i].filename);

        FILE *fp = fopen(path, "w");
        if(fp == NULL)
            continue;

        for(int j=0;j<db->records[i].field_count;j++) {
            fprintf(fp, "%s: %s\n",
                    db->records[i].fields[j].header,
                    db->records[i].fields[j].value);
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

    /* Trim spaces from header and value just in case */
    while(header[0] == ' ') memmove(header, header+1, strlen(header));
    while(value[0]  == ' ') memmove(value,  value+1,  strlen(value));

    for(int i=0;i<db1.record_count;i++) {
        if(strcmp(get_value(&db1.records[i], header), value)==0) {
            result_db.records[result_db.record_count++] = db1.records[i];
        }
    }

    if(result_db.record_count == 0)
        printf("No matching records found.\n");
    else
        printf("Selection completed. %d record(s) found.\n",
               result_db.record_count);
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
        display_result();
    }
    else if(strncmp(query, "PROJECT ", 8)==0) {
        sscanf(query + 8, "%s", header);
        project_records(header);
        display_result();
    }
    else if(strncmp(query, "SELECT WHERE ", 13)==0) {
        sscanf(query + 13, "%[^=]=%s", header, value);
        select_records(header, value);
        display_result();          /* ← THIS shows the output */
    }
    else if(strncmp(query, "SORT ", 5)==0) {
        sscanf(query + 5, "%s", header);
        sort_records(&db1, header);
        return;
    }
    else if(strncmp(query, "OUTERJOIN ", 10)==0) {
        sscanf(query + 10, "%s", header);
        outer_join(header);
        display_result();
    }
    else if(strncmp(query, "FULLJOIN ", 9)==0) {
        sscanf(query + 9, "%s", header);
        full_join(header);
        display_result();
    }
    else {
        printf("Invalid query.\n");
        printf("Valid queries:\n");
        printf("  JOIN col\n");
        printf("  OUTERJOIN col\n");
        printf("  FULLJOIN col\n");
        printf("  SELECT WHERE col=val\n");
        printf("  PROJECT col\n");
        printf("  SORT col\n");
        return;
    }

    clock_t end = clock();
    printf("Query executed successfully.\n");
    printf("Total result records: %d\n", result_db.record_count);
    printf("Execution time: %f seconds\n",
           (double)(end-start)/CLOCKS_PER_SEC);
}

Table* choose_database() {
    char input[20];
    int choice;

    printf("Select database (1 = DB1, 2 = DB2): ");
    fgets(input, sizeof(input), stdin);
    choice = atoi(input);

    if(choice == 1)
        return &db1;
    else if(choice == 2)
        return &db2;
    else {
        printf("Invalid database.\n");
        return NULL;
    }
}
void analyze_table(Table *db) {
    if(db->record_count == 0) {
        printf("No data loaded.\n");
        return;
    }

    printf("\n===== TABLE ANALYSIS =====\n");
    printf("Folder     : %s\n", db->folder);
    printf("Total rows : %d\n", db->record_count);
    printf("Columns found in first record:\n");

    for(int i = 0; i < db->records[0].field_count; i++) {
        /* Count how many records have this field filled */
        int filled = 0;
        for(int r = 0; r < db->record_count; r++) {
            if(strlen(get_value(&db->records[r],
                       db->records[0].fields[i].header)) > 0)
                filled++;
        }
        printf("  [%d] %-15s — %d/%d records have this field\n",
               i+1,
               db->records[0].fields[i].header,
               filled,
               db->record_count);
    }
    printf("==========================\n");
}

int main() {
    setbuf(stdout, NULL);

    int choice;
    char input[100];

    while(1) {
        printf("\n===== MINI DATABASE SYSTEM =====\n");
        printf("1. Load first folder (DB1)\n");
        printf("2. Load second folder (DB2)\n");
        printf("3. Display DB1\n");
        printf("4. Display DB2\n");
printf("5. Sort records\n");
printf("6. Insert new record\n");
printf("7. Delete record\n");
printf("8. Update record\n");
printf("9. Save database files\n");
        printf("10. Inner Join\n");
        printf("11. Outer Join\n");
        printf("12. Full Join\n");
        printf("13. Run Query Language\n");
        printf("14. Save join/query result\n");
        printf("15. Exit\n");
        printf("16. Analyze DB1 (Q1a)\n");
printf("17. Analyze DB2 (Q1a)\n");
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
    display_records(&db2);
}
// In the sort menu section, BEFORE asking for header:
else if(choice == 5) {
    Table *db = choose_database();
    if(db == NULL) continue;

    /* ADD THIS — show available columns */
    if(db->record_count > 0) {
        printf("Available columns: ");
        for(int i=0; i<db->records[0].field_count; i++)
            printf("%s  ", db->records[0].fields[i].header);
        printf("\n");
    }

    printf("Enter header name: ");
    fgets(input, sizeof(input), stdin);
    trim(input);
    sort_records(db, input);
}

else if(choice == 6) {
    Table *db = choose_database();
    if(db == NULL) continue;

    insert_record(db);
}

else if(choice == 7) {
    Table *db = choose_database();
    if(db == NULL) continue;

    printf("Enter record number: ");
    fgets(input, sizeof(input), stdin);

    delete_record(db, atoi(input)-1);
}

else if(choice == 8) {
    Table *db = choose_database();
    if(db == NULL) continue;

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

    update_record(db, index, header, value);
}

else if(choice == 9) {
    Table *db = choose_database();
    if(db == NULL) continue;

    save_files(db);
}

else if(choice == 10) {
    printf("Enter common header: ");
    fgets(input, sizeof(input), stdin);
    trim(input);
    inner_join(input);
    display_result();
}

else if(choice == 11) {
    printf("Enter common header: ");
    fgets(input, sizeof(input), stdin);
    trim(input);
    outer_join(input);
    display_result();
}

else if(choice == 12) {
    printf("Enter common header: ");
    fgets(input, sizeof(input), stdin);
    trim(input);
    full_join(input);
    display_result();
}

else if(choice == 13) {
    printf("Enter query: ");
    fgets(input, sizeof(input), stdin);
    trim(input);
    run_query(input);
}

else if(choice == 14) {
    printf("Enter folder to save result: ");
    fgets(input, sizeof(input), stdin);
    trim(input);
    save_result(input);
}

else if(choice == 15) {
    break;
}
    else if(choice == 16) { analyze_table(&db1); }
else if(choice == 17) { analyze_table(&db2); }
        else {
            printf("Invalid choice.\n");
        }
    }

    return 0;
}
