#include	<Arduino.h>
#include 	<stdint.h>

enum ctrl_status : int
{
	NO_ERROR	= 0,
	NACK_ON_ADDRESS,
	NACK_ON_DATA,
	ARBITRATION_LOST,
	BUS_BUSY,
};

void				bbi2c_init( int sda, int scl, float freq = 100 * 1000 );
void				pin_init( int pin );
void				additional_io_pins( int sda, int scl );
void				force_set_WAIT_VAL( int v );
inline void			short_wait( int duration );
inline void			set_sda( int state );
inline void			set_scl( int state );
inline int			bit_io( int bit );
inline ctrl_status	start_condition( void );
inline void			stop_condition( void );
inline void			prepare_for_repeated_start_condition( void );
ctrl_status			write_byte( uint8_t data );
uint8_t				read_byte( bool last_byte );
ctrl_status			write_transaction( uint8_t address, uint8_t *data, int length, bool repeated_start = false );
ctrl_status			read_transaction( uint8_t address, uint8_t *data, int length, bool repeated_start = false );
