
#include "iodefine.h"
#include "misratypes.h"

#include "sci.h"
#include "dsad.h"
//
//  SCI1 シリアル通信(調歩同期)処理
//

// 受信用
volatile uint8_t  rcv_data[128];
volatile uint8_t rxdata;
volatile uint8_t rcv_over;
volatile uint8_t  rcv_cnt;

// 送信用
volatile uint8_t sd_data[4004];
volatile uint8_t  send_cnt;
volatile uint8_t  send_pt;


#pragma interrupt (Excep_SCI1_RXI1(vect=219))
//
// 受信割り込み
//
void Excep_SCI1_RXI1(void)
{
	uint8_t cmd;
	
	rxdata = SCI1.RDR;	// 受信データ読み出し
 	
 	rcv_data[rcv_cnt] = rxdata;
	
	rcv_cnt++;
	
	LED_RX_PODR = 1;	        // 受信 LEDの点灯
	
	cmd = rcv_data[0];
				
			            // DSAD A/D変換 開始、状態読み出し、収集データ読み出し
	if (( cmd == 0x40 )||( cmd == 0x41)|| ( cmd == 0x42)) {		
 		if ( rcv_cnt == 4 ) {   //  合計 4 byte 受信で、受信完了
		  rcv_over = 1;
 		}
	}
	
}



#pragma interrupt (Excep_SCI1_TEI1(vect=221))

//
// 送信終了割り込み
//
void Excep_SCI1_TEI1(void)
{	 
	SCI1.SCR.BIT.TE = 0;            // 送信禁止
        SCI1.SCR.BIT.TIE = 0;           // 送信割り込み禁止	        
	SCI1.SCR.BIT.TEIE = 0;  	// TEI割り込み(送信終了割り込み)禁止

	LED_TX_PODR = 0;	        // 送信 LEDの消灯
	
 }
 
 
 
// コマンド受信の対する、コマンド処理とレスポンス作成処理
//
// 0x40 :DSAD A/D変換データ(N個)の収集開始コマンド
// 0x41 :DSAD A/D変換データ(N個)の収集状態読み出しコマンド
// 0x42 :DSAD  A/D変換データ(N個)の収集データ読み出しコマンド
//   (N = 1000)

void comm_cmd(void){
   
	uint8_t  cmd;
	uint32_t sd_cnt;

	cmd = rcv_data[0];
        
	sd_cnt = 0;
	
	if ( cmd == 0x40 ) {	 // DSAD A/D変換データ(N個)の収集開始コマンド
	    
	     sd_cnt = resp_dsad_collect_start();
	}
	
	else if ( cmd == 0x41 ) {	 // DSAD A/D変換データ(N個)の収集 状態読み出しコマンド

	    sd_cnt = resp_dsad_collect_status();
	}
	
	else if ( cmd == 0x42 ) {	 // DSAD A/D変換データ(N個)の収集データ読み出しコマンド

	    sd_cnt = resp_dsad_collect_read();
	}
	
	
	
	DMAC1.DMSAR = &sd_data[0];	 // 転送元アドレス		
	DMAC1.DMDAR = &SCI1.TDR;	 // 転送先アドレス TXD12 送信データ

	DMAC1.DMCRA = sd_cnt; 	 	// 転送回数 (送信バイト数)	
	    
	DMAC1.DMCNT.BIT.DTE = 1;    // DMAC1 (DMAC チャンネル1) 転送許可
	
	    			   // 一番最初の送信割り込み(TXI)を発生させる処理。 ( RX23E-A ユーザーズマニュアル　ハードウェア編　28.3.7 シリアルデータの送信（調歩同期式モード）)　
	SCI1.SCR.BIT.TIE = 1;      // 送信割り込み許可
	SCI1.SCR.BIT.TE = 1;	   // 送信許可
	
	LED_TX_PODR = 1;	   // 送信 LEDの点灯
	
	
}



 // DSAD A/D変換データ(N個)の収集開始コマンドのレスポンス作成
 //  受信データ
//  rcv_data[0]: 0x40 (DSAD A/D変換データ(N個)の収集開始コマンド)
//  rcv_data[1]: dummy
//  rcv_data[2]: dummy 
//  rcv_data[3]: dummy
//
//   送信データ :
//  0: sd_data[0] : 0xc0 (DSAD A/D変換データ(N個)の収集開始コマンドに対するレスポンス)
//     se_data[1] : dummy
//     sd_data[2] : dummy
//     sd_data[3] : dummy
//

