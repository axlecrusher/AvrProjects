#include "buffer.h"

void InitBuffer(struct CircleBuffer* cb)
{
	bint i = 0;
	for (i=0; i<BUFFER_LEN; ++i)
		cb->buffer[i] = 0;
	cb->bytesUsed = 0;
	cb->head = cb->tail = 0;
}

bint BytesFree(struct CircleBuffer* cb)
{
	return BUFFER_LEN - cb->bytesUsed;
}

bint BytesUsed(struct CircleBuffer* cb)
{
	return cb->bytesUsed;
}

void MoveBufferHead(struct CircleBuffer* cb, bint i)
{
	cb->head += i;
	cb->head &= BUFFER_LEN;
	cb->bytesUsed += i;
}

void MoveBufferTail(struct CircleBuffer* cb, bint i)
{
	cb->tail += i;
	cb->tail &= BUFFER_LEN;
	cb->bytesUsed -= i;
}

void PushByte(struct CircleBuffer* cb, char byte)
{
	cb->buffer[cb->head] = byte;
	cb->head++;
	cb->head &= BUFFER_LEN;
	cb->bytesUsed++;
}

char PopByte(struct CircleBuffer* cb)
{
	char b = cb->buffer[cb->tail];
	cb->bytesUsed--;
	cb->tail++;
	cb->tail &= BUFFER_LEN;
	return b;
}

void PushBuffer(struct CircleBuffer* cb, char* data, bint bytes)
{
	bint i = 0;
	for (i=0; i<bytes; ++i)
		cb->buffer[i] = data[i];
	MoveBufferHead(cb,bytes);
}

void PopBuffer(struct CircleBuffer* cb, char* data, bint bytes)
{
	bint i = 0;
	for (i=0; i<bytes; ++i)
		data[i] = cb->buffer[i];
	MoveBufferTail(cb,bytes);
}
