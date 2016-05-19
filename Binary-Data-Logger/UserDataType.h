#ifndef UserDataType_h
#define UserDataType_h
const int16_t ADC_DIM = 6;
struct data_t {
  unsigned long time;
  int8_t adc[ADC_DIM];
};
#endif  // UserDataType_h
