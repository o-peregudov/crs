#include <crs/handle.h>
#include <iostream>

class A
{
public:
	A () { }
	virtual ~A ()
	{
		std::cout << "~A" << std::endl;
	}
	
	virtual void print ()
	{
		std::cout << "class A" << std::endl;
	}
};

class B : public A
{
public:
	B () : A () { }
	virtual ~B ()
	{
		std::cout << "~B" << std::endl;
	}
	
	virtual void print ()
	{
		std::cout << "class B" << std::endl;
	}
};

class C : public B
{
public:
	C () : B () { }
	virtual ~C ()
	{
		std::cout << "~C" << std::endl;
	}
	
	virtual void print ()
	{
		std::cout << "class C" << std::endl;
	}
};

int main ()
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
