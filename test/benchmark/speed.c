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

#include <string.h>
#include <stdlib.h>

#include <ec/ec.h>

#ifndef DO_MAX
#define DO_MAX 24
#endif

void dec(size_t *i)
{
    *i = *i - 1;
}

void inc(size_t *i)
{
    *i = *i + 1;

#ifdef DO_THROW
    ec_throw_str_static(ECX_EC, "Woops!");
#endif
}

size_t lots(void (*action)(size_t *), size_t total)
{
    size_t i = 0;

#ifdef DO_TRY
    ec_try {
#endif

#ifdef DO_WITH
        ec_with(&i, (void (*)(void *))dec)
#endif

#ifdef DO_WITH_ON_X
        ec_with_on_x(&i, (void (*)(void *))dec)
#endif

        action(&i);

#ifdef DO_TRY
    }
    ec_finally {
#endif
        total += i;
#ifdef DO_TRY
    }
#endif

    return total;
}

int main()
{
    size_t max = 1;
    max <<= DO_MAX;

    printf("Loop Max = %zu\n", max);

    size_t total = 0;
    size_t i;
    for (i = 0; i < max; i++) {
        total = lots(inc, total);
    }

    /* printf("%zu\n", total); */

    return 0;
}
