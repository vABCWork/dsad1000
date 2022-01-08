#include	<machine.h>
#include	 "iodefine.h"
#include	 "misratypes.h"
#include	"debug_port.h"
#include	 "timer.h"
#include	"dsad.h"
#include	"sci.h"
#include 	"dma.h"

void clear_module_stop(void);
void  delay_msec( uint32_t  n_msec );
static void debug_port_ini(void);

void main(void)
{
	debug_port_ini();	// P31を出力ポートに設定 (デバック用)
	
	clear_module_stop();	//  モジュールストップの解除
 
	Timer10msec_Set();	//  10msec タイマ(CMT0)設定
	Timer10msec_Start();	//  10msec タイマカウント開始
	
	afe_ini();		// AFE(アナログフロントエンド)設定
	dsad0_ini();		// DASD0の設定　
	ad_index = 0;		// A/D変換データの格納位置クリア

	
	DMA1_ini();           	 	// PCへのシリアルデータ送信用のDMA処理　初期設定	
	initSCI_1();			// SCI 1初期設定 76.8K
	LED_comm_port_set();		// 送信時のLEDポート設定
	
	
	while(1) {
	   if ( dsad0_collect_status == 1 ) {	// A/D変換データ(N個)の収集開始(収集中)の場合
	   
	   	if ( flg_20msec_interval == 1 ) { // 20msec経過した場合
            	
		    flg_20msec_interval = 0;	// 20msec経過フラグのクリア
		
		    dsad0_start();			// DSAD0開始
 	
		    DEBUG_0_PODR = 1;		// P31 = ON
	       }

	       if ( dsad0_scan_over == 1 ) {	// dsad0 スキャン完了の場合
	   	
	   	   dsad0_stop();			// DSAD0停止
		
		   DEBUG_0_PODR = 0;		// P31 = OFF
	      }
	   }
	   
	   if ( rcv_over == 1 ) {		// コマンド受信
  	
		comm_cmd();			// レスポンス作成、送信
	   	rcv_over = 0;			// コマンド受信フラグのクリア
		rcv_cnt  = 0;			//  受信バイト数の初期
		LED_RX_PODR = 0;		// 受信 LEDの消灯
	   }
	   
	   
	}

}



// モジュールストップの解除
//   CMTユニット0(CMT0, CMT1)
//   アナログフロントエンド(AFE)
//   24ビットΔ-Σ A/D コンバータ(DSAD0) ユニット0
//   DMA コントローラ(DMACA)
//  シリアルコミュニケーションインタフェース(SCI)
//
void clear_module_stop(void)
{
	SYSTEM.PRCR.WORD = 0xA50F;	// クロック発生、消費電力低減機能関連レジスタの書き込み許可	
	
	MSTP(CMT0) = 0;			// コンペアマッチタイマ(CMT) ユニット0(CMT0, CMT1) モジュールストップの解除
	MSTP(AFE) = 0;			// アナログフロントエンド(AFE) モジュールストップの解除
	MSTP(DSAD0) = 0;		// 24 ビットΔ-Σ A/D コンバータ(DSAD0) ユニット0 モジュールストップの解除
	
	MSTP(DMAC) = 0;                //  DMA モジュールストップ解除
	MSTP(SCI1) = 0;	        	// SCI1 モジュールストップの解除
	
	SYSTEM.PRCR.WORD = 0xA500;	// クロック発生、消費電力低減機能関連レジスタ書き込み禁止
}




//   N [msec] 待つ関数 (ICLK = 32MHz)
//   命令語は、 RX V2 アーキテクチャ
//    コンパイル時　最適化レベル = 2   (-optimize = 2 )
//   n_msec=  1:  1msec
//      

#pragma instalign4 delay_msec

void  delay_msec( unsigned long  n_msec )
{
	unsigned long delay_cnt;

	while( n_msec-- ) {

	   delay_cnt = 10656;
		
	   while(delay_cnt --) 
	   { 			 
	   }

	}
}



//   デバックポートの設定 
//   (debug_port.h)

static void debug_port_ini(void)
{	
        DEBUG_0_PMR = 0;    //  P31 汎用入出力ポート
	DEBUG_0_PDR = 1;    //  出力ポートに指定
	
}
