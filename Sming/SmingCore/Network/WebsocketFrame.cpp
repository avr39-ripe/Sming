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
	// TODO Auto-generated destructor stub
}

void WebsocketFrameClass::encodeFrame(WSOpcode opcode, uint8_t * payload, size_t length, uint8_t mask, uint8_t fin,  uint8_t headerToPayload)
{
	uint8_t maskKey[4] = { 0x00, 0x00, 0x00, 0x00 };
//	uint8_t buffer[WSFrame::MaxHeaderSize] = { 0 };

	_payload = payload;

	bool useInternBuffer = false;
	bool ret = true;

	// calculate header Size
	if(length < 126) {
		_headerLength = 2;
	} else if(length < 0xFFFF) {
		_headerLength = 4;
	} else {
		_headerLength = 10;
	}

	if(mask) {
		_headerLength += 4;
	}


	// only for ESP since AVR has less HEAP
	// try to send data in one TCP package (only if some free Heap is there)
	if(!headerToPayload && ((length > 0) && (length < 1400)) && (system_get_free_heap_size() > 6000)) {
		uint8_t * dataPtr = new uint8_t[length + WSFrame::MaxHeaderLength];
		_flags |= WSFlags::payloadDeleteMemBit;
		if(dataPtr) {
			os_memcpy((dataPtr + WSFrame::MaxHeaderLength), payload, length);
			headerToPayload = true;
			useInternBuffer = true;
			_payload = dataPtr;
		}
	}


	// set Header Pointer
	if(headerToPayload) {
		// calculate offset in payload
		_header = (_payload + (WSFrame::MaxHeaderLength - _headerLength));
	} else {
		_header = new uint8_t[WSFrame::MaxHeaderLength];
		_flags |= WSFlags::headerDeleteMemBit;
	}

	// create header

	// byte 0
	*_header = 0x00;
	if(fin) {
		*_header |= bit(7);    ///< set Fin
	}
	*_header |= (uint8_t) opcode;        ///< set opcode
	_header++;

	// byte 1
	*_header = 0x00;
	if(mask) {
		*_header |= bit(7);    ///< set mask
	}

	if(length < 126) {
		*_header |= length;
		_header++;
	} else if(length < 0xFFFF) {
		*_header |= 126;
		_header++;
		*_header = ((length >> 8) & 0xFF);
		_header++;
		*_header = (length & 0xFF);
		_header++;
	} else {
		// Normally we never get here (to less memory)
		*_header |= 127;
		_header++;
		*_header = 0x00;
		_header++;
		*_header = 0x00;
		_header++;
		*_header = 0x00;
		_header++;
		*_header = 0x00;
		_header++;
		*_header = ((length >> 24) & 0xFF);
		_header++;
		*_header = ((length >> 16) & 0xFF);
		_header++;
		*_header = ((length >> 8) & 0xFF);
		_header++;
		*_header = (length & 0xFF);
		_header++;
	}

	if(mask) {
		if(useInternBuffer) {
			// if we use a Intern Buffer we can modify the data
			// by this fact its possible the do the masking
			for(uint8_t x = 0; x < sizeof(maskKey); x++) {
				maskKey[x] = random(0xFF);
				*_header = maskKey[x];
				_header++;
			}

			uint8_t * dataMaskPtr;

			if(headerToPayload) {
				dataMaskPtr = (_payload + WSFrame::MaxHeaderLength);
			} else {
				dataMaskPtr = _payload;
			}

			for(size_t x = 0; x < length; x++) {
				dataMaskPtr[x] = (dataMaskPtr[x] ^ maskKey[x % 4]);
			}

		} else {
			*_header = maskKey[0];
			_header++;
			*_header = maskKey[1];
			_header++;
			*_header = maskKey[2];
			_header++;
			*_header = maskKey[3];
			_header++;
		}
	}

	if(headerToPayload) {
		// header has be added to payload
		// payload is forced to reserved 14 Byte but we may not need all based on the length and mask settings
		// offset in payload is calculatetd 14 - _headerLength
//	        if(client->tcp->write(&_payload[(WSFrame::MaxHeaderSize - _headerLength)], (length + _headerLength)) != (length + _headerLength)) {
//	            ret = false;
//	        }
		ret = send((char*) &_payload[(WSFrame::MaxHeaderLength - _headerLength)], (length + _headerLength), false);
	} else {
		// send header
//	        if(client->tcp->write(&buffer[0], _headerLength) != _headerLength) {
//	            ret = false;
//	        }
		ret = send((char*) _header, _headerLength, false);

		if(_payload && length > 0) {
			// send payload
//	            if(client->tcp->write(&_payload[0], length) != length) {
//	                ret = false;
//	            }
			ret = send((char*) &_payload[0], length, false);
		}
	}


	if(useInternBuffer && _payload) {
//	    	debugf("Free wsBuffer!\n");
		delete[] _payload;
	}

	return ret;
}
