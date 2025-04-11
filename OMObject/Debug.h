#pragma once

#include "Metronome.h"
#include "OMObject.h"

class DebugConnector : public OMConnector
{
public:
    void Init(OMObject *obj) override;
    void Push(OMObject *obj, OMProperty *prop) override;
    void Pull(OMObject *obj, OMProperty *prop) override;
};

extern DebugConnector DebugConn;

class Debug
{
public:
	void        Setup();
	void        Run();
    OMObject*   DebugObject;
    // get singleton instance
    static Debug& GetInstance() { return debug; }
    // Delete copy constructor and assignment operator to prevent copying singleton
    Debug(const Debug&) = delete;
    Debug& operator=(const Debug&) = delete;
private:
    // Static member variable to hold the single instance
    static Debug debug;
    // private constructor for singleton
	Debug() : Metro(100) { };
	Metronome	Metro;
};
