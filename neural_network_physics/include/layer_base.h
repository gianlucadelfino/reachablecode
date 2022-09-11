#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include "neuron_base.h"

class LayerBase
{
public:
  LayerBase(int num_neurons_, int num_inputs_)
      : _neuron_outputs(num_neurons_), _num_inputs(num_inputs_)
  {
  }

  ssize_t size() const { return _neurons.size(); }

  const std::vector<float> get_outputs() const { return _neuron_outputs; }

  virtual void feed_forward(const std::vector<float>& prev_layer_outputs_)
  {
    // TODO omp pragma
    for (size_t i = 0; i < _neurons.size(); ++i)
    {
      const auto& weights = _neurons[i]->get_input_weights();
      assert(weights.size() == prev_layer_outputs_.size());
      const float inner_prod =
          std::inner_product(weights.cbegin(), weights.cend(), prev_layer_outputs_.cbegin(), 0.f);
      _neuron_outputs[i] = _neurons[i]->activation_function(inner_prod);
    }
  }

  void update_gradient_outer(const std::vector<float>& expected_targets_)
  {
    for (auto& neuron : _neurons)
    {
      neuron->update_gradient_outer(_neuron_outputs[neuron->get_id()],
                                    expected_targets_[neuron->get_id()]);
    }
  }

  void update_gradient_inner(const LayerBase& downstream_layer_)
  {
    // todo omp pragma
    for (auto& neuron : _neurons)
    {
      neuron->update_gradient_inner(_neuron_outputs[neuron->get_id()], downstream_layer_._neurons);
    }
  }

  void update_input_weights(const LayerBase& upstream_layer_)
  {
    // todo omp pragma
    for (auto& neuron : _neurons)
    {
      neuron->update_input_weights(upstream_layer_.get_outputs());
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

  virtual ~LayerBase() = default;

protected:
  std::vector<float> _neuron_outputs;
  const int _num_inputs{};
  std::vector<Neuron_ptr> _neurons;
};