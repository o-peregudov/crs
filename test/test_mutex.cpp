#if defined (NDEBUG)
#  undef NDEBUG
#endif
#include <crs/mutex.h>
#include <cassert>
#include <cstdio>

int test_lock_guard (std::mutex & mtx);
int test_lock_guard_adopt_lock (std::mutex & mtx);

int main (int argc, char * argv [])
{
  std::mutex mtx;
  int rv = 0;

  rv = test_lock_guard (mtx);
  assert (mtx.try_lock () == true);
  assert (rv == 0);

  rv = test_lock_guard_adopt_lock (mtx);
  assert (mtx.try_lock () == true);
  assert (rv == 0);

  return 0;
}

int test_lock_guard (std::mutex & mtx)
{
  std::lock_guard<std::mutex> guard (mtx);
  return (mtx.try_lock () == false) ? 0 : -1;
}

int test_lock_guard_adopt_lock (std::mutex & mtx)
{
  std::lock_guard<std::mutex> guard (mtx, std::adopt_lock_t ());
  return (mtx.try_lock () == false) ? 0 : -1;
}
