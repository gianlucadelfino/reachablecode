#include "layer_base.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <random>
#include <vector>
#include <cmath>

LayerBase::LayerBase(int num_neurons_, int num_inputs_)
    : _neuron_outputs(num_neurons_), _num_inputs(num_inputs_)
{
}

void LayerBase::feed_forward(const std::vector<float>& prev_layer_outputs_)
{
  for (size_t i = 0; i < _neurons.size(); ++i)
  {
    const auto& weights = _neurons[i]->get_input_weights();
    assert(weights.size() == prev_layer_outputs_.size());
    const float inner_prod =
        std::inner_product(weights.cbegin(), weights.cend(), prev_layer_outputs_.cbegin(), 0.f);
    assert(!std::isnan(inner_prod));
    _neuron_outputs[i] = _neurons[i]->activation_function(inner_prod);
    assert(!std::isnan(_neuron_outputs[i]));
  }
}

void LayerBase::update_gradient_outer(const std::vector<float>& expected_targets_)
{
  assert(expected_targets_.size() + 1 == _neurons.size()); // +1 is the ignored bias
  for (size_t i = 0; i < expected_targets_.size(); ++i)
  {
    auto& neuron = _neurons[i];
    neuron->update_gradient_outer(_neuron_outputs[i], expected_targets_[i]);
  }
}

void LayerBase::update_gradient_inner(const LayerBase& downstream_layer_)
{
  for (auto& neuron : _neurons)
  {
    neuron->update_gradient_inner(_neuron_outputs[neuron->get_id()], downstream_layer_._neurons);
  }
}

void LayerBase::update_input_weights(const LayerBase& upstream_layer_)
{
  for (auto& neuron : _neurons)
  {
    neuron->update_input_weights(upstream_layer_.get_outputs());
  }
}

void LayerBase::set_neurons_values(const std::vector<float>& values_)
{
  assert(values_.size() <= _neuron_outputs.size());
  std::copy(values_.cbegin(), values_.cend(), _neuron_outputs.begin());
}

void LayerBase::print() const
{
  for (float neuron_out : _neuron_outputs)
  {
    std::cout << neuron_out << "\t";
  }
  std::cout << std::endl;
}
