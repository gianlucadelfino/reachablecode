#include <fstream>

#include "network.h"

#include "Logger.h"
#include "multiplicative_layer.h"
#include "standard_layer.h"

struct position
{
  float x{};
  float y{};
};

struct velocity : public position
{
};

position get_targets(const velocity& vel_, const float time_)
{
  position pos;
  pos.x = vel_.x * time_;
  constexpr float g = 9.8f;
  // vel = -g*time_ + vel_;
  pos.y = -0.5f * time_ * time_ * g + vel_.y * time_;
  return pos;
}

int main()
{
  // Define the model
  Network net(3);
  net.add_layer<MultiplicativeLayer>(2);
  // net.add_layer<StandardLayer>(2);

  // Training
  std::default_random_engine generator(42);
  std::uniform_real_distribution<float> distribution(0.0f, +1.f);

  // We scale the output by 100 as the network can output only 0-1 values in if a standard layer is
  // the output, but the expected values can be bigger
  const float scale = 0.01f;

  for (int epoch = 0; epoch < 10000000; ++epoch)
  {
    velocity vel{distribution(generator), distribution(generator)};
    float time{distribution(generator)};

    const std::vector<float> outputs = net.feed_forward({vel.x, vel.y, time});

    const position pos = get_targets(vel, time);

    const float err = net.get_cur_network_error({pos.x * scale, pos.y * scale});

    if (!(epoch % 100000))
    {
      Logger::Info("Epoch",
                   epoch,
                   "Input vel (",
                   vel.x,
                   vel.y,
                   ") target: (",
                   pos.x,
                   pos.y,
                   ", outputs (",
                   outputs[0] / scale,
                   outputs[1] / scale,
                   "). Err:",
                   err);
    }

    net.back_propagate({pos.x * scale, pos.y * scale});
  }

  // Compute and dump a parabola with our NN
  const velocity init_vel{distribution(generator), distribution(generator)};
  std::ofstream out("parabola.data");
  out << "time(s)"
      << "\t"
      << "Real"
      << "\t"
      << "NeuralNet" << std::endl;
  for (float time = 0.01f; time < 10.f; time += 0.1f)
  {
    const position expected_pos = get_targets(init_vel, time);

    const std::vector<float> outputs = net.feed_forward({init_vel.x, init_vel.y, time});
    const position predicted_pos{outputs[0] / scale, outputs[1] / scale};

    out << time << "\t" << expected_pos.y << "\t" << predicted_pos.y << std::endl;
  }

  return 0;
}
