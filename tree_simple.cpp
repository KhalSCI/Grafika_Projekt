#include "tree_simple.h"
#include <random>
#include <cmath>
#include <iostream>
#include <algorithm>

/**
 * Tree Constructor - Initializes the procedural tree generation parameters
 * 
 * This constructor sets up the fundamental parameters that control how the tree
 * will grow both structurally and over time during the animation.
 */
Tree::Tree() {
    // === TIMING PARAMETERS ===
    // Total duration for complete tree growth animation (in seconds)
    // This controls how long it takes for the entire tree to fully grow
    max_growth_time = 10.0f;  // 10 seconds total growth time
    current_growth_time = 0.0f;
    max_generations = 6;
    branch_angle_variance = 45.0f; // Degrees of variance for branch angles
    // How much shorter each generation of branches becomes
    length_reduction_factor = 0.7f;  
    // How much thinner each generation of branches becomes
    radius_reduction_factor = 0.7f;
}

Tree::~Tree() {

}


void Tree::generate() {
    /*
     * STEP 1: INITIALIZE TREE GENERATION
     * Clear all existing data structures and reset growth timer
     */
    branches.clear();     // Remove all existing branch data
    leaves.clear();       // Remove all existing leaf data
    branch_vertices.clear(); // Clear OpenGL vertex data for branches
    leaf_vertices.clear();   // Clear OpenGL vertex data for leaves
    
    current_growth_time = 0.0f; // Reset animation timer
    
    /*
     * STEP 2: CREATE TREE TRUNK (GENERATION 0)
     * The trunk is the root of the recursive tree structure
     * Parameters: parent_index=-1 (no parent), start=(0,0,0), direction=upward,
     *            length=3.0, radius=0.2, generation=0
     */
    glm::vec3 trunk_start(0.0f, 0.0f, 0.0f);  // Tree base at origin
    glm::vec3 trunk_direction(0.0f, 1.0f, 0.0f); // Straight up
    generateBranch(-1, trunk_start, trunk_direction, 3.0f, 0.2f, 0);
    
    /*
     * STEP 3: GENERATION STATISTICS (DEBUG OUTPUT)
     * Analyze the generated tree structure for debugging
     */
    std::cout << "Tree generated with " << branches.size() << " branches and " << leaves.size() << " leaves." << std::endl;
    
    // Debug: Count branches by generation
    std::vector<int> generation_counts(max_generations + 1, 0);
    for (const auto& branch : branches) {
        if (branch.generation <= max_generations) {
            generation_counts[branch.generation]++;
        }
    }
    
    for (int g = 0; g <= max_generations; g++) {
        std::cout << "Generation " << g << ": " << generation_counts[g] << " branches" << std::endl;
    }
    
    // Debug: Show some branch details
    std::cout << "Sample branch details:" << std::endl;
    for (int i = 0; i < std::min(10, (int)branches.size()); i++) {
        const auto& branch = branches[i];
        float length = glm::length(branch.end - branch.start);
        std::cout << "Branch " << i << " - Gen: " << branch.generation 
                  << ", Length: " << length << ", Radius: " << branch.radius << std::endl;
    }
}

/**
 * generateBranch - Core recursive function for procedural tree generation
 * 
 * This function implements the heart of the tree generation algorithm. It creates
 * a single branch and then recursively generates child branches, building up the
 * complete tree structure generation by generation.
 * 
 * === RECURSIVE GENERATION PROCESS ===
 * 1. Create current branch with given parameters
 * 2. Add branch to data structures and link to parent
 * 3. If not at max generation depth, create child branches:
 *    - Generate 2-4 child branches in random directions
 *    - Apply reduction factors to make children smaller/thinner
 *    - Recursively call generateBranch for each child
 * 4. If generation >= 2, add leaves to branch tips
 * 
 * === BRANCH HIERARCHY ===
 * Generation 0: Main trunk (single, thick, vertical)
 * Generation 1: Primary branches (2-4 branches from trunk top)
 * Generation 2: Secondary branches + leaves (smaller, first to get leaves)
 * Generation 3+: Tertiary branches + more leaves (progressively smaller)
 * 
 * @param parent_index Index of parent branch (-1 for trunk/root)
 * @param start Starting position of this branch (world coordinates)
 * @param direction Direction vector (normalized) for branch growth
 * @param length Length of this branch segment
 * @param radius Thickness radius of this branch
 * @param generation Generation level (0=trunk, 1=primary, 2=secondary, etc.)
 */