uint32_t resp_dsad_collect_start(void) {

	uint32_t cnt;
	
	dsad0_collect_status = 1;	// A/D変換データ(N個)の収集開始(収集中)
	ad_index =0;			// A/D変換データ格納位置の初期化
	
	cnt = 4;			// 送信バイト数
	
	sd_data[0] = 0xc0;	 	// DSAD A/D変換データ(N個)の収集開始コマンドに対するレスポンス	
	sd_data[1] = 0;
	sd_data[2] = 0;			
	sd_data[3] = 0;
	
	
	return cnt;
	
}



 // DSAD A/D変換データ(N個)の収集 状態読み出しコマンドのレスポンス作成
 //  受信データ
//  rcv_data[0]: 0x41 (DSAD A/D変換データ(N個)の収集　状態読み出しコマンド)
//  rcv_data[1]: dummy
//  rcv_data[2]: dummy 
//  rcv_data[3]: dummy
//
//   送信データ :
//  0: sd_data[0] : 0xc1 (受信コマンドに対するレスポンス)
//     se_data[1] : dsad0_collect_status (0:収集未完了、1:収集中、2:収集完了)
//     sd_data[2] : ad_index (b0-b7)
//     sd_data[3] : ad_index (b8-b15)
//

uint32_t resp_dsad_collect_status(void) {

	uint32_t cnt;

	cnt = 4;			// 送信バイト数
	
	sd_data[0] = 0xc1;	 	   // DSAD A/D変換データ(N個)の収集開始コマンドに対するレスポンス	
	sd_data[1] = dsad0_collect_status; // 0:収集未完了、1:収集中、2:収集完了
	sd_data[2] = ad_index;			
	sd_data[3] = (ad_index >> 8);
	
	return cnt;
}


// DSAD A/D変換データ(N個)の読み出しコマンド
//
//  受信データ
//  rcv_data[0];　0x42 (DSAD A/D　収集データの読み出しコマンド)
//  rcv_data[1]: 
//  rcv_data[2]:  
//  rcv_data[3]:  dummy
//
//   送信データ :
//  0: sd_data[0] : 0xc2 (コマンドに対するレスポンス)
//     se_data[1] : dummy
//     sd_data[2] : dummy
//     sd_data[3] : dummy
//  1: sd_data[4] : ad_ch0_data[0]の最下位バイト 
//     sd_data[5] :
//     sd_data[6] :    
//     sd_data[7] : ad_ch0_data[0]の最上位バイト
//  2: sd_data[8] : ad_ch0_data[1]の最下位バイト   
//     sd_data[9] :
//     sd_data[10] :    
//     sd_data[11] : ad_ch0_data[1]の最上位バイト
//          :
//          :
//          :
//100: sd_data[400] : ad_ch0_data[99]の最下位バイト   
//     sd_data[401] :
//     sd_data[402] :    
//     sd_data[403] : ad_ch0_data[99]の最上位バイト
//          :
//          :
//4000: sd_data[4000]: sd_ch0_data[3999]の最下位バイト   
//      sd_data[4001]:
//      sd_data[4002]:
//      sd_data[4003]: ad_ch0_data[3999]の最上位バイト
//
uint32_t	resp_dsad_collect_read(void)
{

	uint32_t cnt;
	
	dsad0_collect_status = 0;	// 読み出しコマンドで読み出し済みになるため、収集未完了とする。
	
	sd_data[0] = 0xc2;	 	// コマンドに対するレスポンス	
	sd_data[1] = 0;
	sd_data[2] = 0;			
	sd_data[3] = 0;
	
	memcpy( &sd_data[4], &ad_ch0_data[0],4000);    // ad1_calib_ch0[0]から 4000byteを sd_data[4]へ 94[usec]

	cnt = 4004;
	
	return cnt;
}




