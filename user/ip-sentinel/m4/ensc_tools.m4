dnl $Id: ensc_tools.m4,v 1.7 2004/12/23 00:44:18 ensc Exp $

dnl Copyright (C) 2002,2004 Enrico Scholz <enrico.scholz@informatik.tu-chemnitz.de>
dnl  
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; version 2 of the License.
dnl  
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl  
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


AC_DEFUN([ENSC_STANDARD_TOOLS],
[
	AC_PATH_PROGS(SED,   sed)
	AC_PATH_PROGS(CAT,   cat)
	AC_PATH_PROGS(CP,    cp)
	AC_PATH_PROGS(GREP,  grep)
	AC_PATH_PROGS(EGREP, egrep)
	AC_PATH_PROGS(CHMOD, chmod)
	AC_PATH_PROGS(CHOWN, chown)
	AC_PATH_PROGS(TOUCH, touch)
	AC_PATH_PROGS(SH,    sh)
	AC_PATH_PROGS(RM,    rm)
	AC_PATH_PROGS(RMDIR, rmdir)
	AC_PATH_PROGS(DIFF,  diff)
	AC_PATH_PROGS(MV,    mv)
	AC_PATH_PROGS(LN,    ln)
	AC_PATH_PROGS(DIRNAME, dirname)
	AC_PATH_PROGS(MKDIR, mkdir)

	RM_F='${RM} -f'
	AC_SUBST(RM_F)

	LN_S='${LN} -s'
	AC_SUBST(LN_S)

	CP_P='${CP} -p'
	AC_SUBST(CP_P)

	MKDIR_P='${MKDIR} -p'
	AC_SUBST(MKDIR_P)

	testfile='.testfile'
	CHOWN_REFERENCE=:

	AC_MSG_CHECKING([whether chown understands the --references option])
	${TOUCH} ${testfile} && ${CHOWN} --reference ${testfile} ${testfile} && CHOWN_REFERENCE='${CHOWN} --reference'
	if test x"${CHOWN_REFERENCE}" = x:; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
	fi
	AC_SUBST(CHOWN_REFERENCE)

	CHMOD_REFERENCE=:
	AC_MSG_CHECKING([whether chmod understands the --references option])
	${TOUCH} ${testfile} && ${CHMOD} --reference ${testfile} ${testfile} && CHMOD_REFERENCE='${CHMOD} --reference'
	if test x"${CHMOD_REFERENCE}" = x:; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
	fi
	AC_SUBST(CHMOD_REFERENCE)

	${RM} -f ${testfile}
])

AC_DEFUN([ENSC_TEX_TOOLS],
[
	AC_PATH_PROGS(LATEX,	latex)
	AC_PATH_PROGS(PDFLATEX,	pdflatex)
	AC_PATH_PROGS(EPSTOPDF,	epstopdf)
	AC_PATH_PROGS(BIBTEX,	bibtex)
	AC_PATH_PROGS(MAKEINDEX, makeindex)
	AC_PATH_PROGS(DVIPS,	dvips)
	AC_PATH_PROGS(A2PS,	a2ps)
	AC_PATH_PROGS(FIG2DEV,	fig2dev)
	AC_PATH_PROGS(XVCG,	xvcg)
	AC_PATH_PROGS(PSNUP,	psnup)
	AC_PATH_PROGS(DOT,	dot)
	AC_PATH_PROGS(DOXYGEN,  doxygen)
	AC_PATH_PROGS(TEXDEPCOMP, tex-deps.py, [], [${HOME}/lib/make])
	AC_PATH_PROGS(TEXBUILD,   tex-build.sh, [], [${HOME}/lib/make])
])

AC_DEFUN([ENSC_SGML_TOOLS],
[
	AC_PATH_PROGS(JADE,     	jade)
	AC_PATH_PROGS(JADETEX,		jadetex)
	AC_PATH_PROGS(PDFJADETEX,	pdfjadetex)
	AC_PATH_PROGS(NSGMLS,		nsgmls)
])

AC_DEFUN([ENSC_GRAPH_TOOLS],
[
	AC_PATH_PROGS(TGD, tgd)
])
