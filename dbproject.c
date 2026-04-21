/*
 * DSUC + DBMS Assignment 1 — MCA 1st Year, HBTU Kanpur
 * Simple Generic Data Management System
 * Language: C only
 * Author  : [Your Group Names]
 *
 * AI TOOL DISCLOSURE:
 * Tool   : Claude (claude.ai)
 * Prompt : "Write a simple generic C program to load key:value
 *           files into RAM and do sort, insert, delete, update,
 *           joins and a basic query language"
 * Output : Code structure and logic suggested by AI
 * Usage  : Students reviewed and understood all code before use
 *
 * [SW] = Student Written   [AI] = AI Suggested
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---- LIMITS ---- */
#define MAXF   20    /* max fields/columns  */
#define MAXR   200   /* max records/rows    */
#define MAXL   100   /* max string length   */
#define MAXP   300   /* max path length     */

/* ---- TABLE STRUCTURE [AI] ----
   One Table holds everything in RAM.
   headers[] = column names (read from 0.txt)
   data[][]  = all values as strings
*/
typedef struct {
    char headers[MAXF][MAXL];
    char data[MAXR][MAXF][MAXL];
    int  cols;   /* number of columns */
    int  rows;   /* number of rows    */
    char path[MAXP];
} Table;

/* ================================================
   HELPER: remove comma/newline/spaces  [SW]
   ================================================ */
void clean(char *s) {
    int n = strlen(s);
    while (n > 0 && (s[n-1]=='\n' || s[n-1]=='\r' ||
                     s[n-1]==','  || s[n-1]==' '))
        s[--n] = '\0';
    while (*s == ' ') memmove(s, s+1, strlen(s));
}

/* ================================================
   HELPER: find column index by name  [SW]
   Returns -1 if not found
   ================================================ */
int col_of(Table *t, char *name) {
    for (int j = 0; j < t->cols; j++)
        if (strcmp(t->headers[j], name) == 0) return j;
    return -1;
}

/* ================================================
   LOAD TABLE from folder  [AI]
   0.txt = header definitions
   1.txt, 2.txt ... = actual records
   ================================================ */
int load(Table *t, char *folder) {
    memset(t, 0, sizeof(Table));
    strncpy(t->path, folder, MAXP-1);

    /* read headers from 0.txt */
    char path[MAXP];
    snprintf(path, MAXP, "%s/0.txt", folder);
    FILE *f = fopen(path, "r");
    if (!f) { printf("Cannot open %s\n", path); return 0; }

    char line[MAXL*2];
    while (fgets(line, sizeof(line), f)) {
        clean(line);
        char *colon = strchr(line, ':');
        if (!colon) continue;
        *colon = '\0';
        clean(line);
        strncpy(t->headers[t->cols++], line, MAXL-1);
    }
    fclose(f);

    /* read records from 1.txt, 2.txt ... */
    int i = 1;
    while (t->rows < MAXR) {
        snprintf(path, MAXP, "%s/%d.txt", folder, i++);
        f = fopen(path, "r");
        if (!f) break;

        /* default all fields to NA */
        for (int j = 0; j < t->cols; j++)
            strcpy(t->data[t->rows][j], "NA");

        while (fgets(line, sizeof(line), f)) {
            clean(line);
            char *colon = strchr(line, ':');
            if (!colon) continue;
            *colon = '\0';
            char key[MAXL], val[MAXL];
            strncpy(key, line,    MAXL-1); clean(key);
            strncpy(val, colon+1, MAXL-1); clean(val);
            int c = col_of(t, key);
            if (c >= 0) strncpy(t->data[t->rows][c], val, MAXL-1);
        }
        fclose(f);
        t->rows++;
    }

    printf("Loaded: %d rows, %d cols from [%s]\n", t->rows, t->cols, folder);
    return 1;
}

/* ================================================
   DISPLAY TABLE  [SW]
   ================================================ */
