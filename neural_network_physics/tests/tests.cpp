#include "Logger.h"
#include "network.h"
#include "gtest/gtest.h"

namespace
{
void get_xor_training_data(std::vector<float>* inputs, std::vector<float>* targets)
{
  inputs->clear();
  targets->clear();

  const unsigned int a = rand() & 1;
  inputs->push_back(a);

  const unsigned int b = rand() & 1;
  inputs->push_back(b);

  targets->push_back(a ^ b);
}

} // namespace

TEST(Network, xor_learning)
{
  Network net(2);
  net.add_inner_layer(4);
  net.add_output_layer(1);

  for (int epoch = 0; epoch < 20000; ++epoch)
  {
    std::vector<float> inputs;
    std::vector<float> targets;
    get_xor_training_data(&inputs, &targets);

    const std::vector<float> outputs = net.feed_forward(inputs);

    const float err = net.get_cur_network_error(targets);

    // Debug
    // Logger::Info(
    //     "Epoch", epoch, "Input", inputs[0], ", ", inputs[1], "output", outputs[0], "err", err);
    net.back_propagate(targets);
  }

  for (const auto& [a, b] : std::vector<std::pair<int, int>>{{1, 0}, {0, 1}, {1, 1}, {0, 0}})
  {
    const float expected = a ^ b;

    const std::vector<float> outputs =
        net.feed_forward({static_cast<float>(a), static_cast<float>(b)});
    const float err = net.get_cur_network_error({expected});

    // Debug
    // Logger::Info("Test", "Input", a, ", ", b, "output", outputs[0], "err", err);

    EXPECT_NEAR(0.f, err, 0.0001f);
    EXPECT_NEAR(expected, outputs[0], 0.001f);
  }
}