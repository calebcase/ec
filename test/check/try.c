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

START_TEST(try_catch)
{
    ec_try { }
    ec_catch {
        fail("Entered ec_catch, but nothing was thrown.");
    }
}
END_TEST

START_TEST(try_throw_catch)
{
    ec_try {
        ec_throw_str(ECX_EC) strdup("Catch me!");
    }
    ec_catch {
        fail_unless(strcmp(ec_get_data(), "Catch me!") == 0, NULL);
    }
}
END_TEST

START_TEST(try_throw_static_catch)
{
    ec_try {
        ec_throw_str_static(ECX_EC, "Catch me!");
    }
    ec_catch {
        fail_unless(strcmp(ec_get_data(), "Catch me!") == 0, NULL);
    }
}
END_TEST

Suite *
try_suite(void)
{
    Suite *s = suite_create("Try Throw Catch");

    TCase *tc_ttc = tcase_create("basic try/throw/catch");
    tcase_add_test(tc_ttc, try_catch);
    tcase_add_test(tc_ttc, try_throw_catch);
    tcase_add_test(tc_ttc, try_throw_static_catch);
    suite_add_tcase(s, tc_ttc);

    return s;
}

int
main(void)
{
    int failed = 0;

    SRunner *sr = srunner_create(try_suite());

    srunner_run_all(sr, CK_NORMAL);
    failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
