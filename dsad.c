


#include "typedefine.h"
#include  "iodefine.h"
#include "misratypes.h"

#include "debug_port.h"
#include "dsad.h"
    
    
// AD data (DSAD0)

volatile uint8_t ad_ch;		// �ϊ��`���l��: �i�[����Ă���f�[�^���ǂ̃`���l����A/D �ϊ����ʂ��������B
				// ( 0=���ϊ��܂��̓f�[�^����,�@1=�`���l��0�̃f�[�^,�@2=�`���l��1�̃f�[�^, 3=�`���l��2�̃f�[�^, 4=�`���l��3�̃f�[�^)
volatile uint8_t ad_err;	// 0:�ُ�Ȃ�, 1:�ُ팟�o
volatile uint8_t ad_ovf;	// 0:������(�͈͓�), 1:�I�[�o�t���[����

volatile uint32_t ad_cnt;	// A/D�J�E���g�l          
volatile int32_t ad_data;       //  A/D�f�[�^(2�̕␔�`��)

volatile int32_t ad_ch0_data[1000];	// DSAD0 Ch0�̃f�[�^ 

volatile uint16_t ad_index;	// �f�[�^�i�[�ʒu������ index


volatile uint32_t dsad0_scan_over;	// dsad0 ch0�̃X�L��������

volatile uint32_t dsad0_collect_status;  // A/D�ϊ��f�[�^���W�X�e�[�^�X ( 0:���W�������A1:���W���A2:���W����)


//  DSAD0 AD�ϊ������@���荞��
// �`�����l������A/D�ϊ��I���Ŕ����B
//  16.6 msec���ɔ���
//
// �����`�����l����ϊ�����ꍇ�A�e�`�����l���͏���ϊ��ƂȂ邽�߁A�f�W�^���t�B���^�̈��莞��(4T)������B
// 4*T=4x4=16[msec] ������B(T=OSR/0.5 = 2048/(0.5MHz) = 4 [msec])
// (�Q�l :�A�v���P�[�V�����m�[�g�@�uRX23E-A�O���[�v�@AFE�EDSAD�̎g�����v1.2 �`���l���@�\���g�p���������M���̃T���v�����O�@)
//
#pragma interrupt (Excep_DSAD0_ADI0(vect=206))
void Excep_DSAD0_ADI0(void)
{
					 
	ad_ch  = DSAD0.DR.BIT.CCH;	// �ϊ��`���l��( 0=���ϊ��܂��̓f�[�^����,�@1=�`���l��0�̃f�[�^,�@2=�`���l��1�̃f�[�^)
	ad_err =  DSAD0.DR.BIT.ERR;	// 0:�ُ�Ȃ�, 1:�ُ팟�o
	ad_ovf = DSAD0.DR.BIT.OVF;	// 0:������(�͈͓�), 1:�I�[�o�t���[����
	
	ad_cnt = DSAD0.DR.BIT.DATA;		// A/D�ϊ���̃f�[�^�@(32bit�����t���f�[�^)

	if (( ad_cnt & 0x800000 ) == 0x800000 ) {      // 24bit�����t���f�[�^�ɂ���B
		ad_data =  ad_cnt - 16777216;		// 2^24 = 16777216
	}
	else{
		ad_data = ad_cnt;
	}
	
	if ( ad_ch == 1 ) {				// �`�����l��0�̃f�[�^�i�[	
		ad_ch0_data[ad_index] = ad_data;
	}

	
}

// DSAD0 �X�L�����������荞��
//
#pragma interrupt (Excep_DSAD0_SCANEND0(vect=207))
void Excep_DSAD0_SCANEND0(void)
{
	dsad0_scan_over = 1;		// dsad0 �X�L��������
	
	ad_index = ad_index + 1;	// �f�[�^�i�[�ʒu�̍X�V
	
	if ( ad_index > 999 ) {	
		
		dsad0_collect_status = 2;   // A/D�ϊ��f�[�^(N��)�̎��W����
		
		ad_index = 0;		// �f�[�^�i�[�ʒu������
	}
	
}



