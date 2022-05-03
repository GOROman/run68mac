/* $Id: line0.c,v 1.2 2009-08-08 06:49:44 masamic Exp $ */

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2001/05/23 11:22:07  masamic
 * First imported source code and docs
 *
 * Revision 1.6  1999/12/21  10:08:59  yfujii
 * Uptodate source code from Beppu.
 *
 * Revision 1.5  1999/12/07  12:43:24  yfujii
 * *** empty log message ***
 *
 * Revision 1.5  1999/11/22  03:57:08  yfujii
 * Condition code calculations are rewriten.
 *
 * Revision 1.3  1999/10/20  02:39:39  masamichi
 * Add showing more information about errors.
 *
 * Revision 1.2  1999/10/18  03:24:40  yfujii
 * Added RCS keywords and modified for WIN32 a little.
 *
 */

#undef	MAIN

#include <stdio.h>
#include "run68.h"

static	int	Ori( char );
static	int	Ori_t_ccr( void );
static	int	Ori_t_sr( void );
static	int	Andi( char );
static	int	Andi_t_ccr( void );
static	int	Andi_t_sr( void );
static	int	Addi( char );
static	int	Subi( char );
static	int	Eori( char );
static	int	Eori_t_ccr( void );
static	int	Cmpi( char );
static	int	Btsti( char );
static	int	Btst( char, char );
static	int	Bchgi( char );
static	int	Bchg( char, char );
static	int	Bclri( char );
static	int	Bclr( char, char );
static	int	Bseti( char );
static	int	Bset( char, char );
static	int	Movep_f( char, char );
static	int	Movep_t( char, char );

/*
 �@�@�\�F0���C�����߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
int	line0( char *pc_ptr )
{
	char	code1, code2;

	code1 = *(pc_ptr++);
	code2 = *pc_ptr;
	pc += 2;

	switch( code1 ) {
		case 0x00:
			if ( code2 == 0x3C )
				return( Ori_t_ccr() );
			else if ( code2 == 0x7C )
				return( Ori_t_sr() );
			else
				return( Ori( code2 ) );
		case 0x02:
			if ( code2 == 0x3C )
				return( Andi_t_ccr() );
			else if ( code2 == 0x7C )
				return( Andi_t_sr() );
			else
				return( Andi( code2 ) );
		case 0x04:
			return( Subi( code2 ) );
		case 0x06:
			return( Addi( code2 ) );
		case 0x08:
			switch(code2 & 0xC0) {
				case 0x00:
					return( Btsti( code2 ) );
				case 0x40:
					return( Bchgi( code2 ) );
				case 0x80:
					return( Bclri( code2 ) );
				default:	/* 0xC0 */
					return( Bseti( code2 ) );
			}
		case 0x0A:
			if ( code2 == 0x3C )
				return( Eori_t_ccr() );
			if ( code2 == 0x7C ) {	/* eori to SR */
				err68a( "����`���߂����s���܂���", __FILE__, __LINE__ );
				return( TRUE );
			}
			return( Eori( code2 ) );
		case 0x0C:
			return( Cmpi( code2 ) );
		default:
			if ((code2 & 0x38) == 0x08) {
				if ( (code2 & 0x80) != 0 )
					return( Movep_f( code1, code2 ) );
				else
					return( Movep_t( code1, code2 ) );
			}
			switch(code2 & 0xC0) {
				case 0x00:
					return( Btst( code1, code2 ) );
				case 0x40:
					return( Bchg( code1, code2 ) );
				case 0x80:
					return( Bclr( code1, code2 ) );
				default:	/* 0xC0 */
					return( Bset( code1, code2 ) );
			}
	}
}