void show(Table *t) {
    if (t->rows == 0) { printf("Table is empty.\n"); return; }
    printf("\n");
    for (int j = 0; j < t->cols; j++) printf("%-16s", t->headers[j]);
    printf("\n");
    for (int j = 0; j < t->cols * 16; j++) printf("-");
    printf("\n");
    for (int i = 0; i < t->rows; i++) {
        for (int j = 0; j < t->cols; j++)
            printf("%-16s", t->data[i][j]);
        printf("\n");
    }
    printf("\n");
}

/* ================================================
   SAVE TABLE back to folder  [SW]
   ================================================ */
void save(Table *t, char *folder) {
    char path[MAXP];
    /* write 0.txt */
    snprintf(path, MAXP, "%s/0.txt", folder);
    FILE *f = fopen(path, "w");
    if (!f) { printf("Cannot save.\n"); return; }
    for (int j = 0; j < t->cols; j++)
        fprintf(f, "%s:%s,\n", t->headers[j], t->headers[j]);
    fclose(f);
    /* write each record */
    for (int i = 0; i < t->rows; i++) {
        snprintf(path, MAXP, "%s/%d.txt", folder, i+1);
        f = fopen(path, "w");
        if (!f) continue;
        for (int j = 0; j < t->cols; j++)
            fprintf(f, "%s:%s,\n", t->headers[j], t->data[i][j]);
        fclose(f);
    }
    printf("Saved %d records to [%s]\n", t->rows, folder);
}

/* ================================================
   Q1b-i  SORT by any column  [SW]
   Simple bubble sort — easy to understand
   ================================================ */
void sort_by(Table *t) {
    printf("\nChoose column to sort by:\n");
    for (int j = 0; j < t->cols; j++)
        printf("  %d = %s\n", j, t->headers[j]);
    int c; printf("Column number: "); scanf("%d", &c);
    if (c < 0 || c >= t->cols) { printf("Invalid.\n"); return; }

    clock_t start = clock();

    /* bubble sort */
    char tmp[MAXF][MAXL];
    for (int i = 0; i < t->rows - 1; i++)
        for (int k = 0; k < t->rows - i - 1; k++)
            if (strcmp(t->data[k][c], t->data[k+1][c]) > 0) {
                memcpy(tmp,            t->data[k],   sizeof(tmp));
                memcpy(t->data[k],     t->data[k+1], sizeof(tmp));
                memcpy(t->data[k+1],   tmp,           sizeof(tmp));
            }

    printf("[TIME] Sort by '%s': %.4f ms\n",
           t->headers[c],
           (double)(clock()-start)/CLOCKS_PER_SEC*1000.0);
    show(t);
}

/* ================================================
   Q1b-ii  INSERT a new record  [SW]
   ================================================ */
void insert_row(Table *t) {
    if (t->rows >= MAXR) { printf("Table full.\n"); return; }
    clock_t start = clock();
    printf("Enter values:\n");
    /* flush leftover newline */
    int ch; while ((ch=getchar())!='\n' && ch!=EOF);
    for (int j = 0; j < t->cols; j++) {
        printf("  %s: ", t->headers[j]);
        fgets(t->data[t->rows][j], MAXL, stdin);
        clean(t->data[t->rows][j]);
    }
    t->rows++;
    printf("[TIME] Insert: %.4f ms | Rows now: %d\n",
           (double)(clock()-start)/CLOCKS_PER_SEC*1000.0, t->rows);
}

/* ================================================
   Q1b-iii  DELETE a record  [SW]
   User picks which column to search by
   ================================================ */
void delete_row(Table *t) {
    if (t->rows == 0) { printf("Empty table.\n"); return; }

    printf("Search by which column?\n");
    for (int j = 0; j < t->cols; j++)
        printf("  %d = %s\n", j, t->headers[j]);
    int c; printf("Column number: "); scanf("%d", &c);
    if (c < 0 || c >= t->cols) { printf("Invalid.\n"); return; }

    char val[MAXL];
    printf("Value to find: "); scanf(" %[^\n]", val);

    clock_t start = clock();
    int found = -1;
    for (int i = 0; i < t->rows; i++)
        if (strcmp(t->data[i][c], val) == 0) { found = i; break; }

    if (found == -1) { printf("Not found.\n"); return; }

    /* shift rows up to fill gap */
    for (int i = found; i < t->rows - 1; i++)
        memcpy(t->data[i], t->data[i+1], sizeof(t->data[i]));
    t->rows--;

    printf("[TIME] Delete: %.4f ms | Rows now: %d\n",
           (double)(clock()-start)/CLOCKS_PER_SEC*1000.0, t->rows);
}

