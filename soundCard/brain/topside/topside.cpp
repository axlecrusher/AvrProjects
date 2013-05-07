#include <stdlib.h>
#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include <unistd.h>
#include <string.h>

#define DATAGRAM_SIZE 64

		libusb_context *usbContext;
		libusb_device_handle *devh;
		libusb_device* usbDevice;

FILE* AUDIOFILE = NULL;
libusb_transfer* t = 0x00;

enum DeviceState
{
	NODEVICE = 0,
	NOTFOUND,
	NOCLAIM,
	NOCONNECTION,
	ESTABLISHED


};

DeviceState mState;

		fd_set fdSet;
		int nfds;

		bool CanSendControl;

const int bufferLen = sizeof(uint16_t)*320;
uint8_t* frontbuffer;
		timespec m_pollTime;
		timespec m_tPollTime;


bool AquireDevice()
{
	int r;

	devh = libusb_open_device_with_vid_pid( usbContext, 0xabcd, 0xf013 );

	if( !devh )
	{
		mState = NOTFOUND;
		usleep(100000);
		fprintf( stderr,  "Error: Cannot find device.\n" );
		return false;
	}

	if( ( r = libusb_claim_interface(devh, 0) ) < 0 )
	{
		mState = NOCLAIM;
		usleep(100000);
		fprintf(stderr, "usb_claim_interface error %d\n", r);
		return false;
	}

	usbDevice = libusb_get_device(devh);
//	int speed = libusb_get_device_speed(usbDevice);
//	printf("%d\n", speed);

	mState = ESTABLISHED;

	fprintf(stderr, "Found Device\n");
	return true;
}


void InitUSB()
{
	//upgrade to the newest version, its easy to compile and install
	const libusb_version*v = libusb_get_version();
	fprintf(stderr,"Libusb version %d.%d.%d.%d %s\n", v->major, v->minor, v->micro, v->nano, v->rc);

	if( libusb_init(&usbContext) < 0 )
	{
		fprintf( stderr, "Error: Could not initialize libUSB\n" );
		return;
	}

	libusb_set_debug(usbContext,3);

	while ( !AquireDevice() );


//	for (int i = 0; i < 2; ++i)
//		SendTx( MakeIsoTx(0x03 | LIBUSB_ENDPOINT_IN ) );
//	#ifdef RPI
//	SendTx(	MakeBulkTx(0x01 | LIBUSB_ENDPOINT_IN ) );
//	#else
//	for (int i = 0; i < 2; ++i)
//		SendTx(	MakeIsoTx(0x04 | LIBUSB_ENDPOINT_IN ) );
//	#endif

	const libusb_pollfd** fds = libusb_get_pollfds(usbContext);
	for(const libusb_pollfd** i = fds; *i != NULL; ++i)
	{
		FD_SET((*i)->fd, &fdSet);
		nfds = (*i)->fd>nfds?(*i)->fd:nfds;
	}

	free(fds);

	CanSendControl = true;
}

int SendTx(libusb_transfer* tx, int seconds = 0)
{
	int r = libusb_submit_transfer(tx);

	if (r != 0)
	{
		fprintf(stderr, "error submitting transfer %d (%s)\n", r,  libusb_error_name(r));
//		exit(-1);
	}

	return r;

/*
	struct timeval tv;
	tv.tv_sec = seconds;
	tv.tv_usec = 1000;

	r = libusb_handle_events_timeout_completed(usbContext, &tv, NULL);

	if (r!=0 && r != LIBUSB_ERROR_INTERRUPTED)
	{
		fprintf(stderr, "error handling events %d (%s)\n", r,  libusb_error_name(r));
		exit(-1);		
	}
	*/
}

void GetData_Blocking()
{
	clock_gettime(CLOCK_REALTIME, &m_pollTime);
	int r = libusb_control_transfer( devh,
		0x80, //reqtype
		0xA4, //request
		0x0100, //wValue
		0x0000, //wIndex
		frontbuffer,
		bufferLen,
		1000 );

	if( r <= 0 )
	{
		fprintf( stderr,  "Error: Recieving control data: %d.\n", r );
		exit(1);
	}
	else if (r>1)
	{
//		OutputData(frontbuffer+1, r-1);
//		printf("\n");
	}

//	memcpy(frontbuffer, ctrlBuffer, r);
//	++usbCount;
/*
	printf( "ctrl (%d): ", r );
	for( int i = 0; i < r; i++ )
		printf( " %02x ", frontbuffer[i] );
	printf( "\n" );
*/
}