void Tree::generateBranch(int parent_index, glm::vec3 start, glm::vec3 direction, 
                         float length, float radius, int generation) {
    // === STEP 1: CREATE CURRENT BRANCH ===
    // Build the TreeBranch data structure for this generation
    TreeBranch branch;
    branch.start = start;               // Starting point in world space
    branch.end = start + direction * length;  // Calculate end point from direction and length
    branch.radius = radius;             // Branch thickness
    branch.generation = generation;     // Which generation level this belongs to
    branch.growth_progress = 0.0f;      // Animation progress (0=invisible, 1=fully grown)
    branch.parent_index = parent_index; // Link to parent (-1 for trunk)
    
    // === STEP 2: ADD TO DATA STRUCTURES ===
    // Store this branch and get its index for parent-child linking
    int branch_index = branches.size();
    branches.push_back(branch);
    
    // Add this branch as a child to its parent (if not trunk)
    if (parent_index >= 0) {
        branches[parent_index].children.push_back(branch_index);
    }
    
    // === STEP 3: RECURSIVE CHILD GENERATION ===
    // Continue generating child branches if we haven't reached maximum depth
    if (generation < max_generations) {
        // === RANDOM NUMBER GENERATION SETUP ===
        std::random_device rd;
        std::mt19937 gen(rd());  // Note: Using rd() gives different results each time
        std::uniform_real_distribution<float> angle_dist(-1.0f, 1.0f);
        std::uniform_int_distribution<int> child_count_dist(2, 4);  // 2-4 children per branch
        
        int num_children = child_count_dist(gen);
        
        // === GENERATE EACH CHILD BRANCH ===
        for (int i = 0; i < num_children; i++) {
            // === CHILD DIRECTION CALCULATION ===
            // Create pseudo-random but evenly distributed child directions
            // This creates a natural branching pattern that looks organic
            
            // Base angle: evenly distribute children around circle
            float base_angle = (float)i / num_children * 2.0f * (float)M_PI;
            
            // Add random variation to base angle for natural look
            float angle_variation = angle_dist(gen) * branch_angle_variance * (float)M_PI / 180.0f;
            float angle = base_angle + angle_variation;
            
            // Elevation angle: how much branches grow upward vs outward
    
            float elevation = 30.0f + angle_dist(gen) * 20.0f; // Degrees
            
            // Convert spherical coordinates to Cartesian direction vector
            glm::vec3 child_direction;
            child_direction.x = sinf(angle) * cosf(elevation * (float)M_PI / 180.0f);
            child_direction.y = sinf(elevation * (float)M_PI / 180.0f);  // Upward component
            child_direction.z = cosf(angle) * cosf(elevation * (float)M_PI / 180.0f);
            child_direction = glm::normalize(child_direction);
            
            // === APPLY REDUCTION FACTORS ===
            // Each generation gets progressively smaller and thinner
            float child_length = length * length_reduction_factor;  // 70% of parent length
            float child_radius = radius * radius_reduction_factor;  // 70% of parent radius
            
            // === RECURSIVE CALL ===
            // Generate the child branch, which will in turn generate its own children
            generateBranch(branch_index, branch.end, child_direction, child_length, child_radius, generation + 1);
        }
    }
    
    // === STEP 4: LEAF GENERATION ===
    // Add leaves to branches starting from generation 2 (secondary branches)
    // Trunk (gen 0) and primary branches (gen 1) don't get leaves - only wood
    if (generation >= 2) {
        // === LEAF RANDOM GENERATION SETUP ===
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> angle_dist(-1.0f, 1.0f);
        std::uniform_int_distribution<int> leaf_count_dist(6, 14);    // 6-14 leaves per branch
        std::uniform_real_distribution<float> size_dist(0.28f, 0.45f); // Leaf size variation
        std::uniform_real_distribution<float> delay_dist(0.0f, 4.0f);  // Staggered appearance timing
        
        int num_leaves = leaf_count_dist(gen);
        
        // === GENERATE INDIVIDUAL LEAVES ===
        for (int j = 0; j < num_leaves; j++) {
            TreeLeaf leaf;
            
            // === LEAF POSITIONING ===
            // Position leaves randomly around the branch end point
            // Creates a natural cluster effect at branch tips
            leaf.position = branch.end + glm::vec3(
                angle_dist(gen) * 0.4f,  
                angle_dist(gen) * 0.3f, 
                angle_dist(gen) * 0.4f   
            );
            
            // === LEAF ORIENTATION ===
            // Generate a somewhat upward-facing normal for realistic leaf orientation
            glm::vec3 random_dir = glm::vec3(
                angle_dist(gen) * 0.5f,                    // Some X tilt
                0.8f + std::abs(angle_dist(gen)) * 0.2f,   // Mostly upward (0.8-1.0)
                angle_dist(gen) * 0.5f                     // Some Z tilt
            );
            leaf.normal = glm::normalize(random_dir);
            
            // === LEAF PROPERTIES ===
            leaf.size = size_dist(gen);              // Random size within range
            leaf.growth_progress = 0.0f;             // Starts invisible, grows over time
            leaf.parent_branch_index = branch_index; // Link to this branch
            leaf.spawn_delay = delay_dist(gen);      // Random delay for gradual appearance
            
            // === ADD TO LEAF COLLECTION ===
            leaves.push_back(leaf);
        }
    }
}

