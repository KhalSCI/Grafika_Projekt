#include "tree_simple.h"
#include <random>
#include <cmath>
#include <iostream>
#include <algorithm>

Tree::Tree() {
    max_growth_time = 5.0f;  // 120 seconds total growth time
    current_growth_time = 0.0f;
    max_generations = 4;
    branch_angle_variance = 45.0f;
    length_reduction_factor = 0.75f;  // Less reduction to keep branches more visible
    radius_reduction_factor = 0.85f;  // Less reduction to keep branches thicker
}

Tree::~Tree() {
    // Cleanup if needed
}

void Tree::generate() {
    branches.clear();
    leaves.clear();
    branch_vertices.clear();
    leaf_vertices.clear();
    
    current_growth_time = 0.0f;
    
    // Generate trunk
    glm::vec3 trunk_start(0.0f, 0.0f, 0.0f);
    glm::vec3 trunk_direction(0.0f, 1.0f, 0.0f);
    generateBranch(-1, trunk_start, trunk_direction, 3.0f, 0.2f, 0);
    
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

void Tree::generateBranch(int parent_index, glm::vec3 start, glm::vec3 direction, 
                         float length, float radius, int generation) {
    TreeBranch branch;
    branch.start = start;
    branch.end = start + direction * length;
    branch.radius = radius;
    branch.generation = generation;
    branch.growth_progress = 0.0f;
    branch.parent_index = parent_index;
    
    int branch_index = branches.size();
    branches.push_back(branch);
    
    // Add this branch as a child to its parent
    if (parent_index >= 0) {
        branches[parent_index].children.push_back(branch_index);
    }
    
    // Generate child branches
    if (generation < max_generations) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> angle_dist(-1.0f, 1.0f);
        std::uniform_int_distribution<int> child_count_dist(2, 4);
        
        int num_children = child_count_dist(gen);
        
        for (int i = 0; i < num_children; i++) {
            // Create child branch direction
            float angle = (float)i / num_children * 2.0f * (float)M_PI + angle_dist(gen) * branch_angle_variance * (float)M_PI / 180.0f;
            float elevation = 30.0f + angle_dist(gen) * 20.0f; // Degrees
            
            glm::vec3 child_direction;
            child_direction.x = sinf(angle) * cosf(elevation * (float)M_PI / 180.0f);
            child_direction.y = sinf(elevation * (float)M_PI / 180.0f);
            child_direction.z = cosf(angle) * cosf(elevation * (float)M_PI / 180.0f);
            child_direction = glm::normalize(child_direction);
            
            float child_length = length * length_reduction_factor;
            float child_radius = radius * radius_reduction_factor;
            
            generateBranch(branch_index, branch.end, child_direction, child_length, child_radius, generation + 1);
        }
    }
    
    // Generate leaves for branches of generation 2 and higher
    if (generation >= 2) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> angle_dist(-1.0f, 1.0f);
        std::uniform_int_distribution<int> leaf_count_dist(6, 14); // Increase number of leaves
        std::uniform_real_distribution<float> size_dist(0.28f, 0.45f); // Increase leaf size
        std::uniform_real_distribution<float> delay_dist(0.0f, 4.0f); // Random delay for gradual appearance
        
        int num_leaves = leaf_count_dist(gen);
        
        for (int j = 0; j < num_leaves; j++) {
            TreeLeaf leaf;
            leaf.position = branch.end + glm::vec3(
                angle_dist(gen) * 0.4f,
                angle_dist(gen) * 0.3f,
                angle_dist(gen) * 0.4f
            );
            
            // Generate random normal
            glm::vec3 random_dir = glm::vec3(
                angle_dist(gen) * 0.5f,
                0.8f + std::abs(angle_dist(gen)) * 0.2f,
                angle_dist(gen) * 0.5f
            );
            leaf.normal = glm::normalize(random_dir);
            leaf.size = size_dist(gen);
            leaf.growth_progress = 0.0f;
            leaf.parent_branch_index = branch_index;
            leaf.spawn_delay = delay_dist(gen); // Individual spawn delay
            
            leaves.push_back(leaf);
        }
    }
}

