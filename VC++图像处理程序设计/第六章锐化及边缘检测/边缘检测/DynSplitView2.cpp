// DynSplitView2.cpp : implementation file
//

#include "stdafx.h"
#include "DSplit.h"
#include "DynSplitView2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDynSplitView2

IMPLEMENT_DYNCREATE(CDynSplitView2, CView)

CDynSplitView2::CDynSplitView2()
{
	state2=0;
}

void CDynSplitView2::clearmem()
{
	CDSplitDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	pDoc->statedoc=0;
    state2=1;
	CDibNew1=&pDoc->CDibNew;
    CDib1=&pDoc->CDib;
	long int  size=CDib1->GetHeight()*CDib1->GetDibWidthBytes();
	memcpy(CDibNew1->m_pData,CDib1->m_pData,size);
}

CPalette * CDynSplitView2::CreateBitmapPalette(BianYuanJianCeDib * pBitmap)
{
	struct
	{
		WORD Version;
		WORD NumberOfEntries;
		PALETTEENTRY aEntries[256];
	} palette = { 0x300, 256 };
	
	LPRGBQUAD pRGBTable = pBitmap->GetRGB();
	UINT numberOfColors = pBitmap->GetNumberOfColors();
	
	for(UINT x=0; x<numberOfColors; ++x)
	{
		palette.aEntries[x].peRed =
			pRGBTable[x].rgbRed;
		palette.aEntries[x].peGreen =
			pRGBTable[x].rgbGreen;
		palette.aEntries[x].peBlue =
			pRGBTable[x].rgbBlue;
		palette.aEntries[x].peFlags = 0;
	}
	
	hPalette.CreatePalette((LPLOGPALETTE)&palette);
	
	return &hPalette;
}

CDynSplitView2::~CDynSplitView2()
{
}

CDSplitDoc* CDynSplitView2::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDSplitDoc)));
	return (CDSplitDoc*)m_pDocument;
}

BEGIN_MESSAGE_MAP(CDynSplitView2, CView)
	//{{AFX_MSG_MAP(CDynSplitView2)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_FILESAVE, OnFilesave)
	ON_COMMAND(ID_Robert, OnRobert)
	ON_COMMAND(ID_Guasslaplacian, OnGuasslaplacian)
	ON_COMMAND(ID_Krisch, OnKrisch)
	ON_COMMAND(ID_Laplacian, OnLaplacian)
	ON_COMMAND(ID_Prewitt, OnPrewitt)
	ON_COMMAND(ID_Sobel, OnSobel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDynSplitView2 drawing

void CDynSplitView2::OnDraw(CDC* pDC)
{	
	CDSplitDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if(!pDoc ->statedoc&&state2==1)
	{
        BYTE* pBitmapData = CDibNew1->GetData();
        LPBITMAPINFO pBitmapInfo = CDibNew1->GetInfo();
        int bitmapHeight = CDibNew1->GetHeight();
        int bitmapWidth = CDibNew1->GetWidth();
		if (CDibNew1->GetRGB()) // Has a color table
		{
			CPalette * hPalette=CreateBitmapPalette(CDibNew1);
            CPalette * hOldPalette =
                pDC->SelectPalette(hPalette, true);
            pDC->RealizePalette();
			::StretchDIBits(pDC->GetSafeHdc(),0, 0, bitmapWidth, bitmapHeight,
				0, 0, bitmapWidth, bitmapHeight,
				pBitmapData, pBitmapInfo,
				DIB_RGB_COLORS, SRCCOPY);
            pDC->SelectPalette(hOldPalette, true);
            ::DeleteObject(hPalette);
		}
		else
		{
            StretchDIBits(pDC->GetSafeHdc(),
                0, 0, bitmapWidth, bitmapHeight,
                0, 0, bitmapWidth, bitmapHeight,
				pBitmapData, pBitmapInfo,
				DIB_RGB_COLORS, SRCCOPY);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDynSplitView2 diagnostics

#ifdef _DEBUG
void CDynSplitView2::AssertValid() const
{
	CView::AssertValid();
}

void CDynSplitView2::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDynSplitView2 message handlers

BOOL CDynSplitView2::OnEraseBkgnd(CDC* pDC) 
{
	CRect rect;
	GetClientRect(&rect);
	pDC->FillSolidRect(&rect,::GetSysColor(COLOR_WINDOW));
	return TRUE;
}

void CDynSplitView2::OnFilesave() 
{
	CFileDialog dlg(FALSE,_T("BMP"),_T("*.BMP"),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,_T("????????(*.BMP)|*.BMP|"));	
    if(IDOK==dlg.DoModal())
	CString  filename;
    filename.Format ("%s",dlg.GetPathName() );    
    CDibNew1->SaveFile(filename);
	state2=1;
	Invalidate();
}

/*??????????????????*/
void CDynSplitView2::OnRobert() 
{
	clearmem();
	CDibNew1->Robert();             //????Robert????????????????
 	Invalidate();
}



/*????????????????????????*/
void CDynSplitView2::OnGuasslaplacian() 
{
	clearmem();
	CDibNew1->Guasslaplacian();             //????Guasslaplacian????????????????
 	Invalidate(); 
}

/*??????????????????*/
void CDynSplitView2::OnKrisch() 
{
	clearmem();
	CDibNew1->Krisch();             //????Krisch????????????????
 	Invalidate();
}

/*????????????????????*/
void CDynSplitView2::OnLaplacian() 
{
	clearmem();
	CDibNew1->Laplacian();             //????Laplacian????????????????
 	Invalidate();
}

/*????????????????????*/
void CDynSplitView2::OnPrewitt() 
{
	clearmem();
	CDibNew1->PreWitt();             //????Prewitt????????????????
 	Invalidate();
}

/*??????????????????*/
void CDynSplitView2::OnSobel() 
{
	clearmem();
	CDibNew1->Sobel();             //????Sobel????????????????
 	Invalidate();
}
