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

#ifndef EC_STATIC_H
#define EC_STATIC_H 1

/* This is a private header provided for static linking. Everthing in here is
 * subject to change at any time and is not part of the public API. You have
 * been warned.
 */

#include <ec/ec.h>

/* The error stack structure.
 *
 * Initially all fields are NULL or 0.
 *
 * After an exception is thrown @type, @data, @file, and @function could be NULL.
 *
 * There is no guarantee these fields will be set to non-NULL values
 * (especially in out-of-memory situations). However, if they are non-NULL,
 * then they must be valid pointers.
 */
struct ec {
    /* Jump buffer storing the location to jump back to. */
    sigjmp_buf *env;

    /* Data which needs to unwound on an exception. */
    struct ec_winding *winding;

    struct {
        /* Exception type.
         *
         * Any pointer provided here is not expected to be free()'d or modified.
         * The pointer should uniquely identify a given exception type.
         *
         * This dictates what to expect in data.
         * The type's documentation should indicate what data will point to.
         *
         * May be NULL. If it is NULL, then data is expected to be NULL as well.
         */
        const char *type;

        /* Exception data as per the exception type. May be NULL. */
        void *data;

        /* After an exception is caught this function will be called to
         * perform any maintenance required on data (such as calling free).
         *
         * May be NULL.
         */
        void (*data_cleanup)(void *data);

        /* Data printer. */
        void (*data_fprint)(FILE *stream, void *data);
    } error;

    struct {
        /* The file in which the exception occurred.
         *
         * May be NULL. Should be free()'d after handling is complete.
         */
        char *file;

        /* The function in which the exception occurred. */
        char *function;

        /* The line in the file that the exception occurred on. */
        unsigned int line;
    } place;

    struct {
        /* Number of traces in buffer. */
        unsigned int size;

        /* A buffer for backtrace return addresses. */
        void *buf[128];
    } bt;
};

#endif /* EC_STATIC_H */
