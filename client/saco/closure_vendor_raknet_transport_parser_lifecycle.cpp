// Genuine vendor parser construction/destruction. Base16, derived has no data.
#include "../raknet/RakNetTransport.h"
RakNetTransportCommandParser::RakNetTransportCommandParser()
{
    RegisterCommand(1, "SetPassword","Changes the console password to whatever.");
    RegisterCommand(0, "ClearPassword","Removes the console passwords.");
    RegisterCommand(0, "GetPassword","Gets the console password.");
}
RakNetTransportCommandParser::~RakNetTransportCommandParser()
{
}
typedef char R5ParserSize[(sizeof(RakNetTransportCommandParser)==16)?1:-1];
