# VerletSimCuda
A simple verlet solver written in C/C++, SFML and CUDA, running on the CPU and GPU.

### Features
This simulation currently features circles that can interact with the walls of the window and themselves.
In the current demo, the first thousand balls are being shot in from on static point, while the rest of them drop in from random location.

Physical parameters, such as gravity, friction as well as the bounciness can be set at the top via definitions.
The maximum particle count and the number of sub steps for one iteration can also be set there.

### Installation and Execution
To run the simulation, the Nvidia CUDA toolkit is required.
For Ubuntu and Ubuntu-based systems, this can be achieved by running:
> sudo apt-get install nvidia-cuda-toolkit

For other systems take a look at [Nvidia's download page](https://developer.nvidia.com/cuda-downloads) for CUDA toolkits.

SFML is also required for compiling the programm.
In Ubuntu, SFML can be acquired by running:
> sudo apt-get install libsfml-dev

You can also download it from the [official SFML download page](https://www.sfml-dev.org/download.php).

The programm can then be compiled by running:
> make

To execute the simulation, run:
> ./clean


