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

const char API_INTERNAL[] = "An internal failure has occured.";

START_TEST(shadow_simple)
{
    const char *e = NULL;

    ec_try {
        /* Shadow an out of memory exception as an internal exception. */
        ec_shadow_on_x(ECX_ENOMEM, API_INTERNAL) {
            ec_throw_str(ECX_ENOMEM) NULL;
        }
    }
    ec_catch_a(ECX_ENOMEM, e) {
        fail("Exception wasn't shadowed properly!");
    }
    ec_catch_a(API_INTERNAL, e) {
        /* Good! */
    }
    ec_catch {
        fail("Exception should already have been handled!");
    }
}
END_TEST

Suite *
shadow_suite(void)
{
    Suite *s = suite_create("Shadow");

    TCase *tc_simple = tcase_create("Shadow Simple");
    tcase_add_test(tc_simple, shadow_simple);
    suite_add_tcase(s, tc_simple);

    return s;
}

int
main(void)
{
    int failed = 0;

    SRunner *sr = srunner_create(shadow_suite());

    srunner_run_all(sr, CK_NORMAL);
    failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
