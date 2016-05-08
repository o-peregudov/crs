#undef NDEBUG
#include <crs/barrier.h>
#include <cassert>
#include <thread>

int main (int argc, const char ** argv)
{
  {
    CrossClass::barrier barrier (0);
    barrier.wait ();
  }

  {
    CrossClass::barrier barrier (3);

    bool flaga = false;
    std::thread threada ([&]{
	std::this_thread::yield ();
	flaga = true;
	barrier.wait ();
      });

    bool flagb = false;
    std::thread threadb ([&]{
	std::this_thread::yield ();
	flagb = true;
	barrier.wait ();
      });

    barrier.wait ();
    assert (flaga == flagb);
    assert (flaga && flagb);

    threada.join ();
    threadb.join ();
  }

  return 0;
}
