/*
 * DSUC + DBMS Assignment 1 — MCA 1st Year, HBTU Kanpur
 * Mini Database System — Updated Version
 * Language: C only
 *
 * FIXES & UPDATES:
 *   1. sort_records() — now displays FULL table (all columns) after sort, not just sorted column
 *   2. JOIN — upgraded with Inner / Left / Right / Full outer join (from Code 1)
 *   3. SSQL query shell — SELECT, WHERE, PROJECT, JOIN with type, SORT, SHOW, SAVE, HELP
 *
 * Group Students   : Shivkant, Pankaj Kumar, Rupesh Parmar, Priyanka, Prachi Mishra
 * Students Roll No : 250231063, 250231040, 250231055, 250231047, 250231044
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

/* ─── limits ─────────────────────────────────────────── */
#define MAX_RECORDS 200
#define MAX_FIELDS   20
#define MAX_TEXT    100
#define MAX_PATH    300

/* ─── structures ─────────────────────────────────────── */
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
    char   folder[MAX_PATH];
} Table;

/* global tables */
Table db1;
Table db2;
Table result_db;

/* ══════════════════════════════════════════════════════
   HELPERS
   ══════════════════════════════════════════════════════ */

void trim(char *s) {
    s[strcspn(s, "\n\r")] = 0;
    int len = strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == ','))
        s[--len] = '\0';
    while (*s == ' ') memmove(s, s+1, strlen(s)+1);
}

/* case-insensitive string comparison helper */
int str_eq_ci(const char *a, const char *b) {
    char A[MAX_TEXT], B[MAX_TEXT];
    strncpy(A, a, MAX_TEXT-1); A[MAX_TEXT-1] = '\0';
    strncpy(B, b, MAX_TEXT-1); B[MAX_TEXT-1] = '\0';
    for (int i = 0; A[i]; i++) if (A[i]>='a'&&A[i]<='z') A[i]-=32;
    for (int i = 0; B[i]; i++) if (B[i]>='a'&&B[i]<='z') B[i]-=32;
    return strcmp(A, B) == 0;
}

char* get_value(Record *r, const char *header) {
    for (int i = 0; i < r->field_count; i++)
        if (str_eq_ci(r->fields[i].header, header))
            return r->fields[i].value;
    return "";
}

/* ══════════════════════════════════════════════════════
   LOAD FOLDER
   ══════════════════════════════════════════════════════ */
void load_folder(Table *db, char *folder) {
    DIR *dir;
    struct dirent *entry;
    char path[MAX_PATH], line[MAX_TEXT*2];

    db->record_count = 0;
    strncpy(db->folder, folder, MAX_PATH-1);

    dir = opendir(folder);
    if (!dir) { printf("  [ERROR] Folder not found: %s\n", folder); return; }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        if (db->record_count >= MAX_RECORDS) break;

        snprintf(path, MAX_PATH, "%s/%s", folder, entry->d_name);
        FILE *fp = fopen(path, "r");
        if (!fp) continue;

        Record *r = &db->records[db->record_count];
        r->field_count = 0;
        strncpy(r->filename, entry->d_name, MAX_TEXT-1);

        while (fgets(line, sizeof(line), fp) && r->field_count < MAX_FIELDS) {
            trim(line);
            char *colon = strchr(line, ':');
            if (!colon) continue;
            *colon = '\0';
            strncpy(r->fields[r->field_count].header, line, MAX_TEXT-1);
            trim(r->fields[r->field_count].header);
            strncpy(r->fields[r->field_count].value, colon+1, MAX_TEXT-1);
            trim(r->fields[r->field_count].value);
            /* strip leading space after colon */
            char *v = r->fields[r->field_count].value;
            while (*v == ' ') memmove(v, v+1, strlen(v)+1);
            r->field_count++;
        }
        fclose(fp);
        db->record_count++;
    }
    closedir(dir);
    printf("  Loaded %d records from [%s]\n", db->record_count, folder);
}

/* ══════════════════════════════════════════════════════
   TABLE FORMAT DISPLAY  (pretty bordered table)
   ══════════════════════════════════════════════════════ */

