#include "SpatialHashGrid.h"
#include <set>

SpatialHashGrid::SpatialHashGrid(float spacing, int maxNumberOfObject) {
    reset(spacing, maxNumberOfObject);
}

void SpatialHashGrid::reset(float spacing, int maxNumberOfObject) {
    this->spacing = spacing;
    this->tableSize = maxNumberOfObject * 2;
    querySize = 0;

    cellStart.resize(tableSize + 1);
    cellEntries.resize(maxNumberOfObject);
    queryIds.resize(maxNumberOfObject);
}

void SpatialHashGrid::createHashGrid(const std::vector<glm::vec3>& positions) {
    int numberOfObjects = (int)positions.size();
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
        int hash = getHashFromPosition(positions[i]);
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
        int hash = getHashFromPosition(positions[i]); // get hash to each starting index of cell in cellStart
        cellStart[hash]--; // -1 because each each number points to cell entry + 1

        // use the starting index from cellStart as an index for cellEntries to set the starting index 
        // for looping through the objects contained in each cell
        cellEntries[cellStart[hash]] = i;
    }
}

int SpatialHashGrid::coordToHash(int x, int y, int z) {
    int h = (x * 92837111) ^ (y * 689287499) ^ (z * 283923481);
    return (h & 0x7FFFFFFF) % tableSize;
}

int SpatialHashGrid::floatToIntCoord(float coord) {
    return (int)std::floor(coord / spacing);
}

int SpatialHashGrid::getHashFromPosition(glm::vec3 position) {
    return coordToHash(
        floatToIntCoord(position.x),
        floatToIntCoord(position.y),
        floatToIntCoord(position.z)
    );
}

void SpatialHashGrid::query(glm::vec3 position, float maxDistance) {
    int startX = floatToIntCoord(position.x - maxDistance);
    int startY = floatToIntCoord(position.y - maxDistance);
    int startZ = floatToIntCoord(position.z - maxDistance);

    int endX = floatToIntCoord(position.x + maxDistance);
    int endY = floatToIntCoord(position.y + maxDistance);
    int endZ = floatToIntCoord(position.z + maxDistance);

    querySize = 0;

    // adding objects to be queried for use
    for (int x = startX; x <= endX; x++) {
        for (int y = startY; y <= endY; y++) {
            for (int z = startZ; z <= endZ; z++) {
                int hash = coordToHash(x, y, z);
                int start = cellStart[hash];
                int end = cellStart[hash + 1];

                for (int i = start; i < end; i++) {
                    if (querySize < (int)queryIds.size()) {
                        queryIds[querySize] = cellEntries[i]; // cellEntries[i] returns the object index in the object list
                        querySize++;
                    }
                    
                }
            }
        }
    }
}

int SpatialHashGrid::getQuerySize() const {
    return querySize;
}

int SpatialHashGrid::getQueryId(int id) const {
    return queryIds[id];
}