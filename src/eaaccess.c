/* $Id: eaaccess.c,v 1.3 2009-08-08 06:49:44 masamic Exp $ */

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2009/08/05 14:44:33  masamic
 * Some Bug fix, and implemented some instruction
 * Following Modification contributed by TRAP.
 *
 * Fixed Bug: In disassemble.c, shift/rotate as{lr},ls{lr},ro{lr} alway show word size.
 * Modify: enable KEYSNS, register behaiviour of sub ea, Dn.
 * Add: Nbcd, Sbcd.
 *
 * Revision 1.1.1.1  2001/05/23 11:22:07  masamic
 * First imported source code and docs
 *
 * Revision 1.6  2000/01/09  06:49:20  yfujii
 * Push/Pop instruction's word alignment is adjusted.
 *
 * Revision 1.3  1999/12/07  12:42:08  yfujii
 * *** empty log message ***
 *
 * Revision 1.3  1999/11/04  09:05:57  yfujii
 * Wrong addressing mode selection problem is fixed.
 *
 * Revision 1.1  1999/11/01  10:36:33  masamichi
 * Initial revision
 *
 *
 */

/* Get from / Set to Effective Address */

#include "run68.h"

/*
 * �y�����z
 *   �����A�h���X���擾����B
 *
 * �y�֐������z
 *   retcode = get_ea(save_pc, AceptAdrMode, mode, reg, &data);
 *
 * �y�����z
 *   long save_pc;      <in>  PC���Ύ��̊�ƂȂ�PC�l
 *   int  AceptAdrMode; <in>  �A�h���b�V���O���[�h MD_??
 *   int  mode;         <in>  �A�h���b�V���O���[�h MD_??
 *   int  reg;          <in>  ���W�X�^�ԍ��܂��̓A�h���b�V���O���[�h�@MR_??
 *   long *data;        <out> �擾����f�[�^���i�[����ꏊ�ւ̃|�C���^
 *
 * �y�Ԓl�z
 *   TURE:  �G���[
 *   FALSE: ����
 *
 */

BOOL get_ea(long save_pc, int AceptAdrMode, int mode, int reg, long *data)
{
	short	disp;
	long	idx;
	BOOL	retcode = FALSE;

	/* ���삵�₷���悤�Ƀ��[�h�𓝍� */
	int gmode = (mode < 7) ? mode : (7 + reg);	/* gmode = 0-11 */

	/* AceptAdrMode �ŋ����ꂽ�A�h���b�V���O���[�h�łȂ���΃G���[ */

	if ((AceptAdrMode & (1 << gmode)) == 0) {

		err68a( "�A�h���b�V���O���[�h���ُ�ł��B", __FILE__, __LINE__ );
		return TRUE;

	}

	/* �A�h���b�V���O���[�h�ɉ��������� */
	switch (gmode) {
		case EA_AI:
			*data = ra [ reg ];
			break;
		case EA_AID:
			disp = (short)imi_get( S_WORD );
			*data = ra [ reg ] + (int)disp;
			break;
		case EA_AIX:
			idx = idx_get();
			*data = ra [ reg ] + idx;
			break;
		case EA_SRT:
			idx = imi_get( S_WORD );
			if ( (idx & 0x8000) != 0 )
				idx |= 0xFFFF0000;
			*data = idx;
			break;
		case EA_LNG:
			*data = imi_get( S_LONG );
			break;
		case EA_PC:
			disp = (short)imi_get( S_WORD );
			*data = save_pc + (int)disp;
			break;
		case EA_PCX:
			idx = idx_get();
			*data = save_pc + idx;
			break;
		default:
			err68a( "�A�h���b�V���O���[�h���ُ�ł��B", __FILE__, __LINE__ );
			retcode = TRUE;
	}
	return( retcode );
}

/* Get Data at Effective Address */

/*
 * �y�����z
 *   �����A�h���X�Ŏ����ꂽ�l���擾����B
 *
 * �y�֐������z
 *   retcode = get_data_at_ea(AceptAdrMode, mode, reg, &data);
 *
 * �y�����z
 *   int AceptAdrMode; <in>  �����\�ȃA�h���b�V���O���[�h�Q EA_????*
 *   int mode;         <in>  �A�h���b�V���O���[�h MD_??
 *   int reg;          <in>  ���W�X�^�ԍ��܂��̓A�h���b�V���O���[�h�@MR_??
 *   long *data;       <out> �擾����f�[�^���i�[����ꏊ�ւ̃|�C���^
 *
 * �y�Ԓl�z
 *   TURE:  �G���[
 *   FALSE: ����
 *
 */

