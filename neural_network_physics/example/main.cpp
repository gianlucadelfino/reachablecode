#include "network.h"

#include "Logger.h"

void get_xor_training_data(std::vector<float>* inputs,
                           std::vector<float>* targets)
{
    inputs->clear();
    targets->clear();

    const unsigned int a = rand() & 1;
    inputs->push_back(a);

    const unsigned int b = rand() & 1;
    inputs->push_back(b);

    targets->push_back(a ^ b);
}


int main()
{

    Network net(2);
    net.add_inner_layer(4);
    net.add_output_layer(1);


    for(int epoch=0; epoch <20000; ++epoch)
    {
      std::vector<float> inputs;
      std::vector<float> targets;
      get_xor_training_data(&inputs, &targets);

      const std::vector<float> outputs = net.feed_forward(inputs);

      const float err = net.get_cur_network_error(targets);
      Logger::Info("Epoch", epoch, "err", err);
      Logger::Info("Input", inputs[0], ", ", inputs[1], "output", outputs[0]);

      // net.print();

      net.back_propagate(targets);
    }

  return 0;
}
