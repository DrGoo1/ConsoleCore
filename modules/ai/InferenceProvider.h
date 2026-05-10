#pragma once
#include "FeatureVector.h"
#include <string>

class InferenceProvider
{
public:
    virtual ~InferenceProvider() = default;

    virtual bool loadModel(const std::string& path) = 0;
    virtual FeatureVector infer(const FeatureVector& input) = 0;
};