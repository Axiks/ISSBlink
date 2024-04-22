class Satelite{
    public:
      int id;
      String name;
      float lat;
      float lon;
      float alt;
      float minVisiblityBrightness;
      int predictionDays;

      Satelite(int id, String name, float lat, float lon, float alt, float minVisiblityBrightness, int predictionDays){
        this -> id = id;
        this -> name = name;
        this -> lat = lat;
        this -> lon = lon;
        this -> alt = alt;
        this -> minVisiblityBrightness = minVisiblityBrightness;
        this -> predictionDays = predictionDays;}
};