int collect_headers(Record *records, int count, char headers[][MAX_TEXT]) {
    int n = 0;
    for (int r = 0; r < count; r++)
        for (int f = 0; f < records[r].field_count; f++) {
            int found = 0;
            for (int h = 0; h < n; h++)
                if (str_eq_ci(headers[h], records[r].fields[f].header))
                { found = 1; break; }
            if (!found && n < MAX_FIELDS)
                strncpy(headers[n++], records[r].fields[f].header, MAX_TEXT-1);
        }
    return n;
}

void get_col_widths(Record *records, int count,
                    char headers[][MAX_TEXT], int hcount, int *widths) {
    for (int i = 0; i < hcount; i++) widths[i] = (int)strlen(headers[i]);
    for (int r = 0; r < count; r++)
        for (int h = 0; h < hcount; h++) {
            int vl = (int)strlen(get_value(&records[r], headers[h]));
            if (vl > widths[h]) widths[h] = vl;
        }
    for (int i = 0; i < hcount; i++) if (widths[i] < 4) widths[i] = 4;
}

void print_sep(int *w, int n) {
    printf("+");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < w[i]+2; j++) printf("-");
        printf("+");
    }
    printf("\n");
}

void display_table_format(Record *records, int count, const char *title) {
    if (count == 0) { printf("  (no records)\n"); return; }

    char headers[MAX_FIELDS][MAX_TEXT];
    int  hcount = collect_headers(records, count, headers);
    int  widths[MAX_FIELDS];
    get_col_widths(records, count, headers, hcount, widths);

    printf("\n=== %s (%d record%s) ===\n", title, count, count==1?"":"s");
    print_sep(widths, hcount);
    /* header row */
    printf("|");
    for (int i = 0; i < hcount; i++) printf(" %-*s |", widths[i], headers[i]);
    printf("\n");
    print_sep(widths, hcount);
    /* data rows */
    for (int r = 0; r < count; r++) {
        printf("|");
        for (int h = 0; h < hcount; h++) {
            char *v = get_value(&records[r], headers[h]);
            printf(" %-*s |", widths[h], strlen(v)>0 ? v : "NULL");
        }
        printf("\n");
    }
    print_sep(widths, hcount);
}

void display_records(Table *db) {
    if (db->record_count == 0) { printf("  No data loaded.\n"); return; }
    display_table_format(db->records, db->record_count, db->folder);
}

void display_result() {
    if (result_db.record_count == 0) { printf("  No results.\n"); return; }
    display_table_format(result_db.records, result_db.record_count, "RESULT");
}

/* ══════════════════════════════════════════════════════
   SORT  — FIX: shows ALL columns after sort, not just one
   ══════════════════════════════════════════════════════ */
void sort_records(Table *db, char *header) {
    if (db->record_count == 0) { printf("  No data.\n"); return; }

    /* verify column exists */
    int found = 0;
    for (int f = 0; f < db->records[0].field_count; f++)
        if (str_eq_ci(db->records[0].fields[f].header, header))
        { found = 1; break; }
    if (!found) { printf("  Column '%s' not found.\n", header); return; }

    clock_t start = clock();

    /* selection sort (easy to read) */
    for (int i = 0; i < db->record_count - 1; i++)
        for (int j = i+1; j < db->record_count; j++)
            if (strcmp(get_value(&db->records[i], header),
                       get_value(&db->records[j], header)) > 0) {
                Record tmp      = db->records[i];
                db->records[i]  = db->records[j];
                db->records[j]  = tmp;
            }

    double ms = (double)(clock()-start)/CLOCKS_PER_SEC*1000.0;
    printf("  [TIME] Sorted by '%s': %.4f ms\n", header, ms);

    /* ── show FULL table (all columns) ── */
    display_table_format(db->records, db->record_count, db->folder);
}

/* ══════════════════════════════════════════════════════
   INSERT
   ══════════════════════════════════════════════════════ */
