#undef NDEBUG
#include <crs/barrier.h>
#include <cassert>
#include <thread>

int main (int argc, const char ** argv)
{
  {
    crs::barrier barrier (0);
    barrier.wait ();
  }

  {
    crs::barrier barrier (1);
    barrier.wait ();
  }

  {
    crs::barrier barrier (2);

    std::thread aux_thread ([&]{
        const bool wait_complete =
          barrier.wait_for (std::chrono::milliseconds (250));
        assert (wait_complete);
      });

    const bool wait_complete =
      barrier.wait_until (std::chrono::steady_clock::now () +
                          std::chrono::milliseconds (250));
    assert (wait_complete);

    aux_thread.join ();
  }

  return 0;
}