/*
 �@�@�\�Fori���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Ori( char code )
{
	Long	src_data;
	char	mode;
	char	reg;
	char	size;
	Long	save_pc;
	int	work_mode;
	Long	data;

	save_pc = pc;
	size = ((code >> 6) & 0x03);
	if ( size == 3 ) {
		err68a( "�s���ȃA�N�Z�X�T�C�Y�ł�", __FILE__, __LINE__ );
		return( TRUE );
	}
	mode = (code & 0x38) >> 3;
	reg  = (code & 0x07);
	src_data = imi_get( size );

	/* �A�h���b�V���O���[�h���|�X�g�C���N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̎擾 */
	if (mode == EA_AIPI) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (get_data_at_ea_noinc(EA_VariableData, work_mode, reg, size, &data)) {
		return(TRUE);
	}

	/* OR���Z */
	data |= src_data;

	/* �A�h���b�V���O���[�h���v���f�N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̐ݒ� */
	if (mode == EA_AIPD) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (set_data_at_ea(EA_VariableData, work_mode, reg, size, data)) {
		return(TRUE);
	}

	/* �t���O�̃Z�b�g */
	general_conditions(data, size);

	return( FALSE );
}

/*
 �@�@�\�Fori to CCR���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Ori_t_ccr()
{
	char	data;

	data = (char)imi_get( S_BYTE );

#ifdef	TRACE
	printf( "trace: ori_t_ccr src=0x%02X PC=%06lX\n", data, pc - 2 );
#endif

	/* CCR���Z�b�g */
	if ( (data & 0x10) != 0 )
		CCR_X_ON();
	if ( (data & 0x08) != 0 )
		CCR_N_ON();
	if ( (data & 0x04) != 0 )
		CCR_Z_ON();
	if ( (data & 0x02) != 0 )
		CCR_V_ON();
	if ( (data & 0x01) != 0 )
		CCR_C_ON();

	return( FALSE );
}

/*
 �@�@�\�Fori to SR���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Ori_t_sr()
{
	short	data;

	if ( SR_S_REF() == 0 ) {
		err68a( "�������߂����s���܂���", __FILE__, __LINE__ );
		return( TRUE );
	}

	data = (short)imi_get( S_WORD );

#ifdef	TRACE
	printf( "trace: ori_t_sr src=0x%02X PC=%06lX\n", data, pc - 2 );
#endif

	/* SR���Z�b�g */
	sr |= data;

	return( FALSE );
}

/*
 �@�@�\�Fandi���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Andi( char code )
{
	Long	src_data;
	char	mode;
	char	reg;
	char	size;
	Long	save_pc;
	Long	work_mode;
	Long	data;

	save_pc = pc;
	size = ((code >> 6) & 0x03);
	if ( size == 3 ) {
		err68a( "�s���ȃA�N�Z�X�T�C�Y�ł��B", __FILE__, __LINE__ );
		return( TRUE );
	}
	mode = (code & 0x38) >> 3;
	reg  = (code & 0x07);

	src_data = imi_get( size );

	/* �A�h���b�V���O���[�h���|�X�g�C���N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̎擾 */
	if (mode == EA_AIPI) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (get_data_at_ea_noinc(EA_VariableData, work_mode, reg, size, &data)) {
		return(TRUE);
	}

	/* AND���Z */
	data &= src_data;

	/* �A�h���b�V���O���[�h���v���f�N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̐ݒ� */
	if (mode == EA_AIPD) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (set_data_at_ea(EA_VariableData, work_mode, reg, size, data)) {
		return(TRUE);
	}

	/* �t���O�̃Z�b�g */
	general_conditions(data, size);

	return( FALSE );
}

/*
 �@�@�\�Fandi to CCR���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Andi_t_ccr()
{
	char	data;

	data = (char)imi_get( S_BYTE );

#ifdef	TRACE
	printf( "trace: andi_t_ccr src=0x%02X PC=%06lX\n", data, pc - 2 );
#endif

	/* CCR���Z�b�g */
	if ( (data & 0x10) == 0 )
		CCR_X_OFF();
	if ( (data & 0x08) == 0 )
		CCR_N_OFF();
	if ( (data & 0x04) == 0 )
		CCR_Z_OFF();
	if ( (data & 0x02) == 0 )
		CCR_V_OFF();
	if ( (data & 0x01) == 0 )
		CCR_C_OFF();

	return( FALSE );
}

