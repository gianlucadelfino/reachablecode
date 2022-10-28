#pragma once

#include <vector>

#include "neuron_base.h"

/**
 * @brief LayerBase defines the interface for a layer.
 *
 */
class LayerBase
{
public:
  LayerBase(int num_neurons_, int num_inputs_);

  ssize_t size() const { return _neurons.size(); }

  const std::vector<float> get_outputs() const { return _neuron_outputs; }

  virtual void feed_forward(const std::vector<float>& prev_layer_outputs_);

  /**
   * @brief Update the gradients if this layer is the outer layer
   *
   * @param expected_targets_
   */
  void update_gradient_outer(const std::vector<float>& expected_targets_);

  /**
   * @brief Update the gradient if this layer is a hidden layer
   *
   * @param downstream_layer_
   */
  void update_gradient_inner(const LayerBase& downstream_layer_);

  /**
   * @brief Update the input weights. Usually called after updating the gradient
   *
   * @param upstream_layer_
   */
  void update_input_weights(const LayerBase& upstream_layer_);

  /**
   * @brief Set the neurons values
   *
   * @param values_
   */
  void set_neurons_values(const std::vector<float>& values_);

  /**
   * @brief Print the current values of the neurons to stdout.
   * (useful for debugging)
   */
  void print() const;

  virtual ~LayerBase() = default;

protected:
  std::vector<float> _neuron_outputs;
  const int _num_inputs{};
  std::vector<Neuron_ptr> _neurons;
};