/* 
 * Class of Wolke lipo-single-cell-monitor
 * *******************************************
 * This will monitor 1 - 8 cells of your lipo.
 * It display the low cell, the high cell and the difference between this cells
 * this will give you a quick overview about your lipo status and if the lipo is well balanced
 *
 * detailed informations and schematics here
 *
 */
 
/*
   //cell voltage divider. this is dependent from your resitor voltage divider network
  double LIPOCELL_1TO8[13] =
  {
    1905.331599479, // 277.721518987, //1897.85344189,// 10bit 237.350026082,   3,97
    929.011553273,  // 137.358490566, //926.799312208,// 10bit 116.006256517,   8,03
    615.667808219,  // 91.373534338,  //618.198183455,// 10bit 77.3509473318,  12,04
    0.0, // 470.134166514,// 10bit 58.7966886122,
    0.0, // 370.317778975,// 10bit 46.3358699051,
    0.0, // 315.045617465,// 10bit 39.4176445024,
    0.0, // diverders 7-12 not defined because my network includes only 6 voltage dividers
    0.0,
    0.0,
    0.0,
    0.0,
    0.0
  };
  
  double individualcelldivider[MAXCELLS+1];
  uint8_t analogread_threshold = 100;         // threshold for connected zelldetection in mV
  uint8_t cells_in_use = MAXCELLS;
  int32_t zelle[MAXCELLS+1];
  double cell[MAXCELLS+1];
  int32_t alllipocells = 0;
  float lp_filter_val = 0.99; // this determines smoothness  - .0001 is max  0.99 is off (no smoothing)
  double smoothedVal[MAXCELLS+1]; // this holds the last loop value
  */

#include "LSCM.h"

// ----- Initialization and Default Values -----
LSCM::LSCM(uint8_t cells)
{
 
  initLSCM(cells, 13, 0.99);    
    
}
LSCM::LSCM(uint8_t cells, uint8_t analogReadReso)
{
  
   initLSCM(cells, analogReadReso, 0.99);   
    
}
LSCM::LSCM(uint8_t cells, uint8_t analogReadReso, float smoothness)
{
  
   initLSCM(cells, analogReadReso, smoothness);
    
}


// public functions
void LSCM::process()
{
  double aread[_maxcells+1];
  for(int i = 0; i < _maxcells; i++){
    aread[i] = analogRead(i);
    if(aread[i] < _analogread_threshold ){
      _cells_in_use = i;
      break;
    } else {
      _cells_in_use = _maxcells;
    }
    // USE Low Pass filter
    _smoothedVal[i] = ( aread[i] * (1 - _lp_filter_val)) + (_smoothedVal[i]  *  _lp_filter_val);
    aread[i] = round(_smoothedVal[i]);
    _cell[i]  = double (aread[i]/_individualcelldivider[i]) * 1000;
    if( i == 0 ) {
      _zelle[i] = round(_cell[i]);
    } else { 
      _zelle[i] = round(_cell[i] - _cell[i-1]);
    }
  }
  _alllipocells = _cell[_cells_in_use -1];

  #ifdef DEBUG_LIPO_SINGLE_CELL_MONITOR
    //debugSerial.println(aread[0]);
    //debugSerial.println(cell[0]);
    //debugSerial.println("-------");
    debugSerial.print(millis());
    debugSerial.print("\t-\t\t|\tCell 1\t|\tCell 2\t|\tCell3\t|");
    debugSerial.println();
    debugSerial.print(millis());
    debugSerial.print("\t-\t---------------------------------------------------------");
    debugSerial.println();

    debugSerial.print(millis());
    debugSerial.print("\t-\tzelle\t|");
    for(int i = 0; i < _maxcells; i++){
      debugSerial.print("\t");
      debugSerial.print(_zelle[i]/1000.0);
      debugSerial.print(" V\t|");
    }
    debugSerial.println();
    
    debugSerial.print(millis());
    debugSerial.print("\t-\tcell\t|");
    for(int i = 0; i < _maxcells; i++){
      debugSerial.print("\t");
      debugSerial.print(_cell[i]/1000);
      debugSerial.print(" V\t|");
    }
    debugSerial.println();
    
    debugSerial.print(millis());
    debugSerial.print("\t-\taread\t|");
    for(int i = 0; i < _maxcells; i++){
      debugSerial.print("\t");
      debugSerial.print(aread[i]);
      debugSerial.print("\t|");
    }  
    debugSerial.println();
    
    debugSerial.print(millis());
    debugSerial.print("\t-\t---------------------------------------------------------");
    debugSerial.println();

    debugSerial.print(millis());
    debugSerial.print("\t-\tCells in use: ");
    debugSerial.print(_cells_in_use);
    debugSerial.print(" (");
    debugSerial.print(_maxcells);
    debugSerial.print(")");
    debugSerial.println();

    debugSerial.print(millis());
    debugSerial.print("\t-\tsum: ");
    debugSerial.print(_alllipocells);
    debugSerial.print("mV");
    debugSerial.println();
    debugSerial.println();
  #endif    
}

uint32_t LSCM::getCellVoltageAsUint32_T(uint8_t cell)
{
    return _zelle[cell];
}

uint8_t LSCM::getCellsInUse()
{
  return _cells_in_use;
}

int32_t LSCM::getAllLipoCells()
{
  return _alllipocells;
}

/*
uint8_t LSCM::getMaxCells()
{
  return _maxcells;
}
*/

// private functions

void LSCM::initLSCM(uint8_t cells, uint8_t analogReadReso, float smoothness)
{
  _maxcells = cells;
  _cells_in_use = cells;
  _lp_filter_val = smoothness;
  _analogread_threshold = 100;
  _alllipocells = 0;
  analogReadResolution(analogReadReso);
  analogReference(DEFAULT);
  
  _individualcelldivider = new double[cells+1];
  _zelle = new int32_t[cells+1];
  _cell = new double[cells+1];
  _smoothedVal = new double[cells+1];
  
  for(int i = 0; i < cells; i++){
    _zelle[i] = 0;
    _cell[i] = 0.0;
    _individualcelldivider[i] = _LIPOCELL_1TO8[i];
    _smoothedVal[i] = 900.0;
  }
}