void insert_record(Table *db) {
    if (db->record_count == 0) { printf("  No database loaded.\n"); return; }
    if (db->record_count >= MAX_RECORDS) { printf("  Table full.\n"); return; }

    Record temp;
    temp.field_count = db->records[0].field_count;
    /* flush leftover newline */
    int ch; while ((ch=getchar())!='\n' && ch!=EOF);

    for (int i = 0; i < temp.field_count; i++) {
        strncpy(temp.fields[i].header, db->records[0].fields[i].header, MAX_TEXT-1);
        printf("  %s: ", temp.fields[i].header);
        fgets(temp.fields[i].value, MAX_TEXT, stdin);
        trim(temp.fields[i].value);
    }

    clock_t start = clock();
    db->records[db->record_count] = temp;
    snprintf(db->records[db->record_count].filename, MAX_TEXT,
             "record%d.txt", db->record_count+1);
    db->record_count++;
    printf("  [TIME] Insert: %.4f ms | Rows now: %d\n",
           (double)(clock()-start)/CLOCKS_PER_SEC*1000.0, db->record_count);
}

/* ══════════════════════════════════════════════════════
   DELETE
   ══════════════════════════════════════════════════════ */
void delete_record(Table *db, int index) {
    if (index < 0 || index >= db->record_count) {
        printf("  Invalid record number.\n"); return;
    }
    clock_t start = clock();
    for (int i = index; i < db->record_count-1; i++)
        db->records[i] = db->records[i+1];
    db->record_count--;
    printf("  [TIME] Delete: %.4f ms | Rows now: %d\n",
           (double)(clock()-start)/CLOCKS_PER_SEC*1000.0, db->record_count);
}

/* ══════════════════════════════════════════════════════
   UPDATE
   ══════════════════════════════════════════════════════ */
void update_record(Table *db, int index, char *header, char *newvalue) {
    if (index < 0 || index >= db->record_count) {
        printf("  Invalid record number.\n"); return;
    }
    clock_t start = clock();
    for (int i = 0; i < db->records[index].field_count; i++) {
        if (str_eq_ci(db->records[index].fields[i].header, header)) {
            strncpy(db->records[index].fields[i].value, newvalue, MAX_TEXT-1);
            printf("  [TIME] Update '%s': %.4f ms\n", header,
                   (double)(clock()-start)/CLOCKS_PER_SEC*1000.0);
            return;
        }
    }
    printf("  Column '%s' not found.\n", header);
}

/* ══════════════════════════════════════════════════════
   SAVE FILES
   ══════════════════════════════════════════════════════ */
void save_files(Table *db) {
    char path[MAX_PATH];
    for (int i = 0; i < db->record_count; i++) {
        snprintf(path, MAX_PATH, "%s/%s", db->folder, db->records[i].filename);
        FILE *fp = fopen(path, "w");
        if (!fp) continue;
        for (int j = 0; j < db->records[i].field_count; j++)
            fprintf(fp, "%s: %s\n",
                    db->records[i].fields[j].header,
                    db->records[i].fields[j].value);
        fclose(fp);
    }
    printf("  Saved %d records to [%s]\n", db->record_count, db->folder);
}

void save_result_to(char *folder) {
    char cmd[MAX_PATH+10];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", folder);
    system(cmd);
    char path[MAX_PATH];
    for (int i = 0; i < result_db.record_count; i++) {
        snprintf(path, MAX_PATH, "%s/result%d.txt", folder, i+1);
        FILE *fp = fopen(path, "w");
        if (!fp) continue;
        for (int j = 0; j < result_db.records[i].field_count; j++)
            fprintf(fp, "%s: %s\n",
                    result_db.records[i].fields[j].header,
                    result_db.records[i].fields[j].value);
        fclose(fp);
    }
    printf("  Result saved to [%s]\n", folder);
}

/* ══════════════════════════════════════════════════════
   JOIN ENGINE  (upgraded from Code-1)
   type: 0=Inner  1=Left  2=Right  3=Full
   ══════════════════════════════════════════════════════ */
