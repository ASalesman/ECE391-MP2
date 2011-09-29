#include <stdlib.h>
#include "octree.h"

static struct octree_pair
{
	size_t child_count;
	size_t index;
};

/* lies about ordering to ensure sorted in descending order */
static int octree_pair_compare(void *a, void *b)
{
	/* subject to integer over/underflows */
	return ((struct octree_pair *)b)->child_count -
		((struct octree_pair *)a)->child_count;
}

size_t octree_child_index(size_t current_index, uint8_t child_number)
{
	return 8 * (current_index + 1) + child_number;
}

void octree_insert(octree_node_t *octree, uint16_t pixel)
{
}

void octree_calculate_palette(uint8_t palette[192][3], octree_node_t *octree)
{
	int i;
	struct octree_pair pairs[kOctreeSize - kOctreeRowFourIndex];

	for (i = kOctreeRowFourIndex; i < kOctreeSize; ++i) {
		pairs[i - kOctreeRowFourIndex] = { octree[i]->child_count, i };
		octree[i]->palette_index = 0; /* zero the index */
	}

	/* sort the octree 4th row entries according to child_count */
	qsort(pairs, kOctreeSize - kOctreeRowFourIndex, sizeof(struct octree_pair),
	      &octree_pair_compare);

	/* store the top colors in palette */
	for (i = 0; i < 128; ++i) {
		palette[i][0] = pairs[i]->red_total / pairs[i]->child_count;
		palette[i][1] = pairs[i]->green_total / pairs[i]->child_count;
		palette[i][2] = pairs[i]->blue_total / pairs[i]->child_count;
		octree[pairs[i]->index]->palette_index = i;
	}

	/* store the remaining average colors to the palette to ensure coverage */
	for (i = 0; i < 64; ++i) {
		octree_node_t *node = octree[i + kOctreeRowTwoIndex];
		node->palette_index = i + 128;
		octree_prune_children(octree, i + kOctreeRowTwoIndex);
		size_t child_total = node->child_total;
		palette[i + 128][0] = node->red_total / child_total;
		palette[i + 128][1] = node->blue_total / child_total;
		palette[i + 128][2] = node->green_total / child_total;
	}
}

void octree_prune_children(octree_node_t *octree, size_t index)
{
}

uint8_t octree_find_palette_index(octree_node_t *octree, uint16_t pixel)
{
	return 0;
}
