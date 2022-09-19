#pragma once

#include "layer_base.h"
#include "multiplicative_neuron.h"

class MultiplicativeLayer : public LayerBase
{
public:
  MultiplicativeLayer(int num_neurons_, int num_inputs_) : LayerBase(num_neurons_, num_inputs_)
  {
    _neurons.reserve(num_neurons_);
    for (int id = 0; id < num_neurons_; ++id)
    {
      _neurons.emplace_back(std::make_unique<MultiplicativeNeuron>(id, num_inputs_));
    }
  }

  void feed_forward(const std::vector<float>& prev_layer_outputs_) override
  {
    for (size_t n = 0; n < _neurons.size(); ++n)
    {
      const auto& weights = _neurons[n]->get_input_weights();
      // This layer should have N^2-N weights per neuron.
      const size_t num_outputs = prev_layer_outputs_.size();
      [[maybe_unused]] const size_t num_input_weights = MultiplicativeNeuron::gauss(num_outputs);
      assert(weights.size() == num_input_weights);

      /*
      prev outputs:
        a b c

      combinations to consider:
        aa ab ac
        bb bc
        cc

      Neuron_n = Wn_1 aa + Wn_2 ab + Wn_3 ac + Wn_4 bb + Wn_5 bc + Wn_6 cc
      */
      float multiplicative_sum{};
      for (size_t i = 0; i < num_outputs; ++i)
      {
        for (size_t j = i; j < num_outputs; ++j)
        {
          const size_t input_weight_idx = j + i * num_outputs - MultiplicativeNeuron::gauss(i);
          assert(input_weight_idx < num_input_weights);
          multiplicative_sum +=
              weights[input_weight_idx] * prev_layer_outputs_[i] * prev_layer_outputs_[j];
        }
      }

      _neuron_outputs[n] = multiplicative_sum;
      assert(!std::isnan(_neuron_outputs[n]));
    }
  }
};