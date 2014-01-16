// Author: qiyiping@gmail.com (Yiping Qi)

#include "tree.hpp"
#include "fitness.hpp"
#include "util.hpp"
#include <boost/lexical_cast.hpp>

#include <iostream>

namespace gbdt {
void RegressionTree::Fit(DataVector *data,
                         Node *node,
                         int depth) {
  int max_depth = gConf.max_depth;

  if (max_depth == depth || Same(*data)) {
    node->leaf = true;
    node->pred = Average(*data);
    return;
  }

  if (!FindSplit(data, &(node->index), &(node->value))) {
    node->leaf = true;
    node->pred = Average(*data);
    return;
  }

  // std::cout << "depth: " << depth << " index: " << node->index << " value: " << node->value << std::endl;

  DataVector out[Node::CHILDSIZE];

  SplitData(*data, node->index, node->value, out);
  if (out[Node::LT].empty() || out[Node::GE].empty()) {
    node->leaf = true;
    node->pred = Average(*data);
    return;
  }

  node->child[Node::LT] = new Node();
  node->child[Node::GE] = new Node();

  Fit(&out[Node::LT], node->child[Node::LT], depth+1);
  Fit(&out[Node::GE], node->child[Node::GE], depth+1);

  if (!out[Node::UNKNOWN].empty()) {
    node->child[Node::UNKNOWN] = new Node();
    Fit(&out[Node::UNKNOWN], node->child[Node::UNKNOWN], depth+1);
  }
}

ValueType RegressionTree::Predict(const Node *root, const Tuple &t) {
  if (root->leaf) {
    return root->pred;
  }
  if (t.feature[root->index] == kUnknownValue) {
    return Predict(root->child[Node::UNKNOWN], t);
  } else if (t.feature[root->index] < root->value) {
    return Predict(root->child[Node::LT], t);
  } else {
    return Predict(root->child[Node::GE], t);
  }
}

void RegressionTree::Fit(DataVector *data) {
  delete root;
  root = new Node();
  Fit(data, root, 0);
}

ValueType RegressionTree::Predict(const Tuple &t) const {
  return Predict(root, t);
}

std::string RegressionTree::Save() const {
  std::vector<const Node *> nodes;
  std::map<const void *, int> position_map;
  SaveAux(root, &nodes, &position_map);

  if (nodes.empty()) return std::string();

  std::vector<std::string> vs;
  for (size_t i = 0; i < nodes.size(); ++i) {
    std::string ns;
    ns += boost::lexical_cast<std::string>(nodes[i]->index);
    ns += " ";
    ns += boost::lexical_cast<std::string>(nodes[i]->value);
    ns += " ";
    ns += boost::lexical_cast<std::string>(nodes[i]->leaf);
    ns += " ";
    ns += boost::lexical_cast<std::string>(nodes[i]->pred);
    for (int j = 0; j < Node::CHILDSIZE; ++j) {
      ns += " ";
      if (nodes[i]->child[j]) {
        int p = position_map[nodes[i]->child[j]];
        ns += boost::lexical_cast<std::string>(p);
      } else {
        ns += "-1";
      }
    }
    vs.push_back(ns);
  }

  return JoinString(vs, "\n");
}

void RegressionTree::SaveAux(const Node *node,
                             std::vector<const Node *> *nodes,
                             std::map<const void *, int> *position_map) {
  if (!node) return;
  nodes->push_back(node);
  position_map->insert(std::make_pair<const void *, int>(node, nodes->size() -1));

  SaveAux(node->child[Node::LT], nodes, position_map);
  SaveAux(node->child[Node::GE], nodes, position_map);
  SaveAux(node->child[Node::UNKNOWN], nodes, position_map);
}

void RegressionTree::Load(const std::string &s) {
  delete root;
  std::vector<std::string> vs;
  SplitString(s, "\n", &vs);

  std::vector<Node *> nodes;
  std::vector<std::string> items;
  for (int i = 0; i < vs.size(); ++i) {
    Node *n = new Node();
    SplitString(vs[i], " ", &items);
    n->index = boost::lexical_cast<int>(items[0]);
    n->value = boost::lexical_cast<ValueType>(items[1]);
    n->leaf = boost::lexical_cast<bool>(items[2]);
    n->pred = boost::lexical_cast<ValueType>(items[3]);
    long lt = boost::lexical_cast<long>(items[4]);
    long ge = boost::lexical_cast<long>(items[5]);
    long un = boost::lexical_cast<long>(items[6]);
    if (lt > 0) n->child[Node::LT] = (Node *)lt;
    if (ge > 0) n->child[Node::GE] = (Node *)ge;
    if (un > 0) n->child[Node::UNKNOWN] = (Node *)un;
    nodes.push_back(n);
  }

  for (int i = 0; i < nodes.size(); ++i) {
    Node *lt = nodes[i]->child[Node::LT];
    Node *ge = nodes[i]->child[Node::GE];
    Node *un = nodes[i]->child[Node::UNKNOWN];
    if (lt) {
      nodes[i]->child[Node::LT] = nodes[(long)lt];
    }
    if (ge) {
      nodes[i]->child[Node::GE] = nodes[(long)ge];
    }
    if (un) {
      nodes[i]->child[Node::UNKNOWN] = nodes[(long)un];
    }
  }

  root = nodes[0];
}

}
