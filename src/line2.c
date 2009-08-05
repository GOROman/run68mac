/* $Id: line2.c,v 1.2 2009-08-05 14:44:33 masamic Exp $ */

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2001/05/23 11:22:07  masamic
 * First imported source code and docs
 *
 * Revision 1.8  1999/12/07  12:43:51  yfujii
 * *** empty log message ***
 *
 * Revision 1.8  1999/11/22  03:57:08  yfujii
 * Condition code calculations are rewriten.
 *
 * Revision 1.6  1999/11/01  10:36:33  masamichi
 * Reduced move[a].l routine. and Create functions about accessing effective address.
 *
 * Revision 1.4  1999/10/26  06:08:46  masamichi
 * precision implementation for move operation.
 *
 * Revision 1.3  1999/10/20  03:43:08  masamichi
 * Added showing more information about errors.
 *
 * Revision 1.2  1999/10/18  03:24:40  yfujii
 * Added RCS keywords and modified for WIN32 a little.
 *
 */

#undef	MAIN

#include <stdio.h>
#include "run68.h"

/*
 �@�@�\�F�P/�Q/�R���C������(move / movea)�����s����
 �߂�l�F TRUE = ���s�I��
         FALSE = ���s�p��
*/
int	line2( char *pc_ptr )
{
	long	save_pc ;
	char	src_mode ;
	char	dst_mode ;
	char	src_reg ;
	char	dst_reg ;
	char	code1, code2 ;
	long	src_data ;
	int	size;

	code1 = *(pc_ptr++) ;
	code2 = *pc_ptr ;
	pc += 2 ;
	save_pc = pc ;
	dst_reg  = ((code1 & 0x0E) >> 1) ;
	dst_mode = (((code1 & 0x01) << 2) | ((code2 >> 6) & 0x03)) ;
	src_mode = ((code2 & 0x38) >> 3) ;
	src_reg  = (code2 & 0x07) ;

	/* �A�N�Z�X�T�C�Y�̌��� */
	switch ((code1 >> 4) & 0x03) {
		case 1:
			size = S_BYTE;
                        break;
		case 3:
			size = S_WORD;
			break;
		case 2:
			size = S_LONG;
			break;
		default:
			err68a( "���݂��Ȃ��A�N�Z�X�T�C�Y�ł��B", __FILE__, __LINE__ ) ;
			return( TRUE ) ;
	}

	/* �\�[�X�̃A�h���b�V���O���[�h�ɉ��������� */
	if (src_mode == EA_AD && size == S_BYTE) {
		err68a( "�s���Ȗ���: move[a].b An, <ea> �����s���悤�Ƃ��܂����B", __FILE__, __LINE__ ) ;
		return(TRUE);
	} else if (get_data_at_ea(EA_All, src_mode, src_reg, size, &src_data)) {
		return(TRUE);
	}

	/* movea �������̏��� */
	if (dst_mode == EA_AD) {
		if (size == S_BYTE) {
			err68a( "�s���Ȗ���: movea.b <ea>, An �����s���悤�Ƃ��܂����B", __FILE__, __LINE__ ) ;
			return(TRUE);
		} else if (size == S_WORD) {
			if (src_data & 0x8000) {
				src_data |= 0xffff0000;
			} else {
				src_data &= 0x0000ffff;
			}
			size = S_LONG;
		}
	}

	/* �f�B�X�e�B�l�[�V�����̃A�h���b�V���O���[�h�ɉ��������� */
	if (set_data_at_ea(EA_VariableData | (1 << (EA_AI - 1)), dst_mode, dst_reg, size, src_data)) {
		return(TRUE);
	}

	/* movea �̂Ƃ��̓t���O�͕ω����Ȃ� */
	if ( dst_mode != MD_AD ) {

		/* �t���O�̕ω� */
		general_conditions(src_data, size);

	}

	return( FALSE ) ;
}
