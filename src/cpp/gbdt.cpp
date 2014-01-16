// Author: qiyiping@gmail.com (Yiping Qi)
#include "gbdt.hpp"
#include "fitness.hpp"
#include "util.hpp"
#include <iostream>
#include <cmath>
#include <cassert>

namespace gbdt {
ValueType GBDT::Predict(const Tuple &t) const {
  if (!trees)
    return kUnknownValue;

  ValueType r = 0;
  for (int i = 0; i < gConf.iterations; ++i) {
    r += gConf.shrinkage * trees[i].Predict(t);
  }

  return r;
}

void GBDT::Fit(DataVector *d) {
  delete[] trees;
  trees = new RegressionTree[gConf.iterations];
  for (int i = 0; i < gConf.iterations; ++i) {
    trees[i].Fit(d);

    DataVector::iterator iter = d->begin();
    double s = 0;
    for ( ; iter != d->end(); ++iter) {
      ValueType p = trees[i].Predict(**iter);
      (*iter)->target -= gConf.shrinkage * p;
      s += Squared((*iter)->target);
    }

    std::cout  << "iteration: " << i << std::endl
               << "rmse: " << std::sqrt(s/d->size()) << std::endl;
  }
}

std::string GBDT::Save() const {
  std::vector<std::string> vs;
  for (int i = 0; i < gConf.iterations; ++i) {
    vs.push_back(trees[i].Save());
  }
  return JoinString(vs, "\n;\n");
}

void GBDT::Load(const std::string &s) {
  delete[] trees;
  trees = new RegressionTree[gConf.iterations];
  std::vector<std::string> vs;
  SplitString(s, "\n;\n", &vs);
  int j = 0;
  for (size_t i = 0; i < vs.size(); ++i) {
    if (vs[i].empty()) continue;

    trees[j++].Load(vs[i]);
  }

  assert(j == gConf.iterations);
}
}
