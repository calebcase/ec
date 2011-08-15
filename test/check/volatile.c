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

/* Automatic variables which are used inside and outside a ec_try block must
 * carry the volatile specifier. The reason is that optimizations may result in
 * values not being rewritten to storage after being modified. When a
 * longjmp(...) is performed the modification is lost. Volatile requires that
 * the modification be written to storage (and that reads load the value from
 * storage) protecting the modification in the event of a longjmp(...).
 */

START_TEST(volatile_off)
{
    /* Don't do this in real code... Use volatile.*/
    int value = 0;

    ec_try {
        value = 7;
    }
    ec_finally {
        /* Value is 7 because longjmp wasn't invoked. */
        fail_unless(value == 7, NULL);
    }
}
END_TEST

START_TEST(volatile_off_x)
{
    /* Don't do this in real code... Use volatile.*/
    int value = 0;

    ec_try {
        value = 7;
        ec_throw_str_static(ECX_EC, "Cause longjmp.");
    }
    ec_finally {
        /* Value is 0 because longjmp was invoked and modification was lost. */
        fail_unless(value == 0, NULL);
    }
}
END_TEST

START_TEST(volatile_on)
{
    volatile int value = 0;

    ec_try {
        value = 7;
    }
    ec_finally {
        /* Value is 7 because longjmp was not invoked. */
        fail_unless(value == 7, NULL);
    }
}
END_TEST

START_TEST(volatile_on_x)
{
    volatile int value = 0;

    ec_try {
        value = 7;
        ec_throw_str_static(ECX_EC, "Cause longjmp.");
    }
    ec_finally {
        /* Value is 7 because of volatile. */
        fail_unless(value == 7, NULL);
    }
}
END_TEST

/* The following tests are in part a reminder that pointers are themselves
 * automatic variables.
 */

START_TEST(volatile_p_off)
{
    /* Don't do this in real code... Use volatile.*/
    int *value = NULL;

    ec_try {
        value = malloc(sizeof(int));
        fail_unless(value != NULL, NULL);

        *value = 7;
    }
    ec_finally {
        fail_unless(value != NULL, NULL);
        fail_unless(*value == 7, NULL);
        free(value);
    }
}
END_TEST

START_TEST(volatile_p_off_x)
{
    /* Don't do this in real code... Use volatile.*/
    int *value = NULL;

    ec_try {
        value = malloc(sizeof(int));
        fail_unless(value != NULL, NULL);

        *value = 7;
        ec_throw_str_static(ECX_EC, "Cause longjmp.");
    }
    ec_finally {
        /* Value is NULL because longjmp was invoked and modification was lost.
         * On top of that badness, we are now leaking memory.
         */
        fail_unless(value == NULL, NULL);
    }
}
END_TEST

START_TEST(volatile_p_on)
{
    /* Note: It's the pointer that is volatile, not what it's pointing at. */
    int * volatile value = NULL;

    ec_try {
        value = malloc(sizeof(int));
        fail_unless(value != NULL, NULL);

        *value = 7;
    }
    ec_finally {
        fail_unless(value != NULL, NULL);
        fail_unless(*value == 7, NULL);
        free(value);
    }
}
END_TEST

START_TEST(volatile_p_on_x)
{
    int * volatile value = NULL;

    ec_try {
        value = malloc(sizeof(int));
        fail_unless(value != NULL, NULL);

        *value = 7;
        ec_throw_str_static(ECX_EC, "Cause longjmp.");
    }
    ec_finally {
        fail_unless(value != NULL, NULL);
        fail_unless(*value == 7, NULL);
        free(value);
    }
}
END_TEST

/* It is not necessary to specify volatile if the value's modification is not
 * used outside ec_try. In the cases below the exception data is stored in e. e
 * is modified in ec_try, but that modification is replaced with the
 * ec_catch_a(...).
 *
 * This is a reminder that your exception data storage doesn't usually need to
 * be volatile.
 */

START_TEST(volatile_e_off_x)
{
    const char *e = NULL;

    ec_try {
        e = ECX_NULL;
        ec_throw_str_static(ECX_EC, "Cause longjmp.");
    }
    ec_catch_a(ECX_EC, e) {
        fail_unless(strcmp(e, "Cause longjmp.") == 0, NULL);
    }
    ec_catch {
        fail("Exception should already have been handled.");
    }

    fail_unless(strcmp(e, "Cause longjmp.") == 0, NULL);
}
END_TEST

Suite *
volatile_suite(void)
{
    Suite *s = suite_create("Volatile");

    TCase *tc_volatile = tcase_create("Volatile");
    tcase_add_test(tc_volatile, volatile_off);
    tcase_add_test(tc_volatile, volatile_off_x);
    tcase_add_test(tc_volatile, volatile_on);
    tcase_add_test(tc_volatile, volatile_on_x);
    suite_add_tcase(s, tc_volatile);

    TCase *tc_volatile_p = tcase_create("Volatile Pointer");
    tcase_add_test(tc_volatile_p, volatile_p_off);
    tcase_add_test(tc_volatile_p, volatile_p_off_x);
    tcase_add_test(tc_volatile_p, volatile_p_on);
    tcase_add_test(tc_volatile_p, volatile_p_on_x);
    suite_add_tcase(s, tc_volatile_p);

    TCase *tc_volatile_e = tcase_create("Volatile Exception Data");
    tcase_add_test(tc_volatile_e, volatile_e_off_x);
    suite_add_tcase(s, tc_volatile_e);

    return s;
}

int
main(void)
{
    int failed = 0;

    SRunner *sr = srunner_create(volatile_suite());

    srunner_run_all(sr, CK_NORMAL);
    failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