BOOL get_data_at_ea(int AceptAdrMode, int mode, int reg, int size, long *data)
{
	short	disp;
	long	idx;
	BOOL	retcode;
	int	gmode;
	long	save_pc;

	save_pc = pc;
	retcode = FALSE;

	/* ���삵�₷���悤�Ƀ��[�h�𓝍� */
	gmode = mode < 7 ? mode : 7 + reg;	/* gmode = 0-11 */

	/* AceptAdrMode �ŋ����ꂽ�A�h���b�V���O���[�h�łȂ���΃G���[ */

	if ((AceptAdrMode & (1 << gmode)) == 0) {

		err68a( "�A�h���b�V���O���[�h���ُ�ł��B", __FILE__, __LINE__ );
		retcode = TRUE;

	} else {

		/* �A�h���b�V���O���[�h�ɉ��������� */
		switch (gmode) {
			case EA_DD:
				switch( size ) {
					case S_BYTE:
						*data = (rd [ reg ] & 0xFF);
						break;
					case S_WORD:
						*data = (rd [ reg ] & 0xFFFF);
						break;
					case S_LONG:
						*data = rd [ reg ];
						break;
				}
				break;
			case EA_AD:
				switch( size ) {
					case S_BYTE:
						*data = (ra [ reg ] & 0xFF);
						break;
					case S_WORD:
						*data = (ra [ reg ] & 0xFFFF);
						break;
					case S_LONG:
						*data = ra [ reg ];
						break;
				}
				break;
			case EA_AI:
				*data = mem_get( ra [ reg ], (char)size );
				break;
			case EA_AIPI:
				*data = mem_get( ra [ reg ], (char)size );
				if ( reg == 7 && size == S_BYTE ) {
					/* �V�X�e���X�^�b�N�̃|�C���^�͏�ɋ��� */
					inc_ra( (char)reg, (char)S_WORD );
				} else {
					inc_ra( (char)reg, (char)size );
				}
				break;
			case EA_AIPD:
				if ( reg == 7 && size == S_BYTE ) {
					/* �V�X�e���X�^�b�N�̃|�C���^�͏�ɋ��� */
					dec_ra( (char)reg, (char)S_WORD );
				} else {
					dec_ra( (char)reg, (char)size );
				}
				*data = mem_get( ra [ reg ], (char)size );
				break;
			case EA_AID:
				disp = (short)imi_get( S_WORD );
				*data = mem_get( ra [ reg ] + disp, (char)size );
				break;
			case EA_AIX:
				idx = idx_get();
				*data = mem_get( ra [ reg ] + (int)idx, (char)size );
				break;
			case EA_SRT:
				idx = imi_get( S_WORD );
				if ( (idx & 0x8000) != 0 )
					idx |= 0xFFFF0000;
				*data = mem_get( idx, (char)size );
				break;
			case EA_LNG:
				idx = imi_get( S_LONG );
				*data = mem_get( idx, (char)size );
				break;
			case EA_PC:
				disp = (short)imi_get( S_WORD );
				*data = mem_get( save_pc + disp, (char)size );
				break;
			case EA_PCX:
				idx = idx_get();
				*data = mem_get( save_pc + idx, (char)size );
				break;
			case EA_IM:
				*data = imi_get( (char)size );
				break;
			default:
				err68a( "�A�h���b�V���O���[�h���ُ�ł��B", __FILE__, __LINE__ );
				retcode = TRUE;
		}
	}
	return( retcode );
}

/*
 * �y�����z
 *   �^����ꂽ�f�[�^�������A�h���X�Ŏ����ꂽ�ꏊ�ɐݒ肷��B
 *
 * �y�֐������z
 *   retcode = set_data_at_ea(AceptAdrMode, mode, reg, data);
 *
 * �y�����z
 *   int AceptAdrMode; <in>  �����\�ȃA�h���b�V���O���[�h�Q EA_????*
 *   int mode;         <in>  �A�h���b�V���O���[�h MD_??
 *   int reg;          <in>  ���W�X�^�ԍ��܂��̓A�h���b�V���O���[�h�@MR_??
 *   long data;        <in>  �ݒ肷��f�[�^
 *
 * �y�Ԓl�z
 *   TURE:  �G���[
 *   FALSE: ����
 *
 */

