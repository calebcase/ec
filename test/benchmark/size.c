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

#include <ec/ec.h>
#include <stdint.h>
#include <stdio.h>

uintptr_t s_top = 0;
uintptr_t s_try = 0;
uintptr_t s_call = 0;
uintptr_t s_with = 0;

void
call()
{
    int here = 0;
    s_call = (uintptr_t)&here;
}

void
try()
{
    int here = 0;
    s_try = (uintptr_t)&here;

    ec_try {
        call();
    }
    ec_catch { ec_rethrow; }
}

void nada(void *n) { }

void
with()
{
    int here = 0;
    s_with = (uintptr_t)&here;

    ec_with((void *)&here, nada) {
        call();
    }
}

int
main()
{
    int here = 0;
    s_top = (uintptr_t)&here;

    printf("Reliable lower bounds:\n\n");

    printf("ec_try      = %zu\n", sizeof(sigjmp_buf));
    printf("ec_with     = %zu\n", sizeof(struct ec_winding));

    printf("\nComputed stack sizes:\n\n");

    try();

    printf("ec_try      = %zu\n", s_try - s_call - (2 * sizeof(int)));

    with();

    printf("ec_with     = %zu\n", s_with - s_call - (2 * sizeof(int)));

    call();

    printf("func call   = %zu\n", s_top - s_call - sizeof(int));

    return 0;
}
