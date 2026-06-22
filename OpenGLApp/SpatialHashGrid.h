#pragma once
#include <glm/glm.hpp>
#include <vector>

class SpatialHashGrid {
private:
    float spacing;
    int tableSize;
    std::vector<int> cellStart;
    std::vector<int> cellEntries;
    std::vector<int> queryIds;
    int querySize;

    int coordToHash(int x, int y, int z);
    int floatToIntCoord(float coord);
    int getHashFromPosition(glm::vec3 position);

public:
    SpatialHashGrid(float spacing, int maxNumberOfObject);
    void reset(float spacing, int maxNumberOfObject);

    void createHashGrid(const std::vector<glm::vec3>& positions);
    
    void query(glm::vec3 position, float maxDistance);
    int getQuerySize() const;
    int getQueryId(int id) const;
};