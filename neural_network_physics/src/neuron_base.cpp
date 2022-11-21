#include "neuron_base.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <random>

NeuronBase::NeuronBase(int id_, int num_inputs_) : _id(id_)
{
  // Initialize all the weight randomly
  std::default_random_engine generator(id_);
  std::uniform_real_distribution<float> distribution(0.0f, +1.0f);

  _input_weights.reserve(num_inputs_);
  for (int i = 0; i < num_inputs_; ++i)
  {
    _input_weights.push_back(distribution(generator));
  }

  _input_weights_delta.resize(num_inputs_);
}

float NeuronBase::get_input_weight(int neuron_id_) const
{
  assert(neuron_id_ < static_cast<int>(_input_weights.size()));
  return _input_weights[neuron_id_];
}

std::ostream& operator<<(std::ostream& o, const NeuronBase& n)
{
  for (float w : n._input_weights)
  {
    o << w << "\t";
  }
  o << std::endl;
  return o;
}
