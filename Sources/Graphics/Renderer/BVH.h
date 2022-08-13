#pragma once

#include <entt/entt.hpp>
#include <vector>
#include <glm/glm.hpp>

#include "World/Components.h"

struct BVHLeaf {
    glm::vec3 right;    uint32_t packedSize;
    glm::vec3 up;       int PADDING;
    glm::vec3 forward;  int palleteID;
    glm::vec3 position; int volumeRID;
};

struct BVHNode {
    glm::vec3 min;
    int second = -1; //Points to the second child BVHNode
    glm::vec3 max;
    int leaf = -1; //If different from -1 this is an leaf and contains the entity id, -2 means no first child
};

class BVHBuilder {
public:
    std::vector<BVHNode> nodes;
    std::vector<BVHNode> entities;
    std::vector<BVHLeaf> leafs;

    int biggestAxis(glm::vec3 min, glm::vec3 max) {
        glm::vec3 s = max - min;
        if (s.x > s.y && s.x > s.z) {
            return 0;
        }
        else if (s.y > s.z) {
            return 1;
        }
        else {
            return 2;
        }
    }

    int buildNode(int begin, int end) {
        int nodeIndex = nodes.size();
        
        if (end - begin == 1) { // Leaf
            nodes.push_back(entities[begin]);
        } else {
            nodes.push_back(BVHNode());

            glm::vec3 min(FLT_MAX), max(-FLT_MAX);
            for (int i = begin; i < end; i++) {
                min = glm::min(min, entities[i].min);
                max = glm::max(max, entities[i].max);
            }

            BVHNode& node = nodes.back();
            node.min = min;
            node.max = max;
            node.leaf = -1;

            // Nodes
            int axis = biggestAxis(min, max);

            std::sort(entities.begin() + begin, entities.begin() + end, [&](const BVHNode& a, const BVHNode& b) {
                return (a.max[axis] + a.min[axis]) < (b.max[axis] + b.min[axis]);
            });

            int mid = begin + (end - begin) / 2;
            int nodeA = buildNode(begin, mid);
            int nodeB = buildNode(mid, end);

            node.second = nodeB;
        }

        return nodeIndex;
    }

    void buildFrom(entt::registry& R) {
        nodes.clear();
        entities.clear();
        leafs.clear();

        R.view<Transform, VoxRenderer>().each([&](const entt::entity e, Transform& t, VoxRenderer& v) {
            if (!v.Vox.IsValid() || !v.Pallete.IsValid()) return;

            const glm::vec3 CORNERS[] = {
                {0,0,0},
                {0,0,1},
                {0,1,0},
                {0,1,1},
                {1,0,0},
                {1,0,1},
                {1,1,0},
                {1,1,1},
            };

            Extent extent = GetDesc(v.Vox->GetImage()).extent;
            glm::vec3 size = glm::vec3(extent.width, extent.height, extent.depth) * 0.1f;
            glm::vec3 min = glm::vec3(FLT_MAX);
            glm::vec3 max = glm::vec3(-FLT_MAX);

            glm::vec3 offset = t.WorldMatrix*glm::vec4(-v.Pivot, 0.0f);
            for (int i = 0; i < 8; i++) {
                glm::vec3 c = offset + glm::vec3(t.WorldMatrix * glm::vec4(CORNERS[i] * size, 1.0f));
                min = glm::min(min, c);
                max = glm::max(max, c);
            }

            int leaf = leafs.size();
            BVHLeaf l = {};
            l.right = t.WorldMatrix[0];
            l.up = t.WorldMatrix[1];
            l.forward = t.WorldMatrix[2];
            l.position = glm::vec3(t.WorldMatrix[3]) + offset;
            l.packedSize = uint32_t(extent.width) | (uint32_t(extent.height) << 10) | (uint32_t(extent.depth) << 20);
            l.palleteID = v.Pallete->GetPalleteIndex();
            l.volumeRID = GetRID(v.Vox->GetImage());
            leafs.push_back(l);

            BVHNode node = {};
            node.min = min;
            node.max = max;
            node.leaf = leaf;

            entities.push_back(node);
        });

        buildNode(0, entities.size());
    }
};