/* ================================================
   Q1b-iv  UPDATE a record  [SW]
   User picks search column, then field to change
   ================================================ */
void update_row(Table *t) {
    if (t->rows == 0) { printf("Empty table.\n"); return; }

    printf("Search by which column?\n");
    for (int j = 0; j < t->cols; j++)
        printf("  %d = %s\n", j, t->headers[j]);
    int sc; printf("Column number: "); scanf("%d", &sc);
    if (sc < 0 || sc >= t->cols) { printf("Invalid.\n"); return; }

    char val[MAXL];
    printf("Value to find: "); scanf(" %[^\n]", val);

    int found = -1;
    for (int i = 0; i < t->rows; i++)
        if (strcmp(t->data[i][sc], val) == 0) { found = i; break; }

    if (found == -1) { printf("Not found.\n"); return; }

    printf("Current record:\n");
    for (int j = 0; j < t->cols; j++)
        printf("  %d = %s : %s\n", j, t->headers[j], t->data[found][j]);

    int uc; printf("Which field to update? "); scanf("%d", &uc);
    if (uc < 0 || uc >= t->cols) { printf("Invalid.\n"); return; }

    clock_t start = clock();
    char newval[MAXL];
    printf("New value: "); scanf(" %[^\n]", newval);
    strncpy(t->data[found][uc], newval, MAXL-1);

    printf("[TIME] Update '%s': %.4f ms\n",
           t->headers[uc],
           (double)(clock()-start)/CLOCKS_PER_SEC*1000.0);
}

/* ================================================
   Q2a  JOIN two tables  [AI + SW]
   type: 0=inner 1=left 2=right 3=full
   ================================================ */
void join(Table *t1, Table *t2, Table *out, char *on, int type) {
    memset(out, 0, sizeof(Table));

    int c1 = col_of(t1, on);
    int c2 = col_of(t2, on);
    if (c1 < 0 || c2 < 0) {
        printf("Column '%s' not found in both tables.\n", on);
        return;
    }

    /* result headers = all of t1 + t2 (skip duplicate join column) */
    for (int j = 0; j < t1->cols; j++)
        strncpy(out->headers[out->cols++], t1->headers[j], MAXL-1);
    for (int j = 0; j < t2->cols; j++) {
        if (j == c2) continue;
        strncpy(out->headers[out->cols++], t2->headers[j], MAXL-1);
    }

    int used2[MAXR] = {0};  /* track which t2 rows matched */

    clock_t start = clock();

    for (int i = 0; i < t1->rows; i++) {
        int matched = 0;
        for (int k = 0; k < t2->rows; k++) {
            if (strcmp(t1->data[i][c1], t2->data[k][c2]) != 0) continue;
            /* copy t1 row */
            for (int j = 0; j < t1->cols; j++)
                strncpy(out->data[out->rows][j], t1->data[i][j], MAXL-1);
            /* copy t2 row (skip join col) */
            int off = t1->cols;
            for (int j = 0; j < t2->cols; j++) {
                if (j == c2) continue;
                strncpy(out->data[out->rows][off++], t2->data[k][j], MAXL-1);
            }
            out->rows++;
            matched = 1;
            used2[k] = 1;
        }
        /* left / full: keep unmatched t1 rows */
        if (!matched && (type == 1 || type == 3)) {
            for (int j = 0; j < t1->cols; j++)
                strncpy(out->data[out->rows][j], t1->data[i][j], MAXL-1);
            int off = t1->cols;
            for (int j = 0; j < t2->cols; j++) {
                if (j == c2) continue;
                strcpy(out->data[out->rows][off++], "NULL");
            }
            out->rows++;
        }
    }

    /* right / full: keep unmatched t2 rows */
    if (type == 2 || type == 3) {
        for (int k = 0; k < t2->rows; k++) {
            if (used2[k]) continue;
            for (int j = 0; j < t1->cols; j++)
                strcpy(out->data[out->rows][j],
                       j == c1 ? t2->data[k][c2] : "NULL");
            int off = t1->cols;
            for (int j = 0; j < t2->cols; j++) {
                if (j == c2) continue;
                strncpy(out->data[out->rows][off++], t2->data[k][j], MAXL-1);
            }
            out->rows++;
        }
    }

    char *names[] = {"Inner","Left","Right","Full"};
    printf("[TIME] %s Join on '%s': %.4f ms | Result: %d rows\n",
           names[type], on,
           (double)(clock()-start)/CLOCKS_PER_SEC*1000.0,
           out->rows);
}

