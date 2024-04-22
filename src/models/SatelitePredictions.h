class SatelitePredictions{
    private:
      Satelite *satelite;
      short predictionPushIndex;
    public:
      Prediction *predictions[15];

      SatelitePredictions(Satelite *satelite)
      : satelite(satelite)
      {
        predictionPushIndex = 0;
      }

      Satelite *getSatelite(){
        return this -> satelite;
      }

      void addPrediction(Prediction *prediction){
        predictions[predictionPushIndex] = prediction;
        predictionPushIndex++;
      }

      short count(){
        return predictionPushIndex;
      }

      Prediction **getPredictions(){
        return this -> predictions;
      }
};