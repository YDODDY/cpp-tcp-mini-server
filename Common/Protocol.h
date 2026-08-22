#pragma once

#include <cstdint>

enum class MessageType : uint8_t
{
	Login = 1,
	Chat = 2,
	Move = 3
};