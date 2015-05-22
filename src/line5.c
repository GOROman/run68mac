/* $Id: line5.c,v 1.2 2009-08-08 06:49:44 masamic Exp $ */

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2001/05/23 11:22:07  masamic
 * First imported source code and docs
 *
 * Revision 1.6  1999/12/21  10:08:59  yfujii
 * Uptodate source code from Beppu.
 *
 * Revision 1.5  1999/12/07  12:44:27  yfujii
 * *** empty log message ***
 *
 * Revision 1.5  1999/11/22  03:57:08  yfujii
 * Condition code calculations are rewriten.
 *
 * Revision 1.3  1999/10/20  03:55:03  masamichi
 * Added showing more information about errors.
 *
 * Revision 1.2  1999/10/18  03:24:40  yfujii
 * Added RCS keywords and modified for WIN32 a little.
 *
 */

#undef	MAIN

#include <stdio.h>
#include "run68.h"

static	int	Dbcc( char, char );
static	int	Scc( char, char );
static	int	Addq( char, char );
static	int	Subq( char, char );

/*
 �@�@�\�F5���C�����߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
int	line5( char *pc_ptr )
{
	char	code1, code2;

	code1 = *(pc_ptr++);
	code2 = *pc_ptr;
	pc += 2;

	if ( (code2 & 0xC0) == 0xC0 ) {
		if ( (code2 & 0x38) == 0x08 )
			return( Dbcc( code1, code2 ) );
		else
			return( Scc( code1, code2 ) );
	}
	if ( (code1 & 0x01) != 0 )
		return( Subq( code1, code2 ) );
	else
		return( Addq( code1, code2 ) );
}

/*
 �@�@�\�Fdbcc���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Dbcc( char code1, char code2 )
{
	char	reg;
	short	disp;
	UShort	src_data;

	reg  = (code2 & 0x07);
	disp = (short)imi_get( S_WORD );
	src_data = (rd [ reg ] & 0xFFFF);

#ifdef	TRACE
	printf( "trace: dbcc     src=%d PC=%06lX\n", (short)src_data, pc - 2 );
#endif

	if ( (BOOL)get_cond( (char)(code1 & 0x0F) ) == TRUE )
		return( FALSE );

	src_data --;
	rd [ reg ] = ((rd [ reg ] & 0xFFFF0000) | src_data);
	if ( src_data != 0xFFFF )
		pc += (disp - 2);

	return( FALSE );
}

/*
 �@�@�\�Fscc���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Scc( char code1, char code2 )
{
	char	mode;
	char	reg;
	long	save_pc;
	int	ret;
	long	src_data;

	save_pc = pc;
	mode = (code2 & 0x38) >> 3;
	reg  = (code2 & 0x07);
	ret  = get_cond( (char)(code1 & 0x0F) );

	/* �������r�b�g�����߂� */
	if ( ret == TRUE ) {
		src_data = 0xff;
	} else {
		src_data = 0;
	}

	/* �f�B�X�e�B�l�[�V�����̃A�h���b�V���O���[�h�ɉ��������� */
	if (set_data_at_ea(EA_VariableData, mode, reg, S_BYTE, src_data)) {
		return(TRUE);
	}

	return( FALSE );
}

/*
 �@�@�\�Faddq���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Addq( char code1, char code2 )
{
	char	size;
	char	mode;
	char	reg;
	char	src_data;
	short	disp = 0;
	int	work_mode;

	long	dest_data;

	src_data = (code1 & 0x0E) >> 1;
	if ( src_data == 0 )
		src_data = 8;
	size = ((code2 >> 6) & 0x03);
	mode = (code2 & 0x38) >> 3;
	reg  = (code2 & 0x07);

	if (mode == EA_AD) {
		if (size == S_BYTE) {
			err68a( "�s���Ȗ���: addq.b #<data>, An �����s���悤�Ƃ��܂����B", __FILE__, __LINE__ );
			return(TRUE);
		} else {
			/* �A�h���X���W�X�^���ڃ��[�h�̎��̃A�N�Z�X�T�C�Y�͕K�������O���[�h�ɂȂ� */
			size = S_LONG;
		}
	}

	/* �A�h���b�V���O���[�h���|�X�g�C���N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̎擾 */
	if (mode == EA_AIPI) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (get_data_at_ea_noinc(EA_Variable, work_mode, reg, size, &dest_data)) {
		return(TRUE);
	}

	/* ���[�N���W�X�^�ɃR�s�[ */
	rd[8] = dest_data;

	/* Add���Z */
	//rd [ 8 ] = add_rd( 8, (long)src_data, size );
	rd [ 8 ] = add_long((long)src_data, dest_data, size );

	/* �A�h���b�V���O���[�h���v���f�N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̐ݒ� */
	if (mode == EA_AIPD) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (set_data_at_ea(EA_Variable, work_mode, reg, size, rd[8])) {
		return(TRUE);
	}

	// �A�h���X���W�X�^���ڂ̏ꍇ�̓��W�X�^�͕ω����Ȃ�
	if (mode != EA_AD) {
		/* �t���O�̕ω� */
		add_conditions((long)src_data, dest_data, rd[8], size, 1);
	}

	return( FALSE );
}

/*
 �@�@�\�Fsubq���߂����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
static	int	Subq( char code1, char code2 )
{
	char	size;
	char	mode;
	char	reg;
	char	src_data;
	short	disp = 0;
	int	work_mode;
	long	dest_data;

	src_data = (code1 & 0x0E) >> 1;
	if ( src_data == 0 )
		src_data = 8;
	size = ((code2 >> 6) & 0x03);
	mode = (code2 & 0x38) >> 3;
	reg  = (code2 & 0x07);

	if (mode == EA_AD) {
		if (size == S_BYTE) {
			err68a( "�s���Ȗ���: subq.b #<data>, An �����s���悤�Ƃ��܂����B", __FILE__, __LINE__ );
			return(TRUE);
		} else {
			/* �A�h���X���W�X�^���ڃ��[�h�̎��̃A�N�Z�X�T�C�Y�͕K�������O���[�h�ɂȂ� */
			size = S_LONG;
		}
	}

	/* �A�h���b�V���O���[�h���|�X�g�C���N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̎擾 */
	if (mode == EA_AIPI) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (get_data_at_ea_noinc(EA_Variable, work_mode, reg, size, &dest_data)) {
		return(TRUE);
	}

	/* ���[�N���W�X�^�ɃR�s�[ */
	rd[8] = dest_data;

	/* Add���Z */
	//rd [ 8 ] = sub_rd( 8, (long)src_data, size );
	rd [ 8 ] = sub_long((long)src_data, dest_data, size );

	/* �A�h���b�V���O���[�h���v���f�N�������g�Ԑڂ̏ꍇ�͊ԐڂŃf�[�^�̐ݒ� */
	if (mode == EA_AIPD) {
		work_mode = EA_AI;
	} else {
		work_mode = mode;
	}

	if (set_data_at_ea(EA_Variable, work_mode, reg, size, rd[8])) {
		return(TRUE);
	}

	// �A�h���X���W�X�^���ڂ̏ꍇ�̓��W�X�^�͕ω����Ȃ�
	if (mode != EA_AD) {
		/* �t���O�̕ω� */
		sub_conditions((long)src_data, dest_data, rd[8], size, 1);
	}

	return( FALSE );
}
