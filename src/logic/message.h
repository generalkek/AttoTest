#pragma once

#include <string>
#include <stdint.h>

namespace data {

	using MsgId = uint64_t;

	struct message {
		uint16_t MessageSize;
		uint8_t MessageType;
		uint64_t MessageId;
		uint64_t MessageData;
	};

	// both functions assume that there is enough space in buffers
	// and pointers are valid
	void SerialiseMessage(char* outBuf, message* inMsg);
	void DeserialiseMessage(char* inBuf, message* outMsg);
	std::string toString(const message& msg);

	struct MessageHasher {
		using result_type = size_t;
		using argument_type = message;

		size_t operator()(const argument_type& _Keyval) const noexcept;
	};
}

