#undef _NDEBUG
#include <crs/semaphore.h>

#include <cassert>
#include <thread>

int main (int argc, const char ** argv)
{
  {
    CrossClass::semaphore sem;
    assert (false == sem.wait_for (std::chrono::seconds (0)));
  }

  {
    CrossClass::semaphore sem (2);
    assert (true == sem.wait_for (std::chrono::seconds (0)));
    assert (true == sem.wait_for (std::chrono::seconds (0)));
    assert (false == sem.wait_for (std::chrono::seconds (0)));
  }

  {
    CrossClass::semaphore sem;
    sem.post ();
    assert (true == sem.wait_for (std::chrono::seconds (0)));
    assert (false == sem.wait_for (std::chrono::seconds (0)));
  }

  {
    CrossClass::semaphore sem;
    std::thread aux_thread ([&sem](){ sem.post (); });
    sem.wait ();
    aux_thread.join ();
  }
}
