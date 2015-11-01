
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "flag.h"

static flagset_t *set = NULL;

#define ERROR_PARSING (self->error.flag = flag, FLAG_ERROR_PARSING)

#define ERROR_UNDEFINED (self->error.arg = arg, FLAG_ERROR_UNDEFINED_FLAG)

#define CHECK_BOUNDS \
  if (self->nflags >= FLAGS_MAX) { \
    fprintf(stderr, "flag: exceeded max flags\n"); \
    exit(1); \
  }

#define REQUIRE_ARG \
  if (i == argc-1 || is_flag(args[i+1])) { \
    self->error.flag = flag; \
    return FLAG_ERROR_ARG_MISSING; \
  }

#define FLAG_TYPE(NAME, TYPE, CAP) \
  void \
  flagset_##NAME(flagset_t *self, TYPE *value, const char *name, const char *help) { \
    CHECK_BOUNDS; \
    self->flags[self->nflags++] = (flag_t) { \
      .name = name, \
      .help = help, \
      .type = FLAG_TYPE_##CAP, \
      .value = value \
    }; \
  } \
  void \
  flag_##NAME(TYPE *value, const char *name, const char *help) { \
    if (set == NULL) set = flagset_new(); \
    flagset_##NAME(set, value, name, help); \
  }

static inline bool
is_flag(const char *s) {
  return strlen(s) > 2 && s[0] == '-' && s[1] == '-';
}

static inline bool
is_negated(const char *s) {
  return strncmp(s, "--no-", 5) == 0;
}

static int
largest_flag_name(flagset_t *self) {
  int max = 0;
  for (int i = 0; i < self->nflags; ++i) {
    flag_t *flag = &self->flags[i];
    size_t len = strlen(flag->name);
    max = len > max ? len : max;
  }
  return max;
}

flagset_t *
flagset_new() {
  flagset_t *self = calloc(1, sizeof(flagset_t));
  if (!self) return NULL;
  self->usage = "[options] [arguments]";
  return self;
}

void
flagset_free(flagset_t *self) {
  free(self);
}

static flag_t *
flagset_find(flagset_t *self, const char *arg) {
  for (int i = 0; i < self->nflags; ++i) {
    flag_t *flag = &self->flags[i];

    if (strcmp(arg+2, flag->name) == 0) {
      return flag;
    }

    if (is_negated(arg) && strcmp(arg+5, flag->name) == 0) {
      return flag;
    }
  }
  return NULL;
}

flag_error
flagset_parse(flagset_t *self, int argc, const char **args) {
  flagset_bool(self, &self->showVersion, "version", "Output version");
  flagset_bool(self, &self->showHelp, "help", "Output help");

  for (int i = 0; i < argc; ++i) {
    const char *arg = args[i];

    // Regular argument
    if (!is_flag(arg)) {
      if (self->argc >= FLAGS_MAX_ARGS) {
        fprintf(stderr, "flag: exceeded max args\n");
        exit(1);
      }
      self->argv[self->argc++] = arg;
      continue;
    }

    // Flag lookup
    flag_t *flag = flagset_find(self, arg);
    if (flag == NULL) return ERROR_UNDEFINED;

    // Error on non-bool negation
    if (flag->type != FLAG_TYPE_BOOL && is_negated(arg)) {
      return ERROR_UNDEFINED;
    }

    // Parse value
    switch (flag->type) {
      case FLAG_TYPE_INT: {
        REQUIRE_ARG;
        int rc = sscanf(args[++i], "%d", (int *) flag->value);
        if (rc == 0) return ERROR_PARSING;
        break;
      }
      case FLAG_TYPE_STRING:
        REQUIRE_ARG;
        *(const char **) flag->value = args[++i];
        break;
      case FLAG_TYPE_BOOL:
        *(bool *) flag->value = is_negated(arg) ? false : true;
        break;
    }
  }

  if (self->showHelp) {
    return FLAG_SHOW_HELP;
  }

  if (self->showVersion) {
    return FLAG_SHOW_VERSION;
  }

  return FLAG_OK;
}

void
flagset_write_usage(flagset_t *self, FILE *fp, const char *name) {
  fprintf(fp, "\n  Usage: %s %s\n", name, self->usage);
  fprintf(fp, "\n  Options:\n");

  int max = largest_flag_name(self);

  for (int i = 0; i < self->nflags; ++i) {
    flag_t *flag = &self->flags[i];
    fprintf(fp, "    --%-*s %s", max+1, flag->name, flag->help);
    switch (flag->type) {
      case FLAG_TYPE_STRING:
        fprintf(fp, " (%s)", *(char **) flag->value);
        break;
      case FLAG_TYPE_INT:
        fprintf(fp, " (%d)", *(int *) flag->value);
        break;
      case FLAG_TYPE_BOOL:
        break;
    }
    fprintf(fp, "\n");
  }

  fprintf(fp, "\n");
}

void
flag_parse(int argc, const char **args, const char *version) {
  const char *name = args[0];
  flag_error err = flagset_parse(set, argc-1, args+1);

  switch (err) {
    case FLAG_SHOW_VERSION:
      printf("%s\n", version);
      exit(0);
      break;
    case FLAG_SHOW_HELP:
      flagset_write_usage(set, stdout, name);
      exit(1);
      break;
    case FLAG_ERROR_PARSING:
      fprintf(stderr, "invalid value for --%s\n", set->error.flag->name);
      exit(1);
      break;
    case FLAG_ERROR_ARG_MISSING:
      fprintf(stderr, "missing value for --%s\n", set->error.flag->name);
      exit(1);
      break;
    case FLAG_ERROR_UNDEFINED_FLAG:
      fprintf(stderr, "undefined flag %s\n", set->error.arg);
      exit(1);
      break;
    case FLAG_OK:
      break;
  }
}

FLAG_TYPE(int, int, INT);
FLAG_TYPE(bool, bool, BOOL);
FLAG_TYPE(string, const char *, STRING);

#undef CHECK_BOUNDS
#undef REQUIRE_ARG
#undef UNDEFINED_ERROR
#undef PARSE_ERROR
#undef FLAG_TYPE

