#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <vector>

#include "neuron.h"

class Layer
{
public:
  Layer(int num_neurons_, int num_inputs_) : _neuron_outputs(num_neurons_), _num_inputs(num_inputs_)
  {
    _neurons.reserve(num_neurons_);
    for (int id = 0; id < num_neurons_; ++id)
    {
      _neurons.emplace_back(id, num_inputs_);
    }
  }

  ssize_t size() const { return _neurons.size(); }

  const std::vector<float> get_outputs() const { return _neuron_outputs; }

  void feed_forward(const std::vector<float>& prev_layer_outputs_)
  {
    // TODO omp pragma
    for (size_t i = 0; i < _neurons.size(); ++i)
    {
      const auto& weights = _neurons[i].get_input_weights();
      assert(weights.size() == prev_layer_outputs_.size());
      const float inner_prod = std::inner_product(
          weights.cbegin(), weights.cend(), prev_layer_outputs_.cbegin(), inner_prod);
      _neuron_outputs[i] = activation_function(inner_prod);
    }
  }

  void back_propagate_outer(const std::vector<float>& expected_targets_)
  {
    for (Neuron& neuron : _neurons)
    {
      neuron.back_propagate_outer(_neuron_outputs[neuron.get_id()],
                                  expected_targets_[neuron.get_id()]);
    }
  }

  void back_propagate_inner(const Layer& downstream_layer_)
  {
    // todo omp pragma
    for (Neuron& neuron : _neurons)
    {
      neuron.back_propagate_inner(_neuron_outputs[neuron.get_id()], downstream_layer_._neurons);
    }
  }

  void update_input_weights(const Layer& upstream_layer_)
  {
    // todo omp pragma
    for (Neuron& neuron : _neurons)
    {
      neuron.update_input_weights(upstream_layer_.get_outputs());
    }
  }

  void set_neurons_values(const std::vector<float>& values_)
  {
    assert(values_.size() <= _neuron_outputs.size());
    std::copy(values_.cbegin(), values_.cend(), _neuron_outputs.begin());
  }

  void print() const
  {
    for (float neuron_out : _neuron_outputs)
    {
      std::cout << neuron_out << "\t";
    }
    std::cout << std::endl;
  }

private:
  static float activation_function(float val_)
  {
    // TODO: try Relu
    return tanh(val_);
  }

  std::vector<float> _neuron_outputs;
  const int _num_inputs{};
  std::vector<Neuron> _neurons;
};