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

#include <ec/static/ec.h>

#include <stdlib.h>
#include <string.h>

/* Global per-thread error stack. */
__thread struct ec ec_stack = {
    .env = NULL,
    .winding = NULL,
    .error = {
        .type = NULL,
        .data = NULL,
        .data_cleanup = NULL,
        .data_fprint = NULL,
    },
    .place = {
        .file = NULL,
        .function = NULL,
        .line = 0,
    },
};

/*** Winding ***/

int
ec_winding_init_and_wind(
        struct ec_winding *winding,
        void *data,
        ec_unwind_f unwind)
{
    winding->next = ec_stack.winding;
    winding->data = data;
    winding->unwind = unwind;

    ec_stack.winding = winding;

    return 1;
}

/*** Error Stack ***/

ec_jmp_buf *
ec_swap_env(ec_jmp_buf *env)
{
    ec_jmp_buf *previous = ec_stack.env;
    ec_stack.env = env;
    return previous;
}

struct ec_winding *
ec_swap_winding(struct ec_winding *winding)
{
    struct ec_winding *previous = ec_stack.winding;
    ec_stack.winding = winding;
    return previous;
}

ec_jmp_buf *
ec_env(ec_jmp_buf *env)
{
    if (env != NULL) ec_stack.env = env;
    return ec_stack.env;
}

const char *
ec_type(const char *type)
{
    if (type != NULL) ec_stack.error.type = type;
    return ec_stack.error.type;
}

const void *
ec_get_data()
{
    return ec_stack.error.data;
}

const char *
ec_get_file()
{
    return ec_stack.place.file;
}

const char *
ec_get_function()
{
    return ec_stack.place.function;
}

unsigned int
ec_get_line()
{
    return ec_stack.place.line;
}

void
ec_set_error(
        const char *type,
        void *data,
        void (*data_cleanup)(void *data),
        void (*data_fprint)(FILE *stream, void *data))
{
    /* If there is already exception data present
     * then attempt to print it and cleanup.
     */
    if (ec_stack.error.type != NULL) {
        /* Print the exception data to stderr. */
        ec_fprint(stderr);

        /* Cleanup data. */
        if (ec_stack.error.data_cleanup != NULL) {
            ec_stack.error.data_cleanup(ec_stack.error.data);
        }
    }

    ec_stack.error.type = type;
    ec_stack.error.data = data;
    ec_stack.error.data_cleanup = data_cleanup;
    ec_stack.error.data_fprint = data_fprint;
}

void
ec_set_place(
        const char *file,
        const char *function,
        unsigned int line)
{
    if (ec_stack.place.file != NULL) free(ec_stack.place.file);
    ec_stack.place.file = strdup(file);

    if (ec_stack.place.function != NULL) free(ec_stack.place.function);
    ec_stack.place.function = strdup(function);

    ec_stack.place.line = line;
}

void
ec_unwind(enum ec_unwind_amount amount)
{
    struct ec_winding *head = ec_stack.winding;

    /* Its important to remove the winding BEFORE calling the unwind action.
     * Doing so prevents the unwind action from being called multiple times if
     * it throws an exception. Note: In event of an exception thrown in an
     * unwind action the new exception will replace the existing one and
     * continue calling the remaining unwind actions!
     */
    switch (amount) {
        case EC_UNWIND_DISCARD_ONE:
            ec_stack.winding = ec_stack.winding->next;
            break;
        case EC_UNWIND_ONE:
            ec_stack.winding = ec_stack.winding->next;
            head->unwind(head->data);
            break;
        case EC_UNWIND_ALL:
            while (head != NULL) {
                ec_stack.winding = ec_stack.winding->next;
                head->unwind(head->data);
                head = ec_stack.winding;
            }
            break;
    }
}

void
ec_clean()
{
    if (ec_stack.error.data_cleanup != NULL) {
        ec_stack.error.data_cleanup(ec_stack.error.data);
    }
    ec_stack.error.type = NULL;
    ec_stack.error.data = NULL;
    ec_stack.error.data_cleanup = NULL;
    ec_stack.error.data_fprint = NULL;

    free(ec_stack.place.file);
    free(ec_stack.place.function);
    ec_stack.place.file = NULL;
    ec_stack.place.function = NULL;
    ec_stack.place.line = 0;
}

void
ec_fprint(FILE *stream)
{
    fprintf(stream,
            "%s:%u: %s: Exception(%s)",
            ec_stack.place.file,
            ec_stack.place.line,
            ec_stack.place.function,
            ec_stack.error.type);

    if (ec_stack.error.data_fprint != NULL) {
        fprintf(stream, " ");
        ec_stack.error.data_fprint(stream, ec_stack.error.data);
    }

    fprintf(stream, "\n");
}

void
ec_fprint_str(FILE *stream, char *data)
{
    fprintf(stream, "%s", data);
}

void
ec_fprint_errno_str(FILE *stream, char *data)
{
    fprintf(stream, "%s", strerror(errno));
    if (data != NULL) {
        fprintf(stream, " %s", data);
    }
}

