#include "NetworkArchitectures.h"
#include "SpatiallySparseDatasetKaggleDiabeticRetinopathy.h"
#include <iostream>
#include <string>

int epoch=0;
int cudaDevice=-1; //PCI bus ID, -1 for default GPU
int batchSize=5;
std::string dirName("Data/kaggleDiabeticRetinopathy/300_train/")
std::string dirNameTest("Data/kaggleDiabeticRetinopathy/300_sample/");

Picture* OpenCVPicture::distort(RNG& rng, batchType type) {
  OpenCVPicture* pic=new OpenCVPicture(*this);
  pic->loadDataWithoutScaling();
  float
    c00=1, c01=0,  //2x2 identity matrix---starting point for calculating affine distortion matrix
    c10=0, c11=1;
  float r, alpha, beta;
  if (false and type==TRAINBATCH) {
    r=rng.uniform(-0.1,0.1);
    alpha=rng.uniform(0,2*3.1415926535);
    beta=rng.uniform(-0.2,0.2)+alpha;
  } else {
    r=0;
    alpha=rng.uniform(0,2*3.1415926535);
    beta=alpha;
  }
  c00=(1+r)*cos(alpha); c01=(1+r)*sin(alpha);
  c10=-(1-r)*sin(beta); c11=(1-r)*cos(beta);
  if (rng.randint(2)==0) {c00*=-1; c01*=-1;}//Horizontal flip
  pic->affineTransform(c00, c01, c10, c11);
  //  writeImage(pic->mat,epoch++);
  //if (type==TRAINBATCH)
  pic->jiggle(rng,30);
  return pic;
}

class Imagenet : public SparseConvNet {
public:
  Imagenet (int dimension, ActivationFunction fn, int nInputFeatures, int nClasses, int cudaDevice=-1, int nTop=1);
};
Imagenet::Imagenet
(int dimension, ActivationFunction fn,
 int nInputFeatures, int nClasses, int cudaDevice, int nTop)
  : SparseConvNet(dimension,nInputFeatures, nClasses, cudaDevice, nTop) {
  addLeNetLayerPOFMP( 32,5,1,3,1.8,fn);
  addLeNetLayerPOFMP( 64,3,1,3,1.8,fn);
  addLeNetLayerPOFMP( 96,3,1,3,1.8,fn);
  addLeNetLayerPOFMP(128,3,1,3,1.8,fn);
  addLeNetLayerPOFMP(160,3,1,3,1.8,fn);
  addLeNetLayerPOFMP(192,3,1,3,1.8,fn);
  addLeNetLayerPOFMP(224,3,1,3,1.8,fn);
  addLeNetLayerPOFMP(256,3,1,3,1.8,fn,32.0/256);
  addLeNetLayerPOFMP(288,3,1,2,1.5,fn,32.0/288);
  addLeNetLayerMP   (320,2,1,1,1  ,fn,64.0/320);
  addLeNetLayerMP   (356,1,1,1,1  ,fn,64.0/356);
  addSoftmaxLayer();
}

int main() {
  std::string baseName="Data/kaggleDiabeticRetinopathy/kaggleDiabeticRetinopathyCompetitionModelFiles/kaggleDiabeticRetinopathy1";
  SpatiallySparseDataset trainSet=KDRTrainSet(dirName);
  /*SpatiallySparseDataset validationSet=KDRValidationSet(dirName);*/
  SpatiallySparseDataset testSet=KDRSampleSet(dirNameTest);
  trainSet.summary();
  /*validationSet.summary();*/
  testSet.summary();
  {
    Imagenet cnn(2,VLEAKYRELU,trainSet.nFeatures,trainSet.nClasses,cudaDevice);
    cnn.loadWeights(baseName, 65);
    /*if (epoch==0) {
      SpatiallySparseDataset trainSubset=trainSet.subset(20*batchSize);
      trainSubset.type=RESCALEBATCH;
      cnn.processDataset(trainSubset,batchSize);
    } else {
      cnn.loadWeights(baseName,epoch);
    }
    for (epoch++;epoch<=65;epoch++) {
      std::cout <<"epoch: " << epoch << std::endl;
      for (int i=0;i<3;++i) {
        SpatiallySparseDataset trainSubset=trainSet.subset(12000);
        cnn.processDataset(trainSubset, batchSize,0.003*exp(-epoch*0.05),0.999);
        cnn.saveWeights(baseName,epoch);
      }
      cnn.processDatasetRepeatTest(validationSet, batchSize,1,"kaggleDiabetes1.predictions","","confusionMatrix1.train"); //Monitor progress during training
    }
    cnn.processDatasetRepeatTest(validationSet, batchSize,12,"Data/kaggleDiabeticRetinopathy/kaggleDiabeticRetinopathyCompetitionModelFiles/kaggleDiabetes1_epoch65.validation","","confusionMatrix1.validation");
    SpatiallySparseDataset trainSetAsTestSet=KDRTrainSet(dirName);trainSetAsTestSet.type=TESTBATCH;
    cnn.processDatasetRepeatTest(trainSetAsTestSet, batchSize,12,"Data/kaggleDiabeticRetinopathy/kaggleDiabeticRetinopathyCompetitionModelFiles/kaggleDiabetes1_epoch65.train","","confusionMatrix1.train");*/
    cnn.processDatasetRepeatTest(testSet, batchSize, 12,"Data/kaggleDiabeticRetinopathy/kaggleDiabetes1_epoch65_sample.test");
  }
}
