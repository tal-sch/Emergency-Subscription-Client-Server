#include "StompProtocol.h"


StompProtocol::StompProtocol()
    : _pConnection()
    , _data()
    , _exitApp(false)
{
}

bool StompProtocol::exit() const
{
    return _exitApp;
}
