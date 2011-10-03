#include <stdlib.h>
#include <assert.h>
#include "octree.h"

struct octree_pair
{
	size_t child_count;
	size_t index;
};

/* lies about ordering to ensure sorted in descending order */
static int octree_pair_compare(void const *a, void const *b)
{
	/* subject to integer over/underflows */
	return ((struct octree_pair *)b)->child_count -
		((struct octree_pair *)a)->child_count;
}

size_t octree_child_index(size_t current_index, uint8_t child_number)
{
	return 8 * (current_index + 1) + child_number;
}

size_t octree_parent_index(size_t current_index)
{
	assert(current_index >= 8);

	return current_index / 8 - 1;
}

uint8_t octree_find_child_number(uint16_t pixel, uint8_t level)
{
	assert(level <= sizeof(uint8_t));

	uint8_t red   = 0xff & pixel;
	uint8_t green = 0xff & (pixel >> 8);
	uint8_t blue  = 0xff & (pixel >> 16);
	uint8_t child = 0;

	child |= ((red >> (sizeof(red)   - level)) & 1) << 2;
	child |= ((red >> (sizeof(green) - level)) & 1) << 1;
	child |= ((red >> (sizeof(blue)  - level)) & 1);

	return child;
}

void octree_insert(octree_node_t *octree, uint16_t pixel)
{
	uint8_t red = 0xff & pixel;
	uint8_t green = 0xff & (pixel >> 8);
	uint8_t blue = 0xff & (pixel >> 16);
	uint8_t level = 0;
	size_t i = 0;

	while (i < kOctreeSize) {
		++level;
		uint8_t child = octree_find_child_number(pixel, level);
		octree[i].red_total   += red;
		octree[i].green_total += green;
		octree[i].blue_total  += blue;
		++octree[i].child_count;
		i = octree_child_index(i, child);
	}
}

void octree_calculate_palette(uint8_t palette[192][3], octree_node_t *octree)
{
	int i;
	struct octree_pair pairs[kOctreeSize - kOctreeRowFourIndex];

	for (i = kOctreeRowFourIndex; i < kOctreeSize; ++i) {
		pairs[i - kOctreeRowFourIndex].child_count = octree[i].child_count;
		pairs[i - kOctreeRowFourIndex].index = i;
		octree[i].palette_index = 0; /* zero the index */
	}

	/* sort the octree 4th row entries according to child_count */
	qsort(pairs, kOctreeSize - kOctreeRowFourIndex, sizeof(struct octree_pair),
	      &octree_pair_compare);

	/* store the top colors in palette */
	for (i = 0; i < 128; ++i) {
		octree_node_t *node = &octree[pairs[i].index];

		/* check for divide by zero */
		if (node->child_count) {
			palette[i][0] = node->red_total   / node->child_count;
			palette[i][1] = node->green_total / node->child_count;
			palette[i][2] = node->blue_total  / node->child_count;
		}
		node->palette_index = i;
	}

	/* store the remaining average colors to the palette to ensure coverage */
	for (i = 0; i < 64; ++i) {
		octree_node_t *node = &octree[i + kOctreeRowTwoIndex];
		node->palette_index = i + 128;
		octree_prune_children(octree, i + kOctreeRowTwoIndex);

		/* check for divide by zero */
		if (node->child_count) {
			palette[i + 128][0] = node->red_total   / node->child_count;
			palette[i + 128][1] = node->blue_total  / node->child_count;
			palette[i + 128][2] = node->green_total / node->child_count;
		}
	}
}

void octree_prune_children(octree_node_t *octree, size_t index)
{
	uint8_t i;

	if (index >= kOctreeRowFourIndex) return;

	/* clear the totals for this node */
	octree[index].child_count = 0;
	octree[index].red_total   = 0;
	octree[index].green_total = 0;
	octree[index].blue_total  = 0;

	for (i = 0; i < 8; ++i) {
		size_t child_index = octree_child_index(index, i);
		octree_prune_children(octree, child_index);
		octree[index].child_count += octree[child_index].child_count;
		octree[index].red_total   += octree[child_index].red_total;
		octree[index].green_total += octree[child_index].green_total;
		octree[index].blue_total  += octree[child_index].blue_total;
	}
}

uint8_t octree_find_palette_index(octree_node_t *octree, uint16_t pixel)
{
	size_t index = 0;
	uint8_t palette_index = 0;
	uint8_t level;

	for (level = 0; level < 4; ++level) {
		index = octree_child_index(index, octree_find_child_number(pixel, level));
		if (octree[index].palette_index) {
			palette_index = octree[index].palette_index;
		}
	}

	return palette_index;
}
