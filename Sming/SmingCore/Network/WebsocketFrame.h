/*
 * WebsocketFrame.h
 *
 *  Created on: 13 нояб. 2016 г.
 *      Author: shurik
 */

#ifndef SMINGCORE_NETWORK_WEBSOCKETFRAME_H_
#define SMINGCORE_NETWORK_WEBSOCKETFRAME_H_
#include "../Wiring/WiringFrameworkIncludes.h"

namespace WSFrame
{
	static uint8_t MaxHeaderLength = 14;
}

enum class WSOpcode : uint8_t
{
	op_continuation = 0x00,	///< %x0 denotes a continuation frame
	op_text = 0x01,			///< %x1 denotes a text frame
	op_binary = 0x02,		///< %x2 denotes a binary frame
														///< %x3-7 are reserved for further non-control frames
	op_close = 0x08,			///< %x8 denotes a connection close
	op_ping = 0x09,			///< %x9 denotes a ping
	op_pong = 0x0A			///< %xA denotes a pong
	///< %xB-F are reserved for further control frame
};

namespace WSFlags
{
    static const uint8_t payloadDeleteMemBit = 1u; //Delete memory reserved for payload in destructor
    static const uint8_t headerDeleteMemBit = 2u; //Delete memory reserved for header in destructor
    static const uint8_t multiframeBufBit = 4u; //Multiframe buffer was given to decodeFrame
};

class HttpServer;
class Websocket;
class WebsocketClient;

class WebsocketFrameClass
{
	friend class HttpServer;
	friend class Websocket;
	friend class WebsocketClient;
public:
	WebsocketFrameClass();
	virtual ~WebsocketFrameClass();
	uint8_t encodeFrame(WSOpcode opcode, uint8_t * payload, size_t length, uint8_t mask, uint8_t fin,  uint8_t headerToPayload);
	uint8_t decodeFrame(uint8_t * buffer, size_t length); //return nonzero if multiframe buffer given
protected:
	uint8_t* _payload = nullptr;
	size_t _payloadLength = 0;
	uint8_t* _header = nullptr;
	size_t _headerLength = 0;
	WSOpcode _opcode = WSOpcode::op_text;
private:
	uint8_t _flags = 0; //Store flags for further freeing memory
	size_t _nextReadOffset = 0; //Store offset in multiframe tcp buffer for next decodeFrame
};

#endif /* SMINGCORE_NETWORK_WEBSOCKETFRAME_H_ */
