/* cwc.c - C with Classes transpiler (with local variable tracking) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_LINE 1024
#define MAX_CLASS_NAME 64
#define MAX_METHODS 50
#define MAX_CONSTRUCTORS 10
#define MAX_FIELDS 100
#define MAX_BUFFER 10000
#define MAX_SYMS 200
#define MAX_SCOPES 50

// Class state
static int in_class = 0;
static char current_class[MAX_CLASS_NAME];
static char fields[MAX_FIELDS][MAX_LINE];
static int field_count = 0;

// Methods and constructors
static char methods_impl[MAX_METHODS][MAX_BUFFER];
static int method_count = 0;
typedef struct {
    char args[MAX_LINE];
    char body[MAX_BUFFER];
    char type_key[256];
    char arg_types[256];
} Constructor;
static Constructor constructors[MAX_CONSTRUCTORS];
static int ctor_count = 0;

// Parsing state
static int in_method_decl = 0;
static int in_method_body = 0;
static int brace_level = 0; // for method body only
static char current_ret_type[MAX_LINE];
static char current_method_name[MAX_LINE];
static char current_args[MAX_LINE];
static char method_buffer[MAX_BUFFER];

// --- Scope system for local variables ---
typedef struct {
    char name[64];
    char type[64];
} SymEntry;

typedef struct {
    SymEntry syms[MAX_SYMS];
    int count;
} Scope;

static Scope scopes[MAX_SCOPES];
static int scope_depth = 0; // current number of active scopes (0 = global)

static int global_brace_depth = 0; // for tracking {} outside classes/methods

// Initialize global scope
void init_scopes(void) {
    scope_depth = 1; // scopes[0] is global
    scopes[0].count = 0;
}

void push_scope(void) {
    if (scope_depth < MAX_SCOPES) {
        scopes[scope_depth].count = 0;
        scope_depth++;
    }
}

void pop_scope(void) {
    if (scope_depth > 1) {
        scope_depth--;
    }
}

char* lookup_type_of_var(const char* var_name) {
    // Search from innermost to outermost
    for (int i = scope_depth - 1; i >= 0; i--) {
        for (int j = 0; j < scopes[i].count; j++) {
            if (strcmp(scopes[i].syms[j].name, var_name) == 0) {
                return scopes[i].syms[j].type;
            }
        }
    }
    return NULL;
}

void add_var_to_current_scope(const char* name, const char* base_type) {
    if (scope_depth <= 0 || scope_depth > MAX_SCOPES) return;
    Scope* cur = &scopes[scope_depth - 1];
    if (cur->count >= MAX_SYMS) return;
    strcpy(cur->syms[cur->count].name, name);
    snprintf(cur->syms[cur->count].type, sizeof(cur->syms[cur->count].type), "%s*", base_type);
    cur->count++;
}

// --- Utilities ---
char* trim(char* str) {
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = 0;
    return str;
}

int looks_like_method(const char* line) {
    if (!line || !*line) return 0;
    if (strchr(line, '(') == NULL || strchr(line, ')') == NULL) return 0;
    if (strstr(line, "class")) return 0;
    if (strchr(line, ';')) return 0;
    return 1;
}

void parse_sig(const char* line, char* ret, char* name, char* args) {
    char buf[MAX_LINE];
    strncpy(buf, line, MAX_LINE - 1);
    buf[MAX_LINE - 1] = '\0';
    char* p = strchr(buf, '(');
    if (!p) { strcpy(ret, "void"); strcpy(name, "unknown"); strcpy(args, ""); return; }
    *p = '\0';
    char* last = strrchr(buf, ' ');
    if (last && last != buf) {
        *last = '\0';
        strcpy(ret, buf);
        strcpy(name, last + 1);
    } else {
        strcpy(ret, "");
        strcpy(name, buf);
    }
    strcpy(args, p + 1);
    char* q = strchr(args, ')');
    if (q) *q = '\0';
}

int count_char(const char* s, char c) {
    int n = 0;
    while (*s) if (*s++ == c) n++;
    return n;
}

void args_to_type_key_and_list(const char* args, char* key, char* list) {
    if (strlen(args) == 0) {
        strcpy(key, "default");
        strcpy(list, "");
        return;
    }
    key[0] = list[0] = '\0';
    char temp[MAX_LINE];
    strcpy(temp, args);
    char* token = strtok(temp, ",");
    int first = 1;
    while (token) {
        while (*token && isspace((unsigned char)*token)) token++;
        if (*token == '\0') {
            token = strtok(NULL, ",");
            continue;
        }
        char* last_space = strrchr(token, ' ');
        if (last_space) {
            *last_space = '\0';
            char* end = last_space - 1;
            while (end > token && isspace((unsigned char)*end)) end--;
            *(end + 1) = '\0';
        }
        if (first) {
            strcpy(key, token);
            strcpy(list, token);
            first = 0;
        } else {
            strcat(key, "_");
            strcat(key, token);
            strcat(list, ",");
            strcat(list, token);
        }
        token = strtok(NULL, ",");
    }
}

void infer_type_from_literal(const char* lit, char* type) {
    if (strchr(lit, '.')) {
        if (strchr(lit, 'f') || strchr(lit, 'F')) {
            strcpy(type, "float");
        } else {
            strcpy(type, "double");
        }
    } else if (lit[0] == '"' || lit[0] == '\'') {
        strcpy(type, "char*");
    } else {
        strcpy(type, "int");
    }
}

void infer_type_of_arg(const char* arg, char* type) {
    char temp[256];
    strcpy(temp, arg);
    char* p = temp;
    while (*p && isspace((unsigned char)*p)) p++;
    if (!*p) { strcpy(type, "int"); return; }

    // Literal?
    if (*p == '"' || *p == '\'') {
        strcpy(type, "char*");
        return;
    }
    if (isdigit((unsigned char)*p) || *p == '+' || *p == '-') {
        infer_type_from_literal(p, type);
        return;
    }

    // Identifier
    if (isalpha((unsigned char)*p) || *p == '_') {
        char* end = p;
        while (*end && (isalnum((unsigned char)*end) || *end == '_')) end++;
        char var_name[64];
        int len = end - p;
        if (len >= 64) len = 63;
        strncpy(var_name, p, len);
        var_name[len] = '\0';

        char* found = lookup_type_of_var(var_name);
        if (found) {
            char clean[64];
            strncpy(clean, found, 63);
            clean[63] = '\0';
            char* star = strchr(clean, '*');
            if (star) *star = '\0';
            // Trim spaces
            char* s = clean;
            while (*s && isspace((unsigned char)*s)) s++;
            if (*s) {
                char* e = s + strlen(s) - 1;
                while (e > s && isspace((unsigned char)*e)) e--;
                if (e >= s) {
                    int l = e - s + 1;
                    memmove(clean, s, l);
                    clean[l] = '\0';
                    strcpy(type, clean);
                    return;
                }
            }
        }
    }

    strcpy(type, "int");
}

char* find_matching_ctor_by_types(const char* class_name, const char* args_str, char* out_call_args) {
    if (!args_str) args_str = "";
    char args_copy[512];
    strcpy(args_copy, args_str);
    char* arg_list[32];
    int arg_count = 0;
    char* token = strtok(args_copy, ",");
    while (token && arg_count < 32) {
        arg_list[arg_count++] = trim(token);
        token = strtok(NULL, ",");
    }
    char arg_types[512] = "";
    for (int i = 0; i < arg_count; i++) {
        char t[64];
        infer_type_of_arg(arg_list[i], t);
        char* star = strchr(t, '*');
        if (star) *star = '\0';
        if (i > 0) strcat(arg_types, ",");
        strcat(arg_types, t);
    }
    for (int i = 0; i < ctor_count; i++) {
        if (strcmp(constructors[i].arg_types, arg_types) == 0) {
            out_call_args[0] = '\0';
            for (int j = 0; j < arg_count; j++) {
                if (j > 0) strcat(out_call_args, ", ");
                strcat(out_call_args, arg_list[j]);
            }
            return constructors[i].type_key;
        }
    }
    return NULL;
}

void try_parse_variable_declaration(const char* line) {
    if (!strchr(line, ';')) return;
    char buf[MAX_LINE];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf)-1] = '\0';
    char* p = buf;
    while (*p && isspace((unsigned char)*p)) p++;
    if (!isalpha((unsigned char)*p) && *p != '_') return;

    char* type_start = p;
    while (*p && (isalnum((unsigned char)*p) || *p == '_' || *p == '*')) p++;
    int tlen = p - type_start;
    if (tlen >= 64) tlen = 63;
    char base_type[64];
    strncpy(base_type, type_start, tlen);
    base_type[tlen] = '\0';

    char* rest = p;
    while (*rest && isspace((unsigned char)*rest)) rest++;
    if (*rest != ';') {
        char* semicolon = strchr(rest, ';');
        if (!semicolon) return;
        *semicolon = '\0';
        char* token = strtok(rest, ",");
        while (token) {
            char* eq = strchr(token, '=');
            if (eq) *eq = '\0';
            char* vname = trim(token);
            if (*vname) {
                add_var_to_current_scope(vname, base_type);
            }
            token = strtok(NULL, ",");
        }
    }
}

int try_handle_new_call(const char* line, FILE* out) {
    char* np = strstr(line, "new(");
    if (!np) return 0;
    const char* start = np + 4;
    const char* p = start;
    int paren = 1;
    while (*p && paren > 0) {
        if (*p == '(') paren++;
        else if (*p == ')') paren--;
        p++;
    }
    if (paren != 0) return 0;
    int content_len = (p - 1) - start;
    if (content_len <= 0) return 0;
    char content[512];
    strncpy(content, start, content_len);
    content[content_len] = '\0';
    char* comma = strchr(content, ',');
    char type_name[128];
    char args_part[384] = "";
    if (comma) {
        int tn_len = comma - content;
        strncpy(type_name, content, tn_len);
        type_name[tn_len] = '\0';
        strcpy(args_part, comma + 1);
        char* a = args_part;
        while (*a && isspace((unsigned char)*a)) a++;
        if (a != args_part) memmove(args_part, a, strlen(a) + 1);
    } else {
        strcpy(type_name, content);
    }
    char* t = type_name;
    while (*t && isspace((unsigned char)*t)) t++;
    if (t != type_name) memmove(type_name, t, strlen(t) + 1);
    char* te = type_name + strlen(type_name) - 1;
    while (te > type_name && isspace((unsigned char)*te)) te--;
    *(te + 1) = '\0';
    int prefix_len = np - line;
    char prefix[1024];
    strncpy(prefix, line, prefix_len);
    prefix[prefix_len] = '\0';
    const char* suffix = p;
    if (strlen(args_part) == 0) {
        fprintf(out, "%s%s_new_default()%s", prefix, type_name, suffix);
        return 1;
    }
    char call_args[512];
    char* matched_key = find_matching_ctor_by_types(type_name, args_part, call_args);
    if (matched_key) {
        fprintf(out, "%s%s_new_%s(%s)%s", prefix, type_name, matched_key, call_args, suffix);
    } else {
        fprintf(out, "%s%s_new_default()%s", prefix, type_name, suffix);
    }
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file.cwc>\n", argv[0]);
        return 1;
    }
    FILE* fp = fopen(argv[1], "r");
    if (!fp) {
        perror(argv[1]);
        return 1;
    }

    init_scopes();
    global_brace_depth = 0;

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp)) {
        char original_line[MAX_LINE];
        strcpy(original_line, line);
        char* tline = trim(line);

        // Handle braces for scope tracking (only outside class and method)
        if (!in_class && !in_method_body) {
            int open_count = count_char(original_line, '{');
            int close_count = count_char(original_line, '}');
            for (int i = 0; i < open_count; i++) {
                global_brace_depth++;
                push_scope();
            }
            // Parse variable declarations in any context (global or local)
            try_parse_variable_declaration(original_line);
            for (int i = 0; i < close_count; i++) {
                if (global_brace_depth > 0) {
                    pop_scope();
                    global_brace_depth--;
                }
            }
        }

        // Class start
        if (strncmp(tline, "class", 5) == 0 && tline[5] == ' ') {
            in_class = 1;
            field_count = method_count = ctor_count = 0;
            char* name = tline + 6;
            char* end = strchr(name, ' ');
            if (!end) end = strchr(name, '{');
            if (!end) end = name + strlen(name);
            int len = end - name;
            if (len >= MAX_CLASS_NAME) len = MAX_CLASS_NAME - 1;
            strncpy(current_class, name, len);
            current_class[len] = '\0';
            continue;
        }

        // Class end
        if (in_class && strcmp(tline, "};") == 0) {
            printf("typedef struct %s %s;\n", current_class, current_class);
            printf("struct %s {\n", current_class);
            for (int i = 0; i < field_count; i++) {
                printf("    %s", fields[i]);
            }
            printf("};\n\n");
            for (int i = 0; i < method_count; i++) {
                printf("%s", methods_impl[i]);
            }
            for (int i = 0; i < ctor_count; i++) {
                printf("%s* %s_new_%s(%s)\n", current_class, current_class, constructors[i].type_key, constructors[i].args);
                printf("{\n");
                printf("    %s* this = (%s*)malloc(sizeof(%s));\n", current_class, current_class, current_class);
                printf("    if (!this) return NULL;\n");
                printf("%s", constructors[i].body);
                printf("    return this;\n");
                printf("}\n\n");
            }
            int has_default = 0;
            for (int i = 0; i < ctor_count; i++) {
                if (strcmp(constructors[i].type_key, "default") == 0) {
                    has_default = 1;
                    break;
                }
            }
            if (!has_default) {
                printf("%s* %s_new_default(void)\n", current_class, current_class);
                printf("{\n");
                printf("    %s* this = (%s*)malloc(sizeof(%s));\n", current_class, current_class, current_class);
                printf("    if (!this) return NULL;\n");
                printf("    return this;\n");
                printf("}\n\n");
            }
            in_class = 0;
            continue;
        }

        if (in_class) {
            if (in_method_body) {
                strcat(method_buffer, original_line);
                brace_level += count_char(original_line, '{');
                brace_level -= count_char(original_line, '}');
                if (brace_level == 0) {
                    if (strcmp(current_method_name, current_class) == 0) {
                        if (ctor_count < MAX_CONSTRUCTORS) {
                            strcpy(constructors[ctor_count].args, current_args);
                            strcpy(constructors[ctor_count].body, method_buffer);
                            args_to_type_key_and_list(current_args, constructors[ctor_count].type_key, constructors[ctor_count].arg_types);
                            ctor_count++;
                        }
                    } else {
                        char impl[MAX_BUFFER];
                        const char* ret = current_ret_type[0] ? current_ret_type : "void";
                        if (strlen(current_args) == 0) {
                            snprintf(impl, sizeof(impl), "%s %s_%s(%s *this)%s", ret, current_class, current_method_name, current_class, method_buffer);
                        } else {
                            snprintf(impl, sizeof(impl), "%s %s_%s(%s *this, %s)%s", ret, current_class, current_method_name, current_class, current_args, method_buffer);
                        }
                        strcpy(methods_impl[method_count++], impl);
                    }
                    in_method_body = in_method_decl = 0;
                }
                continue;
            }
            if (in_method_decl) {
                if (strcmp(tline, "{") == 0) {
                    memset(method_buffer, 0, sizeof(method_buffer));
                    strcat(method_buffer, original_line);
                    brace_level = 1;
                    in_method_body = 1;
                    in_method_decl = 0;
                }
                continue;
            }
            if (looks_like_method(tline)) {
                parse_sig(tline, current_ret_type, current_method_name, current_args);
                in_method_decl = 1;
                continue;
            }
            if (strlen(tline) > 0 && tline[0] != '}' && tline[0] != '{') {
                if (field_count < MAX_FIELDS) {
                    strcpy(fields[field_count], original_line);
                    field_count++;
                }
            }
            continue;
        }

        // Handle new/delete/->
        if (try_handle_new_call(original_line, stdout)) {
            continue;
        }

        char* dp = strstr(original_line, "delete(");
        if (dp) {
            char* os = dp + 7;
            char* oe = strchr(os, ')');
            if (oe) {
                char on[64]; int ol = oe - os;
                if (ol > 63) ol = 63;
                strncpy(on, os, ol); on[ol] = '\0';
                int pl = dp - original_line;
                char prefix[1024]; strncpy(prefix, original_line, pl); prefix[pl] = '\0';
                char rest[1024]; strcpy(rest, oe + 1);
                printf("%s{ free(%s); %s = NULL; }%s", prefix, on, on, rest);
                continue;
            }
        }

        char* arrow = strstr(original_line, "->");
        if (arrow) {
            char* obj_start = original_line;
            while (isspace((unsigned char)*obj_start)) obj_start++;
            char* obj_end = arrow;
            int obj_len = obj_end - obj_start;
            if (obj_len > 0 && obj_len < 64) {
                char object_name[64];
                strncpy(object_name, obj_start, obj_len);
                object_name[obj_len] = '\0';
                char* meth_start = arrow + 2;
                char* open_paren = strchr(meth_start, '(');
                if (open_paren) {
                    char* close_paren = strchr(open_paren + 1, ')');
                    if (close_paren) {
                        int meth_len = open_paren - meth_start;
                        if (meth_len > 0 && meth_len < 64) {
                            char method_name[64];
                            strncpy(method_name, meth_start, meth_len);
                            method_name[meth_len] = '\0';
                            char* class_ptr_type = lookup_type_of_var(object_name);
                            if (!class_ptr_type) goto fallback;
                            char class_name[64];
                            strncpy(class_name, class_ptr_type, sizeof(class_name)-1);
                            class_name[sizeof(class_name)-1] = '\0';
                            char* star = strchr(class_name, '*');
                            if (star) *star = '\0';
                            char new_call[1024];
                            int args_len = close_paren - (open_paren + 1);
                            if (args_len <= 0) {
                                snprintf(new_call, sizeof(new_call), "%s_%s(%s)", class_name, method_name, object_name);
                            } else {
                                char args_content[512];
                                strncpy(args_content, open_paren + 1, args_len);
                                args_content[args_len] = '\0';
                                snprintf(new_call, sizeof(new_call), "%s_%s(%s, %s)", class_name, method_name, object_name, args_content);
                            }
                            int prefix_len = obj_start - original_line;
                            char prefix[1024]; strncpy(prefix, original_line, prefix_len); prefix[prefix_len] = '\0';
                            printf("%s%s%s", prefix, new_call, close_paren + 1);
                            continue;
                        }
                    }
                }
            }
        }

    fallback:
        printf("%s", original_line);
    }

    fclose(fp);
    return 0;
}