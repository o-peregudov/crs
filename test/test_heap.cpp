#if defined (NDEBUG)
#  undef NDEBUG
#endif
#include <crs/heap.h>
#include <cassert>
#include <cstdlib>
#include <cstdio>

struct int_compare
{
  bool operator () (const int & a, const int & b) const
  {
    return (a < b);
  }
};

int main (int argc, char * argv [])
{
  const size_t nPoints = 10000;
  CrossClass::heap<int, int_compare> heap (nPoints);
  
  /*
   * heap sort
   */
  for (size_t nx = 0; nx < 10; ++nx)
    {
      for (size_t ix = 0; ix < nPoints; ++ix)
	{
	  heap.insert (rand ());
	}
      assert (heap.size () == nPoints);
      
      int rv = heap.get ();
      while ((heap.empty() == false) && (rv <= heap.peek ()))
	{
	  rv = heap.get ();
	}
      assert (heap.empty ());
    }
  
  /*
   * heap sort + removal of arbitrary element
   */
  for (size_t nx = 0; nx < 10; ++nx)
    {
      for (size_t ix = 0; ix < nPoints; ++ix)
	{
	  heap.insert (rand ());
	}
      assert (heap.size () == nPoints);
      
      size_t iremove = rand () * heap.size () / RAND_MAX;
      heap.remove (iremove);
      
      int rv = heap.get ();
      while ((heap.empty() == false) && (rv <= heap.peek ()))
	{
	  rv = heap.get ();
	}
      assert (heap.empty ());
    }
  
  return 0;
}
