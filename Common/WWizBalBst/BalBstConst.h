/**********************************************************************************************************
Program Name			: BalBstConst.h
Description				: This class is written to load object in binary search tree(BST).
BST helps to search the node faster.
Author Name				: Ramkrushna Shelke
Date Of Creation		: 04/06/2016
Version No				: 1.14.1.10
Special Logic Used		:
Modification Log		:
1. Name					: Description
***********************************************************************************************************/
#pragma once


#define BALBSTCLSTMPLT	template<class T>
#define CWWizBalBstT	CWWizBalBst<T>
#define CNodeT			CNode<T>
#define MAX_CHILDREN	3

enum eNodeSize
{
	empty = 0,
	twoNode,
	threeNode
};

enum eTraverseDirection
{
	leftChild = 0,
	middleChild,
	rightChild
};

/* Three tree traversal methods supported. */
enum eTreeTraversal
{
	inorder = 0,
	preorder,
	postorder
};
