#pragma once

#include <vector>
#include <iostream>

class Neuron
{
  public:
  Neuron(int id_, int num_inputs_)
      : _id(id_)
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

  const std::vector<float>& get_input_weights_deltas() const
  {
    return _input_weights_delta;
  }

  int get_id() const { return _id; }

  // For backpropagation...

  float get_last_gradient() const { return _last_gradient; }

  void back_propagate_outer(float cur_neuron_output_, float target_)
  {

    const float delta = target_ - cur_neuron_output_;

    _last_gradient = activation_function_derivative(cur_neuron_output_) * delta;
  }

  void back_propagate_inner(float cur_neuron_output_,
                            const std::vector<Neuron>& downstream_neurons_)
  {

    float delta{};
    // For every neuron we look at the weights connecting it to the next
    // layer to build  up delta
    for (const Neuron& downstream_neuron : downstream_neurons_)
    {
      const float downstream_last_gradient =
          downstream_neuron.get_last_gradient();

      const float downstream_input_weight =
          downstream_neuron.get_input_weight(_id);

      delta += downstream_last_gradient * downstream_input_weight;
    }

    _last_gradient = activation_function_derivative(cur_neuron_output_) * delta;
  }

  void update_input_weights(const std::vector<float>& upstream_layer_outputs_)
  {
    for (size_t input_neuron_id = 0; input_neuron_id < _input_weights.size();
         ++input_neuron_id)
    {
      const float weight_delta =
          eta * upstream_layer_outputs_[input_neuron_id] * _last_gradient +
          alpha * _input_weights_delta[input_neuron_id];

      _input_weights_delta[input_neuron_id] = weight_delta;
      _input_weights[input_neuron_id] += weight_delta;
    }
  }

  std::ostream& operator<<(std::ostream& o)
  {
    for(float w : _input_weights)
    {
      o << w << "\t";
    }
    o << std::endl;
    return o;
  }

  private:
  static float activation_function_derivative(float x_)
  {
    // TODO: try Relu
    const float t = tanh(x_);
    return 1 - t * t;
  }

  const int _id{};
  std::vector<float> _input_weights;
  std::vector<float> _input_weights_delta;
  float _last_gradient{};

  // Learning rate
  const float eta = .15f;
  // Momentum coefficient
  const float alpha = .5f;
};