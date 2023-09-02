/** bit-bang I2C controller
 *
 *  @author Tedd OKANO
 *
 *  Released under the MIT license License
 */

#include	<Arduino.h>
#include 	<stdint.h>

class BbI2C
{
public:
	/** Error code */
	enum ctrl_status : int
	{
		NO_ERROR	= 0,
		NACK_ON_ADDRESS,
		NACK_ON_DATA,
		ARBITRATION_LOST,
		BUS_BUSY,
	};


	/** Constractor
	 * 
	 * @param sda	SDA pin
	 * @param scl	SCL pin
	 * @param freq	SCL frequenct in "Hz"
	 */
	BbI2C( int sda, int scl, float freq = 100 * 1000 );

	/** Destractor */
	virtual ~BbI2C();
	
	/** Write transaction
	 * 
	 * @param address			Target address
	 * @param data				Pointer to data array
	 * @param length			Data length
	 * @param repeated_start	Flag to generate "repeated start"
	 */
	ctrl_status					write_transaction( uint8_t address, uint8_t *data, int length, bool repeated_start = false );

	/** Read transaction
	 * 
	 * @param address			Target address
	 * @param data				Pointer to data array
	 * @param length			Data length
	 * @param repeated_start	Flag to generate "repeated start"
	 */
	ctrl_status					read_transaction( uint8_t address, uint8_t *data, int length, bool repeated_start = false );

	/** Setting additional SDA and SCL pins to get more current drive capability
	 * 
	 * @param sda	SDA pin
	 * @param scl	SCL pin
	 */
	void						additional_io_pins( int sda, int scl );

	/** Manual setting for wait-loop parameter
	 * 
	 * @param v	wait value (loop count)
	 */
	void						force_set_wait_val( int v );

private:
	void						pin_init( int pin );
	__force_inline void			short_wait( int duration );
	__force_inline void			set_sda( int state );
	__force_inline void			set_scl( int state );
	__force_inline int			bit_io( int bit );
	__force_inline ctrl_status	start_condition( void );
	__force_inline void			stop_condition( void );
	__force_inline void			prepare_for_repeated_start_condition( void );
	ctrl_status					write_byte( uint8_t data );
	uint8_t						read_byte( bool last_byte );
	
	int			sda;
	int			scl;
	int			wait_val;
	uint32_t	sda_map;
	uint32_t	scl_map;
};