/**
 * updateGrowth - Advances the tree growth animation over time
 * 
 * This function implements the temporal aspect of tree generation. Rather than
 * showing the complete tree immediately, it gradually reveals branches and leaves
 * over time, creating a realistic growth animation.
 * 
 * === GROWTH ANIMATION STRATEGY ===
 * 1. Generations appear sequentially (trunk first, then branches, then smaller branches)
 * 2. Within each generation, children wait for parents to be partially grown
 * 3. Leaves appear after their parent branches are established
 * 4. Individual elements have staggered timing for organic appearance
 * 
 * === TIMING BREAKDOWN ===
 * - Generation delay: 15% of total time between generations
 * - Growth duration: 40% of total time for each branch to fully grow
 * - Parent dependency: Child branches wait for parent to reach 60% growth
 * - Leaf timing: Appear when parent branch is 40% grown + individual delay
 * 
 * @param delta_time Time elapsed since last update (in seconds)
 */
void Tree::updateGrowth(float delta_time) {
    // === ADVANCE GLOBAL TIMER ===
    // Accumulate total time elapsed since growth started
    current_growth_time += delta_time;
    
    // === DEBUG OUTPUT (PERIODIC) ===
    // Print growth statistics every 2 seconds for monitoring
    static float lastDebugTime = 0.0f;
    if (current_growth_time - lastDebugTime > 2.0f) {
        std::cout << "Growth time: " << current_growth_time << "s / " << max_growth_time << "s" << std::endl;
        
        // Count visible vs total branches by generation
        std::vector<int> visible_by_gen(max_generations + 1, 0);
        std::vector<int> total_by_gen(max_generations + 1, 0);
        
        for (const auto& branch : branches) {
            if (branch.generation <= max_generations) {
                total_by_gen[branch.generation]++;
                if (branch.growth_progress > 0.0f) {
                    visible_by_gen[branch.generation]++;
                }
            }
        }
        
        // Display visibility statistics
        std::cout << "Visibility by generation:" << std::endl;
        for (int g = 0; g <= max_generations; g++) {
            std::cout << "  Gen " << g << ": " << visible_by_gen[g] << "/" << total_by_gen[g];
            if (total_by_gen[g] > 0 && visible_by_gen[g] > 0) {
                // Show first branch progress for this generation
                for (const auto& branch : branches) {
                    if (branch.generation == g && branch.growth_progress > 0.0f) {
                        std::cout << " (progress: " << (branch.growth_progress * 100.0f) << "%)";
                        break;
                    }
                }
            }
            std::cout << std::endl;
        }
        std::cout.flush();  // Force output to appear immediately
        lastDebugTime = current_growth_time;
    }
    
    // === BRANCH GROWTH ANIMATION ===
    // Update each branch's growth progress based on timing rules
    for (auto& branch : branches) {
        // === TIMING CALCULATIONS ===
        // Calculate when this generation should start and how long growth takes
        float generation_delay = max_growth_time * 0.15f;  // 15% of total time between generations
        float growth_duration = max_growth_time * 0.4f;    // 40% of total time for each branch to grow
        float start_time = branch.generation * generation_delay;
        
        // Only update if enough time has passed for this generation
        if (current_growth_time > start_time) {
            if (branch.generation == 0) {
                // === TRUNK GROWTH (GENERATION 0) ===
                // Trunk grows independently, starting immediately
                // It serves as the foundation for all other branches
                float elapsed = current_growth_time - start_time;
                branch.growth_progress = std::min(1.0f, elapsed / growth_duration);
            } else {
                // === CHILD BRANCH GROWTH (GENERATION 1+) ===
                // Child branches must wait for their parent to be partially grown
                // This creates the realistic effect of branches sprouting from existing ones
                if (branch.parent_index >= 0 && branch.parent_index < static_cast<int>(branches.size())) {
                    float parent_progress = branches[branch.parent_index].growth_progress;
                    
                    // Wait until parent is 60% grown before starting to grow
                    if (parent_progress > 0.6f) {
                        // Calculate when parent reached 60% growth
                        float parent_60_percent_time = 0.6f * growth_duration + 
                                                     (branches[branch.parent_index].generation * generation_delay);
                        
                        // Start growing either at scheduled time or when parent is ready, whichever is later
                        float actual_start_time = std::max(start_time, parent_60_percent_time);
                        float elapsed = current_growth_time - actual_start_time;
                        branch.growth_progress = std::min(1.0f, elapsed / growth_duration);
                    }
                }
            }
        }
    }
    
    // === LEAF GROWTH ANIMATION ===
    // Update each leaf's growth progress, which depends on parent branch state
    for (auto& leaf : leaves) {
        // Verify parent branch exists and is valid
        if (leaf.parent_branch_index >= 0 && leaf.parent_branch_index < static_cast<int>(branches.size())) {
            float parent_progress = branches[leaf.parent_branch_index].growth_progress;
            
            // === LEAF TIMING STRATEGY ===
            // Leaves start appearing when their parent branch is 40% grown
            // This creates the natural effect of leaves budding on established branches
            if (parent_progress > 0.4f) {
                // === LEAF TIMING CALCULATIONS ===
                float generation_delay = max_growth_time * 0.15f;     // Match branch timing
                float leaf_start_offset = max_growth_time * 0.1f;     // 10% of total time after branch starts
                float leaf_growth_duration = max_growth_time * 0.1f;  // 10% of total time for leaf to grow
                
                // Calculate base start time for this leaf's parent branch generation
                float base_start_time = branches[leaf.parent_branch_index].generation * generation_delay + leaf_start_offset;
                
                // Add individual spawn delay for staggered leaf appearance
                float start_time = base_start_time + leaf.spawn_delay;
                
                // === LEAF GROWTH PROGRESS ===
                // Only start growing if enough time has passed
                if (current_growth_time > start_time) {
                    float elapsed = current_growth_time - start_time;
                    leaf.growth_progress = std::min(1.0f, elapsed / leaf_growth_duration);
                }
            }
        }
    }
    
    // === MESH UPDATE ===
    // Regenerate the vertex data for rendering based on current growth state
    // This must be called every frame to reflect animation changes
    updateBranchMesh();  // Update branch geometry based on growth_progress
    updateLeafMesh();    // Update leaf geometry based on growth_progress
}