// DSAD0�� �J�n 
//
void  dsad0_start(void)
{
	DSAD0.ADST.BIT.START = 1;	// DSAD0 �I�[�g�X�L�����J�n
	
	dsad0_scan_over = 0;
}


// DSAD0�� ��~
//
void  dsad0_stop(void)
{
	 DSAD0.ADSTP.BIT.STOP = 1;	// DSAD0 �I�[�g�X�L������~
	 
	 while ( DSAD0.SR.BIT.ACT == 1 ) {    // �I�[�g�X�L�������s���̓��[�v�B(�I�[�g�X�L������~�҂�)
	 }
	 
}


// AFE(�A�i���O�t�����g�G���h)�����ݒ� 
//
// �[�q: �p�r
//
//�EDSAD0�ւ̓��͐M���ݒ�
//  AIN4/REF1N: �`�����l��0 -��
//  AIN5/REF1P: �`�����l��0 +��
//
void afe_ini(void)
{
	   
     				//�@DSAD0�ւ̓��͐M���̐ݒ�
    				//�@�`���l��0(��H�}�̃��x���� Ch1)
    AFE.DS00ISR.BIT.NSEL = 4;   // AIN4:�`�����l��0 -�� (Ch1_N)�@			
    AFE.DS00ISR.BIT.PSEL = 5;	// AIN5:�`�����l��0 +�� (Ch1_P)�@
    AFE.DS00ISR.BIT.RSEL = 0x04;   // ��d�� +�� REFOUT(2.5V) , -�� AVSS0 ,���t�@�����X�o�b�t�@����
    
    
    AFE.OPCR.BIT.TEMPSEN = 0;    // ���x�Z���T(TEMPS) �̓���֎~
    AFE.OPCR.BIT.VREFEN = 1;	// ��d�������싖�� (REFOUT �[�q����VREF �Ő������ꂽ�d��(2.5 V) ���o��) (����܂ŁA1msec������B)
    AFE.OPCR.BIT.VBIASEN = 0;   // �o�C�A�X�d��������H(VBIAS) �̓���֎~
    AFE.OPCR.BIT.IEXCEN = 0;	// ��N�d����(IEXC)����֎~
    
    AFE.OPCR.BIT.DSAD0EN = 1;	// DSAD0 ���싖�� (���̃r�b�g���g1�h �ɂ��Ă���DSAD0 ���N������܂ŁA400 ��s �K�v)
    
    AFE.OPCR.BIT.DSAD1EN = 0;	// DSAD1 ���� �֎~
    
    AFE.OPCR.BIT.DSADLVM = 1;	// DSAD����d���I��  0: AVCC0=3.6�`5.5 V, 1:AVCC0 = 2.7�`5.5 V

    delay_msec(1);		// 1 msec�҂�
 
}





