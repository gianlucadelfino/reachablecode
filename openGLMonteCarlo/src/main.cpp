#define CL_USE_DEPRECATED_OPENCL_1_2_APIS // cl.hpp
#define CL_TARGET_OPENCL_VERSION 120

#include <CL/cl.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>


#define PLATFORM 0
#define DEVICE 0

#define COMPILE_OPTS "-I " GENERATOR_LOCATION

int main()
{
    const std::ifstream kernelFile("calculate_trajectory.cl");

    std::stringstream ss;
    ss << kernelFile.rdbuf();
    const std::string kernelSource = ss.str();

    // std::cout << kernelSource << std::endl;

    // Get platform and device information
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    if (platforms.empty())
    {
        throw std::runtime_error("Error getting platformas");
    }

    std::vector<cl::Device> devices;
    platforms[PLATFORM].getDevices(CL_DEVICE_TYPE_ALL, &devices);

    if (devices.empty())
    {
        throw std::runtime_error("Error getting devices");
    }

    cl::Device device = devices[DEVICE];
    cl::Context context({device});

    cl::CommandQueue queue(context, device);
    // build kernel
    cl::Program::Sources sources(
        1, std::make_pair(kernelSource.c_str(), kernelSource.length()));
    cl::Program program(context, sources);

    program.build(std::vector<cl::Device>({device}), COMPILE_OPTS);
    // std::cout << "CL Build info: "
    //           << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << "\n";
    cl::Kernel kernel(program, "calculate_trajectory");

    // Set the kernel arguments
    // kernel void calculate_trajectory(
    //      float share_value_, const float rate_, const float dt_, int
    //      num_iterations_, const float variance_, global float* final_values_)

    const float initial_share_value = 1.f; // Start at 1 dollar
    const float interest_rate = 0.05f;
    const float dt = 0.1f;
    const int num_iterations = 365;
    const float gaussian_variance = 0.1f;

    const size_t num_trajectories = 10000;
    cl::Buffer cl_final_share_values(
        context, CL_MEM_WRITE_ONLY, num_trajectories * sizeof(cl_float));

    kernel.setArg(0, sizeof(initial_share_value), &initial_share_value);
    kernel.setArg(1, sizeof(interest_rate), &interest_rate);
    kernel.setArg(2, sizeof(dt), &dt);
    kernel.setArg(3, sizeof(num_iterations), &num_iterations);
    kernel.setArg(4, sizeof(gaussian_variance), &gaussian_variance);
    kernel.setArg(5, cl_final_share_values);

    // Run The kernel
    cl::Event e;
    queue.enqueueNDRangeKernel(
        kernel, cl::NullRange, cl::NDRange(num_trajectories), cl::NullRange, nullptr, &e);
    e.wait();

    std::vector<float> res(num_trajectories);
    queue.enqueueReadBuffer(cl_final_share_values,
                            true,
                            0,
                            num_trajectories * sizeof(cl_float),
                            &res[0]);

    std::ofstream results_file("out.txt");
    for (cl_float num : res)
    {
        results_file << num << "\n";
    }

    return 0;
}