/**
 * calculateAbsoluteBranchStart - Computes world position where a branch begins
 * 
 * In the tree data structure, branch positions are stored relative to their
 * parent branches. This function traverses the parent hierarchy to compute
 * the absolute world position where a branch should start growing.
 * 
 * === POSITION CALCULATION STRATEGY ===
 * - Trunk (parent_index = -1): Uses stored start position (usually origin)
 * - Child branches: Start at the current end position of their parent
 * - The "current end" depends on parent's growth_progress for animation
 * 
 * @param branch_index Index of the branch to calculate start position for
 * @return Absolute world position where this branch starts
 */
glm::vec3 Tree::calculateAbsoluteBranchStart(int branch_index) {
    // === BOUNDS CHECKING ===
    // Ensure branch index is valid to prevent crashes
    if (branch_index < 0 || branch_index >= static_cast<int>(branches.size())) {
        return glm::vec3(0.0f);  // Return origin for invalid indices
    }
    
    const auto& branch = branches[branch_index];
    
    // === ROOT CASE: TRUNK ===
    // Trunk has no parent, so use its stored start position
    if (branch.parent_index == -1) {
        return branch.start;  // Usually (0,0,0) - base of tree
    }
    
    // === CHILD CASE: DEPENDS ON PARENT ===
    // Child branches start where their parent currently ends
    // This creates the connected, hierarchical tree structure
    glm::vec3 parent_end = calculateAbsoluteBranchEnd(branch.parent_index);
    return parent_end;
}

