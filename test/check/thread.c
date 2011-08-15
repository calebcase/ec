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
#include <pthread.h>
#include <unistd.h>

#include <ec/ec.h>
#include <ec/static/ec.h>


#define THREADS 10

/* The important thing here is that each thread receives its own exception stack. */

void *thread_main(char *name)
{
    const char *e = NULL;

    ec_try {
        usleep(rand() % 1000);

        char *msg = NULL;
        if (0 > asprintf(&msg, "%s", name)) msg = NULL;

        ec_throw_str(ECX_EC) msg;
    }
    ec_catch_a(ECX_EC, e) {
        ec_fprint(stdout);
    }
    ec_catch {
        fail("Exception should already have been handled!");
    }

    free(name);
    return NULL;
}

START_TEST(thread_simple)
{
    /* Seed random number gen. */
    srand(time(NULL));

    /* Start the threads. */
    pthread_t pth[THREADS];

    for (int i = 0; i < THREADS; i++) {
        char *name = NULL;
        if (0 > asprintf(&name, "%i", i)) name = NULL;

        pthread_create(&pth[i], NULL, (void *(*)(void *))thread_main, name);
    }

    /* Wait for the threads to finish. */
    for (int i = 0; i < THREADS; i++) {
        pthread_join(pth[i], NULL);
    }
}
END_TEST

Suite *
thread_suite(void)
{
    Suite *s = suite_create("Thread");

    TCase *tc_thread = tcase_create("Thread");
    tcase_add_test(tc_thread, thread_simple);
    suite_add_tcase(s, tc_thread);

    return s;
}

int
main(void)
{
    int failed = 0;

    SRunner *sr = srunner_create(thread_suite());

    srunner_run_all(sr, CK_NORMAL);
    failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