/*
 �@�@�\�Fandi to SR���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Andi_t_sr()
{
	short	data;

	if ( SR_S_REF() == 0 ) {
		err68a( "�������߂����s���܂���", __FILE__, __LINE__ );
		return( TRUE );
	}

	data = (short)imi_get( S_WORD );

#ifdef	TRACE
	printf( "trace: andi_t_sr src=0x%02X PC=%06lX\n", data, pc - 2 );
#endif

	/* SR���Z�b�g */
	sr &= data;

	return( FALSE );
}

/*
 �@�@�\�Faddi���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Addi( char code )
{
	Long	src_data;
	char	mode;
	char	reg;
	char	size;
	Long	save_pc;
	int	work_mode;
	Long	dest_data;

#ifdef TEST_CCR
	short before;
#endif

	save_pc = pc;
	size = ((code >> 6) & 0x03);
	if ( size == 3 ) {
		err68a( "�s���ȃA�N�Z�X�T�C�Y�ł��B", __FILE__, __LINE__ );
		return( TRUE );
	}
	mode = (code & 0x38) >> 3;
	reg  = (code & 0x07);

	src_data = imi_get( size );

	/* �A�h���b�V���O���[�h���|�X�g�C���N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̎擾 */
	if (mode == EA_AIPI) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (get_data_at_ea_noinc(EA_VariableData, work_mode, reg, size, &dest_data)) {
		return(TRUE);
	}

#ifdef TEST_CCR
	before = sr & 0x1f;
#endif

	/* ���[�N���W�X�^�փR�s�[ */
	rd[8] = dest_data;

	/* Add���Z */
	// rd [ 8 ] = add_rd( 8, src_data, size );
	rd [ 8 ] = add_long(src_data, dest_data, size );

	/* �A�h���b�V���O���[�h���v���f�N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̐ݒ� */
	if (mode == EA_AIPD) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (set_data_at_ea(EA_VariableData, work_mode, reg, size, rd[8])) {
		return(TRUE);
	}

	/* �t���O�̕ω� */
	add_conditions(src_data, dest_data, rd[8], size, 1);

#ifdef TEST_CCR
	check("addi", src_data, dest_data, rd[8], size, before);
#endif

	return( FALSE );
}

/*
 �@�@�\�Fsubi���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Subi( char code )
{
	Long	src_data;
	char	mode;
	char	reg;
	char	size;
	Long	save_pc;
	int	work_mode;
	Long	dest_data;

#ifdef TEST_CCR
	short before;
#endif

	save_pc = pc;
	size = ((code >> 6) & 0x03);
	if ( size == 3 ) {
		err68a( "�s���ȃA�N�Z�X�T�C�Y�ł��B", __FILE__, __LINE__ );
		return( TRUE );
	}
	mode = (code & 0x38) >> 3;
	reg  = (code & 0x07);

	src_data = imi_get( size );

	/* �A�h���b�V���O���[�h���|�X�g�C���N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̎擾 */
	if (mode == EA_AIPI) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (get_data_at_ea_noinc(EA_VariableData, work_mode, reg, size, &dest_data)) {
		return(TRUE);
	}

#ifdef TEST_CCR
	before = sr & 0x1f;
#endif

	/* ���[�N���W�X�^�փR�s�[ */
	rd[8] = dest_data;

	/* Sub���Z */
	//rd [ 8 ] = sub_rd( 8, src_data, size );
	rd [ 8 ] = sub_long(src_data, dest_data, size );

	/* �A�h���b�V���O���[�h���v���f�N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̐ݒ� */
	if (mode == EA_AIPD) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (set_data_at_ea(EA_VariableData, work_mode, reg, size, rd[8])) {
		return(TRUE);
	}

	/* �t���O�̕ω� */
	sub_conditions(src_data, dest_data, rd[8], size, 1);

#ifdef TEST_CCR
	check("subi", src_data, dest_data, rd[8], size, before);
#endif

	return( FALSE );
}