/**
 * calculateAbsoluteBranchEnd - Computes world position where a branch currently ends
 * 
 * This function calculates where a branch's tip is located, taking into account
 * the growth animation. As branches grow, their end position moves from start
 * toward the final end position based on growth_progress.
 * 
 * === ANIMATION INTEGRATION ===
 * - growth_progress = 0.0: Branch end is at start position (invisible)
 * - growth_progress = 0.5: Branch end is halfway to final position
 * - growth_progress = 1.0: Branch end is at final position (fully grown)
 * 
 * This creates the effect of branches extending outward during growth animation.
 * 
 * @param branch_index Index of the branch to calculate end position for
 * @return Current absolute world position where this branch ends
 */
glm::vec3 Tree::calculateAbsoluteBranchEnd(int branch_index) {
    // === BOUNDS CHECKING ===
    if (branch_index < 0 || branch_index >= static_cast<int>(branches.size())) {
        return glm::vec3(0.0f);
    }
    
    const auto& branch = branches[branch_index];
    
    // === GET ABSOLUTE START POSITION ===
    // Find where this branch starts in world coordinates
    glm::vec3 absolute_start = calculateAbsoluteBranchStart(branch_index);
    
    // === CALCULATE GROWTH-ADJUSTED END POSITION ===
    // The local direction vector represents the full branch when grown
    glm::vec3 local_direction = branch.end - branch.start;
    
    // Scale direction by growth progress for animation effect
    // growth_progress = 0.0 -> no extension (branch hasn't started growing)
    // growth_progress = 1.0 -> full extension (branch is fully grown)
    glm::vec3 current_direction = local_direction * branch.growth_progress;
    
    // === RETURN CURRENT END POSITION ===
    return absolute_start + current_direction;
}

/**
 * updateBranchMesh - Generates OpenGL vertex data for all visible branches
 * 
 * This function converts the abstract branch data structures into the specific
 * vertex arrays needed for OpenGL rendering. It only includes branches that
 * have started growing (growth_progress > 0) to implement the growth animation.
 * 
 * === MESH GENERATION PROCESS ===
 * 1. Clear previous vertex data
 * 2. For each branch with growth_progress > 0:
 *    - Calculate current start/end positions (accounting for animation)
 *    - Generate cylindrical geometry with proper radius tapering
 *    - Add vertex data to branch_vertices array
 * 
 * === VERTEX DATA FORMAT ===
 * Each vertex: [x, y, z, w, u, v, nx, ny, nz] (9 floats total)
 * - Position (x,y,z,w): 3D coordinates + homogeneous coordinate
 * - Texture (u,v): Texture mapping coordinates
 * - Normal (nx,ny,nz): Surface normal for lighting calculations
 */
void Tree::updateBranchMesh() {
    // === CLEAR PREVIOUS MESH DATA ===
    branch_vertices.clear();
    
    // === GENERATE GEOMETRY FOR EACH VISIBLE BRANCH ===
    for (int i = 0; i < static_cast<int>(branches.size()); i++) {
        const auto& branch = branches[i];
        
        // Only render branches that have started growing
        if (branch.growth_progress > 0.0f) {
            // === CALCULATE CURRENT BRANCH ENDPOINTS ===
            // Get animated positions that change as branch grows
            glm::vec3 start = calculateAbsoluteBranchStart(i);
            glm::vec3 end = calculateAbsoluteBranchEnd(i);
            
            // === CALCULATE BRANCH RADII ===
            // Branches taper from thicker at base to thinner at tip
            float start_radius = branch.radius;           // Full radius at base
            float end_radius = branch.radius * 0.7f;      // 70% radius at tip
            
            // === GENERATE CYLINDRICAL GEOMETRY ===
            // Create a cylinder segment representing this branch
            addBranchSegment(start, end, start_radius, end_radius);
        }
    }
}

