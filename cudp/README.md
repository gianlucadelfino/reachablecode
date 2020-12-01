# cUDP

This is a UDP streamer that encodes video frames with OpecCV and sends them over the network with ASIO.

## Building
The project can be built on both Windows and Linux. The dependencies are OpenCV v4+ and Asio v1+. To build

    mkdir build
    cd build
    cmake ../
    make

## Usage
Launch the receiver and sender program passing the ip of the receiver to both. The port is hardcoded at the moment to 39009. If you intend to use it outside of your local network recall to open and forward the UDP port on your router settings.

    cUDPreceiver 192.168.0.1  # Or whatever is your local address
    cUDPsender 192.169.0.1

## More Info
[CppFiddler.com](https://www.cppfiddler.com)
