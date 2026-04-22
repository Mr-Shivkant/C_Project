#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_RECORDS 100
#define MAX_FIELDS  20
#define MAX_TEXT    100

typedef struct {
    char header[MAX_TEXT];
    char value[MAX_TEXT];
} Field;

typedef struct {
    Field fields[MAX_FIELDS];
    int   field_count;
    char  filename[MAX_TEXT];
} Record;

typedef struct {
    Record records[MAX_RECORDS];
    int    record_count;
    char   folder[MAX_TEXT];
} Table;

Table db1;
Table db2;
Table result_db;

/* ─────────────────────────────────────────────
   TRIM
   ───────────────────────────────────────────── */
void trim(char *s) {
    s[strcspn(s, "\n")] = 0;
    int len = strlen(s);
    while(len > 0 && s[len-1] == ' ') s[--len] = '\0';
}

/* ─────────────────────────────────────────────
   GET VALUE
   ───────────────────────────────────────────── */
char* get_value(Record *r, char *header) {
    for(int i = 0; i < r->field_count; i++)
        if(strcmp(r->fields[i].header, header) == 0)
            return r->fields[i].value;
    return "";
}

/* ─────────────────────────────────────────────
   LOAD FOLDER
   ───────────────────────────────────────────── */
void load_folder(Table *db, char *folder) {
    DIR *dir;
    struct dirent *entry;
    char path[200], line[200];

    db->record_count = 0;
    strcpy(db->folder, folder);

    dir = opendir(folder);
    if(dir == NULL) { printf("Folder not found.\n"); return; }

    while((entry = readdir(dir)) != NULL) {
        if(entry->d_name[0] == '.') continue;

        sprintf(path, "%s/%s", folder, entry->d_name);
        FILE *fp = fopen(path, "r");
        if(fp == NULL) continue;

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
                while(r->fields[r->field_count].value[0] == ' ')
                    memmove(r->fields[r->field_count].value,
                            r->fields[r->field_count].value + 1,
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

/* ═════════════════════════════════════════════════════════════
   TABLE FORMAT DISPLAY
   Prints data in neat bordered table like:
   +----------+------+--------+
   | Name     | Roll | Course |
   +----------+------+--------+
   | Shivkant | 24   | MCA    |
   +----------+------+--------+
   ═════════════════════════════════════════════════════════════ */

/* Collect all unique headers from records */
int collect_headers(Record *records, int record_count,
                    char headers[][MAX_TEXT]) {
    int count = 0;
    for(int r = 0; r < record_count; r++) {
        for(int f = 0; f < records[r].field_count; f++) {
            int found = 0;
            for(int h = 0; h < count; h++)
                if(strcmp(headers[h], records[r].fields[f].header) == 0)
                { found = 1; break; }
            if(!found && count < MAX_FIELDS)
                strcpy(headers[count++], records[r].fields[f].header);
        }
    }
    return count;
}

/* Calculate max width needed for each column */
void get_col_widths(Record *records, int record_count,
                    char headers[][MAX_TEXT], int header_count,
                    int *widths) {
    for(int i = 0; i < header_count; i++)
        widths[i] = strlen(headers[i]);

    for(int r = 0; r < record_count; r++)
        for(int h = 0; h < header_count; h++) {
            int vlen = strlen(get_value(&records[r], headers[h]));
            if(vlen > widths[h]) widths[h] = vlen;
        }

    for(int i = 0; i < header_count; i++)
        if(widths[i] < 4) widths[i] = 4;
}

/* Print +----+------+---+ line */
void print_separator(int *widths, int count) {
    printf("+");
    for(int i = 0; i < count; i++) {
        for(int j = 0; j < widths[i] + 2; j++) printf("-");
        printf("+");
    }
    printf("\n");
}

/* Print | Name | Roll | Course | line */
void print_header_row(char headers[][MAX_TEXT], int count, int *widths) {
    printf("|");
    for(int i = 0; i < count; i++)
        printf(" %-*s |", widths[i], headers[i]);
    printf("\n");
}

/* Print one data row */
void print_data_row(Record *r, char headers[][MAX_TEXT],
                    int count, int *widths) {
    printf("|");
    for(int i = 0; i < count; i++) {
        char *val = get_value(r, headers[i]);
        printf(" %-*s |", widths[i], strlen(val) > 0 ? val : "NULL");
    }
    printf("\n");
}

/* Master display function — works for any records array */
void display_table_format(Record *records, int record_count, char *title) {
    if(record_count == 0) {
        printf("No records to display.\n");
        return;
    }

    char headers[MAX_FIELDS][MAX_TEXT];
    int  header_count = collect_headers(records, record_count, headers);

    int widths[MAX_FIELDS];
    get_col_widths(records, record_count, headers, header_count, widths);

    printf("\n=== %s (%d records) ===\n", title, record_count);
    print_separator(widths, header_count);
    print_header_row(headers, header_count, widths);
    print_separator(widths, header_count);
    for(int i = 0; i < record_count; i++)
        print_data_row(&records[i], headers, header_count, widths);
    print_separator(widths, header_count);
}

/* ─────────────────────────────────────────────
   DISPLAY WRAPPERS
   ───────────────────────────────────────────── */
void display_records(Table *db) {
    if(db->record_count == 0) { printf("No data loaded.\n"); return; }
    display_table_format(db->records, db->record_count, db->folder);
}

void display_result() {
    if(result_db.record_count == 0) { printf("No results found.\n"); return; }
    display_table_format(result_db.records, result_db.record_count, "RESULT");
}

void display_sorted_column(Table *db, char *header) {
    printf("\nValues sorted by %s:\n", header);
    for(int i = 0; i < db->record_count; i++)
        printf("Record %d : %s\n", i+1, get_value(&db->records[i], header));
}

/* ─────────────────────────────────────────────
   SORT
   ───────────────────────────────────────────── */
void sort_records(Table *db, char *header) {
    clock_t start = clock();
    for(int i = 0; i < db->record_count-1; i++)
        for(int j = i+1; j < db->record_count; j++)
            if(strcmp(get_value(&db->records[i], header),
                      get_value(&db->records[j], header)) > 0) {
                Record temp    = db->records[i];
                db->records[i] = db->records[j];
                db->records[j] = temp;
            }
    clock_t end = clock();
    printf("Sorted by: %s\n", header);
    printf("Execution time: %f seconds\n", (double)(end-start)/CLOCKS_PER_SEC);
    display_sorted_column(db, header);
}

/* ─────────────────────────────────────────────
   INSERT
   ───────────────────────────────────────────── */
void insert_record(Table *db) {
    if(db->record_count == 0) { printf("No database loaded.\n"); return; }

    Record temp;
    temp.field_count = db->records[0].field_count;
    for(int i = 0; i < temp.field_count; i++) {
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
    printf("Execution time: %f seconds\n", (double)(end-start)/CLOCKS_PER_SEC);
}

/* ─────────────────────────────────────────────
   DELETE
   ───────────────────────────────────────────── */
void delete_record(Table *db, int index) {
    if(index < 0 || index >= db->record_count) {
        printf("Invalid record number.\n"); return;
    }
    clock_t start = clock();
    for(int i = index; i < db->record_count-1; i++)
        db->records[i] = db->records[i+1];
    db->record_count--;
    clock_t end = clock();
    printf("Record deleted.\n");
    printf("Execution time: %f seconds\n", (double)(end-start)/CLOCKS_PER_SEC);
}

/* ─────────────────────────────────────────────
   UPDATE
   ───────────────────────────────────────────── */
void update_record(Table *db, int index, char *header, char *newvalue) {
    if(index < 0 || index >= db->record_count) {
        printf("Invalid record number.\n"); return;
    }
    clock_t start = clock();
    for(int i = 0; i < db->records[index].field_count; i++) {
        if(strcmp(db->records[index].fields[i].header, header) == 0) {
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

/* ─────────────────────────────────────────────
   SAVE FILES
   ───────────────────────────────────────────── */
void save_files(Table *db) {
    char path[200];
    for(int i = 0; i < db->record_count; i++) {
        sprintf(path, "%s/%s", db->folder, db->records[i].filename);
        FILE *fp = fopen(path, "w");
        if(fp == NULL) continue;
        for(int j = 0; j < db->records[i].field_count; j++)
            fprintf(fp, "%s: %s\n",
                    db->records[i].fields[j].header,
                    db->records[i].fields[j].value);
        fclose(fp);
    }
    printf("Saved to files.\n");
}

/* ═════════════════════════════════════════════
   JOIN OPERATIONS
   ═════════════════════════════════════════════ */
void inner_join(char *header) {
    clock_t start = clock();
    result_db.record_count = 0;

    for(int i = 0; i < db1.record_count; i++)
        for(int j = 0; j < db2.record_count; j++)
            if(strcmp(get_value(&db1.records[i], header),
                      get_value(&db2.records[j], header)) == 0) {
                Record *r = &result_db.records[result_db.record_count];
                r->field_count = 0;
                for(int k = 0; k < db1.records[i].field_count; k++)
                    r->fields[r->field_count++] = db1.records[i].fields[k];
                for(int k = 0; k < db2.records[j].field_count; k++)
                    if(strcmp(db2.records[j].fields[k].header, header) != 0)
                        r->fields[r->field_count++] = db2.records[j].fields[k];
                result_db.record_count++;
            }

    clock_t end = clock();
    printf("Inner join: %d record(s) matched.\n", result_db.record_count);
    printf("Execution time: %f seconds\n", (double)(end-start)/CLOCKS_PER_SEC);
}

void outer_join(char *header) {
    clock_t start = clock();
    result_db.record_count = 0;

    for(int i = 0; i < db1.record_count; i++) {
        int found = 0;
        for(int j = 0; j < db2.record_count; j++)
            if(strcmp(get_value(&db1.records[i], header),
                      get_value(&db2.records[j], header)) == 0) {
                Record *r = &result_db.records[result_db.record_count];
                r->field_count = 0;
                for(int k = 0; k < db1.records[i].field_count; k++)
                    r->fields[r->field_count++] = db1.records[i].fields[k];
                for(int k = 0; k < db2.records[j].field_count; k++)
                    if(strcmp(db2.records[j].fields[k].header, header) != 0)
                        r->fields[r->field_count++] = db2.records[j].fields[k];
                result_db.record_count++;
                found = 1;
            }
        if(found == 0)
            result_db.records[result_db.record_count++] = db1.records[i];
    }

    clock_t end = clock();
    printf("Outer join: %d record(s) in result.\n", result_db.record_count);
    printf("Execution time: %f seconds\n", (double)(end-start)/CLOCKS_PER_SEC);
}

void full_join(char *header) {
    clock_t start = clock();
    int matched[MAX_RECORDS] = {0};
    result_db.record_count = 0;

    for(int i = 0; i < db1.record_count; i++) {
        int found = 0;
        for(int j = 0; j < db2.record_count; j++)
            if(strcmp(get_value(&db1.records[i], header),
                      get_value(&db2.records[j], header)) == 0) {
                Record *r = &result_db.records[result_db.record_count];
                r->field_count = 0;
                for(int k = 0; k < db1.records[i].field_count; k++)
                    r->fields[r->field_count++] = db1.records[i].fields[k];
                for(int k = 0; k < db2.records[j].field_count; k++)
                    if(strcmp(db2.records[j].fields[k].header, header) != 0)
                        r->fields[r->field_count++] = db2.records[j].fields[k];
                matched[j] = 1;
                result_db.record_count++;
                found = 1;
            }
        if(found == 0)
            result_db.records[result_db.record_count++] = db1.records[i];
    }
    for(int j = 0; j < db2.record_count; j++)
        if(matched[j] == 0)
            result_db.records[result_db.record_count++] = db2.records[j];

    clock_t end = clock();
    printf("Full join: %d record(s) in result.\n", result_db.record_count);
    printf("Execution time: %f seconds\n", (double)(end-start)/CLOCKS_PER_SEC);
}

/* ─────────────────────────────────────────────
   SAVE RESULT
   ───────────────────────────────────────────── */
void save_result(char *folder) {
    char path[200];
    for(int i = 0; i < result_db.record_count; i++) {
        sprintf(path, "%s/result%d.txt", folder, i+1);
        FILE *fp = fopen(path, "w");
        if(fp == NULL) continue;
        for(int j = 0; j < result_db.records[i].field_count; j++)
            fprintf(fp, "%s: %s\n",
                    result_db.records[i].fields[j].header,
                    result_db.records[i].fields[j].value);
        fclose(fp);
    }
    printf("Result saved.\n");
}

/* ═════════════════════════════════════════════
   QUERY LANGUAGE
   ═════════════════════════════════════════════ */
void select_records(char *header, char *value) {
    result_db.record_count = 0;
    while(header[0] == ' ') memmove(header, header+1, strlen(header));
    while(value[0]  == ' ') memmove(value,  value+1,  strlen(value));

    for(int i = 0; i < db1.record_count; i++)
        if(strcmp(get_value(&db1.records[i], header), value) == 0)
            result_db.records[result_db.record_count++] = db1.records[i];

    if(result_db.record_count == 0)
        printf("No matching records found for %s=%s\n", header, value);
    else
        printf("Selection done. %d record(s) found.\n", result_db.record_count);
}

void project_records(char *header) {
    result_db.record_count = 0;
    while(header[0] == ' ') memmove(header, header+1, strlen(header));

    for(int i = 0; i < db1.record_count; i++) {
        Record *r = &result_db.records[result_db.record_count];
        r->field_count = 0;
        for(int j = 0; j < db1.records[i].field_count; j++)
            if(strcmp(db1.records[i].fields[j].header, header) == 0)
                r->fields[r->field_count++] = db1.records[i].fields[j];
        result_db.record_count++;
    }
    printf("Projection done.\n");
}

void run_query(char *query) {
    char header[100], value[100];
    clock_t start = clock();

    if(strncmp(query, "JOIN ", 5) == 0) {
        sscanf(query+5, "%s", header);
        inner_join(header);
        display_result();
    }
    else if(strncmp(query, "OUTERJOIN ", 10) == 0) {
        sscanf(query+10, "%s", header);
        outer_join(header);
        display_result();
    }
    else if(strncmp(query, "FULLJOIN ", 9) == 0) {
        sscanf(query+9, "%s", header);
        full_join(header);
        display_result();
    }
    else if(strncmp(query, "PROJECT ", 8) == 0) {
        sscanf(query+8, "%s", header);
        project_records(header);
        display_result();
    }
    else if(strncmp(query, "SELECT WHERE ", 13) == 0) {
        sscanf(query+13, "%[^=]=%s", header, value);
        select_records(header, value);
        display_result();
    }
    else if(strncmp(query, "SORT ", 5) == 0) {
        sscanf(query+5, "%s", header);
        sort_records(&db1, header);
        return;
    }
    else {
        printf("Invalid query. Valid queries:\n");
        printf("  JOIN col\n  OUTERJOIN col\n  FULLJOIN col\n");
        printf("  SELECT WHERE col=val\n  PROJECT col\n  SORT col\n");
        return;
    }

    clock_t end = clock();
    printf("Query executed. Total result records: %d\n", result_db.record_count);
    printf("Execution time: %f seconds\n", (double)(end-start)/CLOCKS_PER_SEC);
}

/* ─────────────────────────────────────────────
   ANALYZE TABLE
   ───────────────────────────────────────────── */
void analyze_table(Table *db) {
    if(db->record_count == 0) { printf("No data loaded.\n"); return; }
    printf("\n===== TABLE ANALYSIS =====\n");
    printf("Folder     : %s\n", db->folder);
    printf("Total rows : %d\n", db->record_count);
    printf("Columns:\n");
    for(int i = 0; i < db->records[0].field_count; i++) {
        int filled = 0;
        for(int r = 0; r < db->record_count; r++)
            if(strlen(get_value(&db->records[r],
                       db->records[0].fields[i].header)) > 0) filled++;
        printf("  [%d] %-15s - %d/%d filled\n",
               i+1, db->records[0].fields[i].header,
               filled, db->record_count);
    }
    printf("==========================\n");
}

/* ─────────────────────────────────────────────
   CHOOSE DATABASE
   ───────────────────────────────────────────── */
Table* choose_database() {
    char input[20];
    printf("Select database (1=DB1, 2=DB2): ");
    fgets(input, sizeof(input), stdin);
    int c = atoi(input);
    if(c == 1) return &db1;
    if(c == 2) return &db2;
    printf("Invalid database.\n");
    return NULL;
}

/* ═════════════════════════════════════════════
   MAIN
   ═════════════════════════════════════════════ */
int main() {
    setbuf(stdout, NULL);
    int  choice;
    char input[100];

    while(1) {
        printf("\n===== MINI DATABASE SYSTEM =====\n");
        printf("1.  Load first folder  (DB1)\n");
        printf("2.  Load second folder (DB2)\n");
        printf("3.  Display DB1\n");
        printf("4.  Display DB2\n");
        printf("5.  Sort records\n");
        printf("6.  Insert new record\n");
        printf("7.  Delete record\n");
        printf("8.  Update record\n");
        printf("9.  Save database files\n");
        printf("10. Inner Join  (DB1 + DB2)\n");
        printf("11. Outer Join  (DB1 + DB2)\n");
        printf("12. Full Join   (DB1 + DB2)\n");
        printf("13. Run Query Language\n");
        printf("14. Save join/query result\n");
        printf("15. Exit\n");
        printf("16. Analyze DB1\n");
        printf("17. Analyze DB2\n");
        printf("================================\n");
        printf("Enter your choice: ");

        fgets(input, sizeof(input), stdin);
        choice = atoi(input);

        if(choice == 1) {
            printf("Enter folder for DB1: ");
            fgets(input, sizeof(input), stdin); trim(input);
            load_folder(&db1, input);
        }
        else if(choice == 2) {
            printf("Enter folder for DB2: ");
            fgets(input, sizeof(input), stdin); trim(input);
            load_folder(&db2, input);
        }
        else if(choice == 3) { display_records(&db1); }
        else if(choice == 4) { display_records(&db2); }
        else if(choice == 5) {
            Table *db = choose_database();
            if(db == NULL) continue;
            if(db->record_count > 0) {
                printf("Available columns: ");
                for(int i = 0; i < db->records[0].field_count; i++)
                    printf("%s  ", db->records[0].fields[i].header);
                printf("\n");
            }
            printf("Enter header name to sort by: ");
            fgets(input, sizeof(input), stdin); trim(input);
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
            printf("Enter record number to delete: ");
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
            fgets(header, sizeof(header), stdin); trim(header);
            printf("Enter new value: ");
            fgets(value, sizeof(value), stdin); trim(value);
            update_record(db, index, header, value);
        }
        else if(choice == 9) {
            Table *db = choose_database();
            if(db == NULL) continue;
            save_files(db);
        }
        else if(choice == 10) {
            if(db1.record_count==0 || db2.record_count==0) {
                printf("Load BOTH DB1 and DB2 first!\n"); continue;
            }
            printf("Enter common header: ");
            fgets(input, sizeof(input), stdin); trim(input);
            inner_join(input);
            display_result();
        }
        else if(choice == 11) {
            if(db1.record_count==0 || db2.record_count==0) {
                printf("Load BOTH DB1 and DB2 first!\n"); continue;
            }
            printf("Enter common header: ");
            fgets(input, sizeof(input), stdin); trim(input);
            outer_join(input);
            display_result();
        }
        else if(choice == 12) {
            if(db1.record_count==0 || db2.record_count==0) {
                printf("Load BOTH DB1 and DB2 first!\n"); continue;
            }
            printf("Enter common header: ");
            fgets(input, sizeof(input), stdin); trim(input);
            full_join(input);
            display_result();
        }
        else if(choice == 13) {
            printf("\nQuery Help:\n");
            printf("  JOIN col           inner join DB1+DB2\n");
            printf("  OUTERJOIN col      outer join DB1+DB2\n");
            printf("  FULLJOIN col       full  join DB1+DB2\n");
            printf("  SELECT WHERE col=val   filter DB1\n");
            printf("  PROJECT col        show one column\n");
            printf("  SORT col           sort DB1\n");
            printf("\nEnter query: ");
            fgets(input, sizeof(input), stdin); trim(input);
            run_query(input);
        }
        else if(choice == 14) {
            printf("Enter folder to save result: ");
            fgets(input, sizeof(input), stdin); trim(input);
            save_result(input);
        }
        else if(choice == 15) { printf("Goodbye!\n"); break; }
        else if(choice == 16) { analyze_table(&db1); }
        else if(choice == 17) { analyze_table(&db2); }
        else { printf("Invalid choice.\n"); }
    }
    return 0;
}