/* ================================================
   JOIN MENU  [SW]
   ================================================ */
void join_menu(Table *t1, Table *t2) {
    if (t1->rows == 0 || t2->rows == 0) {
        printf("Load both tables first.\n"); return;
    }
    printf("\n0=Inner  1=Left  2=Right  3=Full\n");
    int type; printf("Join type: "); scanf("%d", &type);
    if (type < 0 || type > 3) { printf("Invalid.\n"); return; }

    printf("Table1 cols: ");
    for (int j = 0; j < t1->cols; j++) printf("%s ", t1->headers[j]);
    printf("\nTable2 cols: ");
    for (int j = 0; j < t2->cols; j++) printf("%s ", t2->headers[j]);

    char on[MAXL];
    printf("\nJoin on column: "); scanf("%s", on);

    Table result;
    join(t1, t2, &result, on, type);
    show(&result);

    int sv; printf("Save result? 1=yes 0=no: "); scanf("%d", &sv);
    if (sv) {
        char out[MAXP]; printf("Output folder: "); scanf("%s", out);
        char cmd[MAXP+10]; snprintf(cmd, sizeof(cmd), "mkdir -p %s", out);
        system(cmd);
        save(&result, out);
    }
}

/* ================================================
   Q2b  SIMPLE QUERY LANGUAGE  [AI + SW]
   ================================================
   Commands:
     SELECT * FROM <folder>
     SELECT col1,col2 FROM <folder>
     SELECT * FROM <folder> WHERE col=value
     JOIN <f1> AND <f2> ON <col> TYPE INNER|LEFT|RIGHT|FULL
     SHOW
     SAVE <folder>
     HELP / EXIT
   ================================================ */
