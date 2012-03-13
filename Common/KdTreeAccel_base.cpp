/*
   Copyright (c) 2010 University of Illinois
   All rights reserved.

   Developed by:           DeNovo group, Graphis@Illinois
                           University of Illinois
                           http://denovo.cs.illinois.edu
                           http://graphics.cs.illinois.edu

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the
   "Software"), to deal with the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimers.

    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following disclaimers
      in the documentation and/or other materials provided with the
      distribution.

    * Neither the names of DeNovo group, Graphics@Illinois, 
      University of Illinois, nor the names of its contributors may be used to 
      endorse or promote products derived from this Software without specific 
      prior written permission.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR
   ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
*/

#include <fstream>
#include <iterator>
#include <map>
#include <algorithm>

#include "KdTreeAccel_base.h"

using namespace std;

KdTreeAccel_base::KdTreeAccel_base(TriangleMesh *mesh, 
                                   uint numThreads, uint maxDepth, 
                                   float Ct, float Ci, float emptyBonus) 
  : m_mesh(mesh), m_numThreads(numThreads), m_maxDepth(maxDepth),
    sah(Ct, Ci, emptyBonus) {

  // Sanity checks
  assert(m_mesh);
  assert(m_numThreads > 0);
  assert(m_maxDepth > 0);
}

bool KdTreeAccel_base::writeToFile(char * filename) {
  // fill in item list and node list by traversing the tree
  vector<int> itemList;
  vector<MantaKDTreeNode> nodeList;
  nodeList.push_back(MantaKDTreeNode());
  this->packNodesAndItems(this->m_root, 0, itemList, nodeList);
  
  // write item list and node lists to file
  ofstream out(filename, ios::out | ios::binary);
  if (!out) return false;
  
  const unsigned int itemList_size = itemList.size();
  const unsigned int nodeList_size = nodeList.size();
  out.write((char*) &itemList_size, sizeof(itemList_size));
  out.write((char*) &itemList[0], sizeof(itemList[0])*itemList_size);
  out.write((char*) &nodeList_size, sizeof(nodeList_size));
  out.write((char*) &nodeList[0], sizeof(nodeList[0])*nodeList_size);
  out.close();
  return true;
}

void KdTreeAccel_base::packNodesAndItems(KdTreeNode * nodePtr, 
                                         const int nodeIdx, 
                                         vector<int> & itemList, 
                                         vector<MantaKDTreeNode> & nodeList) {
  MantaKDTreeNode & node = nodeList[nodeIdx];
  // 	node.isLeaf = nodePtr == NULL || (nodePtr->left == NULL && nodePtr->right == NULL);
  if (nodePtr == NULL) { // empty leaf node
    node.isLeaf = true;
    node.numPrimitives = 0;
    node.childIdx = itemList.size();
  } else if (nodePtr->left == NULL && nodePtr->right == NULL) { // leaf
    node.isLeaf = true;
    node.numPrimitives = nodePtr->triangleIndices->size();
    node.childIdx = itemList.size();
    for (int i = 0; i < nodePtr->triangleIndices->size(); i++)
      itemList.push_back((*nodePtr->triangleIndices)[i]);
  } else {
    node.planePos = nodePtr->splitEdge->t;
    node.planeDim = nodePtr->splitEdge->axis;
    node.childIdx = nodeList.size();
    nodeList.push_back(MantaKDTreeNode());	// left child
    nodeList.push_back(MantaKDTreeNode());	// right child
    packNodesAndItems(nodePtr->left, nodeList[nodeIdx].childIdx, itemList, nodeList);
    packNodesAndItems(nodePtr->right, nodeList[nodeIdx].childIdx+1, itemList, nodeList);
  }
}

void KdTreeAccel_base::printGraphviz() const {
  ofstream out("output.dot");
  out << "digraph g {" << endl;
  out << "ratio=compress; fontsize=8; colorscheme=paired12;" << endl;
  out << "node [shape = record,height=.1];" << endl;
  out << "\"" << (void*)m_root << "\"";
  out << "[label = \"";
  if (m_root->triangleIndices) {
    out << "root:" << m_root->triangleIndices->size();
  } else {
    out << "root: 0";
  }
  out << "\"];" << endl;
  printGraphvizHelper(m_root, out, 1);
  out << "}" << endl;
  out.close();
}

