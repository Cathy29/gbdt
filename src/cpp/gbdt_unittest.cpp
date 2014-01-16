#include "gbdt.hpp"
#include "fitness.hpp"
#include <iostream>
#include <fstream>
#include <cassert>
#include <boost/lexical_cast.hpp>
#include "time.hpp"

using namespace gbdt;

int main(int argc, char *argv[]) {
  gConf.number_of_feature = 3;
  gConf.max_depth = 4;
  gConf.iterations = 10;
  gConf.shrinkage = 0.1;

  if (argc > 1) {
    gConf.max_depth = boost::lexical_cast<int>(argv[1]);
  }

  if (argc > 2) {
    gConf.iterations = boost::lexical_cast<int>(argv[2]);
  }

  if (argc > 3) {
    gConf.shrinkage = boost::lexical_cast<float>(argv[3]);
  }

  DataVector d;
  bool r = LoadDataFromFile("../../data/train.dat", &d);
  assert(r);

  GBDT gbdt;

  Elapsed elapsed;
  gbdt.Fit(&d);
  std::cout << "fit time: " << elapsed.Tell() << std::endl;

  std::ofstream model_output("../../data/gbdt.model");
  model_output << gbdt.Save();
  GBDT gbdt2;
  gbdt2.Load(gbdt.Save());

  DataVector d2;
  r = LoadDataFromFile("../../data/test.dat", &d2);
  assert(r);

  elapsed.Reset();
  DataVector::iterator iter = d2.begin();
  PredictVector predict;
  for ( ; iter != d2.end(); ++iter) {
    ValueType p = gbdt2.Predict(**iter);
    predict.push_back(p);
    std::cout << (*iter)->ToString() << std::endl
              << p << "," << gbdt.Predict(**iter) << std::endl;
  }

  std::cout << "predict time: " << elapsed.Tell() << std::endl;

  std::cout << "rmse: " << RMSE(d2, predict) << std::endl;

  CleanDataVector(&d2);
  CleanDataVector(&d);
  return 0;
}