void Tree::updateGrowth(float delta_time) {
    current_growth_time += delta_time;
    
    // Debug: Print timing info frequently
    static float lastDebugTime = 0.0f;
    if (current_growth_time - lastDebugTime > 2.0f) {
        std::cout << "Growth time: " << current_growth_time << "s / " << max_growth_time << "s" << std::endl;
        
        // Count visible branches by generation
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
    
    // Update branch growth
    for (auto& branch : branches) {
        // Use max_growth_time to scale timing - more compressed to fit all generations
        float generation_delay = max_growth_time * 0.15f;  // 15% of total time between generations (18s)
        float growth_duration = max_growth_time * 0.4f;    // 40% of total time for each branch to grow (48s)
        float start_time = branch.generation * generation_delay;
        
        if (current_growth_time > start_time) {
            if (branch.generation == 0) {
                // Trunk grows over the full duration
                float elapsed = current_growth_time - start_time;
                branch.growth_progress = std::min(1.0f, elapsed / growth_duration);
            } else {
                // Child branches wait for parent to be 60% grown, then grow over duration
                if (branch.parent_index >= 0 && branch.parent_index < static_cast<int>(branches.size())) {
                    float parent_progress = branches[branch.parent_index].growth_progress;
                    if (parent_progress > 0.6f) {
                        // Calculate elapsed time from when the branch actually starts growing
                        // Find when parent reached 60% growth
                        float parent_60_percent_time = 0.6f * growth_duration + (branches[branch.parent_index].generation * generation_delay);
                        float actual_start_time = std::max(start_time, parent_60_percent_time);
                        float elapsed = current_growth_time - actual_start_time;
                        branch.growth_progress = std::min(1.0f, elapsed / growth_duration);
                    }
                }
            }
        }
    }
    
    // Update leaf growth
    for (auto& leaf : leaves) {
        if (leaf.parent_branch_index >= 0 && leaf.parent_branch_index < static_cast<int>(branches.size())) {
            float parent_progress = branches[leaf.parent_branch_index].growth_progress;
            if (parent_progress > 0.4f) {
                float generation_delay = max_growth_time * 0.15f;  // Match branch timing
                float leaf_start_offset = max_growth_time * 0.1f;  // 10% of total time after branch starts
                float leaf_growth_duration = max_growth_time * 0.1f;  // 10% of total time for leaf growth
                
                float base_start_time = branches[leaf.parent_branch_index].generation * generation_delay + leaf_start_offset;
                float start_time = base_start_time + leaf.spawn_delay;  // Individual spawn delay
                
                if (current_growth_time > start_time) {
                    float elapsed = current_growth_time - start_time;
                    leaf.growth_progress = std::min(1.0f, elapsed / leaf_growth_duration);
                }
            }
        }
    }
    
    // Update mesh data
    updateBranchMesh();
    updateLeafMesh();
}

glm::vec3 Tree::calculateAbsoluteBranchStart(int branch_index) {
    if (branch_index < 0 || branch_index >= static_cast<int>(branches.size())) {
        return glm::vec3(0.0f);
    }
    
    const auto& branch = branches[branch_index];
    
    if (branch.parent_index == -1) {
        // This is the trunk
        return branch.start;
    }
    
    // Get parent's absolute end position
    glm::vec3 parent_end = calculateAbsoluteBranchEnd(branch.parent_index);
    return parent_end;
}

glm::vec3 Tree::calculateAbsoluteBranchEnd(int branch_index) {
    if (branch_index < 0 || branch_index >= static_cast<int>(branches.size())) {
        return glm::vec3(0.0f);
    }
    
    const auto& branch = branches[branch_index];
    glm::vec3 absolute_start = calculateAbsoluteBranchStart(branch_index);
    
    // Calculate the direction from local start to local end
    glm::vec3 local_direction = branch.end - branch.start;
    
    // Apply growth progress - this makes branches grow from start to end
    glm::vec3 current_direction = local_direction * branch.growth_progress;
    
    return absolute_start + current_direction;
}

void Tree::updateBranchMesh() {
    branch_vertices.clear();
    
    for (int i = 0; i < static_cast<int>(branches.size()); i++) {
        const auto& branch = branches[i];
        
        if (branch.growth_progress > 0.0f) {
            glm::vec3 start = calculateAbsoluteBranchStart(i);
            glm::vec3 end = calculateAbsoluteBranchEnd(i);
            
            // Use actual branch radius without artificial minimum
            float start_radius = branch.radius;
            float end_radius = branch.radius * 0.7f;
            
            addBranchSegment(start, end, start_radius, end_radius);
        }
    }
}

void Tree::updateLeafMesh() {
    leaf_vertices.clear();
    for (const auto& leaf : leaves) {
        if (leaf.growth_progress > 0.0f && leaf.parent_branch_index >= 0 && 
            leaf.parent_branch_index < static_cast<int>(branches.size())) {
            const auto& parent_branch = branches[leaf.parent_branch_index];
            // Calculate the original offset from branch end
            glm::vec3 original_leaf_offset = leaf.position - parent_branch.end;
            // Get current end position of parent branch
            glm::vec3 parent_absolute_end = calculateAbsoluteBranchEnd(leaf.parent_branch_index);
            // Position leaf relative to current branch end
            glm::vec3 pos = parent_absolute_end + original_leaf_offset;
            float dynamic_size = leaf.size * leaf.growth_progress;
            addLeafQuad(pos, leaf.normal, dynamic_size, leaf.growth_progress);
        }
    }
}

void Tree::addBranchSegment(glm::vec3 start, glm::vec3 end, float start_radius, float end_radius) {
    const int segments = 8;
    glm::vec3 direction = glm::normalize(end - start);
    glm::vec3 up = glm::vec3(0, 1, 0);
    if (abs(glm::dot(direction, up)) > 0.9f) {
        up = glm::vec3(1, 0, 0);
    }
    glm::vec3 right = glm::normalize(glm::cross(direction, up));
    up = glm::normalize(glm::cross(right, direction));
    for (int i = 0; i < segments; i++) {
        float angle1 = (float)i / segments * 2.0f * (float)M_PI;
        float angle2 = (float)(i + 1) / segments * 2.0f * (float)M_PI;
        float u1 = (float)i / segments;
        float u2 = (float)(i + 1) / segments;
        // Compute ring points
        glm::vec3 p1 = start + right * cosf(angle1) * start_radius + up * sinf(angle1) * start_radius;
        glm::vec3 p2 = start + right * cosf(angle2) * start_radius + up * sinf(angle2) * start_radius;
        glm::vec3 p3 = end + right * cosf(angle1) * end_radius + up * sinf(angle1) * end_radius;
        glm::vec3 p4 = end + right * cosf(angle2) * end_radius + up * sinf(angle2) * end_radius;
        // Correct normals: perpendicular to the surface, ignore direction (use only right/up plane)
        glm::vec3 n1 = glm::normalize((p1 - start) * glm::vec3(1,0,1));
        glm::vec3 n2 = glm::normalize((p2 - start) * glm::vec3(1,0,1));
        // Texture coordinates
        glm::vec2 t1(u1, 0.0f);
        glm::vec2 t2(u2, 0.0f);
        glm::vec2 t3(u1, 1.0f);
        glm::vec2 t4(u2, 1.0f);
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

void Tree::addLeafQuad(glm::vec3 position, glm::vec3 normal, float size, float growth) {
    float min_size = 0.05f;
    size = min_size + (size - min_size) * growth;
    // Compute quad corners
    glm::vec3 right = glm::normalize(glm::cross(normal, glm::vec3(0,1,0)));
    if (glm::length(right) < 0.01f) right = glm::vec3(1,0,0);
    glm::vec3 up = glm::normalize(glm::cross(right, normal));
    right *= size * 0.5f;
    up *= size * 0.5f;
    glm::vec3 v1 = position - right - up;
    glm::vec3 v2 = position + right - up;
    glm::vec3 v3 = position + right + up;
    glm::vec3 v4 = position - right + up;
    glm::vec2 t1(0.0f, 0.0f);
    glm::vec2 t2(1.0f, 0.0f);
    glm::vec2 t3(1.0f, 1.0f);
    glm::vec2 t4(0.0f, 1.0f);
    // All corners share the same normal (front face)
    leaf_vertices.insert(leaf_vertices.end(), {v1.x, v1.y, v1.z, 1.0f, t1.x, t1.y, normal.x, normal.y, normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v2.x, v2.y, v2.z, 1.0f, t2.x, t2.y, normal.x, normal.y, normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v3.x, v3.y, v3.z, 1.0f, t3.x, t3.y, normal.x, normal.y, normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v1.x, v1.y, v1.z, 1.0f, t1.x, t1.y, normal.x, normal.y, normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v3.x, v3.y, v3.z, 1.0f, t3.x, t3.y, normal.x, normal.y, normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v4.x, v4.y, v4.z, 1.0f, t4.x, t4.y, normal.x, normal.y, normal.z});
    // Back face (-normal)
    glm::vec3 back_normal = -normal;
    leaf_vertices.insert(leaf_vertices.end(), {v1.x, v1.y, v1.z, 1.0f, t1.x, t1.y, back_normal.x, back_normal.y, back_normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v3.x, v3.y, v3.z, 1.0f, t3.x, t3.y, back_normal.x, back_normal.y, back_normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v2.x, v2.y, v2.z, 1.0f, t2.x, t2.y, back_normal.x, back_normal.y, back_normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v1.x, v1.y, v1.z, 1.0f, t1.x, t1.y, back_normal.x, back_normal.y, back_normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v4.x, v4.y, v4.z, 1.0f, t4.x, t4.y, back_normal.x, back_normal.y, back_normal.z});
    leaf_vertices.insert(leaf_vertices.end(), {v3.x, v3.y, v3.z, 1.0f, t3.x, t3.y, back_normal.x, back_normal.y, back_normal.z});
}
