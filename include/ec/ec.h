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

#ifndef EC_H
#define EC_H 1

#include <errno.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

/*** Exception Macros
 *
 * These macros are structured similar to the if/else if/else blocks.
 *
 * ec_try {
 *     Something that might throw an exception.
 * }
 * ec_catch_a(t1, d1) {
 *     Catch an exception of type t1 and store it's data in d1.
 * }
 * ec_catch_a(t2, d2) { }
 * ...
 * ec_catch {
 *     Catch any exception type.
 * }
 *
 * If an exception is thrown in ec_try, then it will be either caught in one of
 * the ec_catch_a(...) clauses, or by the final ec_catch. As an alternative to
 * ec_catch, ec_finally may be used and will be run regardless of whether an
 * exception is thrown in ec_try.
 *
 * You should declare variables 'volatile' if you need to access them both
 * inside and outside an ec_try block.
 *
 * Avoid using ec_try unless you can reasonable expect to recover from an
 * exception and restart executing something useful. A good example of this
 * might be the top level loop of a server. ec_try takes a non-trivial amount
 * of time to perform it's setup and consumes relatively large amounts of stack
 * space. In comparison, ec_with(...) takes almost no time to setup and
 * consumes positively tiny amounts of stack space. Generally consider using
 * ec_with(...) first.
 *
 * ec_with(...) and ec_with_on_x(...) provide a way to have a function called
 * on a given variable in an exception aware way. They are particularly useful
 * for providing cleanup of variables in the event of an exception. Consider a
 * string which needs to be free'd in the event of an exception:
 *
 * char *foo = NULL;
 *
 * ec_with_on_x(foo, free) {
 *     strdup("bar");
 *     i_throw_an_exception();
 * }
 *
 * Exceptions are thrown using ec_throw(...) and friends. See their
 * documentation below for specific usage. A simple example using
 * ec_throw_str(...):
 *
 * ec_throw_str(ECX_EC) strdup("Some error message!");
 *
 * ec_shadow_on_x(...) is provided as an aid to API designers who desire (or
 * presumably should desire) a consistent set of exceptions to be thrown at the
 * interface. ec_shadow_on_x(...) will convert one type of exception to another
 * (assuming that the exception data types are compatible).
 * ec_shadow_on_x_with(...) provides even greater control over how to determine
 * which types to convert and how. They both make use of the ec_with(...)
 * mechanism and thus should be preferentially used over ec_try.
 *
 * Some things to be aware of when using these macros:
 *
 *  - The macros make extensive use of C99 for loops and you will therefor need
 *    to use a compiler which supports them. C99 for loops make it possible to
 *    avoid the use of unpleasant begin/end pairs.
 *
 *  - Signal aware sigsetjmp/siglongjmp are used. This implies a POSIX
 *    environment.
 *
 *  - Arguments to macros may be evaluated multiple times (even if the current
 *    version doesn't). Do not pass them statements like 'i++' or any other
 *    statement which would result in a side-effect.
 *
 ***/

/* Saves the current execution context via sigsetjmp(...). If an exception is
 * thrown via ec_throw(...) then execution resumes here. ec_try may be followed
 * by one or more ec_catch_a(...) and must end with either an ec_catch or
 * ec_finally.
 */
