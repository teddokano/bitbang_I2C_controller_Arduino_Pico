#include	"bitbang_I2C_controller.h"

//#define	BUS_BUSY_CHECK

#define OPTIMZATION_LEVEL_Os

#ifdef OPTIMZATION_LEVEL_Os
	#define	BIT_FREQ_WHEN_WAIT_VAL_IS_ZERO		892000
	#define	BIT_FREQ_WHEN_WAIT_VAL_IS_HUNDRED	66530
#else
	#define	BIT_FREQ_WHEN_WAIT_VAL_IS_ZERO		2000000
	#define	BIT_FREQ_WHEN_WAIT_VAL_IS_HUNDRED	75350
#endif

static int	bbi2c_SDA_PIN;
static int	bbi2c_SCL_PIN;
static int	bbi2c_WAIT_VAL;

void bbi2c_init( int sda, int scl, float freq ){
	bbi2c_SDA_PIN		= sda;
	bbi2c_SCL_PIN		= scl;
	
	float	zero_wait_bit_period	= 1.0 / (float)BIT_FREQ_WHEN_WAIT_VAL_IS_ZERO;
	float	bit_period_coefficient	= ((1.0 / (float)BIT_FREQ_WHEN_WAIT_VAL_IS_HUNDRED) - zero_wait_bit_period) / 100.0;
	
	bbi2c_WAIT_VAL	= ceil(((1.0 / freq) - zero_wait_bit_period) / bit_period_coefficient);
	
	//	Set pin drive strength max.
	//	https://arduino-pico.readthedocs.io/en/latest/digital.html#output-modes-pad-strength
	
	pinMode( bbi2c_SDA_PIN, OUTPUT_12MA );
	pinMode( bbi2c_SCL_PIN, OUTPUT_12MA );
	
	pinMode( bbi2c_SDA_PIN, INPUT_PULLUP );
	pinMode( bbi2c_SCL_PIN, INPUT_PULLUP );
	gpio_put( bbi2c_SDA_PIN, 0 );
	gpio_put( bbi2c_SCL_PIN, 0 );
}

void force_set_bbi2c_WAIT_VAL( int v )
{
	bbi2c_WAIT_VAL	= v;
}

inline void short_wait( int duration ) {
	for ( volatile int i = 0; i < duration; i++ )
		;
}

inline void set_sda( int state ) {
	gpio_set_dir( bbi2c_SDA_PIN, !state );
}

inline void set_scl( int state ) {
	gpio_set_dir( bbi2c_SCL_PIN, !state );
}

inline int bit_io( int bit ) {
	set_scl( 0 );
	set_sda( bit );
	short_wait( bbi2c_WAIT_VAL );

	set_scl( 1 );
	
	while ( !gpio_get( bbi2c_SCL_PIN ) )
		;

	int rtn = gpio_get( bbi2c_SDA_PIN ) ? 1 : 0;

	
#if 0
	short_wait( bbi2c_WAIT_VAL );
#else
	for ( volatile int i = 0; i < bbi2c_WAIT_VAL / 2; i++ )
		if ( !gpio_get( bbi2c_SCL_PIN ) )
			break;
#endif
	
	return rtn;
}

inline ctrl_status start_condition( void ) {
//	set_scl( 1 );
//	set_sda( 1 );

#ifdef BUS_BUSY_CHECK	
	for ( volatile int i = 0; i < bbi2c_WAIT_VAL; i++ )
		if ( 0x3 !=  (gpio_get_all() & 0x3) )
			return BUS_BUSY;
#endif
	
	set_sda( 0 );
	short_wait( bbi2c_WAIT_VAL );
	set_scl( 0 );
	
	return NO_ERROR;
}

inline void stop_condition( void ) {
	set_scl( 0 );
	set_sda( 0 );
	short_wait( bbi2c_WAIT_VAL );
	set_scl( 1 );
	short_wait( bbi2c_WAIT_VAL );
	set_sda( 1 );
}

inline void prepare_for_repeated_start_condition( void ) {
	set_scl( 0 );
	short_wait( bbi2c_WAIT_VAL );
	set_scl( 1 );
	short_wait( bbi2c_WAIT_VAL );
}

ctrl_status write_byte( uint8_t data ) {
	for ( int i = 7; i >= 0; i-- ) {
		int bit		= ( data >> i ) & 0x1;
		int check	= bit_io( bit );
		
		if ( check != bit )
			return ARBITRATION_LOST;
	}

	return bit_io( 1 ) ? NACK_ON_ADDRESS : NO_ERROR;
}

uint8_t read_byte( bool last_byte ) {
	uint8_t data = 0x0;
	for ( int i = 7; i >= 0; i-- ) {
		data |= ( bit_io( 1 ) ? 0x1 : 0x0 ) << i;
	}

	bit_io( last_byte ? 1 : 0 );

	return data;
}

ctrl_status write_transaction( uint8_t address, uint8_t *data, int length, bool repeated_start ) {
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

ctrl_status read_transaction( uint8_t address, uint8_t *data, int length, bool repeated_start ) {
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

