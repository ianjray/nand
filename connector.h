#include "gateif.h"
#include "signal.h"

/// Direct connection between two signals.
class Connector : public Gate
{
    Signal& in_;
    Signal& out_;

public:
    Connector(Signal& in, Signal& out);

    void update() override;
};