/*
 �@�@�\�Feori���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Eori( char code )
{
	char	size;
	char	mode;
	char	reg;
	Long	data;
	Long	src_data;
	Long	save_pc;
	Long	work_mode;

	save_pc = pc;
	size = ((code >> 6) & 0x03);
	mode = ((code & 0x38) >> 3);
	reg  = (code & 0x07);

	src_data = imi_get( size );

	/* �A�h���b�V���O���[�h���|�X�g�C���N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̎擾 */
	if (mode == EA_AIPI) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (get_data_at_ea_noinc(EA_VariableData, work_mode, reg, size, &data)) {
		return(TRUE);
	}

	/* Eor���Z */
	data ^= src_data;

	/* �A�h���b�V���O���[�h���v���f�N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̐ݒ� */
	if (mode == EA_AIPD) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (set_data_at_ea(EA_VariableData, work_mode, reg, size, data)) {
		return(TRUE);
	}

	/* �t���O�̃Z�b�g */
	general_conditions(data, size);

	return( FALSE );
}

/*
 �@�@�\�Feori to CCR���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Eori_t_ccr()
{
	char	data;

	data = (char)imi_get( S_BYTE );

#ifdef	TRACE
	printf( "trace: eori_t_ccr src=0x%02X PC=%06lX\n", data, pc - 2 );
#endif

	/* CCR���Z�b�g */
	if ( (data & 0x10) != 0 ) {
		if ( CCR_X_REF() == 0 )
			CCR_X_ON();
		else
			CCR_X_OFF();
	}
	if ( (data & 0x08) != 0 ) {
		if ( CCR_N_REF() == 0 )
			CCR_N_ON();
		else
			CCR_N_OFF();
	}
	if ( (data & 0x04) != 0 ) {
		if ( CCR_Z_REF() == 0 )
			CCR_Z_ON();
		else
			CCR_Z_OFF();
	}
	if ( (data & 0x02) != 0 ) {
		if ( CCR_V_REF() == 0 )
			CCR_V_ON();
		else
			CCR_V_OFF();
	}
	if ( (data & 0x01) != 0 ) {
		if ( CCR_C_REF() == 0 )
			CCR_C_ON();
		else
			CCR_C_OFF();
	}

	return( FALSE );
}

/*
 �@�@�\�Fcmpi���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Cmpi( char code )
{
	char	mode;
	char	reg;
	char	size;
	Long	src_data;
	Long	save_pc;
	short	save_x;
	Long	dest_data;

#ifdef TEST_CCR
	short	before;
	Long	result;
#endif

	save_pc = pc;
	size = ((code >> 6) & 0x03);
	if ( size == 3 ) {
		err68a( "�s���ȃA�N�Z�X�T�C�Y�ł��B", __FILE__, __LINE__ );
		return( TRUE );
	}
	mode = (code & 0x38) >> 3;
	reg  = (code & 0x07);
	save_x = CCR_X_REF();

	src_data = imi_get( size );

	if (get_data_at_ea(EA_VariableData, mode, reg, size, &dest_data)) {
		return(TRUE);
	}

	/* ���[�N���W�X�^�փR�s�[ */
	rd[8] = dest_data;

#ifdef TEST_CCR
	before = sr & 0x1f;
#endif

	/* Sub���Z */
	// rd[8] = sub_rd( 8, src_data, size );
	rd[8] = sub_long(src_data, dest_data, size );

	if ( save_x == 0 )
		CCR_X_OFF();
	else
		CCR_X_ON();

	/* �t���O�̕ω� */
	cmp_conditions(src_data, dest_data, rd[8], size);

#ifdef TEST_CCR
	check("cmpi", src_data, dest_data, rd[8], size, before);
#endif

	return( FALSE );
}

/*
 �@�@�\�Fbtst #data,<ea>���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Btsti( char code )
{
	Long	save_pc;
	char	mode;
	char	reg;
	UChar	bitno;
	Long	data;
	Long	mask = 1;
	int	size;

	save_pc = pc;
	mode = (code & 0x38) >> 3;
	reg = (code & 0x07);
	bitno = (UChar)imi_get( S_BYTE );
	if ( mode == MD_DD ) {
		bitno = (bitno % 32);
		size  = S_LONG;
	} else {
		bitno = (bitno % 8);
		size  = S_BYTE;
	}

	mask <<= bitno;

	/* �����A�h���X�Ŏ����ꂽ�f�[�^���擾 */
	if (get_data_at_ea(EA_Data, mode, reg, size, &data)) {
		return(TRUE);
	}

	/* Z�t���O�ɔ��f */
	if ( (data & mask) == 0 )
		CCR_Z_ON();
	else
		CCR_Z_OFF();

