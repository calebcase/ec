/* Copyright 2011 Caleb Case
 *
 * This file is part of the EC Library.
 *
 * The EC Library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * The EC Library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the EC Library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <check.h>
#include <stdlib.h>

#include <ec/ec.h>
#include <ec/static/ec.h>

static int with_free_ok_called = 0;
static int with_free_x_called = 0;

static void
with_free_ok(void *data)
{
    free(data);
    with_free_ok_called = 1;
}

static void
with_free_x(void *data)
{
    free(data);
    with_free_x_called = 1;
    ec_throw_str_static(ECX_EC, "Failed to be free? Impossible!");
}

START_TEST(with_ok)
{
    ec_try {
        int *free_me = malloc(sizeof(int));
        fail_unless(free_me != NULL, NULL);

        with_free_ok_called = 0;
        ec_with(free_me, with_free_ok) {
            *free_me = 7;
        }
        fail_unless(with_free_ok_called == 1, NULL);
    }
    ec_catch {
        fail("No exception was thrown...");
    }
}
END_TEST

START_TEST(with_ok_thrown)
{
    ec_try {
        int *free_me = malloc(sizeof(int));
        fail_unless(free_me != NULL, NULL);

        with_free_ok_called = 0;
        ec_with(free_me, with_free_ok) {
            ec_throw_str_static(ECX_EC, "An exception from within.");
        }
        fail("An exception should have been thrown.");
    }
    ec_catch {
        fail_unless(with_free_ok_called == 1, NULL);
    }
}
END_TEST

START_TEST(with_x)
{
    ec_try {
        int *free_me = malloc(sizeof(int));
        fail_unless(free_me != NULL, NULL);

        with_free_x_called = 0;
        ec_with(free_me, with_free_x) {
            *free_me = 7;
        }
        fail("An exception should have been thrown.");
    }
    ec_catch {
        fail_unless(with_free_x_called == 1, NULL);
    }
}
END_TEST

START_TEST(with_x_thrown)
{
    ec_try {
        int *free_me = malloc(sizeof(int));
        fail_unless(free_me != NULL, NULL);

        with_free_x_called = 0;
        ec_with(free_me, with_free_x) {
            /* Note: The exception in with_free_x causes the following to be
             * printed out early and cleaned up.
             */
            ec_throw_str_static(ECX_EC, "An exception from within.");
        }
        fail("An exception should have been thrown.");
    }
    ec_catch {
        fail_unless(with_free_x_called == 1, NULL);
    }
}
END_TEST

START_TEST(with_on_x_ok)
{
    ec_try {
        int *free_me = malloc(sizeof(int));
        fail_unless(free_me != NULL, NULL);

        with_free_ok_called = 0;
        ec_with_on_x(free_me, with_free_ok) {
            *free_me = 7;
        }
        fail_unless(with_free_ok_called == 0, NULL);
        fail_unless(*free_me == 7, NULL);

        free(free_me);
    }
    ec_catch {
        fail("No exception was thrown...");
    }
}
END_TEST

START_TEST(with_on_x_x)
{
    ec_try {
        int *free_me = malloc(sizeof(int));
        fail_unless(free_me != NULL, NULL);

        with_free_x_called = 0;
        ec_with_on_x(free_me, with_free_x) {
            *free_me = 7;
        }
        fail_unless(with_free_x_called == 0, NULL);
        fail_unless(*free_me == 7, NULL);
    }
    ec_catch {
        fail("No exception was thrown...");
    }
}
END_TEST

START_TEST(with_nested_3)
{
    ec_try {
        int *a = malloc(sizeof(int));
        fail_unless(a != NULL, NULL);
        ec_with(a, free) {
            *a = 1;

            int *b = malloc(sizeof(int));
            fail_unless(b != NULL, NULL);
            ec_with(b, free) {
                *b = 2;

                int *c = malloc(sizeof(int));
                fail_unless(c != NULL, NULL);
                ec_with(c, free) {
                    *c = 3;

                    fail_unless(*a == 1, NULL);
                    fail_unless(*b == 2, NULL);
                    fail_unless(*c == 3, NULL);
                }
            }
        }
    }
    ec_catch {
        fail("No exception was thrown...");
    }
}
END_TEST

START_TEST(with_nested_3_alt)
{
    ec_try {
        int *a = malloc(sizeof(int));
        fail_unless(a != NULL, NULL);

        int *b = malloc(sizeof(int));
        fail_unless(b != NULL, NULL);

        int *c = malloc(sizeof(int));
        fail_unless(c != NULL, NULL);

        ec_with(a, free)
        ec_with(b, free)
        ec_with(c, free) {
            *a = 1;
            *b = 2;
            *c = 3;

            fail_unless(*a == 1, NULL);
            fail_unless(*b == 2, NULL);
            fail_unless(*c == 3, NULL);
        }
    }
    ec_catch {
        fail("No exception was thrown...");
    }
}
END_TEST

Suite *
with_suite(void)
{
    Suite *s = suite_create("With");

    TCase *tc_with = tcase_create("basic with");
    tcase_add_test(tc_with, with_ok);
    tcase_add_test(tc_with, with_ok_thrown);
    tcase_add_test(tc_with, with_x);
    tcase_add_test(tc_with, with_x_thrown);
    tcase_add_test(tc_with, with_on_x_ok);
    tcase_add_test(tc_with, with_on_x_x);
    suite_add_tcase(s, tc_with);

    TCase *tc_with_nested = tcase_create("nested with");
    tcase_add_test(tc_with_nested, with_nested_3);
    tcase_add_test(tc_with_nested, with_nested_3_alt);
    suite_add_tcase(s, tc_with_nested);

    return s;
}

int
main(void)
{
    int failed = 0;

    SRunner *sr = srunner_create(with_suite());

    srunner_run_all(sr, CK_NORMAL);
    failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
