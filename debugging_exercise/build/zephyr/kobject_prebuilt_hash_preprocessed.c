/* ANSI-C code produced by gperf version 3.3 */
/* Command-line: gperf --output-file kobject_prebuilt_hash_preprocessed.c --multiple-iterations 10 kobject_prebuilt_hash.gperf  */
/* Computed positions: -k'1-2' */

#line 6 "kobject_prebuilt_hash.gperf"

#include <zephyr/kernel.h>
#include <zephyr/toolchain.h>
#include <zephyr/internal/syscall_handler.h>
#include <string.h>
#line 12 "kobject_prebuilt_hash.gperf"
struct k_object;
static struct k_mutex kernel_mutexes[1] = {
Z_MUTEX_INITIALIZER(kernel_mutexes[0])};
static uint8_t Z_GENERIC_SECTION(.priv_stacks.noinit)  __aligned(Z_KERNEL_STACK_OBJ_ALIGN) priv_stacks[1][K_KERNEL_STACK_LEN(CONFIG_PRIVILEGED_STACK_SIZE)];
static const struct z_stack_data stack_data[1] = {
	{ 1024, (uint8_t *)(&priv_stacks[0]) }
};

#define TOTAL_KEYWORDS 24
#define MIN_WORD_LENGTH 4
#define MAX_WORD_LENGTH 4
#define MIN_HASH_VALUE 0
#define MAX_HASH_VALUE 23
/* maximum key range = 24, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
/*ARGSUSED*/
static unsigned int
hash (register const char *str, register size_t len)
{
  static unsigned char asso_values[] =
    {
      12, 24, 24, 24, 24, 24, 24, 24,  8,  2,
      14, 24, 24, 24, 11,  7, 10, 10, 24, 24,
      24, 24, 24, 24, 24, 24, 24,  9, 24,  5,
      24, 24, 24, 24, 24, 24, 10, 24, 24, 24,
       8, 24, 24, 24, 24, 24, 24, 24,  7, 24,
      24, 24, 24, 24, 24, 24, 11, 24, 24, 24,
      24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
      24, 24,  7, 24, 24, 24, 24, 24, 24, 24,
      24, 24, 24, 24, 24, 24, 24, 24,  6, 24,
      24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
      24, 24, 24, 24, 24, 24, 24, 24,  9, 24,
      24, 24,  5, 24, 24, 24, 24, 24, 24, 24,
      24, 24, 24, 24,  8, 24, 24, 24,  4, 24,
      24, 24,  5, 24, 24, 24, 24, 24, 24, 24,
      24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
      24, 24,  1, 24, 24, 24,  0,  7, 24, 24,
      24, 24, 24, 24, 24, 24, 24, 24,  9, 24,
      24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
       6, 24, 24, 24, 24, 24, 24, 24, 24, 24,
      24, 24, 10, 24, 24, 24, 24, 24, 24, 24,
      24, 24, 24, 24, 24, 24, 24, 24,  0, 24,
      24, 24, 24, 24, 24, 24, 10, 24, 24, 24,
      24, 24, 24, 24, 24, 24, 24, 24,  3, 24,
      24, 24, 24, 24, 24, 24,  4, 24, 24, 24,
      24, 24, 24, 24, 24, 24, 24, 24,  6, 24,
      24, 24, 24, 24, 24, 24
    };
  (void) len;
  return asso_values[(unsigned char)str[1]] + asso_values[(unsigned char)str[0]];
}

static unsigned char lengthtable[] =
  {
     4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
     4,  4,  4,  4,  4,  4,  4,  4,  4,  4
  };

