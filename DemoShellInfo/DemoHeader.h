#pragma once

struct DemoHeader
{
	static constexpr const char EXPECTED_HEADER[] = "HL2DEMO";
	char m_Header[std::size(EXPECTED_HEADER)];

	int m_DemoProtocol;
	int m_NetworkProtocol;

	static constexpr auto STRING_LENGTHS = 260;
	char m_ServerName[STRING_LENGTHS];
	char m_ClientName[STRING_LENGTHS];
	char m_MapName[STRING_LENGTHS];
	char m_GameDirectory[STRING_LENGTHS];

	float m_PlaybackTime;
	int m_Ticks;
	int m_Frames;
	int m_SignonLength;
};