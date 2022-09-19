#pragma once

#include "layer_base.h"
#include "standard_layer.h"

class Network
{
public:
  explicit Network(int input_size_)
  {
    // + 1 is bias
    _layers.emplace_back(std::make_unique<StandardLayer>(input_size_ + 1, 0));
  }

  template <typename LayerType> void add_layer(int num_neurons_)
  {
    // + 1 is bias. NB the last later will have a bias too, but we ignore it
    _layers.emplace_back(std::make_unique<LayerType>(num_neurons_ + 1, _layers.back()->size()));
  }

  std::vector<float> feed_forward(const std::vector<float>& inputs_)
  {
    assert(_layers.size() >= 2);
    assert(static_cast<ssize_t>(inputs_.size()) + 1 == _layers.front()->size() &&
           "Network::feed_forward Error: "
           "the input size does not match the fist layer size!");

    _layers.front()->set_neurons_values(inputs_);

    // NB: It seems that the first layer is useless, but we need to remind that
    // the first layer also has the bias neuron, which will get passed to the
    // second layer with get_outputs()

    for (size_t i = 1; i < _layers.size(); ++i)
    {
      _layers[i]->feed_forward(_layers[i - 1]->get_outputs());
    }

    // Copy the last layer outwithout the bias
    std::vector<float> outputs;
    const std::vector<float>& last_layer_outputs = _layers.back()->get_outputs();
    assert(last_layer_outputs.size());
    outputs.reserve(last_layer_outputs.size());
    const auto bias_iter = std::prev(last_layer_outputs.cend());
    std::copy(last_layer_outputs.cbegin(), bias_iter, outputs.begin());

    return outputs;
  }

  /**
   * @brief Back propagate
   *
   * @param targets_: the wanted outputs
   */
  void back_propagate(const std::vector<float>& targets_)
  {
    assert(_layers.size() >= 2);

    // back propagate output layer
    _layers.back()->update_gradient_outer(targets_);

    // back propagate through inner layers
    for (int i = static_cast<int>(_layers.size()) - 2; i >=0 ; --i)
    {
      _layers[i]->update_gradient_inner(*_layers[i + 1]);
    }

    // for all the layers (but the input) we update the input weights
    for (size_t i = 1; i < _layers.size(); ++i)
    {
      _layers[i]->update_input_weights(*_layers[i - 1]);
    }
  }

  float get_cur_network_error(const std::vector<float>& targets_) const
  {
    assert(!_layers.empty());

    // Calculate overall network error (sum of squared errors)
    const auto& outputs = _layers.back()->get_outputs();
    assert(targets_.size() + 1 == outputs.size());

    float square_sum{};
    for (size_t i = 0; i < targets_.size(); ++i)
    {
      const float delta = targets_[i] - outputs[i];
      square_sum += delta * delta;
    }
    return 0.5f * square_sum;
  }

  void print() const
  {
    for (const auto& layer : _layers)
    {
      layer->print();
    }
  }

private:
  using Layer_ptr = std::unique_ptr<LayerBase>;
  std::vector<Layer_ptr> _layers;
};