//
// DASD0(�f���^�V�O�}(����)A/D�R���o�[�^)�̏�����
//   �`�����l��0   : A/D�ϊ�����
//   �`�����l��1�`5: A/D�ϊ����Ȃ�
//
void dsad0_ini(void){
    
    DSAD0.CCR.BIT.CLKDIV = 7;	// PCLKB/8  (DSAD�́A�m�[�}�����[�h�ł�4MHz�œ��삷��BPCLKB=32MHz����A4MHz�𐶐����邽��8����)
    DSAD0.CCR.BIT.LPMD = 0;	// �m�[�}�����[�h (���W�����[�^�N���b�N���g��(fMOD) = 500[kHz] = 0.5[MHz] )
    
    DSAD0.MR.BIT.SCMD = 1;	// 0:�A���X�L�������[�h, 1:�V���O���X�L�������[�h
    DSAD0.MR.BIT.SYNCST = 0;	// ���j�b�g��(DSAD0,DSAD1)�����X�^�[�g�̖���
    DSAD0.MR.BIT.TRGMD = 0;	// �\�t�g�E�F�A�g���K(ADST���W�X�^�ւ̏������݂ŕϊ��J�n)
    DSAD0.MR.BIT.CH0EN = 0;	// �`�����l��0 A/D�ϊ�����
    DSAD0.MR.BIT.CH1EN = 1;	// �`�����l��1 A/D�ϊ����Ȃ�
    DSAD0.MR.BIT.CH2EN = 1;	// �`�����l��2 A/D�ϊ����Ȃ�
    DSAD0.MR.BIT.CH3EN = 1;	// �`�����l��3 A/D�ϊ����Ȃ�
    DSAD0.MR.BIT.CH4EN = 1;	// �`�����l��4 A/D�ϊ����Ȃ�
    DSAD0.MR.BIT.CH5EN = 1;	// �`�����l��5 A/D�ϊ����Ȃ�
    
    				// �`�����l��0�̓��샂�[�h�ݒ�
    DSAD0.MR0.BIT.CVMD = 0;	// �ʏ퓮��
    DSAD0.MR0.BIT.SDF = 0;	// 2�̕␔�`��(�o�C�i���`��) -8388608 (80 0000h) �` +8388607(7F FFFFh)
				// DSAD�ւ̓��͓d�� = (Vref * 2/Gain) * DR_DATA/(2^24) , 2^24 = 16,777,216
    
    DSAD0.MR0.BIT.OSR = 5;	// �I�[�o�[�T���v�����O�� = 2048
    DSAD0.MR0.BIT.DISAP = 0;	// +�����͐M���f�����o�A�V�X�g �Ȃ�
    DSAD0.MR0.BIT.DISAN = 0;    // -�����͐M���f�����o�A�V�X�g �Ȃ�
    DSAD0.MR0.BIT.AVMD = 0;	// ���ω������Ȃ�
    DSAD0.MR0.BIT.AVDN = 0;	// ���ω��f�[�^���I��
    DSAD0.MR0.BIT.DISC = 0;     //�@�f�����o�A�V�X�g�d�� = 0.5 [uA]
    
    
    
    // �f�W�^���t�B���^��������(T)
    //    T = �I�[�o�[�T���v�����O��(OSR) / ���W�����[�^�N���b�N���g��(fMOD)
    //     OSR = 2048
    //     fMOD = 0.5 [MHz] ( �m�[�}�����[�h )
    //    T = 2048 / 0.5 = 4 [msec]
    //
    //  A/D�ϊ�����(�Z�g�����O����)  (�}�j���A�� 34.3.7.2 �Z�g�����O����)
    //    4 * T + 256[usec] = 16.3 msec
    //
    
				// �`�����l��0  A/D�ϊ���,�Q�C���ݒ�    
    				// A/D �ϊ��� N = x * 32 + y �A(CR0.CNMD = 1:���l���[�h�̏ꍇ)
    DSAD0.CR0.BIT.CNY = 1;	// 
    DSAD0.CR0.BIT.CNX = 0;	//                                                        
    DSAD0.CR0.BIT.CNMD = 1;	// A/D�ϊ��񐔉��Z���[�h �F���l���[�h(A/D�ϊ��񐔂�1�`255��)
    DSAD0.CR0.BIT.GAIN =0x10;	// PGA(�v���O���}�u���Q�C���v���A���v)�L���A�Q�C��=1 �A�i���O���̓o�b�t�@(BUF) �̗L��
   
    
    
    IPR(DSAD0,ADI0) = 4;	// ���荞�݃��x�� = 4�@�@�i15���ō����x��)
    IEN(DSAD0,ADI0) = 1;	// ADI0(A/D�ϊ�����) �����݋���
    
    IPR(DSAD0,SCANEND0) = 5;	// ���荞�݃��x�� = 5�@�@�i15���ō����x��)
    IEN(DSAD0,SCANEND0) = 1;	// �X�L�������� �����݋���
   
}




