#pragma once

#include <cstring>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

#include <AL/al.h>
#include <AL/alc.h>

namespace
{

class AudioDevice
{
  public:
  explicit AudioDevice(const std::string& device_specifier_ = "")
  {
    checked_operation([&] {
      _device = alcCaptureOpenDevice(device_specifier_.c_str(),
                                     SAMPLE_RATE,
                                     AL_FORMAT_MONO16,
                                     SAMPLE_SIZE);
    });

    checked_operation([&] {
      _context = alcCreateContext(_device, nullptr);
    });

    checked_operation([&] {
      alcMakeContextCurrent(_context);
    });

    checked_operation([&] { alcCaptureStart(_device); });
  }

  std::vector<std::string> get_devices_names() const
  {
    // Pass in NULL device handle to get list of devices
    const ALCchar* device{};
    checked_operation([&]
    {
      device = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
    });

    const ALCchar* next{};
    size_t len = 0;

    // devices contains the device names, separated by NULL
    // and terminated by two consecutive NULLs.
    std::vector<std::string> names;
    while (device && *device != '\0' && next && *next != '\0')
    {
      len = strlen(device);
      device += (len + 1);
      next += (len + 2);
      names.push_back(device);
    }

    return names;
  }

  void record()
  {
    ALint samples_number{};

    checked_operation([&] {
      alcGetIntegerv(_device,
                     ALC_CAPTURE_SAMPLES,
                     (ALCsizei)sizeof(ALint),
                     &samples_number);
    });

    _buffer.resize(samples_number*SAMPLE_SIZE);

    checked_operation([&] {
      alcCaptureSamples(_device, (ALCvoid*)_buffer.data(), samples_number);
    });

    // ... do something with the buffer

  }

  ~AudioDevice()
  {
    alcDestroyContext(_context);
    alcCaptureStop(_device);
    alcCaptureCloseDevice(_device);
  }

  private:
  template <typename Callable>
  static void checked_operation(Callable&& callable_)
  {
    alGetError(); // Clear the error state

    callable_();

    ALCenum error{};
    if ((error = alGetError()) != AL_NO_ERROR)
    {
      throw std::runtime_error("Error initiating audio device" +
                               get_error_string(error));
    }
  }

  static std::string get_error_string(ALCenum error_)
  {
    std::string err_msg;
    switch (error_)
    {
    case AL_NO_ERROR:
      err_msg = "None";
      break;
    case AL_INVALID_NAME:
      err_msg = "Invalid name.";
      break;
    case AL_INVALID_ENUM:
      err_msg = "Invalid enum.";
      break;
    case AL_INVALID_VALUE:
      err_msg = "Invalid value.";
      break;
    case AL_INVALID_OPERATION:
      err_msg = "Invalid operation.";
      break;
    case AL_OUT_OF_MEMORY:
      err_msg = "Out of memory.";
      break;
    default:
      err_msg = "Unknown error.";
      break;
    }
    return err_msg;
  }

  const int SAMPLE_RATE = 44100;
  const int SAMPLE_SIZE = 1024;
  const int BUFFER_SIZE = SAMPLE_SIZE * 50;
  ALCdevice* _device;
  ALCcontext* _context;

  std::vector<std::byte> _buffer{BUFFER_SIZE};
};
} // namespace