void
ec_shadow(const char *types[2])
{
    if (ec_type(NULL) == types[0]) {
        ec_type(types[1]);
    }
}

const char *
ec_errno_type(int error) 
{
    switch(error) {
        case E2BIG: return ECX_E2BIG;
        case EACCES: return ECX_EACCES;
        case EADDRINUSE: return ECX_EADDRINUSE;
        case EADDRNOTAVAIL: return ECX_EADDRNOTAVAIL;
        case EAFNOSUPPORT: return ECX_EAFNOSUPPORT;
        case EAGAIN: return ECX_EAGAIN;
        case EALREADY: return ECX_EALREADY;
        case EBADF: return ECX_EBADF;
        case EBADMSG: return ECX_EBADMSG;
        case EBUSY: return ECX_EBUSY;
        case ECANCELED: return ECX_ECANCELED;
        case ECHILD: return ECX_ECHILD;
        case ECONNABORTED: return ECX_ECONNABORTED;
        case ECONNREFUSED: return ECX_ECONNREFUSED;
        case ECONNRESET: return ECX_ECONNRESET;
        case EDEADLK: return ECX_EDEADLK;
        case EDESTADDRREQ: return ECX_EDESTADDRREQ;
        case EDOM: return ECX_EDOM;
        case EDQUOT: return ECX_EDQUOT;
        case EEXIST: return ECX_EEXIST;
        case EFAULT: return ECX_EFAULT;
        case EFBIG: return ECX_EFBIG;
        case EHOSTUNREACH: return ECX_EHOSTUNREACH;
        case EIDRM: return ECX_EIDRM;
        case EILSEQ: return ECX_EILSEQ;
        case EINPROGRESS: return ECX_EINPROGRESS;
        case EINTR: return ECX_EINTR;
        case EINVAL: return ECX_EINVAL;
        case EIO: return ECX_EIO;
        case EISCONN: return ECX_EISCONN;
        case EISDIR: return ECX_EISDIR;
        case ELOOP: return ECX_ELOOP;
        case EMFILE: return ECX_EMFILE;
        case EMLINK: return ECX_EMLINK;
        case EMSGSIZE: return ECX_EMSGSIZE;
        case EMULTIHOP: return ECX_EMULTIHOP;
        case ENAMETOOLONG: return ECX_ENAMETOOLONG;
        case ENETDOWN: return ECX_ENETDOWN;
        case ENETRESET: return ECX_ENETRESET;
        case ENETUNREACH: return ECX_ENETUNREACH;
        case ENFILE: return ECX_ENFILE;
        case ENOBUFS: return ECX_ENOBUFS;
        case ENODATA: return ECX_ENODATA;
        case ENODEV: return ECX_ENODEV;
        case ENOENT: return ECX_ENOENT;
        case ENOEXEC: return ECX_ENOEXEC;
        case ENOLCK: return ECX_ENOLCK;
        case ENOLINK: return ECX_ENOLINK;
        case ENOMEM: return ECX_ENOMEM;
        case ENOMSG: return ECX_ENOMSG;
        case ENOPROTOOPT: return ECX_ENOPROTOOPT;
        case ENOSPC: return ECX_ENOSPC;
        case ENOSR: return ECX_ENOSR;
        case ENOSTR: return ECX_ENOSTR;
        case ENOSYS: return ECX_ENOSYS;
        case ENOTCONN: return ECX_ENOTCONN;
        case ENOTDIR: return ECX_ENOTDIR;
        case ENOTEMPTY: return ECX_ENOTEMPTY;
        case ENOTSOCK: return ECX_ENOTSOCK;
        case ENOTSUP: return ECX_ENOTSUP;
        case ENOTTY: return ECX_ENOTTY;
        case ENXIO: return ECX_ENXIO;
        /* case EOPNOTSUPP: return ECX_EOPNOTSUPP; */
        case EOVERFLOW: return ECX_EOVERFLOW;
        case EPERM: return ECX_EPERM;
        case EPIPE: return ECX_EPIPE;
        case EPROTO: return ECX_EPROTO;
        case EPROTONOSUPPORT: return ECX_EPROTONOSUPPORT;
        case EPROTOTYPE: return ECX_EPROTOTYPE;
        case ERANGE: return ECX_ERANGE;
        case EROFS: return ECX_EROFS;
        case ESPIPE: return ECX_ESPIPE;
        case ESRCH: return ECX_ESRCH;
        case ESTALE: return ECX_ESTALE;
        case ETIME: return ECX_ETIME;
        case ETIMEDOUT: return ECX_ETIMEDOUT;
        case ETXTBSY: return ECX_ETXTBSY;
        /* case EWOULDBLOCK: return ECX_EWOULDBLOCK; */
        case EXDEV: return ECX_EXDEV;
        default: return ECX_EC;
    }
}

/***Exception Types ***/

const char ECX_EC[]  = "Generic";
const char ECX_NULL[]  = "NULL";

