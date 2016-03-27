#include <crs/handle.h>
#include <cassert>

class A
{
protected:
  static crs_uint32_t
public:
  A ()
  {
  }
  
  virtual ~A ()
  {
  }
  
  virtual void print ()
  {
  }
};

class B : public A
{
public:
  B () : A ()
  {
  }
  
  virtual ~B ()
  {
  }
  
  virtual void print ()
  {
  }
};

class C : public B
{
public:
  C () : B ()
  {
  }
  
  virtual ~C ()
  {
  }
  
  virtual void print ()
  {
  }
};

int main (int argc, char * argv [])
{
  CrossClass::cHandle<A> handle [ 3 ];
  CrossClass::cHandle<A> ahandle ( new A );
  CrossClass::cHandle<B> bhandle ( new B );
  CrossClass::cHandle<C> chandle ( new C );
  
  handle[ 0 ] = ahandle;
  handle[ 1 ] = bhandle;
  handle[ 2 ] = chandle;
  
  for( size_t i = 0; i < sizeof( handle ) / sizeof( CrossClass::cHandle<A> ); ++i )
    {
      handle[ i ]->print();
      bhandle = handle[ i ];
      chandle = handle[ i ];
      if( bhandle )
	std::cout << "b handle!" << std::endl;
      if( chandle )
	std::cout << "c handle!" << std::endl;
    }
  
  return 0;
}
