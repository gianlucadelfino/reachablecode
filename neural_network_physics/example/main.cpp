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

void generate_inputs(velocity* vel_, float* time_)
{
  // Initialize all the weight randomly
  static std::default_random_engine generator(42);
  static std::uniform_real_distribution<float> distribution(0.0f, +1.0f);

  vel_->x = distribution(generator);
  vel_->y = distribution(generator);
  *time_ = distribution(generator);
}

position get_targets(const velocity& vel_, const float time_)
{
  position pos;
  // constexpr float g = 9.8f;
  pos.x = vel_.x * time_;
  // vel = -g*time_ + vel_.y;
  // pos.y = -0.5f*time_*time_*g + vel_.y*time_;
  return pos;
}

int main()
{
  Network net(3);

  net.add_inner_layer<StandardLayer>(5);
  net.add_inner_layer<MultiplicativeLayer>(5);
  net.add_output_layer<StandardLayer>(2);

  for (int epoch = 0; epoch < 2000000; ++epoch)
  {
    velocity vel;
    float time{};
    generate_inputs(&vel, &time);

    const std::vector<float> outputs = net.feed_forward({vel.x, vel.y, time});

    const position pos = get_targets(vel, time);

    const float err = net.get_cur_network_error({pos.x * .1f, pos.y * .1f});
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
                   ") outputs (",
                   outputs[0] * 10,
                   outputs[1] * 10,
                   "). Err:",
                   err);
    }

    // net.print();

    net.back_propagate({pos.x * .1f, pos.y * .1f});
  }

  return 0;
}
