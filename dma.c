
#include "iodefine.h"
#include "misratypes.h"
#include "dma.h"

// DMA転送終了割り込み  
// DMAC DMAC1I
#pragma interrupt (Excep_DMAC_DMAC1I(vect=199))
void Excep_DMAC_DMAC1I(void)

{
	        
	SCI1.SCR.BIT.TEIE = 1;  	// TEI割り込み(送信終了割り込み)許可 (全データ送信完了で発生)
}



//
// DMA チャンネル1 初期設定 (パソコンへのシリアルデータ送信用)
//

void DMA1_ini(void) {
	
	DMAC.DMAST.BIT.DMST =0;     // DMAC 停止
	
	DMAC1.DMCNT.BIT.DTE = 0;    // DMAC1 (DMAC チャンネル1) 転送禁止
	
	ICU.DMRSR1 = 220;           // DMA起動要因　TXI1（割り込みベクタ番号=220）  SCI1 送信割り込みは、DMA1で処理
	
	
	DMAC1.DMAMD.WORD = 0x8000;  // 転送元=インクリメント、転送先=アドレス固定
	DMAC1.DMTMD.WORD = 0x2001;  // ノーマル転送、リピート、ブロック領域なし、8bit転送、周辺モジュールからの割り込みにより開始
	
	DMAC1.DMINT.BIT.DTIE = 1;   // 指定した回数のデータ転送が終了したときの転送終了割り込み要求を許可
	
	IPR(DMAC,DMAC1I) = 9;		// 転送終了割り込みレベル = 9
	IEN(DMAC,DMAC1I) = 1;		// 転送終了割り込み許可
	
	DMAC.DMAST.BIT.DMST =1;     // DMAC 起動
	
	
}
