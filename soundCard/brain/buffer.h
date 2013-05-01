#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>
#include <stdint.h>

#define BUFFER_LEN 64*2*2
typedef uint16_t bint;

struct CircleBuffer
{
	volatile bint head, tail;
	volatile bint bytesUsed;
	volatile char buffer[BUFFER_LEN];
};






#include "buffer.h"

void InitBuffer(struct CircleBuffer* cb);

bint BytesFree(struct CircleBuffer* cb);

bint BytesUsed(struct CircleBuffer* cb);

void MoveBufferHead(struct CircleBuffer* cb, bint i);
void MoveBufferTail(struct CircleBuffer* cb, bint i);

void PushByte(struct CircleBuffer* cb, char byte);
char PopByte(struct CircleBuffer* cb);

void PushBuffer(struct CircleBuffer* cb, char* data, bint bytes);

void PopBuffer(struct CircleBuffer* cb, char* data, bint bytes);

#endif
