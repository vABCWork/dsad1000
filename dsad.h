
void Excep_DSAD0_ADI0(void);
void Excep_DSAD0_SCANEND0(void);


void  dsad0_start(void);
void  dsad0_stop(void);

void afe_ini(void);
void dsad0_ini(void);


extern volatile uint8_t ad_err;
extern volatile uint8_t ad_ovf;

extern volatile int32_t ad_ch0_data[1000];

extern volatile uint16_t ad_index;

extern volatile uint32_t dsad0_scan_over;
extern volatile uint32_t dsad0_collect_status;



