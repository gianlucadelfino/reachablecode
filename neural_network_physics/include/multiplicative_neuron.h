#pragma once

#include "multiplicative_layer.h"
#include "neuron_base.h"

class MultiplicativeNeuron : public NeuronBase
{
public:
  MultiplicativeNeuron(int id_, int num_inputs_) : NeuronBase(id_, gauss(num_inputs_)) {}

  virtual void update_gradient_outer(float cur_neuron_output_, float target_) override
  {
    const float delta = target_ - cur_neuron_output_;
    _last_gradient = delta;
  }

  virtual void update_gradient_inner(float cur_neuron_output_,
                                     const std::vector<Neuron_ptr>& downstream_neurons_) override
  {
    float delta{};
    // For every neuron we look at the weights connecting it to the next
    // layer to build up delta
    for (const auto& downstream_neuron : downstream_neurons_)
    {
      const float downstream_last_gradient = downstream_neuron->get_last_gradient();
      const float downstream_input_weight = downstream_neuron->get_input_weight(_id);

      delta += downstream_last_gradient * downstream_input_weight;
    }

    assert(!std::isnan(delta));
    _last_gradient = activation_function_derivative(cur_neuron_output_) * delta;
  }

  virtual void update_input_weights(const std::vector<float>& upstream_layer_outputs_) override
  {
    for (size_t i = 0; i < upstream_layer_outputs_.size(); ++i)
    {
      for (size_t j = i; j < upstream_layer_outputs_.size(); ++j)
      {
        const size_t input_weight_idx =
            j + i * upstream_layer_outputs_.size() - MultiplicativeNeuron::gauss(i);

        const float weight_delta =
            eta * upstream_layer_outputs_[i] * upstream_layer_outputs_[j] * _last_gradient +
            alpha * _input_weights_delta[input_weight_idx];

        assert(!std::isnan(weight_delta));

        _input_weights_delta[input_weight_idx] = weight_delta;
        _input_weights[input_weight_idx] += weight_delta;
      }
    }
  }

  virtual float activation_function(float val_) override
  {
    return val_;
    // TODO: try Relu
    // return val_>0? val_ : val_*0.01f;
    // return val_>0? val_ : 0;
    // return tanh(val_);
  }

  // TODO: move out to utils
  static int gauss(int n) { return 0.5f * (n * n + n); }

protected:
  virtual float activation_function_derivative(float x_) override
  {
    (void)x_;
    return 1;
    // TODO: try Relu
    // return x_? 1 : 0.01f;
    // return x_? 1 : 0;
    // const float t = tanh(x_);
    // return 1 - t * t;
  }
};