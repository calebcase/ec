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
#include <ec/ec.h>

int main()
{
    /* Storage for the exception data. */
    const char *e = NULL;
    
    ec_try {
        char *str = strdup("A string.");
        if (str == NULL)
            ec_throw_str_static(ECX_ENOMEM, "Can't allocate a string.");
    }
    ec_catch_a(ECX_ENOMEM, e) {
        /* Get coffee and try again? */
    }
    ec_catch {
        ec_rethrow;
    }

    return 0;
}