void do_join(Table *t1, Table *t2, char *on_col, int type) {
    memset(&result_db, 0, sizeof(Table));

    int c1 = -1, c2 = -1;

    /* find join column index in t1 */
    if (t1->record_count > 0)
        for (int j = 0; j < t1->records[0].field_count; j++)
            if (str_eq_ci(t1->records[0].fields[j].header, on_col)) {
                c1 = j;
                break;
            }

    /* find join column index in t2 */
    if (t2->record_count > 0)
        for (int j = 0; j < t2->records[0].field_count; j++)
            if (str_eq_ci(t2->records[0].fields[j].header, on_col)) {
                c2 = j;
                break;
            }

    if (c1 < 0 || c2 < 0) {
        printf("  [ERROR] Column '%s' not found in both tables.\n", on_col);
        return;
    }

    int used2[MAX_RECORDS] = {0};
    clock_t start = clock();

    for (int i = 0; i < t1->record_count; i++) {
        int matched = 0;

        for (int k = 0; k < t2->record_count; k++) {
            if (strcmp(t1->records[i].fields[c1].value,
                       t2->records[k].fields[c2].value) != 0)
                continue;

            Record *r = &result_db.records[result_db.record_count];
            r->field_count = 0;

            /* copy table1 fields */
            for (int j = 0; j < t1->records[i].field_count; j++)
                r->fields[r->field_count++] = t1->records[i].fields[j];

            /* copy table2 fields except duplicate join column */
            for (int j = 0; j < t2->records[k].field_count; j++) {
                if (j == c2) continue;
                r->fields[r->field_count++] = t2->records[k].fields[j];
            }

            result_db.record_count++;
            matched = 1;
            used2[k] = 1;
        }

        /* LEFT or FULL join unmatched rows from t1 */
        if (!matched && (type == 1 || type == 3)) {
            Record *r = &result_db.records[result_db.record_count];
            r->field_count = 0;

            /* copy t1 row */
            for (int j = 0; j < t1->records[i].field_count; j++)
                r->fields[r->field_count++] = t1->records[i].fields[j];

            /* fill t2 columns with NULL */
            for (int j = 0; j < t2->records[0].field_count; j++) {
                if (j == c2) continue;
                strcpy(r->fields[r->field_count].header,
                       t2->records[0].fields[j].header);
                strcpy(r->fields[r->field_count].value, "NULL");
                r->field_count++;
            }

            result_db.record_count++;
        }
    }

    /* RIGHT or FULL join unmatched rows from t2 */
    if (type == 2 || type == 3) {
        for (int k = 0; k < t2->record_count; k++) {
            if (used2[k]) continue;

            Record *r = &result_db.records[result_db.record_count];
            r->field_count = 0;

            /* fill t1 columns with NULL */
            for (int j = 0; j < t1->records[0].field_count; j++) {
                strcpy(r->fields[r->field_count].header,
                       t1->records[0].fields[j].header);
                strcpy(r->fields[r->field_count].value,
                       j == c1 ? t2->records[k].fields[c2].value : "NULL");
                r->field_count++;
            }

            /* copy t2 row except join column */
            for (int j = 0; j < t2->records[k].field_count; j++) {
                if (j == c2) continue;
                r->fields[r->field_count++] = t2->records[k].fields[j];
            }

            result_db.record_count++;
        }
    }

    char *names[] = {"Inner", "Left", "Right", "Full"};

    printf("  [TIME] %s Join on '%s': %.4f ms | Result: %d rows\n",
           names[type],
           on_col,
           (double)(clock() - start) / CLOCKS_PER_SEC * 1000.0,
           result_db.record_count);
}
/* ══════════════════════════════════════════════════════
   QUERY OPERATIONS
   ══════════════════════════════════════════════════════ */

/* SELECT WHERE col=val — filter db1 */
void select_records(char *header, char *value) {
    result_db.record_count = 0;
    for (int i = 0; i < db1.record_count; i++)
        if (strcmp(get_value(&db1.records[i], header), value) == 0)
            result_db.records[result_db.record_count++] = db1.records[i];
    printf("  Selection: %d record(s) found for %s = %s\n",
           result_db.record_count, header, value);
}

