#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define ___ARRAY_CONCAT(x, y) x##y
#define __ARRAY_CONCAT(x, y) ___ARRAY_CONCAT(x, y)

#if !defined(__ARRAY_NAME)
#error "__ARRAY_NAME must be defined"
#endif

#if !defined(__ARRAY_FUNCS_NAME)
#error "__ARRAY_FUNCS_NAME must be defined"
#endif

#if !defined(__ARRAY_T)
#error "__ARRAY_T must be defined"
#endif

#if !defined(__ARRAY_INITIAL_CAPACITY)
#define __ARRAY_INITIAL_CAPACITY 4
#endif

#if !defined(__ARRAY_GROWTH_FACTOR)
#define __ARRAY_GROWTH_FACTOR 2
#endif

struct __ARRAY_NAME
{
  __ARRAY_T* items;
  size_t len;
  size_t capacity;
};

struct __ARRAY_NAME __ARRAY_CONCAT(__ARRAY_FUNCS_NAME, _new)(void)
{
  return (struct __ARRAY_NAME){NULL, 0, 0};
}

void __ARRAY_CONCAT(__ARRAY_FUNCS_NAME, _drop)(struct __ARRAY_NAME* array)
{
  if (array->items == NULL)
  {
    return;
  }

  free(array->items);
  *array = __ARRAY_CONCAT(__ARRAY_FUNCS_NAME, _new)();
}

__ARRAY_T* __ARRAY_CONCAT(__ARRAY_FUNCS_NAME, _at)(struct __ARRAY_NAME array, size_t index)
{
  assert(index < array.capacity);
  return &array.items[index];
}

size_t __ARRAY_CONCAT(__ARRAY_FUNCS_NAME, _next_capacity)(size_t current_capacity,
                                                          size_t min_capacity)
{
  size_t new_capacity = current_capacity > 0 ? current_capacity : __ARRAY_INITIAL_CAPACITY;
  while (new_capacity < min_capacity)
  {
    new_capacity *= __ARRAY_GROWTH_FACTOR;
  }
  return new_capacity;
}

bool __ARRAY_CONCAT(__ARRAY_FUNCS_NAME, _resize)(struct __ARRAY_NAME* array, size_t new_capacity)
{
  if (new_capacity <= array->capacity)
  {
    return true;
  }

  __ARRAY_T* new_items = realloc(array->items, new_capacity * sizeof(__ARRAY_T));
  if (new_items == NULL)
  {
    return false;
  }

  array->items = new_items;
  array->capacity = new_capacity;
  return true;
}

bool __ARRAY_CONCAT(__ARRAY_FUNCS_NAME, _reserve)(struct __ARRAY_NAME* array, size_t extra_capacity)
{
  if (array->capacity >= array->len + extra_capacity)
  {
    return true;
  }

  size_t new_capacity = __ARRAY_CONCAT(__ARRAY_FUNCS_NAME, _next_capacity)(
      array->capacity, array->len + extra_capacity);
  return __ARRAY_CONCAT(__ARRAY_FUNCS_NAME, _resize)(array, new_capacity);
}

bool __ARRAY_CONCAT(__ARRAY_FUNCS_NAME, _push)(struct __ARRAY_NAME* array, __ARRAY_T item)
{
  if (!__ARRAY_CONCAT(__ARRAY_FUNCS_NAME, _reserve)(array, 1))
  {
    return false;
  }

  *__ARRAY_CONCAT(__ARRAY_FUNCS_NAME, _at)(*array, array->len) = item;
  array->len++;
  return true;
}

bool __ARRAY_CONCAT(__ARRAY_FUNCS_NAME, _remove)(struct __ARRAY_NAME* array, size_t index,
                                                 __ARRAY_T* out_item)
{
  if (index >= array->len)
  {
    return false;
  }

  if (out_item != NULL)
  {
    *out_item = *__ARRAY_CONCAT(__ARRAY_FUNCS_NAME, _at)(*array, index);
  }

  size_t num_items_after = array->len - index - 1;
  if (num_items_after > 0)
  {
    __ARRAY_T* src = __ARRAY_CONCAT(__ARRAY_FUNCS_NAME, _at)(*array, index + 1);
    __ARRAY_T* dest = __ARRAY_CONCAT(__ARRAY_FUNCS_NAME, _at)(*array, index);
    memmove(dest, src, num_items_after * sizeof(__ARRAY_T));
  }

  array->len--;
  return true;
}

#undef ___ARRAY_CONCAT
#undef __ARRAY_CONCAT

#undef __ARRAY_NAME
#undef __ARRAY_FUNCS_NAME
#undef __ARRAY_T
#undef __ARRAY_INITIAL_CAPACITY
#undef __ARRAY_GROWTH_FACTOR