#ifdef	TRACE
	printf( "trace: btst     src=%d PC=%06lX\n", bitno, save_pc );
#endif

	return( FALSE );
}

/*
 �@�@�\�Fbtst Dn,<ea>���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Btst( char code1, char code2 )
{
	Long	save_pc;
	char	mode;
	char	reg;
	UChar	bitno;
	Long	data;
	Long	mask = 1;
	int	size;

	save_pc = pc;
	mode = (code2 & 0x38) >> 3;
	reg = (code2 & 0x07);
	bitno = ((code1 >> 1) & 0x07);
	bitno = (UChar)(rd [ bitno ]);

	if ( mode == MD_DD ) {
		bitno = (bitno % 32);
		size  = S_LONG;
	} else {
		bitno = (bitno % 8);
		size  = S_BYTE;
	}

	mask <<= bitno;

	/* �����A�h���X�Ŏ����ꂽ�f�[�^���擾 */
	if (get_data_at_ea(EA_Data, mode, reg, size, &data)) {
		return(TRUE);
	}

	/* Z�t���O�ɔ��f */
	if ( (data & mask) == 0 )
		CCR_Z_ON();
	else
		CCR_Z_OFF();

#ifdef	TRACE
	printf( "trace: btst     src=%d PC=%06lX\n", bitno, save_pc );
#endif

	return( FALSE );
}

/*
 �@�@�\�Fbchg #data,<ea>���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Bchgi( char code )
{
	Long	save_pc;
	char	mode;
	char	reg;
	UChar	bitno;
	Long	mask = 1;
	int	size;
	int	work_mode;
	Long	data;

	save_pc = pc;
	mode = (code & 0x38) >> 3;
	reg = (code & 0x07);
	bitno = (UChar)imi_get( S_BYTE );

	if ( mode == MD_DD ) {
		bitno = (bitno % 32);
		size  = S_LONG;
	} else {
		bitno = (bitno % 8);
		size  = S_BYTE;
	}

	mask <<= bitno;

	/* �A�h���b�V���O���[�h���|�X�g�C���N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̎擾 */
	if (mode == EA_AIPI) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (get_data_at_ea_noinc(EA_VariableData, work_mode, reg, size, &data)) {
		return(TRUE);
	}

	/* bchg���Z */
	if ( (data & mask) == 0 ) {
		CCR_Z_ON();
		data |= mask;
	} else {
		CCR_Z_OFF();
		data &= ~mask;
	}

	/* �A�h���b�V���O���[�h���v���f�N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̐ݒ� */
	if (mode == EA_AIPD) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (set_data_at_ea(EA_VariableData, work_mode, reg, size, data)) {
		return(TRUE);
	}

	return( FALSE );
}

/*
 �@�@�\�Fbchg Dn,<ea>���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Bchg( char code1, char code2 )
{
	Long	save_pc;
	char	mode;
	char	reg;
	UChar	bitno;
	Long	data;
	Long	mask = 1;
	int	size;
	int	work_mode;

	save_pc = pc;
	mode = (code2 & 0x38) >> 3;
	reg = (code2 & 0x07);
	bitno = ((code1 >> 1) & 0x07);
	bitno = (UChar)(rd [ bitno ]);

	if ( mode == MD_DD ) {
		bitno = (bitno % 32);
		size  = S_LONG;
	} else {
		bitno = (bitno % 8);
		size  = S_BYTE;
	}

	mask <<= bitno;

	/* �A�h���b�V���O���[�h���|�X�g�C���N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̎擾 */
	if (mode == EA_AIPI) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (get_data_at_ea_noinc(EA_VariableData, work_mode, reg, size, &data)) {
		return(TRUE);
	}

	/* bchg���Z */
	if ( (data & mask) == 0 ) {
		CCR_Z_ON();
		data |= mask;
	} else {
		CCR_Z_OFF();
		data &= ~mask;
	}

	/* �A�h���b�V���O���[�h���v���f�N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̐ݒ� */
	if (mode == EA_AIPD) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (set_data_at_ea(EA_VariableData, work_mode, reg, size, data)) {
		return(TRUE);
	}

	return( FALSE );
}

