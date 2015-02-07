#include "stdafx.h"
#include "GlFont.h"
/*
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glaux.h>
*/
#include "GL\glut.h"

CGlFont::CGlFont()
{
	m_hFont=NULL;	
	InitFont("Arial");
	//InitFont("����");
	m_fExtrusion = 10.0;
}

CGlFont::~CGlFont()
{
}

LOGFONT CGlFont::GetLogFont()
{
	LOGFONT lf;
	GetObject(m_hFont,sizeof(LOGFONT),&lf);
	return lf;
}

void CGlFont::DeleteFont()
{
	if(m_hFont)
	{
		DeleteObject(m_hFont);
		m_hFont=NULL;
	}
}

bool CGlFont::InitFont(LOGFONT &lf)
{
	DeleteFont();
	m_hFont=CreateFontIndirect(&lf);
	if( m_hFont )return true;
	else return false;;
}

bool CGlFont::InitFont(char *fontName)
{
	DeleteFont();
	m_hFont = CreateFont(
		100,									//< lfHeight ����߶�
		100,										//< lfWidth ������ 
		0,										//< lfEscapement �������ת�Ƕ� Angle Of Escapement 
		0,										//< lfOrientation ������ߵ���ת�Ƕ�Orientation Angle 
		FW_BOLD,								//< lfWeight ��������� 
		FALSE,									//< lfItalic �Ƿ�ʹ��б�� 
		FALSE,									//< lfUnderline �Ƿ�ʹ���»��� 
		FALSE,									//< lfStrikeOut �Ƿ�ʹ��ɾ���� 
		GB2312_CHARSET,							//< lfCharSet �����ַ��� 
		OUT_TT_PRECIS,							//< lfOutPrecision ������� 
		CLIP_DEFAULT_PRECIS,					//< lfClipPrecision �ü����� 
		ANTIALIASED_QUALITY,					//< lfQuality ������� 
		FF_DONTCARE|DEFAULT_PITCH,				//< lfPitchAndFamily Family And Pitch 
		fontName);								//< lfFaceName �������� 
	if(m_hFont)return true;
	else return false;
}

void CGlFont::Draw2DText(char *string, float x, float y)
{
	if(strlen(string)<=0)return;

	//HDC hDC=wglGetCurrentDC();
	HDC hDC = ::CreateCompatibleDC(NULL);

	HFONT hOldFont=NULL;
	hOldFont=(HFONT)SelectObject(hDC,m_hFont);
	if(!hOldFont)
	{
		return;
	}

	glRasterPos2f(x, y);

	DWORD dwChar;
	int ListNum;
	for(size_t i=0;i<strlen((char *)string);i++)
	{
		if(IsDBCSLeadByte(string[i]))
		{
			dwChar=((unsigned char)string[i])*0x100+(unsigned char)string[i+1];
			i++;
		}
		else
		{
			dwChar=string[i];
		}
		ListNum=glGenLists(1);
		wglUseFontBitmaps(hDC,dwChar,1,ListNum);
		glCallList(ListNum);
		glDeleteLists(ListNum,1);
	}
	
	SelectObject(hDC,hOldFont);
	::DeleteDC(hDC);
}

float CGlFont::Draw3DText(char *string)
{
	if(strlen(string)<=0)return 0.0;

	GLYPHMETRICSFLOAT pgmf[1];
	//HDC hDC=wglGetCurrentDC();
	HDC hDC = ::CreateCompatibleDC(NULL);

	//���õ�ǰ����
	HFONT hOldFont=NULL;
	hOldFont=(HFONT)SelectObject(hDC,m_hFont);
	if(!hOldFont)
	{
		return 0.0;
	}
	DWORD dwChar;
	int ListNum;

	float length = 0.0;
	for(size_t i=0;i<strlen((char *)string);i++)
	{
		//if(IsDBCSLeadByte(string[i]))
		//{
		//	dwChar=((unsigned char)string[i])*0x100+(unsigned char)string[i+1];
		//	i++;
		//}
		//else
		{
			dwChar=string[i];
		}
		ListNum=glGenLists(1);
		wglUseFontOutlines(hDC,dwChar,1,ListNum,0.1,m_fExtrusion,WGL_FONT_POLYGONS,pgmf);
		glCallList(ListNum);
		//Get the length of this character
		length += pgmf[0].gmfCellIncX;
		glDeleteLists(ListNum,1);
	}
	
	SelectObject(hDC,hOldFont);
	::DeleteDC(hDC);

	return length;
}
