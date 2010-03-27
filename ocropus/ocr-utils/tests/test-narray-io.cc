#include <colib/colib.h>
#include "narray-io.h"

using namespace colib;
using namespace ocropus;

static void test_int32_io() {
    stdio f(tmpfile());
    int n = 0xC0DEFACE;
    write_int32(n, f);
    rewind(f);
    int k = read_int32(f);
    CHECK_CONDITION(k == n);
}

static void test_parse_vector() {
    doublearray a;
    parse_vector(a, "1.5 3.25 -1");
    CHECK_CONDITION(a.length() == 3);
    CHECK_CONDITION(a[0] == 1.5);
    CHECK_CONDITION(a[1] == 3.25);
    CHECK_CONDITION(a[2] == -1);
}

static void test_some_io() {
    stdio f(tmpfile());
    doublearray c;
    parse_vector(c, "3 10 57 42 26 7");
    bytearray a;
    copy(a, c);
    a.reshape(3, 2);
    bin_write(f, a);
    rewind(f);
    bytearray b;
    bin_read(b, f);
    CHECK_CONDITION(equal(a, b));
    rewind(f);
    text_write(f, c);
    doublearray d;
    rewind(f);
    text_read(f, d);
    CHECK_CONDITION(equal(c, d));    
    rewind(f);
    text_write(f, a);
    rewind(f);
    text_read(f, b);
    CHECK_CONDITION(equal(a, b));    
}

static void test_bin_append() {
    stdio f(tmpfile());
    bytearray a(3);
    a[0] = 1; a[1] = 2; a[2] = 3;
    bin_write(f, a);
    rewind(f);
    bin_append(f, a);
    rewind(f);
    bin_read(f, a);
    CHECK_CONDITION(a.length() == 6);
    CHECK_CONDITION(a[0] == 1 && a[1] == 2 && a[2] == 3);
    CHECK_CONDITION(a[3] == 1 && a[4] == 2 && a[5] == 3);
}

int main() {
    test_int32_io();
    test_parse_vector();
    test_some_io();
    test_bin_append();
    return 0;
}