#define ec_try \
    /* Setup jump buffer. */ \
    for (sigjmp_buf ec_env_, \
         *ec_penv_ = ec_swap_env(&ec_env_), \
         *ec_try_outer_once_ = NULL; \
         ec_try_outer_once_ == NULL; \
         ec_try_outer_once_ = (void *)1) \
        /* Swap out and save current winding. */ \
        for (struct ec_winding *ec_pwinding_ = ec_swap_winding(NULL), \
             *ec_winding_once_ = NULL; \
             ec_winding_once_ == NULL; \
             ec_winding_once_ = (void *)1) \
            /* Save current location. */ \
            /* This is where we are restored to after a throw. */ \
            if (sigsetjmp(ec_env_, 1) == 0) { \
                /* If an exception is throw, then the increment is not run, */ \
                /* otherwise the previous exception environment and winding */ \
                /* are restored. */ \
                for (int ec_try_inner_once_ = 0; \
                     ec_try_inner_once_ == 0; \
                     ec_try_inner_once_ = 1, \
                     ec_swap_env(ec_penv_), \
                     ec_swap_winding(ec_pwinding_)) \

/* Catches a specific exception type t and assigns the exception data to d.
 * After the block is exited all exception information will be automatically
 * cleaned up (e.g. type and data). If you need to keep the exception
 * information for use after the block you MUST COPY IT.
 */
#define ec_catch_a(t,d) \
            /* An exception was thrown, catch it here if type matches. */ \
            } else if ((t) == ec_type(NULL)) { \
                for (ec_swap_env(ec_penv_), /* Restore prev environment. */ \
                     ec_swap_winding(ec_pwinding_), /* Restore prev winding. */ \
                     (d) = ec_get_data(), /* Set data. */ \
                     ec_try_outer_once_ = (void *)2; /* Start catching in 'catcha'. */ \
                     ec_try_outer_once_ == (void *)2; /* Only run the loop once. */ \
                     ec_try_outer_once_ = (void *)1, \
                     ec_clean()) /* Clean up the exception. */ \

/* Catches any exception type. Similar to ec_catch_a(...), After the block is
 * exited all exception information will be automatically cleaned up (e.g. type
 * and data). If you need to keep the exception information for use after the
 * block you MUST COPY IT.
 */
#define ec_catch \
            /* An exception was thrown, but not specifically handled. */ \
            } else \
                for (ec_swap_env(ec_penv_), /* Restore prev environment. */ \
                     ec_swap_winding(ec_pwinding_), /* Restore prev winding. */ \
                     ec_try_outer_once_ = (void *)3; /* Start catching in 'catch'. */ \
                     ec_try_outer_once_ == (void *)3; /* Only run the loop once */ \
                     ec_try_outer_once_ = (void *)1, \
                     ec_clean()) \

/* ec_finally will be run whether an exception is thrown or not. If an
 * exception was thrown, then after the block is exited all exception
 * information will be automatically cleaned up (e.g. type and data).  If you
 * need to keep the exception information for use after the block you MUST COPY
 * IT.
 */
#define ec_finally \
            /* An exception was thrown, but not specifically handled. */ \
            } else { \
                ec_swap_env(ec_penv_); /* Restore prev environment. */ \
                ec_swap_winding(ec_pwinding_); /* Restore prev winding. */ \
            } \
            for (int ec_finally_once_ = 0; \
                 ec_finally_once_ == 0; \
                 ec_finally_once_ = 1, \
                 ec_clean()) \

/* Throw an exception of the given type t with cleanup function c and data
 * print function p. If the exception environment has not been setup (ec_try
 * wasn't used further up the call stack), then the exception is printed and
 * abort() called.
 *
 * Before the jump to the catching code the winding stack is unwound.
 */
#define ec_throw(t,c,p) \
    for (   void *ec_throw_data_ = NULL;; \
            ec_set_error((t), ec_throw_data_, (c), (p)), \
            ec_set_place(__FILE__, __func__, __LINE__), \
            ec_unwind(EC_UNWIND_ALL), \
            ec_env(NULL) == NULL ? \
                ec_fprint(stderr), \
                fprintf(stderr, "Error stack empty: Abort!\n"), \
                ec_clean(), \
                abort() : \
                siglongjmp(*ec_env(NULL), 0)) \
            ec_throw_data_ = \

/* Utility macro for throwing an exception with a C string as data. */
#define ec_throw_str(t) ec_throw((t), free, (void (*)(FILE *, void *))ec_fprint_str)

#define ec_throw_str_static(t,d) \
{ \
    static const char ec_throw_str_static_[] = d; \
    ec_throw((t), NULL, (void (*)(FILE *, void *))ec_fprint_str) (void *)ec_throw_str_static_; \
} \