void run_query(char *q, Table *t1, Table *t2) {
    /* uppercase copy for keyword matching */
    char up[512];
    strncpy(up, q, 511);
    for (int i = 0; up[i]; i++)
        if (up[i] >= 'a' && up[i] <= 'z') up[i] -= 32;

    /* HELP */
    if (strncmp(up, "HELP", 4) == 0) {
        printf("\n--- SSQL Commands ---\n");
        printf("SELECT * FROM <folder>\n");
        printf("SELECT col1,col2 FROM <folder>\n");
        printf("SELECT * FROM <folder> WHERE col=value\n");
        printf("JOIN <f1> AND <f2> ON <col> TYPE INNER|LEFT|RIGHT|FULL\n");
        printf("SHOW\n");
        printf("SAVE <folder>\n");
        printf("EXIT\n\n");
        return;
    }

    /* SHOW */
    if (strncmp(up, "SHOW", 4) == 0) {
        printf("T1: [%s]  rows=%d cols=%d\n", t1->path, t1->rows, t1->cols);
        printf("T2: [%s]  rows=%d cols=%d\n", t2->path, t2->rows, t2->cols);
        return;
    }

    /* SAVE */
    if (strncmp(up, "SAVE", 4) == 0) {
        char folder[MAXP];
        if (sscanf(q+5, "%s", folder) == 1) save(t1, folder);
        return;
    }

    /* JOIN */
    if (strncmp(up, "JOIN", 4) == 0) {
        char f1[MAXP], f2[MAXP], col[MAXL], type_s[MAXL];
        if (sscanf(q, "%*s %s %*s %s %*s %s %*s %s", f1, f2, col, type_s) != 4) {
            printf("Usage: JOIN <f1> AND <f2> ON <col> TYPE INNER|LEFT|RIGHT|FULL\n");
            return;
        }
        for (int i = 0; type_s[i]; i++)
            if (type_s[i] >= 'a' && type_s[i] <= 'z') type_s[i] -= 32;
        int type = 0;
        if (strcmp(type_s,"LEFT")==0)  type = 1;
        if (strcmp(type_s,"RIGHT")==0) type = 2;
        if (strcmp(type_s,"FULL")==0)  type = 3;

        Table a, b, r;
        clock_t s = clock();
        load(&a, f1); load(&b, f2);
        printf("[TIME] Load: %.4f ms\n", (double)(clock()-s)/CLOCKS_PER_SEC*1000.0);
        join(&a, &b, &r, col, type);
        show(&r);

        int sv; printf("Save? 1=yes 0=no: "); scanf("%d", &sv);
        if (sv) {
            char out[MAXP]; printf("Folder: "); scanf("%s", out);
            char cmd[MAXP+10]; snprintf(cmd, sizeof(cmd), "mkdir -p %s", out);
            system(cmd);
            save(&r, out);
        }
        return;
    }

    /* SELECT */
    if (strncmp(up, "SELECT", 6) == 0) {
        /* find FROM */
        char *fp = strstr(up, " FROM ");
        if (!fp) { printf("Missing FROM\n"); return; }
        int fpos = fp - up;

        /* column list */
        char colstr[MAXL*MAXF];
        strncpy(colstr, q+7, fpos-7);
        colstr[fpos-7] = '\0';
        clean(colstr);

        /* parse columns */
        char cols[MAXF][MAXL];
        int nc = 0;
        char tmp[MAXL*MAXF];
        strncpy(tmp, colstr, sizeof(tmp)-1);
        char *tok = strtok(tmp, ",");
        while (tok && nc < MAXF) {
            clean(tok);
            strncpy(cols[nc++], tok, MAXL-1);
            tok = strtok(NULL, ",");
        }

        /* rest after FROM */
        char rest[512];
        strncpy(rest, q+fpos+6, 511);
        clean(rest);

        /* check for WHERE */
        char rest_up[512];
        strncpy(rest_up, rest, 511);
        for (int i = 0; rest_up[i]; i++)
            if (rest_up[i]>='a'&&rest_up[i]<='z') rest_up[i]-=32;

        char folder[MAXP], wcol[MAXL]="", wval[MAXL]="";
        char *wp = strstr(rest_up, " WHERE ");
        if (wp) {
            int wpos = wp - rest_up;
            strncpy(folder, rest, wpos); folder[wpos] = '\0'; clean(folder);
            char cond[MAXL*2];
            strncpy(cond, rest+wpos+7, sizeof(cond)-1);
            char *eq = strchr(cond, '=');
            if (eq) {
                *eq = '\0';
                strncpy(wcol, cond, MAXL-1); clean(wcol);
                strncpy(wval, eq+1, MAXL-1); clean(wval);
            }
        } else {
            strncpy(folder, rest, MAXP-1);
        }

        /* load and filter */
        Table src, result;
        clock_t s = clock();
        if (!load(&src, folder)) return;
        printf("[TIME] Load: %.4f ms\n", (double)(clock()-s)/CLOCKS_PER_SEC*1000.0);

        memset(&result, 0, sizeof(Table));

        /* set result columns */
        int cidx[MAXF];
        if (nc==1 && strcmp(cols[0],"*")==0) {
            for (int j=0; j<src.cols; j++) {
                cidx[j] = j;
                strncpy(result.headers[j], src.headers[j], MAXL-1);
            }
            result.cols = src.cols;
            nc = src.cols;
            for (int j=0; j<nc; j++) cidx[j]=j;
        } else {
            result.cols = 0;
            for (int c=0; c<nc; c++) {
                int idx = col_of(&src, cols[c]);
                if (idx < 0) { printf("No column '%s'\n", cols[c]); continue; }
                cidx[result.cols] = idx;
                strncpy(result.headers[result.cols++], cols[c], MAXL-1);
            }
            nc = result.cols;
        }

        int wc = strlen(wcol)>0 ? col_of(&src, wcol) : -1;

        clock_t qs = clock();
        for (int i=0; i<src.rows; i++) {
            if (wc>=0 && strcmp(src.data[i][wc], wval)!=0) continue;
            for (int c=0; c<nc; c++)
                strncpy(result.data[result.rows][c], src.data[i][cidx[c]], MAXL-1);
            result.rows++;
        }
        printf("[TIME] Filter: %.4f ms | Rows: %d\n",
               (double)(clock()-qs)/CLOCKS_PER_SEC*1000.0, result.rows);
        show(&result);

        int sv; printf("Save? 1=yes 0=no: "); scanf("%d", &sv);
        if (sv) {
            char out[MAXP]; printf("Folder: "); scanf("%s", out);
            char cmd[MAXP+10]; snprintf(cmd, sizeof(cmd), "mkdir -p %s", out);
            system(cmd);
            save(&result, out);
        }
        return;
    }

    printf("Unknown command. Type HELP.\n");
}

