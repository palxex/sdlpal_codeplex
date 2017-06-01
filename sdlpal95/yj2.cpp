/*
 * PAL WIN95 compress format (YJ_2) library
 *
 * Author: Yihua Lou <louyihua@21cn.com>
 *
 * Copyright 2006 - 2007 Yihua Lou
 *
 * This file is part of PAL library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <stdlib.h>
#include <memory.h>
#include <string.h>

typedef struct _TreeNode
{
	unsigned short		weight;
	unsigned short		value;
	struct _TreeNode*	parent;
	struct _TreeNode*	left;
	struct _TreeNode*	right;
} TreeNode;

typedef struct _Tree
{
	TreeNode*	node;
	TreeNode**	list;
} Tree;

static unsigned char data1[0x100] =
{
0x3f,0x0b,0x17,0x03,0x2f,0x0a,0x16,0x00,0x2e,0x09,0x15,0x02,0x2d,0x01,0x08,0x00,
0x3e,0x07,0x14,0x03,0x2c,0x06,0x13,0x00,0x2b,0x05,0x12,0x02,0x2a,0x01,0x04,0x00,
0x3d,0x0b,0x11,0x03,0x29,0x0a,0x10,0x00,0x28,0x09,0x0f,0x02,0x27,0x01,0x08,0x00,
0x3c,0x07,0x0e,0x03,0x26,0x06,0x0d,0x00,0x25,0x05,0x0c,0x02,0x24,0x01,0x04,0x00,
0x3b,0x0b,0x17,0x03,0x23,0x0a,0x16,0x00,0x22,0x09,0x15,0x02,0x21,0x01,0x08,0x00,
0x3a,0x07,0x14,0x03,0x20,0x06,0x13,0x00,0x1f,0x05,0x12,0x02,0x1e,0x01,0x04,0x00,
0x39,0x0b,0x11,0x03,0x1d,0x0a,0x10,0x00,0x1c,0x09,0x0f,0x02,0x1b,0x01,0x08,0x00,
0x38,0x07,0x0e,0x03,0x1a,0x06,0x0d,0x00,0x19,0x05,0x0c,0x02,0x18,0x01,0x04,0x00,
0x37,0x0b,0x17,0x03,0x2f,0x0a,0x16,0x00,0x2e,0x09,0x15,0x02,0x2d,0x01,0x08,0x00,
0x36,0x07,0x14,0x03,0x2c,0x06,0x13,0x00,0x2b,0x05,0x12,0x02,0x2a,0x01,0x04,0x00,
0x35,0x0b,0x11,0x03,0x29,0x0a,0x10,0x00,0x28,0x09,0x0f,0x02,0x27,0x01,0x08,0x00,
0x34,0x07,0x0e,0x03,0x26,0x06,0x0d,0x00,0x25,0x05,0x0c,0x02,0x24,0x01,0x04,0x00,
0x33,0x0b,0x17,0x03,0x23,0x0a,0x16,0x00,0x22,0x09,0x15,0x02,0x21,0x01,0x08,0x00,
0x32,0x07,0x14,0x03,0x20,0x06,0x13,0x00,0x1f,0x05,0x12,0x02,0x1e,0x01,0x04,0x00,
0x31,0x0b,0x11,0x03,0x1d,0x0a,0x10,0x00,0x1c,0x09,0x0f,0x02,0x1b,0x01,0x08,0x00,
0x30,0x07,0x0e,0x03,0x1a,0x06,0x0d,0x00,0x19,0x05,0x0c,0x02,0x18,0x01,0x04,0x00
};
static unsigned char data2[0x10] =
{
0x08,0x05,0x06,0x04,0x07,0x05,0x06,0x03,0x07,0x05,0x06,0x04,0x07,0x04,0x05,0x03
};

static void adjust_tree(Tree tree, unsigned short value)
{
	TreeNode* node = tree.list[value];
	TreeNode tmp;
	TreeNode* tmp1;
	TreeNode* temp;
	while(node->value != 0x280)
	{
		temp = node + 1;
		while(node->weight == temp->weight)
			temp++;
		temp--;
		if (temp != node)
		{
			tmp1 = node->parent;
			node->parent = temp->parent;
			temp->parent = tmp1;
			if (node->value > 0x140)
			{
				node->left->parent = temp;
				node->right->parent = temp;
			}
			else
				tree.list[node->value] = temp;
			if (temp->value > 0x140)
			{
				temp->left->parent = node;
				temp->right->parent = node;
			}
			else
				tree.list[temp->value] = node;
			tmp = *node; *node = *temp; *temp = tmp;
			node = temp;
		}
		node->weight++;
		node = node->parent;
	}
	node->weight++;
}

static bool build_tree(Tree& tree)
{
	int i, ptr;
	TreeNode** list;
	TreeNode* node;
	if ((tree.list = list = new TreeNode* [321]) == NULL)
		return false;
	if ((tree.node = node = new TreeNode [641]) == NULL)
	{
		delete [] list;
		return false;
	}
	memset(list, 0, 321 * sizeof(TreeNode*));
	memset(node, 0, 641 * sizeof(TreeNode));
	for(i = 0; i <= 0x140; i++)
		list[i] = node + i;
	for(i = 0; i <= 0x280; i++)
	{
		node[i].value = i;
		node[i].weight = 1;
	}
	tree.node[0x280].parent = tree.node + 0x280;
	for(i = 0, ptr = 0x141; ptr <= 0x280; i += 2, ptr++)
	{
		node[ptr].left = node + i;
		node[ptr].right = node + i + 1;
		node[i].parent = node[i + 1].parent = node + ptr;
		node[ptr].weight = node[i].weight + node[i + 1].weight;
	}
	return true;
}

#pragma pack(1)
typedef struct _BitField
{
	unsigned char	b0:	1;
	unsigned char	b1:	1;
	unsigned char	b2:	1;
	unsigned char	b3:	1;
	unsigned char	b4:	1;
	unsigned char	b5:	1;
	unsigned char	b6:	1;
	unsigned char	b7:	1;
} BitField;
#pragma pack()

static inline bool bt(const void* data, unsigned int pos)
{
	BitField* bit = (BitField*)((unsigned char*)data + (pos >> 3));
	switch(pos & 0x7)
	{
	case 0:	return bit->b0;
	case 1:	return bit->b1;
	case 2:	return bit->b2;
	case 3:	return bit->b3;
	case 4:	return bit->b4;
	case 5:	return bit->b5;
	case 6:	return bit->b6;
	case 7:	return bit->b7;
	}
	return 0;
}

static inline void bit(void* data, unsigned int pos, bool set)
{
	BitField* bit = (BitField*)((unsigned char*)data + (pos >> 3));
	switch(pos & 0x7)
	{
	case 0:
		bit->b0 = set;
		break;
	case 1:
		bit->b1 = set;
		break;
	case 2:
		bit->b2 = set;
		break;
	case 3:
		bit->b3 = set;
		break;
	case 4:
		bit->b4 = set;
		break;
	case 5:
		bit->b5 = set;
		break;
	case 6:
		bit->b6 = set;
		break;
	case 7:
		bit->b7 = set;
		break;
	}
}

extern "C" int DecodeYJ2(const void* Source, void* Destination, int Length)
{
	unsigned int len = 0, ptr = 0;
	unsigned char* src = (unsigned char*)Source + 4;
	unsigned char* dest;
	Tree tree;
	TreeNode* node;

	if (Source == NULL)
		return -1;

	if (!build_tree(tree))
		return -1;

	Length = *((unsigned int*)Source);
	dest = (unsigned char*)Destination;

	while (1)
	{
		unsigned short val;
		node = tree.node + 0x280;
		while(node->value > 0x140)
		{
			if (bt(src, ptr))
				node = node->right;
			else
				node = node->left;
			ptr++;
		}
		val = node->value;
		if (tree.node[0x280].weight == 0x8000)
		{
			int i;
			for(i = 0; i < 0x141; i++)
				if (tree.list[i]->weight & 0x1)
					adjust_tree(tree, i);
			for(i = 0; i <= 0x280; i++)
				tree.node[i].weight >>= 1;
		}
		adjust_tree(tree, val);
		if (val > 0xff)
		{
			int i;
			unsigned int temp, tmp, pos;
			unsigned char* pre;
			for(i = 0, temp = 0; i < 8; i++, ptr++)
				temp |= (unsigned int)bt(src, ptr) << i;
			tmp = temp & 0xff;
			for(; i < data2[tmp & 0xf] + 6; i++, ptr++)
				temp |= (unsigned int)bt(src, ptr) << i;
			temp >>= data2[tmp & 0xf];
			pos = (temp & 0x3f) | ((unsigned int)data1[tmp] << 6);
			if (pos == 0xfff)
				break;
			pre = dest - pos - 1;
			for(i = 0; i < val - 0xfd; i++)
				*dest++ = *pre++;
			len += val - 0xfd;
		}
		else
		{
			*dest++ = (unsigned char)val;
			len++;
		}
	}
	delete [] tree.list;
	delete [] tree.node;
	return Length;
}
