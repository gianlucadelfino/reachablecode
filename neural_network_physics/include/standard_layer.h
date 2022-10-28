#pragma once

#include "layer_base.h"
#include "standard_neuron.h"

/**
 * @brief This class implements a layer of Perceptrons. See StandardNeuron docs
 */
class StandardLayer : public LayerBase
{
public:
  StandardLayer(int num_neurons_, int num_inputs_) : LayerBase(num_neurons_, num_inputs_)
  {
    _neurons.reserve(num_neurons_);
    for (int id = 0; id < num_neurons_; ++id)
    {
      _neurons.emplace_back(std::make_unique<StandardNeuron>(id, num_inputs_));
    }
  }
};