/* ================================================
   QUERY SHELL  [SW]
   ================================================ */
void query_shell(Table *t1, Table *t2) {
    char q[512];
    printf("\n=== SSQL Shell === (HELP for commands, EXIT to quit)\n");
    int ch; while ((ch=getchar())!='\n' && ch!=EOF);
    while (1) {
        printf("ssql> ");
        fgets(q, 512, stdin);
        q[strcspn(q,"\n")] = '\0';
        clean(q);
        if (strlen(q) == 0) continue;
        char up[512]; strncpy(up,q,511);
        for (int i=0;up[i];i++) if(up[i]>='a'&&up[i]<='z') up[i]-=32;
        if (strcmp(up,"EXIT")==0) break;
        run_query(q, t1, t2);
    }
}

/* ================================================
   MAIN MENU  [SW]
   ================================================ */
int main() {
    Table t1, t2;
    memset(&t1, 0, sizeof(Table)); strcpy(t1.path, "none");
    memset(&t2, 0, sizeof(Table)); strcpy(t2.path, "none");

    printf("\n=== HBTU MCA Data System ===\n");
    char folder[MAXP];
    printf("Enter folder for Table1: ");
    scanf("%s", folder);
    clock_t s = clock();
    if (!load(&t1, folder)) return 1;
    printf("[TIME] Load: %.4f ms\n", (double)(clock()-s)/CLOCKS_PER_SEC*1000.0);

    int ch;
    while (1) {
        printf("\n--- MENU ---\n");
        printf(" 1. Show Table1\n");
        printf(" 2. Sort\n");
        printf(" 3. Insert row\n");
        printf(" 4. Delete row\n");
        printf(" 5. Update row\n");
        printf(" 6. Save Table1\n");
        printf(" 7. Reload Table1\n");
        printf(" 8. Load Table2\n");
        printf(" 9. Show Table2\n");
        printf("10. Join T1 and T2\n");
        printf("11. Query Shell (SSQL)\n");
        printf(" 0. Exit\n");
        printf("Choice: "); scanf("%d", &ch);

        char buf[MAXP];
        switch (ch) {
            case 1: show(&t1); break;
            case 2: sort_by(&t1); break;
            case 3: insert_row(&t1); break;
            case 4: delete_row(&t1); break;
            case 5: update_row(&t1); break;
            case 6:
                printf("Save to folder: "); scanf("%s", buf);
                save(&t1, buf); break;
            case 7:
                printf("Folder: "); scanf("%s", buf);
                s = clock(); load(&t1, buf);
                printf("[TIME] %.4f ms\n", (double)(clock()-s)/CLOCKS_PER_SEC*1000.0);
                break;
            case 8:
                printf("Table2 folder: "); scanf("%s", buf);
                s = clock(); load(&t2, buf);
                printf("[TIME] %.4f ms\n", (double)(clock()-s)/CLOCKS_PER_SEC*1000.0);
                break;
            case 9: show(&t2); break;
            case 10: join_menu(&t1, &t2); break;
            case 11: query_shell(&t1, &t2); break;
            case 0: printf("Bye!\n"); return 0;
            default: printf("Invalid.\n");
        }
    }
}
