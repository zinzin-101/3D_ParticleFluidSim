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

    template <class T>
    void createHashGrid(const std::vector<T>& objects);
    
    void query(glm::vec3 position, float maxDistance);
    int getQuerySize() const;
    int getQueryId(int id) const;
};

template <class T>
void SpatialHashGrid::createHashGrid(const std::vector<T>& objects) {
    int numberOfObjects = (int)objects.size();
    int n = (int)cellStart.size();
    // initialize to zeroes
    for (int i = 0; i < n; i++) {
        cellStart[i] = 0;

        if (i < cellEntries.size()) {
            cellEntries[i] = 0;
        }
    }

    // count number of objects present in each cells
    for (int i = 0; i < numberOfObjects; i++) {
        int hash = getHashFromPosition(objects[i].position);
        cellStart[hash]++;
    }

    // compute partial sum
    // e.g.
    // before -> cellStart: 0 0 2 0 4 0 1
    // after  -> cellStart: 0 0 2 2 6 6 7
    // each number points to the last cell entry + 1
    int start = 0;
    for (int i = 0; i < tableSize; i++) {
        start += cellStart[i];
        cellStart[i] = start;
    }
    cellStart[tableSize] = start;


    for (int i = 0; i < numberOfObjects; i++) {
        int hash = getHashFromPosition(objects[i].position); // get hash to each starting index of cell in cellStart
        cellStart[hash]--; // -1 because each each number points to cell entry + 1

        // use the starting index from cellStart as an index for cellEntries to set the starting index 
        // for looping through the objects contained in each cell
        cellEntries[cellStart[hash]] = i;
    }
}