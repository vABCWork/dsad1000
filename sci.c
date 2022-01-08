
#include "iodefine.h"
#include "misratypes.h"

#include "sci.h"
#include "dsad.h"
//
//  SCI1 �V���A���ʐM(��������)����
//

// ��M�p
volatile uint8_t  rcv_data[128];
volatile uint8_t rxdata;
volatile uint8_t rcv_over;
volatile uint8_t  rcv_cnt;

// ���M�p
volatile uint8_t sd_data[4004];
volatile uint8_t  send_cnt;
volatile uint8_t  send_pt;


#pragma interrupt (Excep_SCI1_RXI1(vect=219))
//
// ��M���荞��
//
void Excep_SCI1_RXI1(void)
{
	uint8_t cmd;
	
	rxdata = SCI1.RDR;	// ��M�f�[�^�ǂݏo��
 	
 	rcv_data[rcv_cnt] = rxdata;
	
	rcv_cnt++;
	
	LED_RX_PODR = 1;	        // ��M LED�̓_��
	
	cmd = rcv_data[0];
				
			            // DSAD A/D�ϊ� �J�n�A��ԓǂݏo���A���W�f�[�^�ǂݏo��
	if (( cmd == 0x40 )||( cmd == 0x41)|| ( cmd == 0x42)) {		
 		if ( rcv_cnt == 4 ) {   //  ���v 4 byte ��M�ŁA��M����
		  rcv_over = 1;
 		}
	}
	
}



#pragma interrupt (Excep_SCI1_TEI1(vect=221))

//
// ���M�I�����荞��
//
void Excep_SCI1_TEI1(void)
{	 
	SCI1.SCR.BIT.TE = 0;            // ���M�֎~
        SCI1.SCR.BIT.TIE = 0;           // ���M���荞�݋֎~	        
	SCI1.SCR.BIT.TEIE = 0;  	// TEI���荞��(���M�I�����荞��)�֎~

	LED_TX_PODR = 0;	        // ���M LED�̏���
	
 }
 
 
 
// �R�}���h��M�̑΂���A�R�}���h�����ƃ��X�|���X�쐬����
//
// 0x40 :DSAD A/D�ϊ��f�[�^(N��)�̎��W�J�n�R�}���h
// 0x41 :DSAD A/D�ϊ��f�[�^(N��)�̎��W��ԓǂݏo���R�}���h
// 0x42 :DSAD  A/D�ϊ��f�[�^(N��)�̎��W�f�[�^�ǂݏo���R�}���h
//   (N = 1000)

void comm_cmd(void){
   
	uint8_t  cmd;
	uint32_t sd_cnt;

	cmd = rcv_data[0];
        
	sd_cnt = 0;
	
	if ( cmd == 0x40 ) {	 // DSAD A/D�ϊ��f�[�^(N��)�̎��W�J�n�R�}���h
	    
	     sd_cnt = resp_dsad_collect_start();
	}
	
	else if ( cmd == 0x41 ) {	 // DSAD A/D�ϊ��f�[�^(N��)�̎��W ��ԓǂݏo���R�}���h

	    sd_cnt = resp_dsad_collect_status();
	}
	
	else if ( cmd == 0x42 ) {	 // DSAD A/D�ϊ��f�[�^(N��)�̎��W�f�[�^�ǂݏo���R�}���h

	    sd_cnt = resp_dsad_collect_read();
	}
	
	
	
	DMAC1.DMSAR = &sd_data[0];	 // �]�����A�h���X		
	DMAC1.DMDAR = &SCI1.TDR;	 // �]����A�h���X TXD12 ���M�f�[�^

	DMAC1.DMCRA = sd_cnt; 	 	// �]���� (���M�o�C�g��)	
	    
	DMAC1.DMCNT.BIT.DTE = 1;    // DMAC1 (DMAC �`�����l��1) �]������
	
	    			   // ��ԍŏ��̑��M���荞��(TXI)�𔭐������鏈���B ( RX23E-A ���[�U�[�Y�}�j���A���@�n�[�h�E�F�A�ҁ@28.3.7 �V���A���f�[�^�̑��M�i�������������[�h�j)�@
	SCI1.SCR.BIT.TIE = 1;      // ���M���荞�݋���
	SCI1.SCR.BIT.TE = 1;	   // ���M����
	
	LED_TX_PODR = 1;	   // ���M LED�̓_��
	
	
}



 // DSAD A/D�ϊ��f�[�^(N��)�̎��W�J�n�R�}���h�̃��X�|���X�쐬
 //  ��M�f�[�^
//  rcv_data[0]: 0x40 (DSAD A/D�ϊ��f�[�^(N��)�̎��W�J�n�R�}���h)
//  rcv_data[1]: dummy
//  rcv_data[2]: dummy 
//  rcv_data[3]: dummy
//
//   ���M�f�[�^ :
//  0: sd_data[0] : 0xc0 (DSAD A/D�ϊ��f�[�^(N��)�̎��W�J�n�R�}���h�ɑ΂��郌�X�|���X)
//     se_data[1] : dummy
//     sd_data[2] : dummy
//     sd_data[3] : dummy
//

