#ifndef UserDataType_h
#define UserDataType_h
const int16_t ADC_DIM = 7;
struct data_t {
  unsigned long time;
  int16_t adc[ADC_DIM];
};
#endif  // UserDataType_h