/**
 * updateLeafMesh - Generates OpenGL vertex data for all visible leaves
 * 
 * This function creates the geometric representation of leaves as textured quads.
 * Like branches, only leaves with growth_progress > 0 are included, and their
 * size scales with growth progress to create a budding animation effect.
 * 
 * === LEAF POSITIONING STRATEGY ===
 * Leaves are positioned relative to their parent branch's current end position.
 * As the parent branch grows during animation, the leaves move with it, creating
 * the realistic effect of leaves appearing at the tips of growing branches.
 * 
 * === LEAF ANIMATION ===
 * - growth_progress = 0.0: Leaf is invisible (not rendered)
 * - growth_progress = 0.5: Leaf is half-size (budding)
 * - growth_progress = 1.0: Leaf is full-size (mature)
 */
void Tree::updateLeafMesh() {
    // === CLEAR PREVIOUS LEAF MESH DATA ===
    leaf_vertices.clear();
    
    // === GENERATE GEOMETRY FOR EACH VISIBLE LEAF ===
    for (const auto& leaf : leaves) {
        // Only process leaves that have started growing and have valid parent
        if (leaf.growth_progress > 0.0f && 
            leaf.parent_branch_index >= 0 && 
            leaf.parent_branch_index < static_cast<int>(branches.size())) {
            
            const auto& parent_branch = branches[leaf.parent_branch_index];
            
            // === CALCULATE LEAF POSITION ===
            // Leaves maintain their relative offset from their parent branch end
            // This way they move naturally as the parent branch grows
            glm::vec3 original_leaf_offset = leaf.position - parent_branch.end;
            glm::vec3 parent_absolute_end = calculateAbsoluteBranchEnd(leaf.parent_branch_index);
            glm::vec3 pos = parent_absolute_end + original_leaf_offset;
            
            // === CALCULATE ANIMATED LEAF SIZE ===
            // Scale leaf size by growth progress for budding animation
            float dynamic_size = leaf.size * leaf.growth_progress;
            
            // === GENERATE LEAF QUAD GEOMETRY ===
            // Create a billboard quad that faces a specific direction
            addLeafQuad(pos, leaf.normal, dynamic_size, leaf.growth_progress);
        }
    }
}

/**
 * addBranchSegment - Creates cylindrical geometry for a branch segment
 * 
 * This function generates the 3D mesh data for a single branch represented
 * as a tapered cylinder. The cylinder is created by generating rings of
 * vertices at the start and end positions, then connecting them with triangles.
 * 
 * === CYLINDER GENERATION ALGORITHM ===
 * 1. Calculate coordinate system (direction, right, up vectors)
 * 2. Generate ring of vertices at start position with start_radius
 * 3. Generate ring of vertices at end position with end_radius
 * 4. Connect corresponding vertices between rings to form triangle strips
 * 5. Calculate proper normals for lighting
 * 6. Assign texture coordinates for bark texture mapping
 * 
 * === COORDINATE SYSTEM ===
 * - direction: Vector from start to end (branch growth direction)
 * - right: Perpendicular to direction (for ring generation)
 * - up: Perpendicular to both direction and right (completes coordinate system)
 * 
 * @param start Starting position of branch segment
 * @param end Ending position of branch segment  
 * @param start_radius Radius at the start (base) of segment
 * @param end_radius Radius at the end (tip) of segment
 */