uint32_t resp_dsad_collect_start(void) {

	uint32_t cnt;
	
	dsad0_collect_status = 1;	// A/D�ϊ��f�[�^(N��)�̎��W�J�n(���W��)
	ad_index =0;			// A/D�ϊ��f�[�^�i�[�ʒu�̏�����
	
	cnt = 4;			// ���M�o�C�g��
	
	sd_data[0] = 0xc0;	 	// DSAD A/D�ϊ��f�[�^(N��)�̎��W�J�n�R�}���h�ɑ΂��郌�X�|���X	
	sd_data[1] = 0;
	sd_data[2] = 0;			
	sd_data[3] = 0;
	
	
	return cnt;
	
}



 // DSAD A/D�ϊ��f�[�^(N��)�̎��W ��ԓǂݏo���R�}���h�̃��X�|���X�쐬
 //  ��M�f�[�^
//  rcv_data[0]: 0x41 (DSAD A/D�ϊ��f�[�^(N��)�̎��W�@��ԓǂݏo���R�}���h)
//  rcv_data[1]: dummy
//  rcv_data[2]: dummy 
//  rcv_data[3]: dummy
//
//   ���M�f�[�^ :
//  0: sd_data[0] : 0xc1 (��M�R�}���h�ɑ΂��郌�X�|���X)
//     se_data[1] : dsad0_collect_status (0:���W�������A1:���W���A2:���W����)
//     sd_data[2] : ad_index (b0-b7)
//     sd_data[3] : ad_index (b8-b15)
//

uint32_t resp_dsad_collect_status(void) {

	uint32_t cnt;

	cnt = 4;			// ���M�o�C�g��
	
	sd_data[0] = 0xc1;	 	   // DSAD A/D�ϊ��f�[�^(N��)�̎��W�J�n�R�}���h�ɑ΂��郌�X�|���X	
	sd_data[1] = dsad0_collect_status; // 0:���W�������A1:���W���A2:���W����
	sd_data[2] = ad_index;			
	sd_data[3] = (ad_index >> 8);
	
	return cnt;
}


// DSAD A/D�ϊ��f�[�^(N��)�̓ǂݏo���R�}���h
//
//  ��M�f�[�^
//  rcv_data[0];�@0x42 (DSAD A/D�@���W�f�[�^�̓ǂݏo���R�}���h)
//  rcv_data[1]: 
//  rcv_data[2]:  
//  rcv_data[3]:  dummy
//
//   ���M�f�[�^ :
//  0: sd_data[0] : 0xc2 (�R�}���h�ɑ΂��郌�X�|���X)
//     se_data[1] : dummy
//     sd_data[2] : dummy
//     sd_data[3] : dummy
//  1: sd_data[4] : ad_ch0_data[0]�̍ŉ��ʃo�C�g 
//     sd_data[5] :
//     sd_data[6] :    
//     sd_data[7] : ad_ch0_data[0]�̍ŏ�ʃo�C�g
//  2: sd_data[8] : ad_ch0_data[1]�̍ŉ��ʃo�C�g   
//     sd_data[9] :
//     sd_data[10] :    
//     sd_data[11] : ad_ch0_data[1]�̍ŏ�ʃo�C�g
//          :
//          :
//          :
//100: sd_data[400] : ad_ch0_data[99]�̍ŉ��ʃo�C�g   
//     sd_data[401] :
//     sd_data[402] :    
//     sd_data[403] : ad_ch0_data[99]�̍ŏ�ʃo�C�g
//          :
//          :
//4000: sd_data[4000]: sd_ch0_data[3999]�̍ŉ��ʃo�C�g   
//      sd_data[4001]:
//      sd_data[4002]:
//      sd_data[4003]: ad_ch0_data[3999]�̍ŏ�ʃo�C�g
//
uint32_t	resp_dsad_collect_read(void)
{

	uint32_t cnt;
	
	dsad0_collect_status = 0;	// �ǂݏo���R�}���h�œǂݏo���ς݂ɂȂ邽�߁A���W�������Ƃ���B
	
	sd_data[0] = 0xc2;	 	// �R�}���h�ɑ΂��郌�X�|���X	
	sd_data[1] = 0;
	sd_data[2] = 0;			
	sd_data[3] = 0;
	
	memcpy( &sd_data[4], &ad_ch0_data[0],4000);    // ad1_calib_ch0[0]���� 4000byte�� sd_data[4]�� 94[usec]

	cnt = 4004;
	
	return cnt;
}