/* PROJECT col — show only one column from db1 */
void project_records(char *header) {
    result_db.record_count = 0;
    for (int i = 0; i < db1.record_count; i++) {
        Record *r = &result_db.records[result_db.record_count];
        r->field_count = 0;
        for (int j = 0; j < db1.records[i].field_count; j++)
            if (str_eq_ci(db1.records[i].fields[j].header, header)) {
                r->fields[r->field_count++] = db1.records[i].fields[j];
                break;
            }
        result_db.record_count++;
    }
    printf("  Projection on '%s' done.\n", header);
}

/* ══════════════════════════════════════════════════════
   SSQL QUERY LANGUAGE  (upgraded)
   ══════════════════════════════════════════════════════
   Commands:
     SELECT * FROM db1
     SELECT col1,col2 FROM db1
     SELECT * FROM db1 WHERE col=value
     PROJECT col
     SORT col
     JOIN col TYPE INNER|LEFT|RIGHT|FULL
     SHOW
     SAVE <folder>
     HELP / EXIT
   ══════════════════════════════════════════════════════ */
void run_query(char *raw) {
    /* make uppercase copy for keyword matching */
    char up[512];
    strncpy(up, raw, 511); up[511] = '\0';
    for (int i = 0; up[i]; i++) if (up[i]>='a'&&up[i]<='z') up[i]-=32;

    clock_t qs = clock();

    /* ── HELP ── */
    if (strncmp(up, "HELP", 4) == 0) {
        printf("\n--- SSQL Commands ---\n");
        printf("  SELECT * FROM db1|db2\n");
        printf("  SELECT col1,col2 FROM db1|db2\n");
        printf("  SELECT * FROM db1|db2 WHERE col=value\n");
        printf("  PROJECT col\n");
        printf("  SORT col\n");
        printf("  JOIN col TYPE INNER|LEFT|RIGHT|FULL\n");
        printf("  SHOW\n");
        printf("  SAVE <folder>\n");
        printf("  EXIT\n\n");
        return;
    }

    /* ── SHOW ── */
    if (strncmp(up, "SHOW", 4) == 0) {
        printf("  DB1: [%s]  rows=%d\n", db1.folder, db1.record_count);
        printf("  DB2: [%s]  rows=%d\n", db2.folder, db2.record_count);
        printf("  RES: rows=%d\n", result_db.record_count);
        return;
    }

    /* ── SAVE ── */
    if (strncmp(up, "SAVE", 4) == 0) {
        char folder[MAX_PATH] = "";
        sscanf(raw+5, "%299s", folder);
        if (strlen(folder) == 0) { printf("  Usage: SAVE <folder>\n"); return; }
        save_result_to(folder);
        return;
    }

    /* ── SORT col ── */
    if (strncmp(up, "SORT", 4) == 0) {
        char col[MAX_TEXT] = "";
        sscanf(raw+5, "%99s", col);
        if (strlen(col) == 0) { printf("  Usage: SORT col\n"); return; }
        sort_records(&db1, col);
        return;
    }

    /* ── PROJECT col ── */
    if (strncmp(up, "PROJECT", 7) == 0) {
        char col[MAX_TEXT] = "";
        sscanf(raw+8, "%99s", col);
        if (strlen(col) == 0) { printf("  Usage: PROJECT col\n"); return; }
        project_records(col);
        display_result();
        goto show_time;
    }

    /* ── JOIN col TYPE INNER|LEFT|RIGHT|FULL ── */
    if (strncmp(up, "JOIN", 4) == 0) {
        if (db1.record_count == 0 || db2.record_count == 0) {
            printf("  Load both DB1 and DB2 first.\n"); return;
        }
        char col[MAX_TEXT] = "", type_s[MAX_TEXT] = "INNER";
        sscanf(raw, "%*s %99s %*s %99s", col, type_s);
        if (strlen(col) == 0) {
            printf("  Usage: JOIN col TYPE INNER|LEFT|RIGHT|FULL\n"); return;
        }
        for (int i = 0; type_s[i]; i++) if (type_s[i]>='a'&&type_s[i]<='z') type_s[i]-=32;
        int type = 0;
        if (strcmp(type_s,"LEFT")==0)  type = 1;
        if (strcmp(type_s,"RIGHT")==0) type = 2;
        if (strcmp(type_s,"FULL")==0)  type = 3;
        do_join(&db1, &db2, col, type);
        display_result();
        goto show_time;
    }

    /* ── SELECT [cols|*] FROM db1|db2 [WHERE col=val] ── */
    if (strncmp(up, "SELECT", 6) == 0) {
        /* find FROM */
        char *fp = strstr(up, " FROM ");
        if (!fp) { printf("  Missing FROM\n"); return; }
        int fpos = fp - up;

        /* column list */
        char collist[MAX_TEXT*MAX_FIELDS];
        int clen = fpos - 7;
        if (clen <= 0) { printf("  Bad SELECT syntax\n"); return; }
        strncpy(collist, raw+7, clen); collist[clen] = '\0'; trim(collist);

        /* rest after FROM */
        char rest[512];
        strncpy(rest, raw+fpos+6, 511); rest[511]='\0'; trim(rest);

        /* check for WHERE */
        char rest_up[512];
        strncpy(rest_up, rest, 511);
        for (int i = 0; rest_up[i]; i++) if (rest_up[i]>='a'&&rest_up[i]<='z') rest_up[i]-=32;

        char db_name[MAX_PATH] = "", wcol[MAX_TEXT] = "", wval[MAX_TEXT] = "";
        char *wp = strstr(rest_up, " WHERE ");
        if (wp) {
            int wpos = wp - rest_up;
            strncpy(db_name, rest, wpos); db_name[wpos]='\0'; trim(db_name);
            char cond[MAX_TEXT*2];
            strncpy(cond, rest+wpos+7, sizeof(cond)-1);
            char *eq = strchr(cond, '=');
            if (eq) {
                *eq = '\0';
                strncpy(wcol, cond, MAX_TEXT-1); trim(wcol);
                strncpy(wval, eq+1,  MAX_TEXT-1); trim(wval);
            }
        } else {
            strncpy(db_name, rest, MAX_PATH-1);
        }

        /* pick source table */
        Table *src = &db1;
        char dbu[16]; strncpy(dbu, db_name, 15);
        for (int i = 0; dbu[i]; i++) if (dbu[i]>='a'&&dbu[i]<='z') dbu[i]-=32;
        if (strcmp(dbu,"DB2")==0) src = &db2;

        if (src->record_count == 0) { printf("  Source table is empty.\n"); return; }

        /* resolve columns */
        char cols[MAX_FIELDS][MAX_TEXT];
        int  nc = 0;
        char tmp[MAX_TEXT*MAX_FIELDS];
        strncpy(tmp, collist, sizeof(tmp)-1);
        char *tok = strtok(tmp, ",");
        while (tok && nc < MAX_FIELDS) {
            trim(tok);
            strncpy(cols[nc++], tok, MAX_TEXT-1);
            tok = strtok(NULL, ",");
        }
        int all_cols = (nc==1 && strcmp(cols[0],"*")==0);

        /* build result */
        result_db.record_count = 0;
        for (int i = 0; i < src->record_count; i++) {
            /* apply WHERE filter */
            if (strlen(wcol) > 0 &&
                strcmp(get_value(&src->records[i], wcol), wval) != 0) continue;
            /* copy selected columns */
            Record *r = &result_db.records[result_db.record_count];
            r->field_count = 0;
            if (all_cols) {
                *r = src->records[i];
            } else {
                for (int c = 0; c < nc; c++) {
                    char *v = get_value(&src->records[i], cols[c]);
                    strncpy(r->fields[r->field_count].header, cols[c], MAX_TEXT-1);
                    strncpy(r->fields[r->field_count].value,  v,       MAX_TEXT-1);
                    r->field_count++;
                }
            }
            result_db.record_count++;
        }

        printf("  SELECT result: %d row(s)\n", result_db.record_count);
        display_result();
        goto show_time;
    }

    /* ── SELECT WHERE (old shorthand) ── */
    if (strncmp(up, "SELECT WHERE", 12) == 0) {
        char header[MAX_TEXT], value[MAX_TEXT];
        if (sscanf(raw+13, "%[^=]=%99s", header, value) == 2) {
            trim(header); trim(value);
            select_records(header, value);
            display_result();
        } else {
            printf("  Usage: SELECT WHERE col=val\n");
        }
        goto show_time;
    }

    /* ── OUTERJOIN (legacy shorthand) ── */
    if (strncmp(up, "OUTERJOIN", 9) == 0) {
        char col[MAX_TEXT] = ""; sscanf(raw+10, "%99s", col);
        do_join(&db1, &db2, col, 1);  /* Left join */
        display_result();
        goto show_time;
    }

    /* ── FULLJOIN (legacy shorthand) ── */
    if (strncmp(up, "FULLJOIN", 8) == 0) {
        char col[MAX_TEXT] = ""; sscanf(raw+9, "%99s", col);
        do_join(&db1, &db2, col, 3);  /* Full join */
        display_result();
        goto show_time;
    }

    printf("  Unknown command. Type HELP.\n");
    return;

show_time:
    printf("  [TIME] Query: %.4f ms\n",
           (double)(clock()-qs)/CLOCKS_PER_SEC*1000.0);
}

