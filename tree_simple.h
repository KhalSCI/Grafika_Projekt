#ifndef TREE_H
#define TREE_H

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

struct TreeBranch {
    glm::vec3 start;
    glm::vec3 end;
    float radius;
    int generation;
    float growth_progress; // 0.0 to 1.0
    std::vector<int> children; // indices to child branches
    int parent_index; // index to parent branch (-1 for trunk)
};

struct TreeLeaf {
    glm::vec3 position;
    glm::vec3 normal;
    float size;
    float growth_progress;
    int parent_branch_index; // Index of the branch this leaf belongs to
    float spawn_delay; // Individual delay for gradual appearance
};

class Tree {
private:
    std::vector<TreeBranch> branches;
    std::vector<TreeLeaf> leaves;
    
    // Vertex data for rendering - simplified to just vertices
    std::vector<float> branch_vertices;
    std::vector<float> leaf_vertices;
    
    // Generation parameters
    float max_growth_time;
    float current_growth_time;
    int max_generations;
    float branch_angle_variance;
    float length_reduction_factor;
    float radius_reduction_factor;
    
    // Helper functions for branch positioning
    glm::vec3 calculateAbsoluteBranchStart(int branch_index);
    glm::vec3 calculateAbsoluteBranchEnd(int branch_index);
    
    // Generation methods
    void generateBranch(int parent_index, glm::vec3 start, glm::vec3 direction, 
                       float length, float radius, int generation);
    void updateBranchMesh();
    void updateLeafMesh();
    void addBranchSegment(glm::vec3 start, glm::vec3 end, float start_radius, float end_radius);
    void addLeafQuad(glm::vec3 position, glm::vec3 normal, float size, float growth);

public:
    Tree();
    ~Tree();
    
    void generate();
    void updateGrowth(float delta_time);
    
    // Getters for rendering
    const std::vector<float>& getBranchVertices() const { return branch_vertices; }
    const std::vector<float>& getLeafVertices() const { return leaf_vertices; }
    
    int getBranchVertexCount() const { return branch_vertices.size() / 9; }
    int getLeafVertexCount() const { return leaf_vertices.size() / 9; }
    
    // Tree information
    int getBranchCount() const { return branches.size(); }
    int getLeafCount() const { return leaves.size(); }
    float getGrowthProgress() const { return current_growth_time / max_growth_time; }
};

#endif // TREE_H
