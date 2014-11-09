#pragma once

#include "Direct3DBase.h"

#define HACK_PORTRAIT_VIEW      1

// This class renders a simple spinning cube.
ref class CubeRenderer sealed : public Direct3DBase
{
public:
    CubeRenderer();

    // Direct3DBase methods.
    virtual void CreateDeviceResources() override;
    virtual void CreateWindowSizeDependentResources() override;
    virtual void Render() override;

private:
};