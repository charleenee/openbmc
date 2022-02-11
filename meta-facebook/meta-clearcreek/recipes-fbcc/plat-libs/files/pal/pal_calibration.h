struct calibration_table {
  float ein;
  float coeff;
};

/* Quanta BMC correction table */
struct calibration_table power_table[] = {
  {12.19,  1.0381},
  {59.43,  1.0230},
  {119.31, 1.0146},
  {179.14, 1.0092},
  {238.56, 1.0107},
  {297.83, 1.0110},
  {357.36, 1.0117},
  {416.88, 1.0129},
  {476.20, 1.0127},
  {535.42, 1.0055},
  {594.94, 1.0053},
  {653.60, 1.0058},
  {712.51, 1.0063},
  {771.98, 1.0069},
  {830.26, 1.0072},
  {889.44, 1.0067},
  {949.43, 1.0082},
  {1007.16, 1.0093},
  {1066.94, 1.0104},
  {1125.81, 1.0089},
  {1184.51, 1.0083},
  {1242.75, 1.0091},
  {1301.20, 1.0106},
  {1360.01, 1.0102},
  {1417.69, 1.0104},
  {0.0,   0.0}
};

struct calibration_table current_table[] = {
  {1.02, 1.0304},
  {4.90, 1.0304},
  {9.92, 1.0134},
  {14.86, 1.0108},
  {19.86, 1.0111},
  {24.81, 1.0101},
  {29.79, 1.0109},
  {34.73, 1.0132},
  {39.70, 1.0131},
  {44.68, 1.0058},
  {49.70, 1.0050},
  {54.64, 1.0056},
  {59.61, 1.0062},
  {64.55, 1.0078},
  {69.53, 1.0074},
  {74.47, 1.0079},
  {79.40, 1.0111},
  {84.50, 1.0098},
  {89.44, 1.0121},
  {94.45, 1.0108},
  {99.43, 1.0104},
  {104.33, 1.0120},
  {109.35, 1.0120},
  {114.33, 1.0122},
  {119.30, 1.0122},
  {0.0,   0.0}
};

