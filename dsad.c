


#include "typedefine.h"
#include  "iodefine.h"
#include "misratypes.h"

#include "debug_port.h"
#include "dsad.h"
    
    
// AD data (DSAD0)

volatile uint8_t ad_ch;		// 変換チャネル: 格納されているデータがどのチャネルのA/D 変換結果かを示す。
				// ( 0=未変換またはデータ無効,　1=チャネル0のデータ,　2=チャネル1のデータ, 3=チャネル2のデータ, 4=チャネル3のデータ)
volatile uint8_t ad_err;	// 0:異常なし, 1:異常検出
volatile uint8_t ad_ovf;	// 0:正常状態(範囲内), 1:オーバフロー発生

volatile uint32_t ad_cnt;	// A/Dカウント値          
volatile int32_t ad_data;       //  A/Dデータ(2の補数形式)

volatile int32_t ad_ch0_data[1000];	// DSAD0 Ch0のデータ 

volatile uint16_t ad_index;	// データ格納位置を示す index


volatile uint32_t dsad0_scan_over;	// dsad0 ch0のスキャン完了

volatile uint32_t dsad0_collect_status;  // A/D変換データ収集ステータス ( 0:収集未完了、1:収集中、2:収集完了)


//  DSAD0 AD変換完了　割り込み
// チャンネル毎のA/D変換終了で発生。
//  16.6 msec毎に発生
//
// 複数チャンネルを変換する場合、各チャンネルは初回変換となるため、デジタルフィルタの安定時間(4T)かかる。
// 4*T=4x4=16[msec] かかる。(T=OSR/0.5 = 2048/(0.5MHz) = 4 [msec])
// (参考 :アプリケーションノート　「RX23E-Aグループ　AFE・DSADの使い方」1.2 チャネル機能を使用した複数信号のサンプリング　)
//
#pragma interrupt (Excep_DSAD0_ADI0(vect=206))
void Excep_DSAD0_ADI0(void)
{
					 
	ad_ch  = DSAD0.DR.BIT.CCH;	// 変換チャネル( 0=未変換またはデータ無効,　1=チャネル0のデータ,　2=チャネル1のデータ)
	ad_err =  DSAD0.DR.BIT.ERR;	// 0:異常なし, 1:異常検出
	ad_ovf = DSAD0.DR.BIT.OVF;	// 0:正常状態(範囲内), 1:オーバフロー発生
	
	ad_cnt = DSAD0.DR.BIT.DATA;		// A/D変換後のデータ　(32bit符号付きデータ)

	if (( ad_cnt & 0x800000 ) == 0x800000 ) {      // 24bit符号付きデータにする。
		ad_data =  ad_cnt - 16777216;		// 2^24 = 16777216
	}
	else{
		ad_data = ad_cnt;
	}
	
	if ( ad_ch == 1 ) {				// チャンネル0のデータ格納	
		ad_ch0_data[ad_index] = ad_data;
	}

	
}

// DSAD0 スキャン完了割り込み
//
#pragma interrupt (Excep_DSAD0_SCANEND0(vect=207))
void Excep_DSAD0_SCANEND0(void)
{
	dsad0_scan_over = 1;		// dsad0 スキャン完了
	
	ad_index = ad_index + 1;	// データ格納位置の更新
	
	if ( ad_index > 999 ) {	
		
		dsad0_collect_status = 2;   // A/D変換データ(N個)の収集完了
		
		ad_index = 0;		// データ格納位置初期化
	}
	
}



// DSAD0の 開始 
//
void  dsad0_start(void)
{
	DSAD0.ADST.BIT.START = 1;	// DSAD0 オートスキャン開始
	
	dsad0_scan_over = 0;
}


// DSAD0の 停止
//
void  dsad0_stop(void)
{
	 DSAD0.ADSTP.BIT.STOP = 1;	// DSAD0 オートスキャン停止
	 
	 while ( DSAD0.SR.BIT.ACT == 1 ) {    // オートスキャン実行中はループ。(オートスキャン停止待ち)
	 }
	 
}


// AFE(アナログフロントエンド)初期設定 
//
// 端子: 用途
//
//・DSAD0への入力信号設定
//  AIN4/REF1N: チャンネル0 -側
//  AIN5/REF1P: チャンネル0 +側
//
void afe_ini(void)
{
	   
     				//　DSAD0への入力信号の設定
    				//　チャネル0(回路図のラベルは Ch1)
    AFE.DS00ISR.BIT.NSEL = 4;   // AIN4:チャンネル0 -側 (Ch1_N)　			
    AFE.DS00ISR.BIT.PSEL = 5;	// AIN5:チャンネル0 +側 (Ch1_P)　
    AFE.DS00ISR.BIT.RSEL = 0x04;   // 基準電圧 +側 REFOUT(2.5V) , -側 AVSS0 ,リファレンスバッファ無効
    
    
    AFE.OPCR.BIT.TEMPSEN = 0;    // 温度センサ(TEMPS) の動作禁止
    AFE.OPCR.BIT.VREFEN = 1;	// 基準電圧源動作許可 (REFOUT 端子からVREF で生成された電圧(2.5 V) が出力) (安定まで、1msecかかる。)
    AFE.OPCR.BIT.VBIASEN = 0;   // バイアス電圧生成回路(VBIAS) の動作禁止
    AFE.OPCR.BIT.IEXCEN = 0;	// 励起電流源(IEXC)動作禁止
    
    AFE.OPCR.BIT.DSAD0EN = 1;	// DSAD0 動作許可 (このビットを“1” にしてからDSAD0 が起動するまで、400 μs 必要)
    
    AFE.OPCR.BIT.DSAD1EN = 0;	// DSAD1 動作 禁止
    
    AFE.OPCR.BIT.DSADLVM = 1;	// DSAD動作電圧選択  0: AVCC0=3.6～5.5 V, 1:AVCC0 = 2.7～5.5 V

    delay_msec(1);		// 1 msec待ち
 
}