const char ECX_E2BIG[]   = "E2BIG";
const char ECX_EACCES[]  = "EACCESS";
const char ECX_EADDRINUSE[] = "EADDRINUSE";
const char ECX_EADDRNOTAVAIL[] = "EADDRNOTAVAIL";
const char ECX_EAFNOSUPPORT[] = "EAFNOSUPPORT";
const char ECX_EAGAIN[] = "EAGAIN";
const char ECX_EALREADY[] = "EALREADY";
const char ECX_EBADF[] = "EBADF";
const char ECX_EBADMSG[] = "EBADMSG";
const char ECX_EBUSY[] = "EBUSY";
const char ECX_ECANCELED[] = "ECANCELED";
const char ECX_ECHILD[] = "ECHILD";
const char ECX_ECONNABORTED[] = "ECONNABORTED";
const char ECX_ECONNREFUSED[] = "ECONNREFUSED";
const char ECX_ECONNRESET[] = "ECONNRESET";
const char ECX_EDEADLK[] = "EDEADLK";
const char ECX_EDESTADDRREQ[] = "EDESTADDRREQ";
const char ECX_EDOM[] = "EDOM";
const char ECX_EDQUOT[] = "EDQUOT";
const char ECX_EEXIST[] = "EEXIST";
const char ECX_EFAULT[] = "EFAULT";
const char ECX_EFBIG[] = "EFBIG";
const char ECX_EHOSTUNREACH[] = "EHOSTUNREACH";
const char ECX_EIDRM[] = "EIDRM";
const char ECX_EILSEQ[] = "EILSEQ";
const char ECX_EINPROGRESS[] = "EINPROGRESS";
const char ECX_EINTR[] = "EINTR";
const char ECX_EINVAL[] = "EINVAL";
const char ECX_EIO[] = "EIO";
const char ECX_EISCONN[] = "EISCONN";
const char ECX_EISDIR[] = "EISDIR";
const char ECX_ELOOP[] = "ELOOP";
const char ECX_EMFILE[] = "EMFILE";
const char ECX_EMLINK[] = "EMLINK";
const char ECX_EMSGSIZE[] = "EMSGSIZE";
const char ECX_EMULTIHOP[] = "EMULTIHOP";
const char ECX_ENAMETOOLONG[] = "ENAMETOOLONG";
const char ECX_ENETDOWN[] = "ENETDOWN";
const char ECX_ENETRESET[] = "ENETRESET";
const char ECX_ENETUNREACH[] = "ENETUNREACH";
const char ECX_ENFILE[] = "ENFILE";
const char ECX_ENOBUFS[] = "ENOBUFS";
const char ECX_ENODATA[] = "ENODATA";
const char ECX_ENODEV[] = "ENODEV";
const char ECX_ENOENT[] = "ENOENT";
const char ECX_ENOEXEC[] = "ENOEXEC";
const char ECX_ENOLCK[] = "ENOLCK";
const char ECX_ENOLINK[] = "ENOLINK";
const char ECX_ENOMEM[] = "ENOMEM";
const char ECX_ENOMSG[] = "ENOMSG";
const char ECX_ENOPROTOOPT[] = "ENOPROTOOPT";
const char ECX_ENOSPC[] = "ENOSPC";
const char ECX_ENOSR[] = "ENOSR";
const char ECX_ENOSTR[] = "ENOSTR";
const char ECX_ENOSYS[] = "ENOSYS";
const char ECX_ENOTCONN[] = "ENOTCONN";
const char ECX_ENOTDIR[] = "ENOTDIR";
const char ECX_ENOTEMPTY[] = "ENOTEMPTY";
const char ECX_ENOTSOCK[] = "ENOTSOCK";
const char ECX_ENOTSUP[] = "ENOTSUP";
const char ECX_ENOTTY[] = "ENOTTY";
const char ECX_ENXIO[] = "ENXIO";
const char ECX_EOPNOTSUPP[] = "EOPNOTSUPP";
const char ECX_EOVERFLOW[] = "EOVERFLOW";
const char ECX_EPERM[] = "EPERM";
const char ECX_EPIPE[] = "EPIPE";
const char ECX_EPROTO[] = "EPROTO";
const char ECX_EPROTONOSUPPORT[] = "EPROTONOSUPPORT";
const char ECX_EPROTOTYPE[] = "EPROTOTYPE";
const char ECX_ERANGE[] = "ERANGE";
const char ECX_EROFS[] = "EROFS";
const char ECX_ESPIPE[] = "ESPIPE";
const char ECX_ESRCH[] = "ESRCH";
const char ECX_ESTALE[] = "ESTALE";
const char ECX_ETIME[] = "ETIME";
const char ECX_ETIMEDOUT[] = "ETIMEDOUT";
const char ECX_ETXTBSY[] = "ETXTBSY";
const char ECX_EWOULDBLOCK[] = "EWOULDBLOCK";
const char ECX_EXDEV[] = "EXDEV";

