
#include "describe/describe.h"
#include "flag.h"

int
main(){
  describe("flag_parse()") {
    it("should act as a singleton") {
      int n = 100, c = 5;
      const char *addr = NULL;

      const char *args[] = { "program", "--requests", "500", "--address", ":3000" };
      flag_int(&n, "requests", "Number of requests");
      flag_int(&c, "concurrency", "Number of concurrent requests");
      flag_string(&addr, "address", "Target URL");
      flag_parse(5, args, "1.0.0");

      assert(n == 500);
      assert(c == 5);
      assert(addr == ":3000");
    }
  }

  describe("flagset_parse()") {
    it("should parse flags") {
      int n = 1000, c = 5, b = 50;
      const char *args[] = { "--requests", "100", "--concurrency", "10" };

      flagset_t *set = flagset_new();
      flagset_int(set, &n, "requests", "Number of requests");
      flagset_int(set, &c, "concurrency", "Number of concurrent requests");
      flagset_int(set, &b, "batch-size", "Batch size");
      flag_error err = flagset_parse(set, 4, args);

      assert(err == 0);
      assert(err == FLAG_OK);
      assert(n == 100);
      assert(c == 10);
      assert(b == 50);

      flagset_free(set);
    }

    it("should return error when parsing fails") {
      int n = 0;
      const char *args[] = { "--num", "foo" };

      flagset_t *set = flagset_new();
      flagset_int(set, &n, "num", "Number of requests");
      flag_error err = flagset_parse(set, 2, args);

      assert(err == FLAG_ERROR_PARSING);
      assert(set->error.flag->name == "num");

      flagset_free(set);
    }

    it("should return error when arg is missing") {
      int n = 0;
      const char *args[] = { "--num" };

      flagset_t *set = flagset_new();
      flagset_int(set, &n, "num", "Number of requests");
      flag_error err = flagset_parse(set, 1, args);

      assert(err == FLAG_ERROR_ARG_MISSING);
      assert(set->error.flag->name == "num");

      flagset_free(set);
    }

    it("should return error when arg is flag") {
      int n = 0;
      const char *args[] = { "--num", "--other" };

      flagset_t *set = flagset_new();
      flagset_int(set, &n, "num", "Number of requests");
      flag_error err = flagset_parse(set, 2, args);

      assert(err == FLAG_ERROR_ARG_MISSING);
      assert(set->error.flag->name == "num");

      flagset_free(set);
    }

    it("should error on undefined flags") {
      const char *args[] = { "--whatever" };

      flagset_t *set = flagset_new();
      flag_error err = flagset_parse(set, 1, args);

      assert(err == FLAG_ERROR_UNDEFINED_FLAG);

      flagset_free(set);
    }

    it("should not match flag prefixes") {
      const char *args[] = { "--versioning" };

      flagset_t *set = flagset_new();
      flag_error err = flagset_parse(set, 1, args);

      assert(err == FLAG_ERROR_UNDEFINED_FLAG);

      flagset_free(set);
    }

    it("should error on non-bool negation") {
      int n = 0;
      const char *args[] = { "--no-num" };

      flagset_t *set = flagset_new();
      flagset_int(set, &n, "num", "");
      flag_error err = flagset_parse(set, 1, args);

      assert(err == FLAG_ERROR_UNDEFINED_FLAG);

      flagset_free(set);
    }

    it("should error on missing flag negation") {
      const char *args[] = { "--no-whatever" };

      flagset_t *set = flagset_new();
      flag_error err = flagset_parse(set, 1, args);

      assert(err == FLAG_ERROR_UNDEFINED_FLAG);

      flagset_free(set);
    }

    it("should provide resulting argc and argv") {
      int n = 0;
      const char *args[] = { "some", "--num", "5", "other", "args" };

      flagset_t *set = flagset_new();
      flagset_int(set, &n, "num", "Number of requests");
      flag_error err = flagset_parse(set, 5, args);

      assert(err == FLAG_OK);
      assert(3 == set->argc);
      assert("some" == set->argv[0]);
      assert("other" == set->argv[1]);
      assert("args" == set->argv[2]);

      flagset_free(set);
    }
  }

  describe("flagset_int()") {
    it("should support ints") {
      int n = 0;
      const char *args[] = { "--num", "123" };

      flagset_t *set = flagset_new();
      flagset_int(set, &n, "num", "");
      flag_error err = flagset_parse(set, 2, args);

      assert(err == FLAG_OK);
      assert(n == 123);

      flagset_free(set);
    }
  }

  describe("flagset_string()") {
    it("should support strings") {
      const char *addr = NULL;
      const char *args[] = { "--address", "localhost:3000" };

      flagset_t *set = flagset_new();
      flagset_string(set, &addr, "address", "Bind address");
      flag_error err = flagset_parse(set, 2, args);

      assert(err == FLAG_OK);
      assert(addr == "localhost:3000");

      flagset_free(set);
    }
  }

  describe("flagset_bool()") {
    it("should support bools") {
      bool v = false;
      const char *args[] = { "--enable" };

      flagset_t *set = flagset_new();
      flagset_bool(set, &v, "enable", "");
      flag_error err = flagset_parse(set, 1, args);

      assert(err == FLAG_OK);
      assert(v == true);

      flagset_free(set);
    }

    it("should support negation") {
      bool v = true;
      const char *args[] = { "--no-colors" };

      flagset_t *set = flagset_new();
      flagset_bool(set, &v, "colors", "");
      flag_error err = flagset_parse(set, 1, args);

      assert(err == FLAG_OK);
      assert(v == false);

      flagset_free(set);
    }
  }

  return assert_failures();
}
