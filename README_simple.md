# Simplified Tree Generation Demo

This is a stripped-down version of the tree generation project that focuses purely on the procedural tree generation algorithm without any visual enhancements like lighting, texturing, or colors.

## Recent Updates (v2)

- **Slower Tree Growth**: Tree now grows much more slowly for better observation (30 seconds total vs 10 seconds)
- **Interactive Camera**: Added mouse and keyboard camera controls for exploring the tree from different angles
- **Improved Timing**: More realistic growth progression with longer delays between generations

## What's Included

### Core Tree Generation
- **Procedural L-system-like branching**: Trees grow naturally with realistic branching patterns
- **Animated growth**: Branches and leaves grow over time with proper timing dependencies
- **Multi-generation hierarchy**: Supports up to 4 generations of branches (trunk → main branches → sub-branches → twigs)
- **Leaf generation**: Procedural leaf placement on branches of generation 2 and higher

### Key Features
- **Pure geometry**: Only vertex data, no colors, textures, or lighting
- **Hierarchical growth**: Child branches wait for parent branches to reach 60% growth before starting
- **Realistic timing**: Different growth speeds and delays for different generations
- **Proper connectivity**: Branches are properly connected using recursive positioning system

## Files

### Core Implementation
- `tree_simple.h` - Simplified tree class header with only essential functionality
- `tree_simple.cpp` - Core tree generation algorithm implementation
- `main_simple.cpp` - Main application with interactive camera controls
- `camera.h/cpp` - Interactive camera system for viewing the tree

### Shaders
- `v_simplest.glsl` - Basic vertex shader (just transforms vertices)
- `f_simplest.glsl` - Basic fragment shader (white color output)

### Build System
- `Makefile_simple` - Build configuration for the simplified version

## Building and Running

```bash
# Build the demo
make -f Makefile_simple

# Run the demo
./tree_demo
```

## Controls
- **Mouse**: Click and drag to rotate camera around the tree
- **Scroll Wheel**: Zoom in and out
- **WASD**: Move camera (if implemented in camera class)
- **ESC**: Exit the application

## Tree Parameters

The tree generation can be customized by modifying these parameters in `Tree::Tree()`:

```cpp
max_growth_time = 30.0f;          // Total time for full growth (increased for slower growth)
max_generations = 4;              // Number of branch generations
branch_angle_variance = 45.0f;    // Random angle variation in degrees
length_reduction_factor = 0.7f;   // How much shorter each generation gets
radius_reduction_factor = 0.8f;   // How much thinner each generation gets
```

## Growth Timing Changes

The growth has been significantly slowed down for better observation:
- **Total Growth Time**: 30+ seconds for complete tree
- **Trunk Growth**: Same speed as branches (consistent timing)
- **Generation Delay**: 7 seconds between generation starts (trunk at 0s, gen1 at 7s, gen2 at 14s, gen3 at 21s)
- **Branch Growth Duration**: 15 seconds per branch (good speed for observation)
- **All Branch Growth Speed**: 0.1x (consistent speed for trunk and all branches)
- **Leaf Growth**: Individual leaves appear gradually with 0-4 second random delays
- **Leaf Growth Duration**: 5 seconds per leaf with very slow growth speed (0.1x)

## Algorithm Details

### Branch Generation
1. **Trunk**: Starts at origin, grows upward
2. **Child Branches**: Generated with random angles and reduced size
3. **Recursive Structure**: Each branch can spawn 2-4 child branches
4. **Proper Positioning**: Uses recursive functions to calculate absolute positions

### Growth Animation
- **Trunk grows independently** (generation 0)
- **Child branches wait** for parent to reach 60% growth
- **Leaves appear** on branches of generation 2+ when parent reaches 40% growth
- **Different speeds** for different elements (branches vs leaves)

### Key Functions
- `generateBranch()`: Recursive branch creation
- `updateGrowth()`: Handles growth animation timing
- `calculateAbsoluteBranchStart/End()`: Recursive position calculation
- `updateBranchMesh()`: Converts branch data to renderable geometry
- `updateLeafMesh()`: Converts leaf data to renderable geometry

## Extending the Code

This simplified version provides a solid foundation for adding:
- **Lighting and materials**: Modify shaders and add normal/color data
- **Texturing**: Add texture coordinates and texture loading
- **Wind effects**: Add time-based deformation to vertices
- **Different tree types**: Modify generation parameters
- **Interactive controls**: Add camera movement, parameter adjustment
- **Multiple trees**: Instance the Tree class for forests

## Performance

The current implementation generates:
- ~82 branches with multiple generations
- ~430 leaves with realistic placement
- Smooth 60fps animation during growth phase
- Efficient geometry generation using OpenGL triangle primitives

This provides an excellent base for more complex tree rendering systems while keeping the core generation algorithm clean and understandable.