#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
static struct k_object wordlist[] =
  {
#line 23 "kobject_prebuilt_hash.gperf"
    {"\320\234\001\000", {0}, K_OBJ_DRIVER_UART, 0 | K_OBJ_FLAG_DRIVER, { .unused = 0 }},
#line 21 "kobject_prebuilt_hash.gperf"
    {"\230\234\001\000", {0}, K_OBJ_DRIVER_GPIO, 0 | K_OBJ_FLAG_DRIVER, { .unused = 0 }},
#line 30 "kobject_prebuilt_hash.gperf"
    {"\320\011\000 ", {0}, K_OBJ_MUTEX, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 29 "kobject_prebuilt_hash.gperf"
    {"\230\011\000 ", {0}, K_OBJ_TIMER, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 24 "kobject_prebuilt_hash.gperf"
    {"\354\234\001\000", {0}, K_OBJ_DRIVER_I2C, 0 | K_OBJ_FLAG_DRIVER, { .unused = 0 }},
#line 31 "kobject_prebuilt_hash.gperf"
    {"\344\011\000 ", {0}, K_OBJ_SEM, 0 | K_OBJ_FLAG_INITIALIZED, { .unused = 0 }},
#line 22 "kobject_prebuilt_hash.gperf"
    {"\264\234\001\000", {0}, K_OBJ_DRIVER_GPIO, 0 | K_OBJ_FLAG_DRIVER, { .unused = 0 }},
#line 28 "kobject_prebuilt_hash.gperf"
    {"\204\011\000 ", {0}, K_OBJ_SEM, 0, { .unused = 0 }},
#line 20 "kobject_prebuilt_hash.gperf"
    {"|\234\001\000", {0}, K_OBJ_DRIVER_CLOCK_CONTROL, 0 | K_OBJ_FLAG_DRIVER, { .unused = 0 }},
#line 42 "kobject_prebuilt_hash.gperf"
    {"\200\035\000 ", {0}, K_OBJ_MUTEX, 0, { .unused = 0 }},
#line 41 "kobject_prebuilt_hash.gperf"
    {"p\035\000 ", {0}, K_OBJ_SEM, 0, { .unused = 0 }},
#line 40 "kobject_prebuilt_hash.gperf"
    {"X\035\000 ", {0}, K_OBJ_SEM, 0, { .unused = 0 }},
#line 39 "kobject_prebuilt_hash.gperf"
    {"H\035\000 ", {0}, K_OBJ_SEM, 0, { .unused = 0 }},
#line 35 "kobject_prebuilt_hash.gperf"
    {"\370\017\000 ", {0}, K_OBJ_THREAD, 0, { .thread_id = 1 }},
#line 34 "kobject_prebuilt_hash.gperf"
    {"0\017\000 ", {0}, K_OBJ_THREAD, 0, { .thread_id = 2 }},
#line 25 "kobject_prebuilt_hash.gperf"
    {"\010\235\001\000", {0}, K_OBJ_DRIVER_SENSOR, 0 | K_OBJ_FLAG_DRIVER, { .unused = 0 }},
#line 27 "kobject_prebuilt_hash.gperf"
    {"(\010\000 ", {0}, K_OBJ_SYS_MUTEX, 0, { .mutex = &kernel_mutexes[0] }},
#line 26 "kobject_prebuilt_hash.gperf"
    {"$\235\001\000", {0}, K_OBJ_DRIVER_SENSOR, 0 | K_OBJ_FLAG_DRIVER, { .unused = 0 }},
#line 38 "kobject_prebuilt_hash.gperf"
    {"l\033\000 ", {0}, K_OBJ_SEM, 0, { .unused = 0 }},
#line 37 "kobject_prebuilt_hash.gperf"
    {"\250\021\000 ", {0}, K_OBJ_THREAD, 0, { .thread_id = 4 }},
#line 36 "kobject_prebuilt_hash.gperf"
    {"\300\020\000 ", {0}, K_OBJ_THREAD, 0, { .thread_id = 3 }},
#line 33 "kobject_prebuilt_hash.gperf"
    {"\330\016\000 ", {0}, K_OBJ_TIMER, 0, { .unused = 0 }},
#line 32 "kobject_prebuilt_hash.gperf"
    {"\010\012\000 ", {0}, K_OBJ_THREAD, 0, { .thread_id = 0 }},
#line 43 "kobject_prebuilt_hash.gperf"
    {"\0008\000 ", {0}, K_OBJ_THREAD_STACK_ELEMENT, 0, { .stack_data = &stack_data[0] }}
  };
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic pop
#endif

struct k_object *
z_object_lookup (register const char *str, register size_t len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        if (len == lengthtable[key])
          {
            register const char *s = wordlist[key].name;

            if (*str == *s && !memcmp (str + 1, s + 1, len - 1))
              return &wordlist[key];
          }
    }
  return (struct k_object *) 0;
}
#line 44 "kobject_prebuilt_hash.gperf"

struct k_object *z_object_gperf_find(const void *obj)
{
    return z_object_lookup((const char *)obj, sizeof(void *));
}

void z_object_gperf_wordlist_foreach(_wordlist_cb_func_t func, void *context)
{
    int i;

    for (i = MIN_HASH_VALUE; i <= MAX_HASH_VALUE; i++) {
        if (wordlist[i].name != NULL) {
            func(&wordlist[i], context);
        }
    }
}

#ifndef CONFIG_DYNAMIC_OBJECTS
struct k_object *k_object_find(const void *obj)
	ALIAS_OF(z_object_gperf_find);

void k_object_wordlist_foreach(_wordlist_cb_func_t func, void *context)
	ALIAS_OF(z_object_gperf_wordlist_foreach);
#endif

Z_GENERIC_DOT_SECTION(data)
uint8_t _thread_idx_map[2] = { 0xe0,  0xff, };
