#pragma once

#include <cassert>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

class NeuronBase;
using Neuron_ptr = std::unique_ptr<NeuronBase>;

class NeuronBase
{
public:
  NeuronBase(int id_, int num_inputs_) : _id(id_)
  {
    // Initialize all the weight randomly
    std::default_random_engine generator(42);
    std::uniform_real_distribution<float> distribution(0.0f, +1.0f);

    _input_weights.reserve(num_inputs_);
    for (int i = 0; i < num_inputs_; ++i)
    {
      _input_weights.push_back(distribution(generator));
    }

    _input_weights_delta.resize(num_inputs_);
  }

  const std::vector<float>& get_input_weights() const { return _input_weights; }

  float get_input_weight(int neuron_id_) const
  {
    assert(neuron_id_ < static_cast<int>(_input_weights.size()));
    return _input_weights[neuron_id_];
  }

  int get_id() const { return _id; }

  // For backpropagation...

  float get_last_gradient() const { return _last_gradient; }

  virtual void update_gradient_outer(float cur_neuron_output_, float target_) = 0;

  virtual void update_gradient_inner(float cur_neuron_output_,
                                     const std::vector<Neuron_ptr>& downstream_neurons_) = 0;

  void update_input_weights(const std::vector<float>& upstream_layer_outputs_)
  {
    for (size_t input_neuron_id = 0; input_neuron_id < _input_weights.size(); ++input_neuron_id)
    {
      const float weight_delta = eta * upstream_layer_outputs_[input_neuron_id] * _last_gradient +
                                 alpha * _input_weights_delta[input_neuron_id];

      _input_weights_delta[input_neuron_id] = weight_delta;
      _input_weights[input_neuron_id] += weight_delta;
    }
  }

  std::ostream& operator<<(std::ostream& o)
  {
    for (float w : _input_weights)
    {
      o << w << "\t";
    }
    o << std::endl;
    return o;
  }

  virtual float activation_function(float val_) = 0;

  virtual ~NeuronBase() = default;

protected:
  virtual float activation_function_derivative(float x_) = 0;

  const int _id{};
  std::vector<float> _input_weights;
  std::vector<float> _input_weights_delta;
  float _last_gradient{};

  // Learning rate
  const float eta = .15f;
  // Momentum coefficient
  const float alpha = .5f;
};