// 
// SCI11 初期設定
//  39.4kbps,   8bit-non parity-1stop
//  PCLKB = 32MHz
//  TXD1= P16,  RXD1 = P15
//
//   BRRの値(N):
//   SEMR ABCS (調歩同期基本クロックセレクトビット) = 0
//        BGDM (ボーレートジェネレータ倍速モードセレクトビット) = 0 の場合
//
//    N= (32 x 1000000/(64/2)xB)-1
//　　　　B: ボーレート bps
//        
// 例1)    B=38.4 kbpsとする。　32x38.4K = 1228.8 K	
//     32000 K / 1228.8 K= 26.04	
//     N= 26 - 1 = 25
//
// 38.4Kbps:
//     SCI12.BRR = 25
//     SCI12.SEMR.BIT.BGDM = 0
//
//
// 例2)  B=76.8 kbpsとする。　32x76.8K = 2457.6 K	
//     32000 K / 2457.6 K= 13.02	
//     N= 13 - 1 = 12
//
// 76.8Kbps:
//     SCI12.BRR = 12
//     SCI12.SEMR.BIT.BGDM = 0
//
//
// 例2)  B=153.6 kbpsとする。　32x153.6K = 4915.2 K	
//     32000 K / 4915.2 K= 6.5	
//     N= 6 - 1 = 5
//
// 76.8Kbps:
//     SCI12.BRR = 5
//     SCI12.SEMR.BIT.BGDM = 0
//

void initSCI_1(void)
{
	
	MPC.PWPR.BIT.B0WI = 0;   // マルチファンクションピンコントローラ　プロテクト解除
	MPC.PWPR.BIT.PFSWE = 1;  // PmnPFS ライトプロテクト解除
	
	MPC.P30PFS.BYTE = 0x0A;  // P30 = RXD1
	MPC.P26PFS.BYTE = 0x0A;  // P26 = TXD1
	
	
	MPC.PWPR.BYTE = 0x80;    //  PmnPFS ライトプロテクト 設定
			
	PORT3.PMR.BIT.B0 = 1;	// P30 周辺モジュールとして使用
	PORT2.PMR.BIT.B6 = 1;   // P26 周辺モジュールとして使用
		
	
	SCI1.SCR.BYTE = 0;	// 内蔵ボーレートジェネレータ、送受信禁止
	SCI1.SMR.BYTE = 0;	// PCLKB(=27MHz), 調歩同期,8bit,parity なし,1stop
	
	//SCI1.BRR = 25;		// 38.4K 
	
	SCI1.BRR = 12;			// 76.8K  
	SCI1.SEMR.BIT.BGDM = 0;     
	SCI1.SEMR.BIT.ABCS = 0;
	
	
	//SCI12.SEMR.BIT.BGDM = 1;        // 倍速モード で 153.6 Kbps 
	//SCI12.SEMR.BIT.ABCS = 0;
	
	//SCI12.SEMR.BIT.ABCS = 1;	// 基本クロック8サイクルの期間が1ビット期間の転送レート で、307.2 Kbps

	
	SCI1.SCR.BIT.TIE = 0;		// TXI割り込み要求を 禁止
	SCI1.SCR.BIT.RIE = 1;		// RXIおよびERI割り込み要求を 許可
	SCI1.SCR.BIT.TE = 0;		// シリアル送信動作を 禁止　（ここで TE=1にすると、一番最初の送信割り込みが発生しない)
	SCI1.SCR.BIT.RE = 1;		// シリアル受信動作を 許可
	
	SCI1.SCR.BIT.MPIE = 0;         // (調歩同期式モードで、SMR.MPビット= 1のとき有効)
	SCI1.SCR.BIT.TEIE = 0;         // TEI割り込み要求を禁止
	SCI1.SCR.BIT.CKE = 0;          // 内蔵ボーレートジェネレータ
	
	
	IPR(SCI1,RXI1) = 12;		// 受信 割り込みレベル = 12（15が最高レベル)
	IEN(SCI1,RXI1) = 1;		// 受信割り込み許可
	
	IPR(SCI1,TXI1) = 12;		// 送信 割り込みレベル = 12 （15が最高レベル)   
	IEN(SCI1,TXI1) = 1;		// 送信割り込み許可
	
	IPR(SCI1,TEI1) = 12;		// 送信完了 割り込みレベル = 12 （15が最高レベル)
	IEN(SCI1,TEI1) = 1;		// 送信完了割り込み許可
	
	rcv_cnt = 0;			// 受信バイト数の初期化
	
	
}


//  送信時と受信時のLED用　出力ポート設定
 void LED_comm_port_set(void)	
 {
	 				// 送信　表示用LED
	  LED_TX_PMR = 0;		// 汎用入出力ポート
	  LED_TX_PDR = 1;		// 出力ポートに指定
	  
	 				// 受信　表示用LED
	  LED_RX_PMR = 0;		// 汎用入出力ポート
	  LED_RX_PDR = 1;		// 出力ポートに指定
 }

