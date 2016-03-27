#include <iostream>
#include <crs/interval_map.h>
#include <cassert>

int main (int argc, char * argv [])
{
  CrossClass::interval_map<unsigned int, char> m ('A');
  unsigned int ix = 0;
  
  /*
   * Test 1
   */
  std::cout << "Test 1: initial state ... ";
  assert (m[std::numeric_limits<unsigned int>::min ()] == 'A');
  assert (m[std::numeric_limits<unsigned int>::max ()] == 'A');
  assert (m.size () == 1);
  std::cout << "passed"
	    << std::endl;
  
  /*
   * Test 2
   */
  std::cout << "Test 2: simple interval assignment ... ";
  m.assign (3, 5, 'B');
  
  assert (m.size () == 3);
  for (ix = 0; ix < 3; ++ix)
    {
      assert (m[ix] == 'A');
    }
  for (; ix < 5; ++ix)
    {
      assert (m[ix] == 'B');
    }
  for (; ix < 100; ++ix)
    {
      assert (m[ix] == 'A');
    }
  
  std::cout << "passed"
	    << std::endl;
  
  /*
   * Test 3
   */
  std::cout << "Test 3: full interval overlapping ... ";
  m.assign (2, 7, 'C');
  
  assert (m.size () == 3);
  for (ix = 0; ix < 2; ++ix)
    {
      assert (m[ix] == 'A');
    }
  for (; ix < 7; ++ix)
    {
      assert (m[ix] == 'C');
    }
  for (; ix < 100; ++ix)
    {
      assert (m[ix] == 'A');
    }
  
  std::cout << "passed"
	    << std::endl;
  
  /*
   * Test 4
   */
  std::cout << "Test 4: assignment for non overlapped interval ... ";
  m.assign (8, 9, 'D');
  
  assert (m.size () == 5);
  for (ix = 0; ix < 2; ++ix)
    {
      assert (m[ix] == 'A');
    }
  for (; ix < 7; ++ix)
    {
      assert (m[ix] == 'C');
    }
  for (; ix < 8; ++ix)
    {
      assert (m[ix] == 'A');
    }
  for (; ix < 9; ++ix)
    {
      assert (m[ix] == 'D');
    }
  for (; ix < 100; ++ix)
    {
      assert (m[ix] == 'A');
    }
  
  std::cout << "passed"
	    << std::endl;
  
  /*
   * Test 5
   */
  std::cout << "Test 5: overlapped interval that covers two others ... ";
  m.assign (6, 15, 'E');
  
  assert (m.size () == 4);
  for (ix = 0; ix < 2; ++ix)
    {
      assert (m[ix] == 'A');
    }
  for (; ix < 6; ++ix)
    {
      assert (m[ix] == 'C');
    }
  for (; ix < 15; ++ix)
    {
      assert (m[ix] == 'E');
    }
  for (; ix < 100; ++ix)
    {
      assert (m[ix] == 'A');
    }
  
  std::cout << "passed"
	    << std::endl;
  
  /*
   * Test 6
   */
  std::cout << "Test 6: another overlapped interval that covers several others ... ";
  m.assign (1, 12, 'F');
  
  assert (m.size () == 4);
  for (ix = 0; ix < 1; ++ix)
    {
      assert (m[ix] == 'A');
    }
  for (; ix < 12; ++ix)
    {
      assert (m[ix] == 'F');
    }
  for (; ix < 15; ++ix)
    {
      assert (m[ix] == 'E');
    }
  for (; ix < 100; ++ix)
    {
      assert (m[ix] == 'A');
    }
  
  std::cout << "passed"
	    << std::endl;
  
  /*
   * Test 7
   */
  std::cout << "Test 7: adding of empty and invalid interval ... ";
  m.assign (10, 10, 'X');
  m.assign (11, 10, 'X');
  
  assert (m.size () == 4);
  for (ix = 0; ix < 1; ++ix)
    {
      assert (m[ix] == 'A');
    }
  for (; ix < 12; ++ix)
    {
      assert (m[ix] == 'F');
    }
  for (; ix < 15; ++ix)
    {
      assert (m[ix] == 'E');
    }
  for (; ix < 100; ++ix)
    {
      assert (m[ix] == 'A');
    }
  
  std::cout << "passed"
	    << std::endl;
  
  /*
   * Test 8
   */
  std::cout << "Test 8: boundary condition - overlapped interval with existing keyBegin ... ";
  m.assign (1, 3, 'K');
  
  assert (m.size () == 5);
  for (ix = 0; ix < 1; ++ix)
    {
      assert (m[ix] == 'A');
    }
  for (; ix < 3; ++ix)
    {
      assert (m[ix] == 'K');
    }
  for (; ix < 12; ++ix)
    {
      assert (m[ix] == 'F');
    }
  for (; ix < 15; ++ix)
    {
      assert (m[ix] == 'E');
    }
  for (; ix < 100; ++ix)
    {
      assert (m[ix] == 'A');
    }
  
  std::cout << "passed"
	    << std::endl;
  
  /*
   * Test 9
   */
  std::cout << "Test 9: boundary condition - overlapped interval with existing keyEnd ... ";
  m.assign (14, 15, 'J');
  
  assert (m.size () == 6);
  for (ix = 0; ix < 1; ++ix)
    {
      assert (m[ix] == 'A');
    }
  for (; ix < 3; ++ix)
    {
      assert (m[ix] == 'K');
    }
  for (; ix < 12; ++ix)
    {
      assert (m[ix] == 'F');
    }
  for (; ix < 14; ++ix)
    {
      assert (m[ix] == 'E');
    }
  for (; ix < 15; ++ix)
    {
      assert (m[ix] == 'J');
    }
  for (; ix < 100; ++ix)
    {
      assert (m[ix] == 'A');
    }
  
  std::cout << "passed"
	    << std::endl;
  
  /*
   * Test 10
   */
  std::cout << "Test 10: boundary condition - overlapped interval with existing (keyBegin, keyEnd) ... ";
  m.assign (14, 15, 'L');
  
  assert (m.size () == 6);
  for (ix = 0; ix < 1; ++ix)
    {
      assert (m[ix] == 'A');
    }
  for (; ix < 3; ++ix)
    {
      assert (m[ix] == 'K');
    }
  for (; ix < 12; ++ix)
    {
      assert (m[ix] == 'F');
    }
  for (; ix < 14; ++ix)
    {
      assert (m[ix] == 'E');
    }
  for (; ix < 15; ++ix)
    {
      assert (m[ix] == 'L');
    }
  for (; ix < 100; ++ix)
    {
      assert (m[ix] == 'A');
    }
  
  std::cout << "passed"
	    << std::endl;
  
  /*
   * Test 11
   */
  std::cout << "Test 11: interval extension (right) ... ";
  m.assign (14, 16, 'L');
  
  assert (m.size () == 6);
  for (ix = 0; ix < 1; ++ix)
    {
      assert (m[ix] == 'A');
    }
  for (; ix < 3; ++ix)
    {
      assert (m[ix] == 'K');
    }
  for (; ix < 12; ++ix)
    {
      assert (m[ix] == 'F');
    }
  for (; ix < 14; ++ix)
    {
      assert (m[ix] == 'E');
    }
  for (; ix < 16; ++ix)
    {
      assert (m[ix] == 'L');
    }
  for (; ix < 100; ++ix)
    {
      assert (m[ix] == 'A');
    }
  
  std::cout << "passed"
	    << std::endl;
  
  /*
   * Test 12
   */
  std::cout << "Test 12: interval extension (left) ... ";
  m.assign (13, 16, 'L');
  
  assert (m.size () == 6);
  for (ix = 0; ix < 1; ++ix)
    {
      assert (m[ix] == 'A');
    }
  for (; ix < 3; ++ix)
    {
      assert (m[ix] == 'K');
    }
  for (; ix < 12; ++ix)
    {
      assert (m[ix] == 'F');
    }
  for (; ix < 13; ++ix)
    {
      assert (m[ix] == 'E');
    }
  for (; ix < 16; ++ix)
    {
      assert (m[ix] == 'L');
    }
  for (; ix < 100; ++ix)
    {
      assert (m[ix] == 'A');
    }
  
  std::cout << "passed"
	    << std::endl;
  
  /*
   * Test 13
   */
  std::cout << "Test 13: full overlapping ... ";
  m.assign (1, 16, 'A');

  assert (m.size () == 1);
  for (ix = 0; ix < 100; ++ix)
    {
      assert (m[ix] == 'A');
    }
  
  std::cout << "passed"
	    << std::endl;
  
  /*
   * Test 14
   */
  std::cout << "Test 14: maximum possible interval ... ";
  m.assign (std::numeric_limits<unsigned int>::min (), std::numeric_limits<unsigned int>::max (), '*');
  assert (m[std::numeric_limits<unsigned int>::min ()] == '*');
  assert (m[std::numeric_limits<unsigned int>::max ()] == 'A');
  
  assert (m.size () == 2);
  for (ix = 0; ix < 100; ++ix)
    {
      assert (m[ix] == '*');
    }
  
  std::cout << "passed"
	    << std::endl;
  
  /*
   * Test 15
   */
  std::cout << "Test 15: already mapped interval ... ";
  m.assign (1, 10, '*');
  
  assert (m.size () == 2);
  for (ix = 0; ix < 100; ++ix)
    {
      assert (m[ix] == '*');
    }
  
  std::cout << "passed"
	    << std::endl;
  
  return 0;
}