/* Throws the exception type associated with the given error number. */
#define ec_throw_errno(e,c) ec_throw(ec_errno_type((e)), (c), (void (*)(FILE *, void *))ec_fprint_errno_str)

/* Rethrows the current exception if one exists (otherwise does nothing). This
 * is useful in situations where an exception was caught, but only partially
 * handled (where additional handling needs to done further up the call stack).
 */
#define ec_rethrow \
    if (ec_type(NULL) != NULL) { \
        if (ec_env(NULL) == NULL) { \
            ec_unwind(EC_UNWIND_ALL); \
            ec_fprint(stderr); \
            fprintf(stderr, "Error stack empty: Abort!\n"); \
            abort(); \
        } \
        ec_unwind(EC_UNWIND_ALL); \
        siglongjmp(*ec_env(NULL), 0); \
    } \

/* Calls the function u passing the data d as its argument. This is called
 * regardless of whether an exception is thrown or not. It is useful for
 * unconditional cleanup (such as freeing temporary memory or closing file).
 *
 * Using ec_with is preferable to ec_try/ec_finally because of the cost associated
 * with setting up the ec_try. ec_with is significantly faster and has lower
 * stack memory usage.
 */
#define ec_with(d,u) \
    for (struct ec_winding ec_winding_, \
         *ec_with_once_ = NULL; \
         ec_with_once_ == NULL && \
         ec_winding_init_and_wind(&ec_winding_, (d), (u)); \
         ec_unwind(EC_UNWIND_ONE), \
         ec_with_once_ = (void *)1) \

/* Similar to ec_with, but u is only called if an exception occurs. It is useful
 * for cleanup that only needs to be done if an exception occurs.
 *
 * Using ec_with_on_x is preferable to ec_try/ec_catch because of the cost
 * associated with setting up the ec_try. ec_with_on_x is significantly faster
 * and has lower stack memory usage.
 */
#define ec_with_on_x(d,u) \
    for (struct ec_winding ec_winding_, \
         *ec_with_once_ = NULL; \
         ec_with_once_ == NULL && \
         ec_winding_init_and_wind(&ec_winding_, (d), (u)); \
         ec_unwind(EC_UNWIND_DISCARD_ONE), \
         ec_with_once_ = (void *)1) \

/* Similar to ec_shadow_on_x, but also taking a function s that performs the
 * shadowing. This should only be used when the exception data is incompatible
 * requiring more involved transformation of the exception type and data.
 */
#define ec_shadow_on_x_with(ot,nt,s) \
    for (const char *ec_types_[2] = {ot, nt}, \
         *ec_shadow_once_ = NULL; \
         ec_shadow_once_ == NULL; \
         ec_shadow_once_ = (void *)1) \
        ec_with_on_x(ec_types_, (void (*)(void *))(s)) \

/* Shadow (replace) the current exception type if it matches the first type
 * with the second type. This is only processed on an exception being thrown.
 * This is provided as a utility to avoid spurious use of try/catch for the
 * simple purpose of directly shadowing a compatible exception type. Compatible
 * means the exception data is the same between the two types (for example, the
 * data type of ECX_EC and X_NULL is a C string). For incompatible exception
 * data type use ec_shadow_on_x_with.
 */
#define ec_shadow_on_x(ot,nt) ec_shadow_on_x_with((ot), (nt), ec_shadow)

/*** Exception Types
 *
 * The prefix ECX_* and ecx_* are meta-global namespaces for exception related
 * equivalents of non-exception symbols. Use the prefix as you would the global
 * namespace.  For example, the global error constant ENOMEM becomes the global
 * exception type ECX_ENOMEM. Similarly, the function malloc becomes the
 * exception throwing equivalent ecx_malloc.
 *
 * If you create a ECX_* symbol take care that a global doesn't already exist
 * and that you use proper prefixing to avoid conflicting with existing or
 * future globals. You have been warned. If a global (now or in the future)
 * conflicts with your use of ECX_*, the globals use of ECX_* takes precedence.
 *
 ***/