void Tree::addBranchSegment(glm::vec3 start, glm::vec3 end, float start_radius, float end_radius) {
    // === GEOMETRIC SETUP ===
    const int segments = 8;  // Number of sides around cylinder circumference
    
    // === COORDINATE SYSTEM CALCULATION ===
    // Calculate local coordinate system for cylinder generation
    glm::vec3 direction = glm::normalize(end - start);  // Branch growth direction
    glm::vec3 up = glm::vec3(0, 1, 0);                  // Default up vector
    
    // Handle near-vertical branches to avoid parallel vectors
    if (abs(glm::dot(direction, up)) > 0.9f) {
        up = glm::vec3(1, 0, 0);  // Use X-axis if branch is nearly vertical
    }
    
    // Complete orthogonal coordinate system
    glm::vec3 right = glm::normalize(glm::cross(direction, up));
    up = glm::normalize(glm::cross(right, direction));
    
    // === CYLINDER MESH GENERATION ===
    // Generate triangular mesh by connecting rings at start and end
    for (int i = 0; i < segments; i++) {
        // === ANGULAR CALCULATIONS ===
        // Calculate angles for current and next vertex around circumference
        float angle1 = (float)i / segments * 2.0f * (float)M_PI;
        float angle2 = (float)(i + 1) / segments * 2.0f * (float)M_PI;
        
        // === TEXTURE COORDINATES ===
        // Map cylinder surface to rectangular texture
        float u1 = (float)i / segments;        // Texture U wraps around circumference
        float u2 = (float)(i + 1) / segments;  
        // Texture V goes from base (0) to tip (1) along branch length
        
        // === VERTEX POSITION CALCULATION ===
        // Calculate positions of 4 vertices forming a quad on cylinder surface
        glm::vec3 p1 = start + right * cosf(angle1) * start_radius + up * sinf(angle1) * start_radius;
        glm::vec3 p2 = start + right * cosf(angle2) * start_radius + up * sinf(angle2) * start_radius;
        glm::vec3 p3 = end + right * cosf(angle1) * end_radius + up * sinf(angle1) * end_radius;
        glm::vec3 p4 = end + right * cosf(angle2) * end_radius + up * sinf(angle2) * end_radius;
        
        // === NORMAL VECTOR CALCULATION ===
        // Calculate surface normals for proper lighting
        // Normals point outward from cylinder axis
        glm::vec3 n1 = glm::normalize((p1 - start) * glm::vec3(1,0,1));  // Ignore Y for radial normal
        glm::vec3 n2 = glm::normalize((p2 - start) * glm::vec3(1,0,1));
        
        // === TEXTURE COORDINATE ASSIGNMENT ===
        glm::vec2 t1(u1, 0.0f);  // Base of cylinder
        glm::vec2 t2(u2, 0.0f);  
        glm::vec2 t3(u1, 1.0f);  // Tip of cylinder
        glm::vec2 t4(u2, 1.0f);
        
        // === TRIANGLE GENERATION ===
        // Each quad becomes 2 triangles: (p1,p2,p3) and (p2,p4,p3)
        
        // Triangle 1: p1, p2, p3
        branch_vertices.insert(branch_vertices.end(), {p1.x, p1.y, p1.z, 1.0f, t1.x, t1.y, n1.x, n1.y, n1.z});
        branch_vertices.insert(branch_vertices.end(), {p2.x, p2.y, p2.z, 1.0f, t2.x, t2.y, n2.x, n2.y, n2.z});
        branch_vertices.insert(branch_vertices.end(), {p3.x, p3.y, p3.z, 1.0f, t3.x, t3.y, n1.x, n1.y, n1.z});
        
        // Triangle 2: p2, p4, p3  
        branch_vertices.insert(branch_vertices.end(), {p2.x, p2.y, p2.z, 1.0f, t2.x, t2.y, n2.x, n2.y, n2.z});
        branch_vertices.insert(branch_vertices.end(), {p4.x, p4.y, p4.z, 1.0f, t4.x, t4.y, n2.x, n2.y, n2.z});
        branch_vertices.insert(branch_vertices.end(), {p3.x, p3.y, p3.z, 1.0f, t3.x, t3.y, n1.x, n1.y, n1.z});
    }
}

/**
 * addLeafQuad - Creates billboard quad geometry for a single leaf
 * 
 * This function generates a textured quad to represent a leaf. The quad is
 * oriented according to the leaf's normal vector and scaled by the growth
 * parameter for animation. Each leaf is rendered as a double-sided billboard
 * so it's visible from both front and back.
 * 
 * === LEAF REPRESENTATION STRATEGY ===
 * Leaves are modeled as flat quads rather than complex 3D geometry for
 * performance reasons. The illusion of volume is created through:
 * - Proper texture with alpha channel for leaf shape
 * - Random orientations for natural variation
 * - Double-sided rendering for visibility from all angles
 * 
 * === COORDINATE SYSTEM GENERATION ===
 * From the leaf normal, we construct a local coordinate system:
 * - normal: Direction leaf is facing (toward light)
 * - right: Horizontal direction across leaf width  
 * - up: Vertical direction across leaf height
 * 
 * @param position World position of leaf center
 * @param normal Direction the leaf is facing (for lighting)
 * @param size Full size of the leaf when mature
 * @param growth Growth factor (0.0 = invisible, 1.0 = full size)
 */
