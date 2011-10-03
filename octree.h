#ifndef OCTREE_H
#define OCTREE_H

#include <stdint.h>

#define kOctreeSize (8 + 8*8 + 8*8*8 + 8*8*8*8)
#define kOctreeRowTwoIndex 8
#define kOctreeRowFourIndex (8 + 8*8 + 8*8*8)

typedef struct octree_node
{
	size_t child_count;
	size_t red_total;
	size_t green_total;
	size_t blue_total;
	uint8_t palette_index;
} octree_node_t;

/* finds the index of the child of the current_index at child_number */
size_t octree_child_index(size_t current_index, uint8_t child_number);

/* finds the index of the parent of current_index */
size_t octree_parent_index(size_t current_index);

/* find the child number for level */
uint8_t octree_find_child_number(uint16_t pixel, uint8_t level);

/* insert pixel into the octree */
void octree_insert(octree_node_t *octree, uint16_t pixel);

/* calculate the "optimal" color palette for the pixels in octree, and store
 * the result in palette */
void octree_calculate_palette(uint8_t palette[192][3], octree_node_t *octree);

/* prune the values stored in the octree_node at index (and it's children) by
 * subtracting the values from all child nodes (recursively) that are indexed
 * in the current palette. This function is only expected to be called once
 * per node */
void octree_prune_children(octree_node_t *octree, size_t index);

/* find the index in the color palette for pixel, requires that
 * octree_calculate_palette has already been called */
uint8_t octree_find_palette_index(octree_node_t *octree, uint16_t pixel);

#endif /* OCTREE_H */
