#include <crs/sc/i41serial.h>
#include <crs/thread.h>
#include <crs/timer.h>
#include <iostream>

void	asyncPacketHandler ( void * pData )
{
	sc::i41serial::comPacket inPacket;
	sc::i41serial * port = reinterpret_cast<sc::i41serial*>( pData );
	while( port->peekPacket( inPacket ) )
	{
		port->pop();
		std::cout	<< "A-Reply:\t\t" << inPacket.byteString() << std::endl;
	}
}

class asyncReadThread : public CrossClass::cThread
{
protected:
	sc::i41serial * port;
	
	virtual bool Step ()
	{
		try
		{
			if( !port->receive() )
				Stop();
		}
		catch( sc::basicRS232port::errRead & e )
		{
			std::cerr << "port read error: " << e.what() << std::endl;
		}
		catch( ... )
		{
			std::cerr << "unexpected port read error" << std::endl;
		}
		return false;
	}
	
public:
	asyncReadThread ( sc::i41serial * p )
		: CrossClass::cThread()
		, port( p )
	{
	}
	
	~asyncReadThread( )
	{
		kill();
	}
};

int main ( int argc, const char ** argv )
{
	std::cout.precision( 4 );
	std::cout.setf( std::ios_base::fixed );
	sc::i41serial::comPacket packet, inPacket;
	double startTime = 0, diffTime = 0;
	CrossClass::cTimer timer;
	
	int a;
	std::cout << "pause?";
	std::cin >> a;
	
	try
	{
		sc::i41serial port;
		
#if defined( USE_POSIX_API )
		std::cout << "Trying to open '/dev/ttyS1' ... ";
		port.open( "/dev/ttyS1", B115200 );
#elif defined( USE_WIN32_API )
		std::cout << "Trying to open 'COM1' ... ";
		port.open( "COM1", CBR_115200 );
#endif
		std::cout << "success" << std::endl;
		asyncReadThread readThread ( &port );
		readThread.Resume();
		
		if( port.synchronize() )
			std::cout << "SYNC - ok!" << std::endl;
		else
			std::cout << "SYNC - failed!" << std::endl;
		
		port.setAsyncDataCallback( asyncPacketHandler, &port );
		for( int i = 0; i < 5; ++i )
		{
			packet.byteArray[ 0 ] = 0x01;
			packet.byteArray[ 1 ] = 0x05;
			packet.byteArray[ 2 ] = 0x01;
			packet.byteArray[ 3 ] = 0x00;
			packet.byteArray[ 4 ] = 0x03;
			packet.byteArray[ 5 ] = 0xFF;
			packet.byteArray[ 6 ] = 0x00;
			packet.buildCRC( );
			std::cout	<< "Request:\t" << packet.byteString() << std::endl;
			startTime = timer();			//std::chrono::high_resolution_clock::now();
			port.comSection( packet, inPacket );
			diffTime = timer() - startTime;	//std::chrono::high_resolution_clock::now().time_since_epoch() - startTime.time_since_epoch();
			std::cout	<< "Reply:\t\t" << inPacket.byteString() << ", "
//					<< static_cast<double>( diffTime.count() ) / 1e3
					<< diffTime * 1e3
					<< " ms" << std::endl;
			CrossClass::sleep( 250 );
			packet.byteArray[ 1 ] = 0x05;
			packet.byteArray[ 2 ] = 0x02;
			packet.byteArray[ 3 ] = 0x00;
			packet.byteArray[ 4 ] = 0x03;
			packet.byteArray[ 5 ] = 0xFF;
			packet.byteArray[ 6 ] = 0x00;
			packet.buildCRC( );
			std::cout	<< "Request:\t" << packet.byteString() << std::endl;
			startTime = timer();			//std::chrono::high_resolution_clock::now();
			port.comSection( packet, inPacket );
			diffTime = timer() - startTime;	//std::chrono::high_resolution_clock::now().time_since_epoch() - startTime.time_since_epoch();
			std::cout	<< "Reply:\t�O܃v�3�c�*�a�H'p'3�B���<��?�,ݩ�Q.Y �܏��^�Dԝ�Ag���e;鐓sOUk2"g��'��k���S�p�H��U���_O��@Q��V��u�^ɀ�tS+�d��P5��tz07Y�.�L˪&[P��m�Z
$����@�%y�j7K�&w+����s�<��$�L��FH@�@��|��b��#]	��s
Z�:��a������kZ_�8��cL��n�3F����}C0#�ݎ�eI<I����7x��ݺ���.��7,�
�υ�R�y�J+r���[��,~����ǅ���?$J(Y	6zR��u�s��9:��-Pk��C����[����_���f.�1��a�`��eY��|��y#n^������k�q�fVI��ܩe{�]��v�B񆿥R�� ��:�5��B��E5�@+��`j���0�U)���ֶ���l�]^a����g/ZǪ����߈b~������;�l����2�,b�ry7+Q����x�2��]�Z�%"����v�_j���XT�x9�XVP.���J1�]z�9x?V�E��@��j���_<z_��9�]��y�Gj9P� ��yhB<��GJ٧�^��CD�@�9���m�:���O#L��2�FWUV��
\b.���B��A�s
�����@�[F���.�/��̟)�.˨#ZUS]U8t��,5F�c�����(J�Gaa��l�([�׋N��<���i��B�2}��W��P���ש7]8Dw]�"Gǉ��2ț��xZ
C��m��^��f�>\˕��A&���Q�fyF*�:9#���+�%%�K\R����8	o���>��-w��[��{�p���؊��HO�O�QpU����M2�2̖�wo���C�S�S^u�i���M���L�I�0�NU}<�H6�~#N����'���ʲ�*��F�	���"g���UB��4�e�C��M�q5r�^�}��>1��}:��c*ⅡD�@�x�' ����L3� 6�xuB&ˬ�iB�~2n�yޙ%E�3��,m�#wĪ�� �����}m��OG�h�5���p��b�.�iL��4d�m���v��~�oj���'�j]��#��$����6:��	�J�̡��� ��KO��(a��%G'e�kRz�Ȭ����