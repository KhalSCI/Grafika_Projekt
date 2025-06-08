# Tree Generation Algorithm - Detailed Technical Documentation

## Overview

This document provides a comprehensive technical analysis of the procedural tree generation system implemented in this OpenGL project. The system uses an L-system-inspired approach combined with hierarchical growth animation to create realistic, organically growing trees in real-time.

## Table of Contents
1. [Core Data Structures](#core-data-structures)
2. [Generation Algorithm](#generation-algorithm)
3. [Growth Animation System](#growth-animation-system)
4. [Geometric Construction](#geometric-construction)
5. [Rendering Pipeline](#rendering-pipeline)
6. [Mathematical Foundations](#mathematical-foundations)
7. [Parameter Configuration](#parameter-configuration)

---

## Core Data Structures

### TreeBranch Structure
```cpp
struct TreeBranch {
    glm::vec3 start;          // Local start position relative to parent
    glm::vec3 end;            // Local end position relative to parent
    float radius;             // Branch thickness
    int generation;           // Hierarchy level (0=trunk, 1=main branches, etc.)
    float growth_progress;    // Animation state (0.0 to 1.0)
    std::vector<int> children; // Indices to child branches
    int parent_index;         // Index to parent branch (-1 for trunk)
};
```

**Key Design Decisions:**
- **Local Coordinates**: Start/end positions are stored relative to the parent branch, enabling hierarchical transformations
- **Parent-Child Relationships**: Bidirectional links allow both top-down generation and bottom-up position calculation
- **Growth Progress**: Enables smooth animation interpolation between growth states

### TreeLeaf Structure
```cpp
struct TreeLeaf {
    glm::vec3 position;       // Position relative to parent branch end
    glm::vec3 normal;         // Surface normal for lighting calculations
    float size;               // Base size of the leaf
    float growth_progress;    // Animation state (0.0 to 1.0)
    int parent_branch_index;  // Which branch this leaf belongs to
    float spawn_delay;        // Individual timing offset for organic appearance
};
```

**Key Features:**
- **Organic Positioning**: Each leaf has individual spawn delay for natural, non-uniform growth
- **Billboard Rendering**: Normals are used to orient leaf quads toward the camera/light
- **Size Scaling**: Leaves grow from minimum size to full size during animation

---

## Generation Algorithm

### Phase 1: Recursive Branch Generation

The tree generation starts with a single trunk and recursively creates child branches:

```cpp
void Tree::generateBranch(int parent_index, glm::vec3 start, glm::vec3 direction, 
                         float length, float radius, int generation)
```

**Algorithm Steps:**

1. **Branch Creation**
   ```cpp
   TreeBranch branch;
   branch.start = start;
   branch.end = start + direction * length;
   branch.radius = radius;
   branch.generation = generation;
   ```

2. **Hierarchy Linking**
   ```cpp
   // Add to parent's children list
   if (parent_index >= 0) {
       branches[parent_index].children.push_back(branch_index);
   }
   ```

3. **Child Generation** (if not at maximum generation)
   ```cpp
   std::uniform_int_distribution<int> child_count_dist(2, 4);
   int num_children = child_count_dist(gen);
   ```

4. **Angular Distribution**
   ```cpp
   for (int i = 0; i < num_children; i++) {
       float angle = (float)i / num_children * 2.0f * M_PI + random_variance;
       float elevation = 30.0f + random_elevation_variance;
   }
   ```

### Phase 2: Leaf Generation

Leaves are generated for branches of generation 2 and higher:

```cpp
if (generation >= 2) {
    std::uniform_int_distribution<int> leaf_count_dist(6, 14);
    int num_leaves = leaf_count_dist(gen);
    
    for (int j = 0; j < num_leaves; j++) {
        TreeLeaf leaf;
        leaf.position = branch.end + random_offset;
        leaf.spawn_delay = delay_dist(gen);  // 0-4 second random delay
    }
}
```

**Leaf Placement Strategy:**
- Random positioning around branch endpoints
- Upward-biased normals for realistic lighting
- Variable spawn delays for organic growth patterns

---

## Growth Animation System

### Timing Architecture

The animation system uses a sophisticated timing model to create realistic growth sequences:

```cpp
// Timing parameters (scaled by max_growth_time)
float generation_delay = max_growth_time * 0.15f;    // 15% delay between generations
float growth_duration = max_growth_time * 0.4f;      // 40% duration for each branch
float start_time = branch.generation * generation_delay;
```

### Branch Growth Logic

1. **Trunk Growth** (Generation 0)
   ```cpp
   if (branch.generation == 0) {
       float elapsed = current_growth_time - start_time;
       branch.growth_progress = std::min(1.0f, elapsed / growth_duration);
   }
   ```

2. **Child Branch Growth** (Generations 1+)
   ```cpp
   if (parent_progress > 0.6f) {  // Wait for parent to reach 60%
       float parent_60_percent_time = 0.6f * growth_duration + ...;
       float actual_start_time = std::max(start_time, parent_60_percent_time);
       float elapsed = current_growth_time - actual_start_time;
       branch.growth_progress = std::min(1.0f, elapsed / growth_duration);
   }
   ```

### Leaf Growth Logic

Leaves begin growing when their parent branch reaches 40% completion:

```cpp
if (parent_progress > 0.4f) {
    float leaf_start_offset = max_growth_time * 0.1f;
    float leaf_growth_duration = max_growth_time * 0.1f;
    float start_time = base_start_time + leaf.spawn_delay;
    
    if (current_growth_time > start_time) {
        float elapsed = current_growth_time - start_time;
        leaf.growth_progress = std::min(1.0f, elapsed / leaf_growth_duration);
    }
}
```

**Growth Sequence Timeline:**
- **0-15%**: Trunk growth begins
- **15-30%**: Generation 1 branches start (when trunk reaches 60%)
- **30-45%**: Generation 2 branches start
- **45-60%**: Generation 3 branches start
- **Throughout**: Leaves appear with individual delays when parent branches reach 40%

---

## Geometric Construction

### Absolute Position Calculation

Since branches store local coordinates relative to their parents, absolute world positions must be calculated recursively:

```cpp
glm::vec3 Tree::calculateAbsoluteBranchStart(int branch_index) {
    const auto& branch = branches[branch_index];
    
    if (branch.parent_index == -1) {
        return branch.start;  // Trunk uses local coordinates directly
    }
    
    // Child branch starts at parent's absolute end position
    return calculateAbsoluteBranchEnd(branch.parent_index);
}

glm::vec3 Tree::calculateAbsoluteBranchEnd(int branch_index) {
    const auto& branch = branches[branch_index];
    glm::vec3 absolute_start = calculateAbsoluteBranchStart(branch_index);
    glm::vec3 local_direction = branch.end - branch.start;
    
    // Apply growth progress for animation
    glm::vec3 current_direction = local_direction * branch.growth_progress;
    return absolute_start + current_direction;
}
```

### Branch Mesh Generation

Branches are rendered as cylindrical segments with circular cross-sections:

```cpp
void Tree::addBranchSegment(glm::vec3 start, glm::vec3 end, 
                           float start_radius, float end_radius) {
    const int segments = 8;  // 8-sided cylinder
    
    // Create local coordinate system
    glm::vec3 direction = glm::normalize(end - start);
    glm::vec3 right = glm::normalize(glm::cross(direction, up));
    glm::vec3 up = glm::normalize(glm::cross(right, direction));
    
    // Generate ring vertices
    for (int i = 0; i < segments; i++) {
        float angle1 = (float)i / segments * 2.0f * M_PI;
        float angle2 = (float)(i + 1) / segments * 2.0f * M_PI;
        
        // Compute cylinder surface points
        glm::vec3 p1 = start + right * cos(angle1) * start_radius + 
                              up * sin(angle1) * start_radius;
        // ... generate quad vertices and triangles
    }
}
```

### Leaf Mesh Generation

Leaves are rendered as billboarded quads with both front and back faces:

```cpp
void Tree::addLeafQuad(glm::vec3 position, glm::vec3 normal, 
                      float size, float growth) {
    // Scale size based on growth progress
    float min_size = 0.05f;
    size = min_size + (size - min_size) * growth;
    
    // Create quad coordinate system
    glm::vec3 right = glm::normalize(glm::cross(normal, glm::vec3(0,1,0)));
    glm::vec3 up = glm::normalize(glm::cross(right, normal));
    
    // Generate quad corners
    glm::vec3 v1 = position - right - up;
    glm::vec3 v2 = position + right - up;
    glm::vec3 v3 = position + right + up;
    glm::vec3 v4 = position - right + up;
    
    // Create two triangles for front face, two for back face
}
```

---

## Rendering Pipeline

### Vertex Shader Processing

The vertex shader (`v_simplest.glsl`) transforms vertices and prepares lighting data:

```glsl
void main(void) {
    vec4 vertexEyeSpace = V * M * vertex;
    vec4 lightEyeSpace = V * vec4(lightPos, 1.0);
    
    l = normalize(lightEyeSpace - vertexEyeSpace);  // Light vector
    v = normalize(vec4(0, 0, 0, 1) - vertexEyeSpace);  // View vector
    n = normalize(V * M * vec4(normal, 0.0));  // Normal vector
    
    gl_Position = P * V * M * vertex;
}
```

### Fragment Shader Lighting

The fragment shader (`f_simplest.glsl`) implements Phong lighting with texture mixing:

```glsl
void main(void) {
    vec4 ml = normalize(l);
    vec4 mn = normalize(n);
    vec4 mv = normalize(v);
    vec4 mr = reflect(-ml, mn);
    
    // Select texture based on uniform flags
    vec4 kd;
    if (useBarkTex == 1) {
        kd = mix(texture(textureMap0, iTexCoord0), 
                texture(textureMap0, iTexCoord1), 0.25);
    } else if (useLeafTex == 1) {
        kd = mix(texture(textureMap1, iTexCoord0), 
                texture(textureMap1, iTexCoord1), 0.3);
    }
    
    // Phong lighting calculation
    float nl = clamp(dot(mn, ml), 0.0, 1.0);
    float rv = pow(clamp(dot(mr, mv), 0.0, 1.0), 25.0);
    
    vec3 ambient = 0.05 * kd.rgb;
    vec3 diffuse = kd.rgb * nl;
    vec3 specular = ks.rgb * rv;
    
    pixelColor = vec4(ambient + diffuse + specular, kd.a);
}
```

### Rendering Loop Integration

The main rendering loop (`main_simple.cpp`) coordinates the animation:

```cpp
void renderLoop() {
    double currentTime = glfwGetTime();
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    
    // Update tree growth animation
    tree.updateGrowth(deltaTime);
    
    // Update camera
    camera.update(window);
    
    // Render tree components
    renderBranches();
    renderLeaves();
    renderGround();
}
```

---

## Mathematical Foundations

### Spherical Coordinate Branch Placement

Child branches are positioned using spherical coordinates for natural distribution:

```cpp
// Azimuthal angle (around parent branch)
float angle = (float)i / num_children * 2.0f * M_PI + random_variance;

// Elevation angle (upward bias)
float elevation = 30.0f + random_elevation_variance;

// Convert to Cartesian coordinates
glm::vec3 child_direction;
child_direction.x = sin(angle) * cos(elevation * M_PI / 180.0f);
child_direction.y = sin(elevation * M_PI / 180.0f);
child_direction.z = cos(angle) * cos(elevation * M_PI / 180.0f);
```

### Reduction Factors

Each generation becomes progressively smaller:

```cpp
float child_length = length * length_reduction_factor;  // 0.7f
float child_radius = radius * radius_reduction_factor;  // 0.7f
```

This creates natural taper from trunk to twigs.

### Growth Interpolation

Linear interpolation is used for smooth animation:

```cpp
glm::vec3 current_end = start + (end - start) * growth_progress;
```

---

## Parameter Configuration

### Timing Parameters
```cpp
max_growth_time = 10.0f;           // Total animation duration
generation_delay = 15% of total    // Delay between generation starts
growth_duration = 40% of total     // Time for each branch to grow
leaf_growth_duration = 10% of total // Time for leaf growth
```

### Structural Parameters
```cpp
max_generations = 6;               // Maximum hierarchy depth
branch_angle_variance = 45.0f;     // Random angle variation (degrees)
length_reduction_factor = 0.7f;    // Size reduction per generation
radius_reduction_factor = 0.7f;    // Thickness reduction per generation
```

### Leaf Parameters
```cpp
leaf_count_range = [6, 14];        // Number of leaves per branch
leaf_size_range = [0.28f, 0.45f];  // Leaf size variation
spawn_delay_range = [0.0f, 4.0f];  // Individual leaf timing variation
```

## Performance Characteristics

The current implementation generates approximately:
- **82 branches** across 6 generations
- **430 leaves** with individual animation timing
- **60fps** smooth animation during growth phase
- **Efficient memory usage** with vector-based storage

The algorithm scales O(n) with the number of branches, making it suitable for real-time applications with hundreds of trees.

---

## Extension Possibilities

This foundation supports numerous enhancements:

1. **Wind Simulation**: Add time-based vertex deformation
2. **Seasonal Changes**: Modify leaf colors and density over time
3. **Interactive Pruning**: Remove branches and regrow
4. **Forest Generation**: Instance multiple trees with parameter variation
5. **Level-of-Detail**: Reduce geometry for distant trees
6. **Advanced Materials**: Add subsurface scattering for leaves
7. **Procedural Bark**: Generate detailed bark texture patterns

The modular design of the system makes these extensions straightforward to implement while maintaining the core generation algorithm's integrity.