/*
 �@�@�\�Fbclr #data,<ea>���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Bclri( char code )
{
	Long	save_pc;
	char	mode;
	char	reg;
	UChar	bitno;
	short	disp = 0;
	Long	data;
	Long	mask = 1;
	int	size;
	int	work_mode;

	save_pc = pc;
	mode = (code & 0x38) >> 3;
	reg = (code & 0x07);
	bitno = (UChar)imi_get( S_BYTE );

	if ( mode == MD_DD ) {
		bitno = (bitno % 32);
		size  = S_LONG;
	} else {
		bitno = (bitno % 8);
		size  = S_BYTE;
	}

	mask <<= bitno;

	/* �A�h���b�V���O���[�h���|�X�g�C���N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̎擾 */
	if (mode == EA_AIPI) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (get_data_at_ea_noinc(EA_VariableData, work_mode, reg, size, &data)) {
		return(TRUE);
	}

	/* bclr���Z */
	if ( (data & mask) == 0 ) {
		CCR_Z_ON();
	} else {
		CCR_Z_OFF();
		data &= ~mask;
	}

	/* �A�h���b�V���O���[�h���v���f�N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̐ݒ� */
	if (mode == EA_AIPD) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (set_data_at_ea(EA_VariableData, work_mode, reg, size, data)) {
		return(TRUE);
	}

	return( FALSE );
}

/*
 �@�@�\�Fbclr Dn,<ea>���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Bclr( char code1, char code2 )
{
	Long	save_pc;
	char	mode;
	char	reg;
	UChar	bitno;
	short	disp = 0;
	Long	data;
	Long	mask = 1;
	int	size;
	int	work_mode;

	save_pc = pc;
	mode = (code2 & 0x38) >> 3;
	reg = (code2 & 0x07);
	bitno = ((code1 >> 1) & 0x07);
	bitno = (UChar)(rd [ bitno ]);

	if ( mode == MD_DD ) {
		bitno = (bitno % 32);
		size  = S_LONG;
	} else {
		bitno = (bitno % 8);
		size  = S_BYTE;
	}

	mask <<= bitno;

	/* �A�h���b�V���O���[�h���|�X�g�C���N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̎擾 */
	if (mode == EA_AIPI) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (get_data_at_ea_noinc(EA_VariableData, work_mode, reg, size, &data)) {
		return(TRUE);
	}

	/* bclr���Z */
	if ( (data & mask) == 0 ) {
		CCR_Z_ON();
	} else {
		CCR_Z_OFF();
		data &= ~mask;
	}

	/* �A�h���b�V���O���[�h���v���f�N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̐ݒ� */
	if (mode == EA_AIPD) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (set_data_at_ea(EA_VariableData, work_mode, reg, size, data)) {
		return(TRUE);
	}

	return( FALSE );
}

/*
 �@�@�\�Fbset #data,<ea>���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Bseti( char code )
{
	Long	save_pc;
	char	mode;
	char	reg;
	UChar	bitno;
	Long	data;
	short	disp = 0;
	ULong	mask = 1;
	int	size;
	int	work_mode;

	save_pc = pc;
	mode = (code & 0x38) >> 3;
	reg = (code & 0x07);
	bitno = (UChar)imi_get( S_BYTE );

	if ( mode == MD_DD ) {
		bitno = (bitno % 32);
		size  = S_LONG;
	} else {
		bitno = (bitno % 8);
		size  = S_BYTE;
	}

	mask <<= bitno;

	/* �A�h���b�V���O���[�h���|�X�g�C���N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̎擾 */
	if (mode == EA_AIPI) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (get_data_at_ea_noinc(EA_VariableData, work_mode, reg, size, &data)) {
		return(TRUE);
	}

	/* bset���Z */
	if ( (data & mask) == 0 ) {
		CCR_Z_ON();
		data |= mask;
	} else {
		CCR_Z_OFF();
	}

	/* �A�h���b�V���O���[�h���v���f�N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̐ݒ� */
	if (mode == EA_AIPD) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (set_data_at_ea(EA_VariableData, work_mode, reg, size, data)) {
		return(TRUE);
	}

	return( FALSE );
}

