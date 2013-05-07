static inline void SendChannelDataFromUSB()
{
//	uint8_t usb = UENUM;
	UENUM = 4; //interrupts can change this
	if ( USB_READY(UEINTX) )
	{
		PORTD &= ~_BV(PD6);

		UEINTX &= ~_BV(RXOUTI); //ack

		//normally we would do this last but we want to allow as muhc time as
		//possible for the USB to respond
		//it would be even better to decouple this from the ISP
		bytesRead += 4;
		if (bytesRead >= DATAGRAM_SIZE)
		{
//			PORTD |= _BV(PD6);
			UEINTX &= ~_BV(FIFOCON);
			bytesRead = 0;
		}

		//send left MSB
		SPDR = UEDATX; //17 clock cycles for this to send
		while(!(SPSR & _BV(SPIF))); //wait for complete

		//send left LSB
		SPDR = UEDATX;
		while(!(SPSR & _BV(SPIF))); //wait for complete

		//send right MSB
		SPDR = UEDATX;
		while(!(SPSR & _BV(SPIF))); //wait for complete

		//send right LSB
		SPDR = UEDATX;
		while(!(SPSR & _BV(SPIF))); //wait for complete
	}
	else
	{
		PORTD |= _BV(PD6);
	}

//	UENUM = usb;
}
