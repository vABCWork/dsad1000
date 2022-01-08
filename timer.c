

#include "typedefine.h"
#include  "iodefine.h"
#include "misratypes.h"
#include "timer.h"
#include "debug_port.h"


// Timer 
volatile uint8_t flg_20msec_interval;	// 20msec毎にON



volatile uint8_t timer_a_cnt;          //  20msecまでカウント　(10msec毎にカウントアップ)


//  コンペアマッチタイマ CMT0
//   10msec毎の割り込み
//

#pragma interrupt (Excep_CMT0_CMI0(vect=28))

void Excep_CMT0_CMI0(void){
	
	uint8_t	i;
	
	
	timer_a_cnt++;	       // カウントのインクリメント
	
	if ( timer_a_cnt > 1 ) {	// 20msec経過
		
		flg_20msec_interval = 1;  // 20msecフラグ ON
		
		timer_a_cnt = 0;	//  カウンターのクリア
	}
	
	
	
}

//
//    10msec タイマ(CMT0)
//    CMTユニット0のCMT0を使用。 
//
//  PCLKB(=32MHz)の128分周でカウント 32/128 = 1/4 MHz
//      1カウント = 4/1 = 4 [usec]  
//    1*10,000 usec =  N * 4 usec 
//      N = 2500


void Timer10msec_Set(void)
{	
	IPR(CMT0,CMI0) = 3;		// 割り込みレベル = 3　　（15が最高レベル)
	IEN(CMT0,CMI0) = 1;		// CMT0 割込み許可
		
	CMT0.CMCR.BIT.CKS = 0x2;        // PCLKB/128       
	CMT0.CMCOR = 2499;		// CMCNTは0からカウント 	


}


//   CMT0 タイマ開始　
//  割り込み許可してカウント開始

void Timer10msec_Start(void)
{	
	CMT0.CMCR.BIT.CMIE = 1;		// コンペアマッチ割込み　許可
		
	CMT.CMSTR0.BIT.STR0 = 1;	// CMT0 カウント開始
	
	timer_a_cnt = 0;		//  タイマのカウント値クリア
}