//
// DASD0(デルタシグマ(ΔΣ)A/Dコンバータ)の初期化
//   チャンネル0   : A/D変換する
//   チャンネル1～5: A/D変換しない
//
void dsad0_ini(void){
    
    DSAD0.CCR.BIT.CLKDIV = 7;	// PCLKB/8  (DSADは、ノーマルモードでは4MHzで動作する。PCLKB=32MHzから、4MHzを生成するため8分周)
    DSAD0.CCR.BIT.LPMD = 0;	// ノーマルモード (モジュレータクロック周波数(fMOD) = 500[kHz] = 0.5[MHz] )
    
    DSAD0.MR.BIT.SCMD = 1;	// 0:連続スキャンモード, 1:シングルスキャンモード
    DSAD0.MR.BIT.SYNCST = 0;	// ユニット間(DSAD0,DSAD1)同期スタートの無効
    DSAD0.MR.BIT.TRGMD = 0;	// ソフトウェアトリガ(ADSTレジスタへの書き込みで変換開始)
    DSAD0.MR.BIT.CH0EN = 0;	// チャンネル0 A/D変換する
    DSAD0.MR.BIT.CH1EN = 1;	// チャンネル1 A/D変換しない
    DSAD0.MR.BIT.CH2EN = 1;	// チャンネル2 A/D変換しない
    DSAD0.MR.BIT.CH3EN = 1;	// チャンネル3 A/D変換しない
    DSAD0.MR.BIT.CH4EN = 1;	// チャンネル4 A/D変換しない
    DSAD0.MR.BIT.CH5EN = 1;	// チャンネル5 A/D変換しない
    
    				// チャンネル0の動作モード設定
    DSAD0.MR0.BIT.CVMD = 0;	// 通常動作
    DSAD0.MR0.BIT.SDF = 0;	// 2の補数形式(バイナリ形式) -8388608 (80 0000h) ～ +8388607(7F FFFFh)
				// DSADへの入力電圧 = (Vref * 2/Gain) * DR_DATA/(2^24) , 2^24 = 16,777,216
    
    DSAD0.MR0.BIT.OSR = 5;	// オーバーサンプリング比 = 2048
    DSAD0.MR0.BIT.DISAP = 0;	// +側入力信号断線検出アシスト なし
    DSAD0.MR0.BIT.DISAN = 0;    // -側入力信号断線検出アシスト なし
    DSAD0.MR0.BIT.AVMD = 0;	// 平均化処理なし
    DSAD0.MR0.BIT.AVDN = 0;	// 平均化データ数選択
    DSAD0.MR0.BIT.DISC = 0;     //　断線検出アシスト電流 = 0.5 [uA]
    
    
    
    // デジタルフィルタ処理時間(T)
    //    T = オーバーサンプリング比(OSR) / モジュレータクロック周波数(fMOD)
    //     OSR = 2048
    //     fMOD = 0.5 [MHz] ( ノーマルモード )
    //    T = 2048 / 0.5 = 4 [msec]
    //
    //  A/D変換時間(セトリング時間)  (マニュアル 34.3.7.2 セトリング時間)
    //    4 * T + 256[usec] = 16.3 msec
    //
    
				// チャンネル0  A/D変換回数,ゲイン設定    
    				// A/D 変換回数 N = x * 32 + y 、(CR0.CNMD = 1:即値モードの場合)
    DSAD0.CR0.BIT.CNY = 1;	// 
    DSAD0.CR0.BIT.CNX = 0;	//                                                        
    DSAD0.CR0.BIT.CNMD = 1;	// A/D変換回数演算モード ：即値モード(A/D変換回数は1～255回)
    DSAD0.CR0.BIT.GAIN =0x10;	// PGA(プログラマブルゲイン計装アンプ)有効、ゲイン=1 アナログ入力バッファ(BUF) の有効
   
    
    
    IPR(DSAD0,ADI0) = 4;	// 割り込みレベル = 4　　（15が最高レベル)
    IEN(DSAD0,ADI0) = 1;	// ADI0(A/D変換完了) 割込み許可
    
    IPR(DSAD0,SCANEND0) = 5;	// 割り込みレベル = 5　　（15が最高レベル)
    IEN(DSAD0,SCANEND0) = 1;	// スキャン完了 割込み許可
   
}