/* ══════════════════════════════════════════════════════
   SSQL SHELL
   ══════════════════════════════════════════════════════ */
void query_shell() {
    char q[512];
    printf("\n=== SSQL Shell === (type HELP for commands, EXIT to quit)\n");
    /* flush */
    int ch; while ((ch=getchar())!='\n' && ch!=EOF);
    while (1) {
        printf("ssql> ");
        if (!fgets(q, 512, stdin)) break;
        q[strcspn(q, "\n")] = '\0';
        trim(q);
        if (strlen(q) == 0) continue;
        char up[512]; strncpy(up, q, 511);
        for (int i = 0; up[i]; i++) if (up[i]>='a'&&up[i]<='z') up[i]-=32;
        if (strcmp(up, "EXIT") == 0) break;
        run_query(q);
    }
}

/* ══════════════════════════════════════════════════════
   ANALYZE TABLE
   ══════════════════════════════════════════════════════ */
void analyze_table(Table *db) {
    if (db->record_count == 0) { printf("  No data loaded.\n"); return; }
    printf("\n===== TABLE ANALYSIS =====\n");
    printf("  Folder     : %s\n", db->folder);
    printf("  Total rows : %d\n", db->record_count);
    if (db->records[0].field_count == 0) return;
    printf("  Columns:\n");
    for (int i = 0; i < db->records[0].field_count; i++) {
        int filled = 0;
        const char *h = db->records[0].fields[i].header;
        for (int r = 0; r < db->record_count; r++)
            if (strlen(get_value(&db->records[r], h)) > 0) filled++;
        printf("    [%d] %-20s — %d/%d filled\n",
               i+1, h, filled, db->record_count);
    }
    printf("==========================\n");
}