BOOL set_data_at_ea(int AceptAdrMode, int mode, int reg, int size, long data)
{
	short	disp;
	long	idx;
	BOOL	retcode;
	int	gmode;
	long	save_pc;

	save_pc = pc;
	retcode = FALSE;

	/* ���삵�₷���悤�Ƀ��[�h�𓝍� */
	gmode = mode < 7 ? mode : 7 + reg;	/* gmode = 0-11 */

	/* AceptAdrMode �ŋ����ꂽ�A�h���b�V���O���[�h�łȂ���΃G���[ */

	if ((AceptAdrMode & (1 << gmode)) == 0) {

		err68a( "�A�h���b�V���O���[�h���ُ�ł��B", __FILE__, __LINE__ );
		retcode = TRUE;

	} else {

		/* �f�B�X�e�B�l�[�V�����̃A�h���b�V���O���[�h�ɉ��������� */
		switch( gmode ) {
			case EA_DD:
				switch( size ) {
					case S_BYTE:
						rd [ reg ] = ( rd [ reg ] & 0xFFFFFF00 ) |
								( data & 0xFF);
						break;
					case S_WORD:
						rd [ reg ] = ( rd [ reg ] & 0xFFFF0000 ) |
								( data & 0xFFFF);
						break;
					case S_LONG:
						rd [ reg ] = data;
						break;
				}
				break;
			case EA_AD:
				switch( size ) {
					case S_BYTE:
						ra [ reg ] = ( ra [ reg ] & 0xFFFFFF00 ) |
								( data & 0xFF );
						break;
					case S_WORD:
						ra [ reg ] = ( ra [ reg ] & 0xFFFF0000 ) |
								( data & 0xFFFF );
						break;
					case S_LONG:
						ra [ reg ] = data;
						break;
				}
				break;
			case EA_AI:
				mem_set( ra [ reg ], data, (char)size );
				break;
			case EA_AIPI:
				mem_set( ra [ reg ], data, (char)size );
				if ( reg == 7 && size == S_BYTE ) {
					/* �V�X�e���X�^�b�N�̃|�C���^�͏�ɋ��� */
					inc_ra( (char)reg, (char)S_WORD );
				} else {
					inc_ra ( (char)reg , (char)size );
				}
				break;
			case EA_AIPD:
				if ( reg == 7 && size == S_BYTE ) {
					/* �V�X�e���X�^�b�N�̃|�C���^�͏�ɋ��� */
					dec_ra( (char)reg, (char)S_WORD );
				} else {
					dec_ra ( (char)reg , (char)size );
				}

				mem_set( ra [ reg ], data, (char)size );
				break;
			case EA_AID:
				disp = (short)imi_get( S_WORD );
				mem_set( ra [ reg ] + (int)disp, data, (char)size );
				break;
			case EA_AIX:
				idx = idx_get();
				mem_set( ra [ reg ] + idx, data, (char)size );
				break;
			case EA_SRT:
				idx = imi_get( S_WORD );
				if ( (idx & 0x8000) != 0 )
					idx |= 0xFFFF0000;
				mem_set( idx, data, (char)size );
				break;
			case EA_LNG:
				idx = imi_get( S_LONG );
				mem_set( idx, data, (char)size );
				break;
			case EA_PC:
				disp = (short)imi_get( S_WORD );
				mem_set( save_pc + (int)disp, data, (char)size );
				break;
			case EA_PCX:
				idx = idx_get();
				mem_set( save_pc + idx, data, (char)size );
				break;
			default:
				err68a( "�A�h���b�V���O���[�h���ُ�ł��B", __FILE__, __LINE__ );
				retcode = TRUE;
		}
	}

	return( retcode );
}

/*
 * �y�����z
 *   �����A�h���X�Ŏ����ꂽ�l���擾����B
 *   ���̎��APC���ړ������Ȃ��B
 *
 * �y�֐������z
 *   retcode = get_data_at_ea_noinc(AceptAdrMode, mode, reg, &data);
 *
 * �y�����z
 *   int AceptAdrMode; <in>  �����\�ȃA�h���b�V���O���[�h�Q EA_????*
 *   int mode;         <in>  �A�h���b�V���O���[�h MD_??
 *   int reg;          <in>  ���W�X�^�ԍ��܂��̓A�h���b�V���O���[�h�@MR_??
 *   long *data;       <out> �擾����f�[�^���i�[����ꏊ�ւ̃|�C���^
 *
 * �y�Ԓl�z
 *   TURE:  �G���[
 *   FALSE: ����
 *
 */

BOOL get_data_at_ea_noinc(int AceptAdrMode, int mode, int reg, int size, long *data)
{
	long save_pc;
	BOOL retcode;

	save_pc = pc;
	retcode = get_data_at_ea(AceptAdrMode, mode, reg, size, data);
	pc = save_pc;

	return(retcode);
}