/* A generic exception with string data. Used to indicate a non-specific
 * exception has occurred. If possible use a more specific exception type.
 *
 * The thrower of this exception should take care to ensure that data is either
 * a valid free-able, NULL terminated C string or is NULL.
 */
extern const char ECX_EC[];

/* A null pointer exception: An attempt has been made to use a NULL pointer.
 *
 * For example, passing a NULL pointer as a function argument which is not
 * allowed to be NULL because it would result in a dereference of a NULL
 * pointer.
 *
 * The thrower of this exception should take care to ensure that data is either
 * a valid free-able, NULL terminated C string or is NULL.
 */
extern const char ECX_NULL[];

/* Standard errors from errno.h.
 *
 * The thrower of this exception should take care to ensure that data is either
 * a valid free-able, NULL terminated C string or is NULL.
 */
extern const char ECX_E2BIG[];
extern const char ECX_EACCES[];
extern const char ECX_EADDRINUSE[];
extern const char ECX_EADDRNOTAVAIL[];
extern const char ECX_EAFNOSUPPORT[];
extern const char ECX_EAGAIN[];
extern const char ECX_EALREADY[];
extern const char ECX_EBADF[];
extern const char ECX_EBADMSG[];
extern const char ECX_EBUSY[];
extern const char ECX_ECANCELED[];
extern const char ECX_ECHILD[];
extern const char ECX_ECONNABORTED[];
extern const char ECX_ECONNREFUSED[];
extern const char ECX_ECONNRESET[];
extern const char ECX_EDEADLK[];
extern const char ECX_EDESTADDRREQ[];
extern const char ECX_EDOM[];
extern const char ECX_EDQUOT[];
extern const char ECX_EEXIST[];
extern const char ECX_EFAULT[];
extern const char ECX_EFBIG[];
extern const char ECX_EHOSTUNREACH[];
extern const char ECX_EIDRM[];
extern const char ECX_EILSEQ[];
extern const char ECX_EINPROGRESS[];
extern const char ECX_EINTR[];
extern const char ECX_EINVAL[];
extern const char ECX_EIO[];
extern const char ECX_EISCONN[];
extern const char ECX_EISDIR[];
extern const char ECX_ELOOP[];
extern const char ECX_EMFILE[];
extern const char ECX_EMLINK[];
extern const char ECX_EMSGSIZE[];
extern const char ECX_EMULTIHOP[];
extern const char ECX_ENAMETOOLONG[];
extern const char ECX_ENETDOWN[];
extern const char ECX_ENETRESET[];
extern const char ECX_ENETUNREACH[];
extern const char ECX_ENFILE[];
extern const char ECX_ENOBUFS[];
extern const char ECX_ENODATA[];
extern const char ECX_ENODEV[];
extern const char ECX_ENOENT[];
extern const char ECX_ENOEXEC[];
extern const char ECX_ENOLCK[];
extern const char ECX_ENOLINK[];
extern const char ECX_ENOMEM[];
extern const char ECX_ENOMSG[];
extern const char ECX_ENOPROTOOPT[];
extern const char ECX_ENOSPC[];
extern const char ECX_ENOSR[];
extern const char ECX_ENOSTR[];
extern const char ECX_ENOSYS[];
extern const char ECX_ENOTCONN[];
extern const char ECX_ENOTDIR[];
extern const char ECX_ENOTEMPTY[];
extern const char ECX_ENOTSOCK[];
extern const char ECX_ENOTSUP[];
extern const char ECX_ENOTTY[];
extern const char ECX_ENXIO[];
extern const char ECX_EOPNOTSUPP[];
extern const char ECX_EOVERFLOW[];
extern const char ECX_EPERM[];
extern const char ECX_EPIPE[];
extern const char ECX_EPROTO[];
extern const char ECX_EPROTONOSUPPORT[];
extern const char ECX_EPROTOTYPE[];
extern const char ECX_ERANGE[];
extern const char ECX_EROFS[];
extern const char ECX_ESPIPE[];
extern const char ECX_ESRCH[];
extern const char ECX_ESTALE[];
extern const char ECX_ETIME[];
extern const char ECX_ETIMEDOUT[];
extern const char ECX_ETXTBSY[];
extern const char ECX_EWOULDBLOCK[];
extern const char ECX_EXDEV[];

