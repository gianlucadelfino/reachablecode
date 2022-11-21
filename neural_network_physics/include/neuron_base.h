#pragma once

#include <cassert>
#include <memory>
#include <vector>

class NeuronBase;
using Neuron_ptr = std::unique_ptr<NeuronBase>;

/**
 * @brief NeuronBase defines the interface of a Neuron
 *
 */
class NeuronBase
{
public:
  NeuronBase(int id_, int num_inputs_);

  // Getters
  const std::vector<float>& get_input_weights() const { return _input_weights; }
  float get_input_weight(int neuron_id_) const;
  int get_id() const { return _id; }
  float get_last_gradient() const { return _last_gradient; }

  /**
   * @brief Update the gradients if this layer is the outer layer
   *
   * @param cur_neuron_output_
   * @param target_
   */
  virtual void update_gradient_outer(float cur_neuron_output_, float target_) = 0;

  /**
   * @brief Update the gradient if this layer is a hidden layer
   *
   * @param cur_neuron_output_
   * @param downstream_neurons_
   */
  virtual void update_gradient_inner(float cur_neuron_output_,
                                     const std::vector<Neuron_ptr>& downstream_neurons_) = 0;

  /**
   * @brief Update the input weights given the upstream layer's outputs
   *
   * @param upstream_layer_outputs_
   */
  virtual void update_input_weights(const std::vector<float>& upstream_layer_outputs_) = 0;

  /**
   * @brief Returns the value of the activation function
   *
   * @param val_
   */
  virtual float activation_function(float val_) = 0;

  /**
   * @brief Prints the values of the input weights in the passed ostream
   */
  friend std::ostream& operator<<(std::ostream& o, const NeuronBase&);

  virtual ~NeuronBase() = default;

protected:
  virtual float activation_function_derivative(float x_) = 0;

  const int _id{};
  std::vector<float> _input_weights;
  std::vector<float> _input_weights_delta;
  float _last_gradient{};
};
