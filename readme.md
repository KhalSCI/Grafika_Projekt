 # Procedural Tree Generation System

This project implements a **procedural tree generation system** with animated growth. The system creates natural-looking trees that grow organically over time using mathematical algorithms rather than pre-made models.

## 🌱 System Overview

### 1. Initialization Phase (`main_simple.cpp`)

- **OpenGL Setup**: Initialize GLFW window, GLEW, shaders, and textures
- **Tree Generation**: Call `tree.generate()` to create the complete tree structure
- **Camera Setup**: Initialize orbital camera for viewing

### 2. Tree Structure Generation (`tree_simple.cpp:generate()`)

- **Recursive Branching**: Starting with trunk (generation 0), recursively creates child branches
- **Hierarchy Building**: Each branch can have 2-4 children, creating a realistic tree structure
- **6 Generations**: Trunk → Primary → Secondary → Tertiary → Quaternary → Quinary branches
- **Leaf Placement**: Generations 2+ get 6-14 randomly positioned leaves

### 3. Growth Animation System (`updateGrowth()`)

#### Sequential Generation Appearance:
- **Generation 0** (trunk): Starts immediately
- **Generation 1+**: Delayed by 15% of total time per generation
- Children wait for parent to reach 60% growth before starting

#### Timing Breakdown (10-second total):
- Each generation waits **1.5s** before starting
- Each branch takes **4s** to fully grow
- Leaves appear when parent is **40% grown** + individual delay

### 4. Position Calculation System

- **Relative Storage**: Branches store local positions relative to parents
- **Absolute Calculation**: `calculateAbsoluteBranchStart/End()` compute world positions
- **Animation Integration**: End positions scale by `growth_progress` for extending effect

### 5. Mesh Generation (`updateBranchMesh()`, `updateLeafMesh()`)

- **Branches**: Converted to tapered cylinders with 8-sided geometry
- **Leaves**: Rendered as double-sided textured quads (billboards)
- **Dynamic Updates**: Mesh regenerated each frame to reflect growth animation

### 6. Rendering Pipeline (`drawScene()`)

- **Matrix Setup**: Perspective projection + camera view matrix
- **Lighting**: Dynamic sun that orbits around the scene
- **Multi-texture Rendering**: Bark, leaf, grass, and sun textures
- **Scene Elements**: Ground plane, moving sun, animated tree

### 7. Camera Control

- **Orbital System**: Spherical coordinates around tree center
- **Mouse Control**: Drag to rotate, scroll to zoom
- **Keyboard Control**: WASD for movement, QE for tilt

### 8. Key Data Structures

#### `TreeBranch`:
- `start/end` positions
- `radius`
- `generation`
- `growth_progress`
- `parent/child` links

#### `TreeLeaf`:
- `position`
- `normal`
- `size`
- `growth_progress`
- `parent_branch_index`
- `spawn_delay`

## 🎯 Features

- ✨ **Procedural Generation**: Mathematical algorithms create unique trees each time
- 🎬 **Animated Growth**: Realistic growth animation over 10 seconds
- 🌿 **Natural Branching**: Organic branching patterns with proper hierarchy
- 🍃 **Dynamic Leaves**: Leaves appear and grow with their parent branches
- 💡 **Dynamic Lighting**: Moving sun creates realistic lighting effects
- 🎮 **Interactive Camera**: Full orbital camera control

The system creates a natural-looking tree that grows organically over time, with realistic branching patterns and leaf distribution, all controlled by mathematical algorithms rather than pre-made models.