/*
 �@�@�\�Fbset Dn,<ea>���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Bset( char code1, char code2 )
{
	Long	save_pc;
	char	mode;
	char	reg;
	UChar	bitno;
	Long	data;
	short	disp = 0;
	ULong	mask = 1;
	int	size;
	int	work_mode;

	save_pc = pc;
	mode = (code2 & 0x38) >> 3;
	reg = (code2 & 0x07);
	bitno = ((code1 >> 1) & 0x07);
	bitno = (UChar)(rd [ bitno ]);

	if ( mode == MD_DD ) {
		bitno = (bitno % 32);
		size  = S_LONG;
	} else {
		bitno = (bitno % 8);
		size  = S_BYTE;
	}

	mask <<= bitno;

	/* �A�h���b�V���O���[�h���|�X�g�C���N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̎擾 */
	if (mode == EA_AIPI) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (get_data_at_ea_noinc(EA_VariableData, work_mode, reg, size, &data)) {
		return(TRUE);
	}

	/* bset���Z */
	if ( (data & mask) == 0 ) {
		CCR_Z_ON();
		data |= mask;
	} else {
		CCR_Z_OFF();
	}

	/* �A�h���b�V���O���[�h���v���f�N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̐ݒ� */
	if (mode == EA_AIPD) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (set_data_at_ea(EA_VariableData, work_mode, reg, size, data)) {
		return(TRUE);
	}

	return( FALSE );
}

/*
 �@�@�\�Fmovep from Dn���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Movep_f( char code1, char code2 )
{
	char	d_reg;
	char	a_reg;
	short	disp;
	Long	adr;

	d_reg = ((code1 >> 1) & 0x07);
	a_reg = (code2 & 0x07);
	disp = (UChar)imi_get( S_WORD );
	adr = ra [ a_reg ] + disp;

	if ( (code2 & 0x40) != 0 ) {
		/* LONG */
		mem_set( adr, ((rd [ d_reg ] >> 24) & 0xFF), S_BYTE );
		mem_set( adr + 2, ((rd [ d_reg ] >> 16) & 0xFF), S_BYTE );
		mem_set( adr + 4, ((rd [ d_reg ] >> 8) & 0xFF), S_BYTE );
		mem_set( adr + 6, rd [ d_reg ] & 0xFF, S_BYTE );
	} else {
		/* WORD */
		mem_set( adr, ((rd [ d_reg ] >> 8) & 0xFF), S_BYTE );
		mem_set( adr + 2, rd [ d_reg ] & 0xFF, S_BYTE );
	}

#ifdef	TRACE
	printf( "trace: movep_f  src=%d PC=%06lX\n", rd [ d_reg ], pc - 2 );
#endif

	return( FALSE );
}

/*
 �@�@�\�Fmovep to Dn���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Movep_t( char code1, char code2 )
{
	char	d_reg;
	char	a_reg;
	short	disp;
	ULong	data;
	Long	adr;

	d_reg = ((code1 >> 1) & 0x07);
	a_reg = (code2 & 0x07);
	disp = (UChar)imi_get( S_WORD );
	adr = ra [ a_reg ] + disp;

	data = mem_get( adr, S_BYTE );
	data = ((data << 8) | (mem_get( adr + 2, S_BYTE ) & 0xFF));
	if ( (code2 & 0x40) != 0 ) {	/* LONG */
		data = ((data << 8) | (mem_get( adr + 4, S_BYTE ) & 0xFF));
		data = ((data << 8) | (mem_get( adr + 6, S_BYTE ) & 0xFF));
		rd [ d_reg ] = data;
	} else {
		rd [ d_reg ] = ((rd [ d_reg ] & 0xFFFF0000) | (data & 0xFFFF));
	}

#ifdef	TRACE
	printf( "trace: movep_t  PC=%06lX\n", pc - 2 );
#endif

	return( FALSE );
}
