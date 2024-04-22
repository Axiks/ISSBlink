class Prediction{
    private:
      int startUTC;
      int maxUTC;
      int endUTC;
      int duration;
    public:
      Prediction(int startUTC, int maxUTC, int endUTC, int duration)
      : startUTC(startUTC), maxUTC(maxUTC), endUTC(endUTC), duration(duration)
      {}

      void setStartUTC(int startUTC){
        this -> startUTC = startUTC;
      }

      int getStartUTC(){
        return this -> startUTC;
      }

      void setMaxUTC(int maxUTC){
        this -> maxUTC = maxUTC;
      }

      int getMaxUTC(){
        return this -> maxUTC;
      }

      void setEndUTC(int endUTC){
        this -> endUTC = endUTC;
      }

      int getEndUTC(){
        return this -> endUTC;
      }

      void setDuration(int duration){
        this -> duration = duration;
      }

      int getDuration(){
        return this -> duration;
      }
};