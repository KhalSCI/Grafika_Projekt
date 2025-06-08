#include "tree_simple.h"
#include <iostream>
#include <iomanip>

int main() {
    Tree tree;
    tree.generate();
    
    std::cout << "=== Tree Growth Test ===" << std::endl;
    std::cout << "Total branches: " << tree.getBranchCount() << std::endl;
    std::cout << "Total leaves: " << tree.getLeafCount() << std::endl;
    std::cout << std::endl;
    
    // Simulate growth over time
    for (float time = 0; time <= 120; time += 6.0f) {
        tree.updateGrowth(6.0f); // 6 second increments
        
        std::cout << std::fixed << std::setprecision(1);
        std::cout << "Time: " << time << "s (" << (tree.getGrowthProgress() * 100.0f) << "% complete)" << std::endl;
        
        int branch_vertex_count = tree.getBranchVertexCount();
        int leaf_vertex_count = tree.getLeafVertexCount();
        
        // Estimate number of visible branches (each branch uses approximately 24 vertices for cylinder)
        int estimated_visible_branches = branch_vertex_count / 24;
        int estimated_visible_leaves = leaf_vertex_count / 4;
        
        std::cout << "  Branch vertices: " << branch_vertex_count << " (~" << estimated_visible_branches << " branches)" << std::endl;
        std::cout << "  Leaf vertices: " << leaf_vertex_count << " (~" << estimated_visible_leaves << " leaves)" << std::endl;
        std::cout << std::endl;
    }
    
    return 0;
}