void Tree::addLeafQuad(glm::vec3 position, glm::vec3 normal, float size, float growth) {
    // === SIZE ANIMATION ===
    // Apply minimum size to prevent leaves from completely disappearing
    float min_size = 0.05f;
    size = min_size + (size - min_size) * growth;
    
    // === LOCAL COORDINATE SYSTEM GENERATION ===
    // Create orthogonal coordinate system for quad orientation
    
    // Calculate right vector (horizontal across leaf)
    glm::vec3 right = glm::normalize(glm::cross(normal, glm::vec3(0,1,0)));
    if (glm::length(right) < 0.01f) {
        // Handle case where normal is parallel to Y-axis
        right = glm::vec3(1,0,0);
    }
    
    // Calculate up vector (vertical across leaf) 
    glm::vec3 up = glm::normalize(glm::cross(right, normal));
    
    // Scale vectors by half-size for quad corner calculation
    right *= size * 0.5f;
    up *= size * 0.5f;
    
    // === QUAD CORNER CALCULATION ===
    // Generate 4 corners of rectangular leaf quad
    glm::vec3 v1 = position - right - up;  // Bottom-left
    glm::vec3 v2 = position + right - up;  // Bottom-right  
    glm::vec3 v3 = position + right + up;  // Top-right
    glm::vec3 v4 = position - right + up;  // Top-left
    
    // === TEXTURE COORDINATES ===
    // Map leaf texture to quad corners
    glm::vec2 t1(0.0f, 0.0f);  // Bottom-left of texture
    glm::vec2 t2(1.0f, 0.0f);  // Bottom-right of texture
    glm::vec2 t3(1.0f, 1.0f);  // Top-right of texture
    glm::vec2 t4(0.0f, 1.0f);  // Top-left of texture
    
    // === FRONT FACE TRIANGLES ===
    // Generate front-facing side of leaf (normal direction)
    // Triangle 1: v1, v2, v3
    leaf_vertices.insert(leaf_vertices.end(), {v1.x, v1.y, v1.z, 1.0f, t1.x, t1.y, normal.x, normal.y, normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v2.x, v2.y, v2.z, 1.0f, t2.x, t2.y, normal.x, normal.y, normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v3.x, v3.y, v3.z, 1.0f, t3.x, t3.y, normal.x, normal.y, normal.z});
    
    // Triangle 2: v1, v3, v4
    leaf_vertices.insert(leaf_vertices.end(), {v1.x, v1.y, v1.z, 1.0f, t1.x, t1.y, normal.x, normal.y, normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v3.x, v3.y, v3.z, 1.0f, t3.x, t3.y, normal.x, normal.y, normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v4.x, v4.y, v4.z, 1.0f, t4.x, t4.y, normal.x, normal.y, normal.z});
    
    // === BACK FACE TRIANGLES ===
    // Generate back-facing side of leaf (-normal direction)
    // Vertices in reverse order to maintain proper winding
    glm::vec3 back_normal = -normal;
    
    // Triangle 1: v1, v3, v2 (reversed winding)
    leaf_vertices.insert(leaf_vertices.end(), {v1.x, v1.y, v1.z, 1.0f, t1.x, t1.y, back_normal.x, back_normal.y, back_normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v3.x, v3.y, v3.z, 1.0f, t3.x, t3.y, back_normal.x, back_normal.y, back_normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v2.x, v2.y, v2.z, 1.0f, t2.x, t2.y, back_normal.x, back_normal.y, back_normal.z});
    
    // Triangle 2: v1, v4, v3 (reversed winding)
    leaf_vertices.insert(leaf_vertices.end(), {v1.x, v1.y, v1.z, 1.0f, t1.x, t1.y, back_normal.x, back_normal.y, back_normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v4.x, v4.y, v4.z, 1.0f, t4.x, t4.y, back_normal.x, back_normal.y, back_normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v3.x, v3.y, v3.z, 1.0f, t3.x, t3.y, back_normal.x, back_normal.y, back_normal.z});
}
