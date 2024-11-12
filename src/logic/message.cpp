#include "message.h"

#include <stddef.h> // offsetof
#include <sstream>

void data::SerialiseMessage(char* outBuf, message* inMsg)
{
	std::ostringstream stream{ std::string{outBuf, sizeof(message)} };
	char* base = reinterpret_cast<char*>(inMsg);
	char* src = base;
	stream.write(src, sizeof(inMsg->MessageSize));
	src = base + offsetof(message, MessageType);
	stream.write(src, sizeof(inMsg->MessageType));
	src = base + offsetof(message, MessageId);
	stream.write(src, sizeof(inMsg->MessageId));
	src = base + offsetof(message, MessageData);
	stream.write(src, sizeof(inMsg->MessageData));
	memcpy_s(outBuf, sizeof(message), stream.str().c_str(), stream.str().length());
}

void data::DeserialiseMessage(char* inBuf, message* outMsg)
{
	std::istringstream stream{ std::string{inBuf, sizeof(message)} };
	char* base = reinterpret_cast<char*>(outMsg);
	char* dst = base;
	stream.read(dst, sizeof(outMsg->MessageSize));
	dst = base + offsetof(message, MessageType);
	stream.read(dst, sizeof(outMsg->MessageType));
	dst = base + offsetof(message, MessageId);
	stream.read(dst, sizeof(outMsg->MessageId));
	dst = base + offsetof(message, MessageData);
	stream.read(dst, sizeof(outMsg->MessageData));	
}

std::string data::toString(const message& msg)
{
	std::ostringstream stream{};
	stream << "[Msg] ";
	stream << "Size: " << msg.MessageSize << ", ";
	stream << "Type: " << static_cast<short>(msg.MessageType) << ", ";
	stream << "Id: " << msg.MessageId << ", ";
	stream << "Data: " << msg.MessageData;
	return stream.str();
}

size_t data::MessageHasher::operator()(const argument_type& _Keyval) const noexcept
{
	return std::_Hash_representation(_Keyval.MessageId);
}