int ReadAudio()
{
	if (feof(AUDIOFILE))
	{
		fprintf(stderr, "eof\n");
		exit(1);
	}

	int r = fread(t->buffer, DATAGRAM_SIZE, 1, AUDIOFILE);

	uint8_t tmp;
	for (int i = 0; i<DATAGRAM_SIZE; i+=2)
	{
//		printf("%02X%02X ", t->buffer[i], t->buffer[i+1]);	
		tmp = t->buffer[i];
		t->buffer[i] = t->buffer[i+1];
		t->buffer[i+1] = tmp;
	}

//	printf("\n");

	return r;
}

bool SendAudioData()
{
	printf("Sending... ");
	fflush(stdout);

	SendTx(t,10);
}

void HandleBulk(libusb_transfer *tx)
{
	switch(tx->status)
	{
		case LIBUSB_TRANSFER_COMPLETED:
//			fprintf(stderr,"Ok\n");
			ReadAudio();

//			if (tx->actual_length>1)
//				OutputData(tx->buffer, tx->actual_length);
			break;
		case LIBUSB_TRANSFER_ERROR:
			fprintf(stderr,"Error\n");
			exit(1);
			break;
		case LIBUSB_TRANSFER_TIMED_OUT:
			fprintf(stderr,"Timed out\n");
//			exit(1);
			break;
		case LIBUSB_TRANSFER_CANCELLED:
			fprintf(stderr,"Cancelled\n");
			exit(1);
			break;
		case LIBUSB_TRANSFER_NO_DEVICE:
			fprintf(stderr,"No device\n");
			exit(1);
			break;
		case LIBUSB_TRANSFER_OVERFLOW:
			fprintf(stderr,"Overflow\n");
			exit(1);
			break;
		case LIBUSB_TRANSFER_STALL:
			fprintf(stderr,"stall\n");
			break;
	}
/*
	if (tx->status == LIBUSB_TRANSFER_COMPLETED)
	{
		switch (tx->endpoint)
		{
			case 0x84:
				OutputData(tx->buffer, tx->actual_length);
				break;
			case 0x83:
				OutputData(tx->buffer, tx->actual_length);
				break;
			default:
				break;
		}
	}
*/
//	t = tx;
//	SendAudioData();
	SendTx(t);
}

libusb_transfer* MakeBulkTx(uint8_t endpoint, uint16_t bLen)
{
	uint8_t* data = (uint8_t*)malloc(sizeof(uint8_t)*bLen);

	libusb_transfer *trx = libusb_alloc_transfer(0);
	if (trx == NULL)
	{
		fprintf(stderr, "Could not allocate transfer\n");
		exit(-1);
	}

	libusb_fill_bulk_transfer(trx,devh,endpoint,data,bLen,HandleBulk,NULL,2000);

	return trx;
}


int main()
{
	fprintf(stderr,"Realtime USB Sound Caard Test\n");

	devh =NULL;
	mState=NODEVICE;
	nfds = 0;

	frontbuffer = (uint8_t*)malloc(bufferLen);
	memset(frontbuffer, 0, bufferLen);

	FD_ZERO(&fdSet);

	InitUSB();

	AUDIOFILE = fopen("audio.raw", "rb");

	t = MakeBulkTx(0x04 | LIBUSB_ENDPOINT_OUT, DATAGRAM_SIZE);

ReadAudio();
SendAudioData();
//		SendTx(	MakeBulkTx(0x04 | LIBUSB_ENDPOINT_OUT, 32) );

int r;
	struct timeval tv;
	while(1)
	{
//		printf("loop\n");
	tv.tv_sec = 0;
	tv.tv_usec = 1000;

		if( (r = libusb_handle_events_timeout_completed(usbContext, &tv, NULL)) < 0 )
		{
			fprintf(stderr,"handle events error %d\n", r);
			if (r != LIBUSB_ERROR_INTERRUPTED) exit(1);
		}
//		SendAudioData();
//		SendTx(	MakeBulkTx(0x03 | LIBUSB_ENDPOINT_OUT, DATAGRAM_SIZE) );
//		GetData_Blocking();
	}
	return 0;
}