/* ══════════════════════════════════════════════════════
   CHOOSE DB HELPER
   ══════════════════════════════════════════════════════ */
Table* choose_database() {
    char input[20];
    printf("  Select database (1=DB1, 2=DB2): ");
    fgets(input, sizeof(input), stdin);
    int c = atoi(input);
    if (c == 1) return &db1;
    if (c == 2) return &db2;
    printf("  Invalid choice.\n");
    return NULL;
}

/* ══════════════════════════════════════════════════════
   MAIN
   ══════════════════════════════════════════════════════ */
int main() {
    setbuf(stdout, NULL);

    memset(&db1,     0, sizeof(Table));
    memset(&db2,     0, sizeof(Table));
    memset(&result_db, 0, sizeof(Table));
    strcpy(db1.folder, "none");
    strcpy(db2.folder, "none");

    int  choice;
    char input[MAX_PATH];

    while (1) {
        printf("\n╔══════════════════════════════════════╗\n");
        printf("║     HBTU MCA Mini Database System    ║\n");
        printf("╠══════════════════════════════════════╣\n");
        printf("║  1.  Load folder  → DB1              ║\n");
        printf("║  2.  Load folder  → DB2              ║\n");
        printf("║  3.  Display DB1                     ║\n");
        printf("║  4.  Display DB2                     ║\n");
        printf("║  5.  Sort records (shows full table) ║\n");
        printf("║  6.  Insert new record               ║\n");
        printf("║  7.  Delete a record                 ║\n");
        printf("║  8.  Update a record                 ║\n");
        printf("║  9.  Save DB to files                ║\n");
        printf("║ 10.  Inner Join  (DB1 + DB2)         ║\n");
        printf("║ 11.  Left  Join  (DB1 + DB2)         ║\n");
        printf("║ 12.  Right Join  (DB1 + DB2)         ║\n");
        printf("║ 13.  Full  Join  (DB1 + DB2)         ║\n");
        printf("║ 14.  SSQL Query Shell                ║\n");
        printf("║ 15.  Save join/query result          ║\n");
        printf("║ 16.  Analyze DB1                     ║\n");
        printf("║ 17.  Analyze DB2                     ║\n");
        printf("║  0.  Exit                            ║\n");
        printf("╚══════════════════════════════════════╝\n");
        printf("Choice: ");

        fgets(input, sizeof(input), stdin);
        choice = atoi(input);

        char header[MAX_TEXT], value[MAX_TEXT];

        switch (choice) {

        case 1:
            printf("  Folder for DB1: ");
            fgets(input, sizeof(input), stdin); trim(input);
            load_folder(&db1, input);
            break;

        case 2:
            printf("  Folder for DB2: ");
            fgets(input, sizeof(input), stdin); trim(input);
            load_folder(&db2, input);
            break;

        case 3: display_records(&db1); break;
        case 4: display_records(&db2); break;

        case 5: {
            Table *db = choose_database();
            if (!db) break;
            if (db->record_count > 0) {
                printf("  Available columns: ");
                for (int i = 0; i < db->records[0].field_count; i++)
                    printf("%s  ", db->records[0].fields[i].header);
                printf("\n");
            }
            printf("  Column to sort by: ");
            fgets(header, sizeof(header), stdin); trim(header);
            sort_records(db, header);
            break;
        }

        case 6: {
            Table *db = choose_database();
            if (!db) break;
            insert_record(db);
            break;
        }

        case 7: {
            Table *db = choose_database();
            if (!db) break;
            display_table_format(db->records, db->record_count, db->folder);
            printf("  Record number to delete: ");
            fgets(input, sizeof(input), stdin);
            delete_record(db, atoi(input)-1);
            break;
        }

        case 8: {
            Table *db = choose_database();
            if (!db) break;
            display_table_format(db->records, db->record_count, db->folder);
            printf("  Record number to update: ");
            fgets(input, sizeof(input), stdin);
            int idx = atoi(input) - 1;
            printf("  Column name: ");
            fgets(header, sizeof(header), stdin); trim(header);
            printf("  New value: ");
            fgets(value, sizeof(value), stdin); trim(value);
            update_record(db, idx, header, value);
            break;
        }

        case 9: {
            Table *db = choose_database();
            if (!db) break;
            save_files(db);
            break;
        }

        case 10:   /* Inner */
        case 11:   /* Left  */
        case 12:   /* Right */
        case 13: { /* Full  */
            if (db1.record_count==0 || db2.record_count==0) {
                printf("  Load BOTH DB1 and DB2 first!\n"); break;
            }
            printf("  Common column to join on: ");
            fgets(header, sizeof(header), stdin); trim(header);
            int jtype = choice - 10;  /* 0/1/2/3 */
            do_join(&db1, &db2, header, jtype);
            display_result();
            break;
        }

        case 14:
            query_shell();
            break;

        case 15:
            printf("  Folder to save result: ");
            fgets(input, sizeof(input), stdin); trim(input);
            save_result_to(input);
            break;

        case 16: analyze_table(&db1); break;
        case 17: analyze_table(&db2); break;

        case 0:
            printf("  Goodbye!\n");
            return 0;

        default:
            printf("  Invalid choice.\n");
        }
    }
    return 0;
}
