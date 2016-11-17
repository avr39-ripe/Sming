/*
 * WebsocketFrame.cpp
 *
 *  Created on: 13 нояб. 2016 г.
 *      Author: shurik
 */

#include <Network/WebsocketFrame.h>

WebsocketFrameClass::WebsocketFrameClass()
{
	// TODO Auto-generated constructor stub

}

WebsocketFrameClass::~WebsocketFrameClass()
{
	if ((_flags & WSFlags::payloadDeleteMemBit) != 0)
	{
		delete[] _payload;
	}

	if ((_flags & WSFlags::headerDeleteMemBit) != 0)
	{
		delete[] _header;
	}
}

uint8_t WebsocketFrameClass::encodeFrame(WSOpcode opcode, uint8_t * payload, size_t length, uint8_t mask, uint8_t fin,  uint8_t headerToPayload)
{
	if (length == 0 || length > 0xFFFF)
	{
		return false; //either zero length or too big for poor esp8266
	}

	uint8_t maskKey[4] = { 0x00, 0x00, 0x00, 0x00 };
	uint8_t* headerPtr = nullptr;
	uint8_t* dataPtr = nullptr;

	_payload = payload;
	_payloadLength = length;

	//calculate header size
	length < 126 ? _headerLength = 2 : _headerLength = 4;
	if (mask)
	{
		_headerLength += 4;
	}

	//try to allocate single buffer for both frame header and frame payload
	if ( headerToPayload && (length < 1400) && (system_get_free_heap_size() > 6000))
	{
		_payloadLength = length + _headerLength;
		dataPtr = new uint8_t[_payloadLength];
		if (dataPtr)
		{
			_flags |= WSFlags::payloadDeleteMemBit;
			os_memcpy((dataPtr + _headerLength), payload, length); //copy original data to newly created buffer with _headerLength offset
			_payload = dataPtr;
		}
		else
		{
			headerToPayload = false; //memory allocation failed for single buffer, continue in light-memory mode with separate header and original payload
		}
	}

	// set Header Pointer
	if (headerToPayload)
	{
		_header = _payload; //Header is inside _payload buffer and occupy first _headerLength bytes
	}
	else
	{
		headerPtr = new uint8_t[_headerLength];
		if (headerPtr)
		{
			_header = headerPtr;
			_flags |= WSFlags::headerDeleteMemBit;
		}
		else
		{
			return false; //memory allocation failed
		}
	}

	os_memset(_header, 0, _headerLength); //set initial header state to be all zero

	// create header

	// byte 0
	if (fin)
	{
		*_header |= bit(7);    ///< set Fin
	}
	*_header |= (uint8_t) opcode;        ///< set opcode
	_header++;

	// byte 1
	if (mask)
	{
		*_header |= bit(7);    ///< set mask
	}

	if (length < 126)
	{
		*_header |= length;
		_header++;
	}
	else if (length < 0xFFFF)
	{
		*_header |= 126;
		_header++;
		*_header = ((length >> 8) & 0xFF);
		_header++;
		*_header = (length & 0xFF);
		_header++;
	}

	if (mask && headerToPayload)
	{
		//we work on copy of original data so we can mask it without affecting original

		for (uint8_t x = 0; x < sizeof(maskKey); x++)
		{
			maskKey[x] = random(0xFF);
			*_header = maskKey[x];
			_header++;
		}

		uint8_t * dataMaskPtr = (_payload + _headerLength);

		for (size_t x = 0; x < length; x++)
		{
			dataMaskPtr[x] = (dataMaskPtr[x] ^ maskKey[x % 4]);
		}

	}

	if (headerToPayload)
	{
		_header = nullptr; //mark _header as nullptr to indicate external world that whole frame is referenced by _payload
		_headerLength = 0;
	}
	else
	{
		_header = headerPtr;
	}

	return true;
}
