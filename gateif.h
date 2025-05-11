/// Abstract gate interface.
/// A gate transforms one or more inputs to one or more outputs.
class Gate
{
public:
    /// Destructor.
    virtual ~Gate();

    /// Update output.
    virtual void update() = 0;
};
