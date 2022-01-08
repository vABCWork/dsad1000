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
	debug_port_ini();	// P31���o�̓|�[�g�ɐݒ� (�f�o�b�N�p)
	
	clear_module_stop();	//  ���W���[���X�g�b�v�̉���
 
	Timer10msec_Set();	//  10msec �^�C�}(CMT0)�ݒ�
	Timer10msec_Start();	//  10msec �^�C�}�J�E���g�J�n
	
	afe_ini();		// AFE(�A�i���O�t�����g�G���h)�ݒ�
	dsad0_ini();		// DASD0�̐ݒ�@
	ad_index = 0;		// A/D�ϊ��f�[�^�̊i�[�ʒu�N���A

	
	DMA1_ini();           	 	// PC�ւ̃V���A���f�[�^���M�p��DMA�����@�����ݒ�	
	initSCI_1();			// SCI 1�����ݒ� 76.8K
	LED_comm_port_set();		// ���M����LED�|�[�g�ݒ�
	
	
	while(1) {
	   if ( dsad0_collect_status == 1 ) {	// A/D�ϊ��f�[�^(N��)�̎��W�J�n(���W��)�̏ꍇ
	   
	   	if ( flg_20msec_interval == 1 ) { // 20msec�o�߂����ꍇ
            	
		    flg_20msec_interval = 0;	// 20msec�o�߃t���O�̃N���A
		
		    dsad0_start();			// DSAD0�J�n
 	
		    DEBUG_0_PODR = 1;		// P31 = ON
	       }

	       if ( dsad0_scan_over == 1 ) {	// dsad0 �X�L���������̏ꍇ
	   	
	   	   dsad0_stop();			// DSAD0��~
		
		   DEBUG_0_PODR = 0;		// P31 = OFF
	      }
	   }
	   
	   if ( rcv_over == 1 ) {		// �R�}���h��M
  	
		comm_cmd();			// ���X�|���X�쐬�A���M
	   	rcv_over = 0;			// �R�}���h��M�t���O�̃N���A
		rcv_cnt  = 0;			//  ��M�o�C�g���̏���
		LED_RX_PODR = 0;		// ��M LED�̏���
	   }
	   
	   
	}

}



// ���W���[���X�g�b�v�̉���
//   CMT���j�b�g0(CMT0, CMT1)
//   �A�i���O�t�����g�G���h(AFE)
//   24�r�b�g��-�� A/D �R���o�[�^(DSAD0) ���j�b�g0
//   DMA �R���g���[��(DMACA)
//  �V���A���R�~���j�P�[�V�����C���^�t�F�[�X(SCI)
//
void clear_module_stop(void)
{
	SYSTEM.PRCR.WORD = 0xA50F;	// �N���b�N�����A����d�͒ጸ�@�\�֘A���W�X�^�̏������݋���	
	
	MSTP(CMT0) = 0;			// �R���y�A�}�b�`�^�C�}(CMT) ���j�b�g0(CMT0, CMT1) ���W���[���X�g�b�v�̉���
	MSTP(AFE) = 0;			// �A�i���O�t�����g�G���h(AFE) ���W���[���X�g�b�v�̉���
	MSTP(DSAD0) = 0;		// 24 �r�b�g��-�� A/D �R���o�[�^(DSAD0) ���j�b�g0 ���W���[���X�g�b�v�̉���
	
	MSTP(DMAC) = 0;                //  DMA ���W���[���X�g�b�v����
	MSTP(SCI1) = 0;	        	// SCI1 ���W���[���X�g�b�v�̉���
	
	SYSTEM.PRCR.WORD = 0xA500;	// �N���b�N�����A����d�͒ጸ�@�\�֘A���W�X�^�������݋֎~
}




//   N [msec] �҂֐� (ICLK = 32MHz)
//   ���ߌ�́A RX V2 �A�[�L�e�N�`��
//    �R���p�C�����@�œK�����x�� = 2   (-optimize = 2 )
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



//   �f�o�b�N�|�[�g�̐ݒ� 
//   (debug_port.h)

static void debug_port_ini(void)
{	
        DEBUG_0_PMR = 0;    //  P31 �ėp���o�̓|�[�g
	DEBUG_0_PDR = 1;    //  �o�̓|�[�g�Ɏw��
	
}