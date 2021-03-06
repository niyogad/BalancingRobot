#include <avr/io.h>
#include <util/twi.h>

void setupTWI(void)
{
    #ifndef __AVR_ATmega328P__
    PRR0 &= ~(1 << PRTWI);
    #endif // __AVR_ATmega328P__
    //TWSR |= (1 << TWPS1); // preescaler=16
    //TWSR |= (1 << TWPS0); // preescaler=4

    //TWSR &= 0x3; // preescaler=1
    TWSR = 0; // preescaler=1
    // preescaler = 16
    //TWBR = 1; // Freq(SCL) = 16e6/(16+2*1*16)=333kHz
    //TWBR = 2; // Freq(SCL) = 16e6/(16+2*2*16)=200kHz
    //TWBR = 8; // Freq(SCL) = 16e6/(16+2*2*16)=200kHz
    //TWBR = 48; // Freq(SCL) = 16e6/(16+2*2*16)=200kHz
    // preescaler = 1
    TWBR = 13; // 380952 Hz
}

uint8_t twi_start(uint8_t addr, uint8_t w)
{
    uint8_t tw_st;
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // enviar START
    while( !(TWCR & (1 << (TWINT) ) ) ); // esperar a que se envie el START
    // si se envio correctamente el START, continuar
    tw_st = TWSR & 0xF8;
    // verificar si se envio correctamente el start
    // o el rep start
    if ( tw_st != TW_START && tw_st != TW_REP_START ){
        //TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
        return tw_st;
    }

    w = w != 0 ? 0 : 1;
    TWDR = (addr << 1) | w; // cargar la direccion del esclavo
    TWCR = (1 << TWINT) | (1 << TWEN); // enviar la direccion

    while( !(TWCR & (1 << (TWINT) ) ) ); // esperar a que se envie la direccion

    tw_st = TWSR & 0xF8;
    // verificar si el esclavo responde la solicitud
    // de escritura o lectura
    if ( tw_st != TW_MT_SLA_ACK  && tw_st != TW_MR_SLA_ACK ){
        //TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
        return tw_st;
    }

    return 0;
}

void twi_stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); // enviar STOP
}

inline uint8_t twi_write(uint8_t d)
{
    uint8_t tw_st;
    TWDR = d; // cargar los datos
    TWCR = (1 << TWINT) | (1 << TWEN); // enviar los datos

    while( !(TWCR & (1 << (TWINT) ) ) ); // esperar a que se envien los datos

    // si se envio correctamente los datos, continuar
    tw_st = TWSR & 0xF8;
    if (  tw_st != TW_MT_DATA_ACK)
        return tw_st;
    return 0;
}

uint8_t twi_send(uint8_t sl_addr, uint8_t * data, uint8_t n)
{
    twi_start(sl_addr, 1);

    while (n--){
        twi_write(data[n]);
    }

    twi_stop();

    return 0;
}

inline uint8_t twi_read(uint8_t ack)
{
    if (ack)
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    else
        TWCR = (1 << TWINT) | (1 << TWEN);
    while( !( TWCR & (1 << TWINT)) );
    return TWDR;
}