void KdTreeAccel_base::printGraphvizAccm() const {
  ofstream out("output.dot");
  out << "digraph g {" << endl;
  out << "ratio=compress; fontsize=8; colorscheme=paired12;" << endl;
  out << "node [shape = record,height=.1];" << endl;
  out << "\"" << (void*)m_root << "\"";
  out << "[label = \"";
  if (m_root->triangleIndices) {
    out << "root:" << m_root->triangleIndices->size();
  } else {
    out << "root: 0";
  }
  out << "\"];" << endl;
  printGraphvizHelper(m_root, out, 1, m_root->triangleIndices->size());
  out << "}" << endl;
  out.close();
}

void KdTreeAccel_base::printTreeHelper(KdTreeNode *node) const
{
  if (node->triangleIndices) {
    // prefix traversal
    sort(node->triangleIndices->begin(), node->triangleIndices->end());
    copy(node->triangleIndices->begin(), node->triangleIndices->end(),
         ostream_iterator<int>(cout, " "));
    cout << endl;
  }

  if (node->left != NULL)
    printTreeHelper(node->left);
  if (node->right != NULL)
    printTreeHelper(node->right);
}

// { pointer to task, color id }
std::map<long,int> colors;
const int color_limit = 12;
int color_i = 0;

int getNextColor() {
  return (color_i++)%color_limit + 1;
}

void KdTreeAccel_base::printGraphvizHelper(KdTreeNode *node, ostream &out,
                                           unsigned int level) const {
  if (node->left) {
    out << "\"" << (void*)node->left << "\""
        << "[label=\"" << level << ": ";
    if (node->left->triangleIndices) {
      out << node->left->triangleIndices->size();
    } else {
      out << 0;
    }
    out << "\"];";
    out << "\"" << (void*)node << "\"";
    out << " -> ";
    out << "\"" << (void*)node->left << "\"";
    out << ";" << endl;
  }

  if (node->right) {
    out << "\"" << (void*)node->right << "\""
        << "[label=\"" << level << ": ";
    if (node->right->triangleIndices) {
      out << node->right->triangleIndices->size();
    } else {
      out << 0;
    }
    out << "\"];";
    out << "\"" << (void*)node << "\"";
    out << " -> ";
    out << "\"" << (void*)node->right << "\"";
    out << ";" << endl;
  }
  if (node->left) {
    printGraphvizHelper(node->left, out, level+1);
  }
  if (node->right) {
    printGraphvizHelper(node->right, out, level+1);
  }
}

void KdTreeAccel_base::printGraphvizHelper(KdTreeNode *node, ostream &out,
                                           unsigned int level, uint accm) const {
  accm += node->triangleIndices->size();
  if (node->left) {
    out << "\"" << (void*)node->left << "\""
        << "[label=\"" << level << ": ";
    if (node->left->triangleIndices) {
      out << accm+node->left->triangleIndices->size();
    } else {
      out << accm;
    }
    out << "\"];";
    out << "\"" << (void*)node << "\"";
    out << " -> ";
    out << "\"" << (void*)node->left << "\"";
    out << ";" << endl;
  }
  if (node->right) {
    out << "\"" << (void*)node->right << "\""
        << "[label=\"" << level << ": ";
    if (node->right->triangleIndices) {
      out << accm+node->right->triangleIndices->size();
    } else {
      out << accm;
    }
    out << "\"];";
    out << "\"" << (void*)node << "\"";
    out << " -> ";
    out << "\"" << (void*)node->right << "\"";
    out << ";" << endl;
  }
  if (node->left) {
    printGraphvizHelper(node->left, out, level+1, accm);
  }
  if (node->right) {
    printGraphvizHelper(node->right, out, level+1, accm);
  }
}
