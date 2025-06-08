# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

Build the simplified tree demo:
```bash
make -f Makefile_simple
```

Run the tree demo:
```bash
./tree_demo
```

Clean build artifacts:
```bash
make clean
```

## Project Architecture

This is a procedural tree generation demo built with OpenGL that simulates realistic tree growth using L-system-like algorithms. The project focuses on real-time 3D tree generation with animated growth sequences.

### Core Components

**Tree Generation System** (`tree_simple.h/cpp`):
- Hierarchical branch structure with parent-child relationships
- Multi-generation growth (trunk → main branches → sub-branches → twigs)
- Procedural leaf placement on generation 2+ branches
- Animated growth with timing dependencies (children wait for parents to reach 60% growth)
- Configurable parameters: max_generations, branch_angle_variance, length/radius_reduction_factors

**Camera System** (`camera.h/cpp`):
- Interactive orbital camera with mouse controls
- Spherical coordinate system (radius, theta, phi)
- Scroll wheel zoom and click-drag rotation
- Automatic target tracking for tree observation

**Rendering Pipeline** (`main_simple.cpp`):
- OpenGL 3.x with GLEW/GLFW
- Basic shader system for vertices and fragments (`v_simplest.glsl`, `f_simplest.glsl`)
- Mesh generation from procedural tree data
- Real-time vertex buffer updates during growth animation

### Key Data Structures

**TreeBranch**: Contains start/end positions, radius, generation level, growth progress, parent/child indices
**TreeLeaf**: Position, normal, size, growth progress, parent branch reference, spawn delay

### Growth Algorithm

1. **Trunk Generation**: Starts at origin, grows upward (generation 0)
2. **Recursive Branching**: Each branch spawns 2-4 children with random angles
3. **Timing System**: 7-second delays between generation starts, 15-second growth duration per branch
4. **Leaf Spawning**: Appears on generation 2+ branches when parent reaches 40% growth
5. **Position Calculation**: Uses recursive functions to calculate absolute world positions from relative branch data

### Dependencies

- OpenGL 3.x with GLEW, GLFW, GLU
- GLM for math operations (matrices, vectors, transformations)
- C++11 standard
- lodepng for texture loading (though simplified version uses minimal texturing)

### Controls

- Mouse drag: Rotate camera around tree
- Scroll wheel: Zoom in/out
- ESC: Exit application

The codebase is designed for educational purposes, demonstrating procedural generation techniques while maintaining clean, readable code structure.