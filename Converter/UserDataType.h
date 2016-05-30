#ifndef UserDataType_h
#define UserDataType_h
const uint16_t ADC_DIM = 7;
#pragma pack(1)
struct data_t {
  uint32_t time;
  int16_t adc[ADC_DIM];
};
#endif  // UserDataType_h
