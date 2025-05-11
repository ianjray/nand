#include "connector.h"

Connector::Connector(Signal& in, Signal& out) : in_{in}, out_{out}
{
}

void Connector::update()
{
    out_.set(in_.get());
}
