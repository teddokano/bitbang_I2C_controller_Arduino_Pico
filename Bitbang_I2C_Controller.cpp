#include	"Bitbang_I2C_Controller.h"

//#pragma GCC optimize ("O3")

//#define	BUS_BUSY_CHECK

#define	MULTI_PIN_IO
//#define OPTIMZATION_LEVEL_Os

#ifdef OPTIMZATION_LEVEL_Os
	#define	BIT_FREQ_WHEN_WAIT_VAL_IS_ZERO		157700
	#define	BIT_FREQ_WHEN_WAIT_VAL_IS_HUNDRED	59000
#else
	#define	BIT_FREQ_WHEN_WAIT_VAL_IS_ZERO		2000000
	#define	BIT_FREQ_WHEN_WAIT_VAL_IS_HUNDRED	75350
#endif


Bitbang_I2C_Controller::Bitbang_I2C_Controller( int sda_pin, int scl_pin, float freq )
	: sda( sda_pin ), scl( scl_pin )
{
	float	zero_wait_bit_period	= 1.0 / (float)BIT_FREQ_WHEN_WAIT_VAL_IS_ZERO;
	float	bit_period_coefficient	= ((1.0 / (float)BIT_FREQ_WHEN_WAIT_VAL_IS_HUNDRED) - zero_wait_bit_period) / 100.0;
	
	wait_val	= ceil(((1.0 / freq) - zero_wait_bit_period) / bit_period_coefficient);

	Serial.printf( "wait_val = %d\n", wait_val );
	
	pin_init( sda );
	pin_init( scl );

	sda_map = 1 << sda;
	scl_map = 1 << scl;
}

Bitbang_I2C_Controller::~Bitbang_I2C_Controller()
{
}

void Bitbang_I2C_Controller::pin_init( int pin )
{
	pinMode( pin, OUTPUT_12MA );
	pinMode( pin, INPUT_PULLUP );
	gpio_put( pin, 0 );
}

void Bitbang_I2C_Controller::additional_io_pins( int sda, int scl )
{
	pin_init( sda );
	pin_init( scl );

	sda_map |= 1 << sda;
	scl_map |= 1 << scl;	
}

void Bitbang_I2C_Controller::force_set_wait_val( int v )
{
	wait_val	= v;
}

inline void Bitbang_I2C_Controller::short_wait( int duration ) {
	for ( volatile int i = 0; i < duration; i++ )
		;
}

inline void Bitbang_I2C_Controller::set_sda( int state ) {
#ifdef MULTI_PIN_IO
	if ( state )
		gpio_set_dir_in_masked( sda_map );
	else
		gpio_set_dir_out_masked( sda_map );
#else
	gpio_set_dir( sda, !state );
#endif
}

inline void Bitbang_I2C_Controller::set_scl( int state ) {
#ifdef MULTI_PIN_IO
	if ( state )
		gpio_set_dir_in_masked( scl_map );
	else
		gpio_set_dir_out_masked( scl_map );
#else
	gpio_set_dir( scl, !state );
#endif
}

inline int Bitbang_I2C_Controller::bit_io( int bit ) {
	set_scl( 0 );
	set_sda( bit );
	short_wait( wait_val );

	set_scl( 1 );
	
	while ( !gpio_get( scl ) )
		;

	int rtn = gpio_get( sda ) ? 1 : 0;

	for ( volatile int i = 0; i < wait_val / 2; i++ )
		if ( !gpio_get( scl ) )
			break;
	
	return rtn;
}

inline ctrl_status Bitbang_I2C_Controller::start_condition( void ) {
//	set_scl( 1 );
//	set_sda( 1 );

#ifdef BUS_BUSY_CHECK	
	for ( volatile int i = 0; i < wait_val / 2; i++ )
		if ( 0x3 !=  (gpio_get_all() & 0x3) )
			return BUS_BUSY;
#endif
	
	set_sda( 0 );
	short_wait( wait_val );
	set_scl( 0 );
	
	return NO_ERROR;
}

inline void Bitbang_I2C_Controller::stop_condition( void ) {
	set_scl( 0 );
	set_sda( 0 );
	short_wait( wait_val );
	set_scl( 1 );
	short_wait( wait_val );
	set_sda( 1 );
}

inline void Bitbang_I2C_Controller::prepare_for_repeated_start_condition( void ) {
	set_scl( 0 );
	short_wait( wait_val );
	set_scl( 1 );
	short_wait( wait_val );
}

ctrl_status Bitbang_I2C_Controller::write_byte( uint8_t data ) {
	for ( int i = 7; i >= 0; i-- ) {
		int bit		= ( data >> i ) & 0x1;
		int check	= bit_io( bit );
		
		if ( check != bit )
			return ARBITRATION_LOST;
	}

	return bit_io( 1 ) ? NACK_ON_ADDRESS : NO_ERROR;
}

uint8_t Bitbang_I2C_Controller::read_byte( bool last_byte ) {
	uint8_t data = 0x0;
	for ( int i = 7; i >= 0; i-- ) {
		data |= ( bit_io( 1 ) ? 0x1 : 0x0 ) << i;
	}

	bit_io( last_byte ? 1 : 0 );

	return data;
}

ctrl_status Bitbang_I2C_Controller::write_transaction( uint8_t address, uint8_t *data, int length, bool repeated_start ) {
	ctrl_status	err	= NO_ERROR;
	
	err	= start_condition();
	
	if ( err )
		return err;
	
	err	= write_byte( address & ~0x1 );

	if ( ARBITRATION_LOST == err )
		return err;		
	
	if ( err ) {
		err = NACK_ON_ADDRESS;		
	}
	else
	{
		for ( int i = 0; i < length; i++ ) {
			err	= write_byte( data[i] );
			
			if ( err ) {
				err = NACK_ON_DATA;
				break;
			}
		}		
	}
	
	if ( repeated_start && !err )
		prepare_for_repeated_start_condition();
	else
		stop_condition();
	
	return err;
}

ctrl_status Bitbang_I2C_Controller::read_transaction( uint8_t address, uint8_t *data, int length, bool repeated_start ) {
	ctrl_status	err	= NO_ERROR;
	
	err	= start_condition();
	
	if ( err )
		return err;
	
	err	= write_byte( address | 0x1 );

	if ( err  ) {
		err = NACK_ON_ADDRESS;		
	}
	else
	{
		for ( int i = 0; i < length; i++ )
			data[i] = read_byte( ( i == ( length - 1 ) ? 1 : 0 ) );
	}
	
	if ( repeated_start )
		prepare_for_repeated_start_condition( );
	else
		stop_condition( );

	return err;
}