/*** Winding 
 *
 * The winding mechanism is used by the ec_with(...) macros to provide a
 * light-weight exception aware solution to data management (such as cleanup).
 *
 ***/

typedef void (*ec_unwind_f)(void *data);

struct ec_winding {
    struct ec_winding *next;
    void *data;
    ec_unwind_f unwind;
};

/* Initializes winding and adds to the winding stack.
 *
 * Return 1 on success (for use in 'with' macros).
 */
int ec_winding_init_and_wind(
        struct ec_winding *winding,
        void *data,
        ec_unwind_f unwind);

/*** Error Stack
 *
 * The error stack structure keeps track of the booking necessary to facilitate
 * exceptions. Manipulating this directly, rather than through the provided
 * macros, is likely to result in significant pain. However, there may be
 * situations where the macros cannot be used (e.g. compilers that don't
 * support C99 for(...) loops) and direct manipulation is necessary. Use with
 * extreme care.
 *
 ***/

/* Opaque error stack structure. */
struct ec;

/* Swap:
 *
 * Return current value and replace with provided.
 */
sigjmp_buf *ec_swap_env(sigjmp_buf *env);
struct ec_winding *ec_swap_winding(struct ec_winding *winding);

/* Get/Set:
 *
 * Returns the current value of the given field. If the argument is non-NULL,
 * then it sets the current value to it (this will be the value returned).
 */
sigjmp_buf *ec_env(sigjmp_buf *env);
const char *ec_type(const char *type);

/* Get the current exception data. */
const void *ec_get_data();

/* Get the current exception file. */
const char *ec_get_file();

/* Get the current exception function. */
const char *ec_get_function();

/* Get the current exception line. */
unsigned int ec_get_line();

/* Set exception type, data, cleanup, and printer. */
void ec_set_error(
        const char *type,
        void *data,
        void (*data_cleanup)(void *data),
        void (*data_fprint)(FILE *stream, void *data));

/* Set exception file, function, and line. */
void ec_set_place(
        const char *file,
        const char *function,
        unsigned int line);

/* Indicates how much unwinding should be done. */
enum ec_unwind_amount {
    EC_UNWIND_ALL           = -1,
    EC_UNWIND_ONE           =  1,

    /* Discards a winding without performing its unwind action. */
    EC_UNWIND_DISCARD_ONE   =  2,
};

/* Unwind current winding stack by the given amount. */
void ec_unwind(enum ec_unwind_amount amount);

/* Cleans up error and place data. */
void ec_clean();

/* Print the exception information to the provided stream in a 'standard' way
 * (inspired by perror).  If a data printer was provided (via
 * ec_set_error(...)), then it will also be called to print the data.
 *
 * The output will be in this format:
 *
 * filename:1234: function: Exception(exception) data\n
 *
 * If the data printer wasn't provided then no data is printed (and the
 * trailing space is omitted).
 */
void ec_fprint(FILE *stream);

/* Print the provided exception data (which must be a NULL terminated C string)
 * to the stream.
 */
void ec_fprint_str(FILE *stream, char *data);

/* Print the standard errno string from sys_errlist and the provided exception
 * data (which must be a NULL terminated C string) to the stream.
 */
void ec_fprint_errno_str(FILE *stream, char *data);

/* If the first type matches the current error type,
 * then replace it with the second type. See ec_shadow_on_x(...).
 */
void ec_shadow(const char *types[2]);

/* Returns the char * representing the type of the given error number. */
const char *ec_errno_type(int error);

#endif /* EC_H */