// 
// SCI11 �����ݒ�
//  39.4kbps,   8bit-non parity-1stop
//  PCLKB = 32MHz
//  TXD1= P16,  RXD1 = P15
//
//   BRR�̒l(N):
//   SEMR ABCS (����������{�N���b�N�Z���N�g�r�b�g) = 0
//        BGDM (�{�[���[�g�W�F�l���[�^�{�����[�h�Z���N�g�r�b�g) = 0 �̏ꍇ
//
//    N= (32 x 1000000/(64/2)xB)-1
//�@�@�@�@B: �{�[���[�g bps
//        
// ��1)    B=38.4 kbps�Ƃ���B�@32x38.4K = 1228.8 K	
//     32000 K / 1228.8 K= 26.04	
//     N= 26 - 1 = 25
//
// 38.4Kbps:
//     SCI12.BRR = 25
//     SCI12.SEMR.BIT.BGDM = 0
//
//
// ��2)  B=76.8 kbps�Ƃ���B�@32x76.8K = 2457.6 K	
//     32000 K / 2457.6 K= 13.02	
//     N= 13 - 1 = 12
//
// 76.8Kbps:
//     SCI12.BRR = 12
//     SCI12.SEMR.BIT.BGDM = 0
//
//
// ��2)  B=153.6 kbps�Ƃ���B�@32x153.6K = 4915.2 K	
//     32000 K / 4915.2 K= 6.5	
//     N= 6 - 1 = 5
//
// 76.8Kbps:
//     SCI12.BRR = 5
//     SCI12.SEMR.BIT.BGDM = 0
//

void initSCI_1(void)
{
	
	MPC.PWPR.BIT.B0WI = 0;   // �}���`�t�@���N�V�����s���R���g���[���@�v���e�N�g����
	MPC.PWPR.BIT.PFSWE = 1;  // PmnPFS ���C�g�v���e�N�g����
	
	MPC.P30PFS.BYTE = 0x0A;  // P30 = RXD1
	MPC.P26PFS.BYTE = 0x0A;  // P26 = TXD1
	
	
	MPC.PWPR.BYTE = 0x80;    //  PmnPFS ���C�g�v���e�N�g �ݒ�
			
	PORT3.PMR.BIT.B0 = 1;	// P30 ���Ӄ��W���[���Ƃ��Ďg�p
	PORT2.PMR.BIT.B6 = 1;   // P26 ���Ӄ��W���[���Ƃ��Ďg�p
		
	
	SCI1.SCR.BYTE = 0;	// �����{�[���[�g�W�F�l���[�^�A����M�֎~
	SCI1.SMR.BYTE = 0;	// PCLKB(=27MHz), ��������,8bit,parity �Ȃ�,1stop
	
	//SCI1.BRR = 25;		// 38.4K 
	
	SCI1.BRR = 12;			// 76.8K  
	SCI1.SEMR.BIT.BGDM = 0;     
	SCI1.SEMR.BIT.ABCS = 0;
	
	
	//SCI12.SEMR.BIT.BGDM = 1;        // �{�����[�h �� 153.6 Kbps 
	//SCI12.SEMR.BIT.ABCS = 0;
	
	//SCI12.SEMR.BIT.ABCS = 1;	// ��{�N���b�N8�T�C�N���̊��Ԃ�1�r�b�g���Ԃ̓]�����[�g �ŁA307.2 Kbps

	
	SCI1.SCR.BIT.TIE = 0;		// TXI���荞�ݗv���� �֎~
	SCI1.SCR.BIT.RIE = 1;		// RXI�����ERI���荞�ݗv���� ����
	SCI1.SCR.BIT.TE = 0;		// �V���A�����M����� �֎~�@�i������ TE=1�ɂ���ƁA��ԍŏ��̑��M���荞�݂��������Ȃ�)
	SCI1.SCR.BIT.RE = 1;		// �V���A����M����� ����
	
	SCI1.SCR.BIT.MPIE = 0;         // (�������������[�h�ŁASMR.MP�r�b�g= 1�̂Ƃ��L��)
	SCI1.SCR.BIT.TEIE = 0;         // TEI���荞�ݗv�����֎~
	SCI1.SCR.BIT.CKE = 0;          // �����{�[���[�g�W�F�l���[�^
	
	
	IPR(SCI1,RXI1) = 12;		// ��M ���荞�݃��x�� = 12�i15���ō����x��)
	IEN(SCI1,RXI1) = 1;		// ��M���荞�݋���
	
	IPR(SCI1,TXI1) = 12;		// ���M ���荞�݃��x�� = 12 �i15���ō����x��)   
	IEN(SCI1,TXI1) = 1;		// ���M���荞�݋���
	
	IPR(SCI1,TEI1) = 12;		// ���M���� ���荞�݃��x�� = 12 �i15���ō����x��)
	IEN(SCI1,TEI1) = 1;		// ���M�������荞�݋���
	
	rcv_cnt = 0;			// ��M�o�C�g���̏�����
	
	
}


//  ���M���Ǝ�M����LED�p�@�o�̓|�[�g�ݒ�
 void LED_comm_port_set(void)	
 {
	 				// ���M�@�\���pLED
	  LED_TX_PMR = 0;		// �ėp���o�̓|�[�g
	  LED_TX_PDR = 1;		// �o�̓|�[�g�Ɏw��
	  
	 				// ��M�@�\���pLED
	  LED_RX_PMR = 0;		// �ėp���o�̓|�[�g
	  LED_RX_PDR = 1;		// �o�̓|�[�g�Ɏw��
 }

