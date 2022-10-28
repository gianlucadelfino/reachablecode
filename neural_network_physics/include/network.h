#pragma once

#include "layer_base.h"
#include "standard_layer.h"

/**
 * @brief Instatiate a Network object and add as many layer as needed.
 * The network can be trained via the back_propagate/get_cur_network_error apis
 */
class Network
{
public:
  explicit Network(int input_size_);

  template <typename LayerType> void add_layer(int num_neurons_)
  {
    // + 1 is bias. NB the last later will have a bias too, but we ignore it
    _layers.emplace_back(std::make_unique<LayerType>(num_neurons_ + 1, _layers.back()->size()));
  }

  /**
   * @brief Run the network forward given a set of inputs
   *
   * @param inputs_
   * @return std::vector<float>: the outputs of the network
   */
  std::vector<float> feed_forward(const std::vector<float>& inputs_);

  /**
   * @brief Run the back propagate algorithm
   *
   * @param targets_: the wanted outputs
   */
  void back_propagate(const std::vector<float>& targets_);

  /**
   * @brief Get the cur network error value. This is the sum of the squares of
   *  the differences between the targets and the current values of the output
   * layer
   *
   * @param targets_
   * @return float
   */
  float get_cur_network_error(const std::vector<float>& targets_) const;

  void print() const;

private:
  using Layer_ptr = std::unique_ptr<LayerBase>;
  std::vector<Layer_ptr> _layers;
};