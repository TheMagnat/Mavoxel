# Mavoxel
> A project of a voxel game engine.

Mavoxel aims to be a complete game engine to produce voxel-based games

## Getting Started

### Prerequisites

You need to have cmake installed.

To compile this project, you must install these libraries :

- [GLFW](https://www.glfw.org/)

- [Glad](https://glad.dav1d.de/)

- [GLM](https://glm.g-truc.net/)

- [FastNoise2](https://github.com/Auburn/FastNoise2)

### Installing

### Generate project files

Open a terminal at the root of the project.
Run this command to create project files :

```
cmake -S . -B build
```

### Compile project
Using the same terminal as above, run this command to compile the project :

```
cmake --build build --config Release
```

You can also change the build type to Debug instead of Release.

You will find the generated static library in build/Mavoxel/{build type}.

## Run

You can run the application generated in build/App/{build type}.


## Author

* **Magniadas Guillaume** - *Personal work* - [Magnat](https://github.com/TheMagnat)


## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
